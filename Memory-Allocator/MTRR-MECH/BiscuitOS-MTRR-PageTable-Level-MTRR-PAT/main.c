/*
 * Page-Table level: MTRR and PAT
 *
 * (C) 2020.10.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
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
#include <linux/mm.h>
#include <uapi/asm/mtrr.h>
#include <linux/mm_types.h>
#include <asm/msr.h>
#include <asm/mtrr.h>

#define DEV_NAME		"BiscuitOS"
/* Memory Type and Mnemonic */
#define MTRR_MEMTYPE_UC		0x00
#define MTRR_MEMTYPE_WC		0x01
#define MTRR_MEMTYPE_WT		0x04
#define MTRR_MEMTYPE_WP		0x05
#define MTRR_MEMTYPE_WB		0x06

static void show_PAT_MSR(void)
{
	static int run_once = 0;
	u64 tmp_pat;
	int i;

	if (run_once)
		return;

	rdmsrl(MSR_IA32_CR_PAT, tmp_pat);

	printk("PAT Table:\n");
	for (i = 0; i < 8; i++) {
		int pat = (tmp_pat >> (i * 8)) & 0x7;
		/*
		 * +-----+-----+-----+-----------+
                 * | PAT | PCD | PWT | PAT Entry |
		 * +-----+-----+-----+-----------+
                 * | 0   | 0   | 0   | PAT0      |
		 * +-----+-----+-----+-----------+
                 * | 0   | 0   | 1   | PAT1      |
		 * +-----+-----+-----+-----------+
                 * | 0   | 1   | 0   | PAT2      |
		 * +-----+-----+-----+-----------+
                 * | 0   | 1   | 1   | PAT3      |
		 * +-----+-----+-----+-----------+
                 * | 1   | 0   | 0   | PAT4      |
		 * +-----+-----+-----+-----------+
                 * | 1   | 0   | 1   | PAT5      |
		 * +-----+-----+-----+-----------+
                 * | 1   | 1   | 0   | PAT6      |
		 * +-----+-----+-----+-----------+
                 * | 1   | 1   | 1   | PAT7      |
		 * +-----+-----+-----+-----------+
		 */

		switch (pat) {
		case 0:
			printk("  PA%d: Uncachable(UC)\n", i);
			break;
		case 1:
			printk("  PA%d: Write Combining(WC)\n", i);
			break;
		case 4:
			printk("  PA%d: Write Through(WT)\n", i);
			break;
		case 5:
			printk("  PA%d: Write Protected(WP)\n", i);
			break;
		case 6:
			printk("  PA%d: Write Back(WB)\n", i);
			break;
		case 7:
			printk("  PA%d: Uncached(UC-)\n", i);
			break;
		default:
			printk("  PA%d: Reserved.\n", i);
			break;
		}
	}
	run_once = 1;
}

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;
	struct page *fault_page;
	unsigned long pfn;
	int memory_type;

	/* Clear PCD/PAT/PWT */
	pgprot_val(vma->vm_page_prot) &= ~(_PAGE_PCD | _PAGE_PWT | _PAGE_PAT);
	show_PAT_MSR();

	/* 
	 * Setup Effective Page-Level Memory Types
	 * +------------------+-----------------+-----------------------+
         * | MTRR Memory Type | PAT Entry Value | Effective Memory Type |
	 * +------------------+-----------------+-----------------------+
	 * | UC               | UC              | UC(1)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | UC-             | UC(1)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | WC              | WC                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WT              | UC                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WB              | UC(1)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | WP              | UC(1)                 |
	 * +------------------+-----------------+-----------------------+
	 * | WC               | UC              | UC(1)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | UC-             | WC                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WC              | WC                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WT              | UC(2,3)               |
	 * |                  +-----------------+-----------------------+
	 * |                  | WB              | WC                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WP              | UC(2,3)               |
	 * +------------------+-----------------+-----------------------+
	 * | WT               | UC              | UC(2)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | UC-             | UC(2)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | WC              | WC                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WT              | WT                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WB              | WT                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WP              | WP(3)                 |
	 * +------------------+-----------------+-----------------------+
	 * | WB               | UC              | UC(2)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | UC-             | UC(2)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | WC              | WC                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WT              | WT                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WB              | WB                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WP              | WP                    |
	 * +------------------+-----------------+-----------------------+
	 * | WP               | UC              | UC(2)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | UC-             | WC(3)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | WC              | WC                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WT              | WT(3)                 |
	 * |                  +-----------------+-----------------------+
	 * |                  | WB              | WP                    |
	 * |                  +-----------------+-----------------------+
	 * |                  | WP              | WP                    |
	 * +------------------+-----------------+-----------------------+
	 */
	/* _PAGE_PAT: _PAGE_PCD: _PAGE_PWT */
	pgprot_val(vma->vm_page_prot) |= _PAGE_PAT ;
	memory_type = MTRR_MEMTYPE_WB;

	/* Allocate Physical Memory */
	fault_page = alloc_page(GFP_KERNEL);
	if (!fault_page)
		return -ENOMEM;

	/* Physical Address */
	pfn = page_to_pfn(fault_page);
	mtrr_add(pfn << PAGE_SHIFT, PAGE_SIZE, memory_type, true);

	/* Fill PTE and INC _mapcount for page */
	vma->vm_flags |= VM_MIXEDMAP;
	if (vm_insert_page(vma, address, fault_page)) {
		__free_page(fault_page);
		return -EAGAIN;
	}

	/* Add refcount for page */
	atomic_inc(&fault_page->_refcount);
	/* Bind fault page */
	vmf->page = fault_page;

	return 0;
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

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int __init BiscuitOS_init(void)
{
	misc_register(&BiscuitOS_drv);
	return 0;
}
device_initcall(BiscuitOS_init);
