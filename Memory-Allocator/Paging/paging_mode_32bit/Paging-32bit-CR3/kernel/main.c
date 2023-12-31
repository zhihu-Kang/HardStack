/*
 * Paging Mechanism: CR3 on 32-bit Paging
 *
 * CR3 With 32-Bit Paging
 *
 *  63         32 31                           12 11         5 4 3 2      0
 * +-------------+-------------------------------+------------+-+-+--------+
 * |             |                               |            |P|P|        |
 * |   Ignored   | Physical Address of 4KiB Page |   Ignore   |C|W| Ignore |
 * |             |                               |            |D|T|        |
 * +-------------+-------------------------------+------------+-+-+--------+
 *
 * (C) 2021.01.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#ifdef __i386__
#include <linux/highmem.h>
#endif

/* Paging/fault header*/
#include <linux/mm.h>
#include <linux/kallsyms.h>

/* DD Platform Name */
#define DEV_NAME			"BiscuitOS"

/* Timer */
#define BISCUITOS_SCANNER_PERIOD	1000 /* 1000ms -> 1s */
static struct timer_list BiscuitOS_scanner;

/* Speical */
static struct vm_area_struct *BiscuitOS_vma;
unsigned long BiscuitOS_address;
unsigned long BiscuitOS_cr3;
/* kallsyms unexport symbol */
typedef int (*__p4d_alloc_t)(struct mm_struct *, pgd_t *, unsigned long);
typedef int (*__pud_alloc_t)(struct mm_struct *, p4d_t *, unsigned long);
typedef int (*__pmd_alloc_t)(struct mm_struct *, pud_t *, unsigned long);
typedef int (*__pte_alloc_t)(struct mm_struct *, pmd_t *);
typedef void (*page_add_f_rmap_t)(struct page *, bool);

static __p4d_alloc_t __p4d_alloc_func;
static __pud_alloc_t __pud_alloc_func;
static __pmd_alloc_t __pmd_alloc_func;
static __pte_alloc_t __pte_alloc_func;
static page_add_f_rmap_t page_add_file_rmap_func;

/* Follow: CR3 */
static int BiscuitOS_follow_cr3(struct mm_struct *mm, 
			unsigned long address, unsigned long cr3)
{
	phys_addr_t pgd_addr;
	pgd_t *pgd;

	/* pgd */
	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		return -EINVAL;

	/* load physical address for pgd */
	pgd_addr = __pa(pgd) & PAGE_MASK;

	printk("The Physical Address of PGD: %#lx\n", (unsigned long)pgd_addr);
	printk("The content of CR3: %#lx\n", cr3);

	/* PWT */
	if (cr3 & X86_CR3_PWT)
		printk("  CR3 PWT (Page Write Through) Enable.\n");
	else
		printk("  CR3 PWT (Page Write Through) Disable.\n");

	/* PCD */
	if (cr3 & X86_CR3_PCD)
		printk("  CR3 PCD (Page Cache Disable) Enable.\n");
	else
		printk("  CR3 PCD (Page Cache Disable) Disable.\n");

	/* Physical Address */
	printk("  Physical Address from CR3: %#lx\n", cr3 & PAGE_MASK);

	return 0;
}

/* Build Page table */
static int BiscuitOS_build_page_table(struct vm_area_struct *vma, 
				unsigned long address, struct page *page)
{
	struct mm_struct *mm = vma->vm_mm;
	unsigned long pfn = page_to_pfn(page);
	spinlock_t *ptl;
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		goto out;

	/* alloc p4d */
#ifdef __PAGETABLE_P4D_FOLDED
	p4d = (p4d_t *)pgd;
#else
	p4d = (__p4d_alloc_func(mm, pgd, address)) ? 
			NULL : p4d_offset(pgd, address);
#endif
	if (!p4d || p4d_none(*p4d) || unlikely(p4d_bad(*p4d)))
		goto out;

	/* alloc pud */
#ifdef __PAGETABLE_PUD_FOLDED
	pud = (pud_t *)p4d;
#else
	pud = (__pud_alloc_func(mm, p4d, address)) ?
			NULL : pud_offset(p4d, address);
#endif
	if (!pud || pud_none(*pud) || unlikely(pud_bad(*pud)))
		goto out;

	/* alloc pmd */
#ifdef __PAGETABLE_PMD_FOLDED
	pmd = (pmd_t *)pud;
#else
	pmd = (__pmd_alloc_func(mm, pud, address)) ?
			NULL : pmd_offset(pud, address);
#endif
	if (!pmd || pmd_none(*pmd) || unlikely(pmd_bad(*pmd)))
		goto out;

	/* alloc pte */
	pte = __pte_alloc_func(mm, pmd) ? 
		NULL : pte_offset_map_lock(mm, pmd, address, &ptl);
	if (!pte)
		goto out;

	/* MMU Lazy mode */
	arch_enter_lazy_mmu_mode();

	get_page(page);
	inc_mm_counter(mm, mm_counter_file(page));
	page_add_file_rmap_func(page, false);

	set_pte_at(mm, address, pte, 
			pte_mkwrite(pfn_pte(pfn, vma->vm_page_prot)));
	pte_unmap_unlock(pte, ptl);
	
	arch_leave_lazy_mmu_mode();

	return 0;

out:
	return -EINVAL;
}

/* Scanner */
static void BiscuitOS_scanner_paging(struct timer_list *unused)
{
	if (BiscuitOS_address) {
		/* follow CR3 */
		BiscuitOS_follow_cr3(BiscuitOS_vma->vm_mm, 
					BiscuitOS_address, BiscuitOS_cr3);

		BiscuitOS_address = 0;
	}

	/* watchdog */
	mod_timer(&BiscuitOS_scanner, 
			jiffies + msecs_to_jiffies(BISCUITOS_SCANNER_PERIOD));
}

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;
	struct page *fault_page;
	int r;

	/* Allocate new page from buddy */
	fault_page = alloc_page(GFP_KERNEL);
	if (!fault_page) {
		printk("ERROR: System doesn't has enough physical memory.\n");
		r = -ENOMEM;
		goto err_alloc;
	}

	/* Build page table */
	BiscuitOS_build_page_table(vma, address, fault_page);

	/* bind fault page */
	vmf->page = fault_page;

	/* bind special data */
	BiscuitOS_vma = vma;
	BiscuitOS_address = address;
	BiscuitOS_cr3 = __read_cr3();

	return 0;

err_alloc:
	return r;
}

static inline void init_symbol(void)
{
	__p4d_alloc_func =
		(__p4d_alloc_t)kallsyms_lookup_name("__p4d_alloc");
	__pud_alloc_func =
		(__pud_alloc_t)kallsyms_lookup_name("__pud_alloc");
	__pmd_alloc_func =
		(__pmd_alloc_t)kallsyms_lookup_name("__pmd_alloc");
	__pte_alloc_func = 
		(__pte_alloc_t)kallsyms_lookup_name("__pte_alloc");
	page_add_file_rmap_func = 
		(page_add_f_rmap_t)kallsyms_lookup_name("page_add_file_rmap");
}

static const struct vm_operations_struct BiscuitOS_vm_ops = {
	.fault	= vm_fault,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* setup vm_ops */
	vma->vm_ops = &BiscuitOS_vm_ops;

	return 0;
}

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
};

/* Misc device driver */
static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	/* Register Misc device */
	misc_register(&BiscuitOS_drv);
	init_symbol();

	/* Timer for PTE Scanner */
	timer_setup(&BiscuitOS_scanner, BiscuitOS_scanner_paging, 0);
	mod_timer(&BiscuitOS_scanner, 
			jiffies + msecs_to_jiffies(BISCUITOS_SCANNER_PERIOD));

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	del_timer(&BiscuitOS_scanner);
	/* Un-Register Misc device */
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Paging/Page-fault Mechanism");
