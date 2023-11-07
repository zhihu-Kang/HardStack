// SPDX-License-Identifier: GPL-2.0
/*
 * MMIO Mapping
 *
 * (C) 2023.11.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/io.h>

#define BROILER_MMIO_BASE	0xF0000000UL
#define BROILER_MMIO_LEN	0x1000UL

static struct resource Broiler_mmio_res = {
	.name	= "Broiler MMIO",
	.start	= BROILER_MMIO_BASE,
	.end	= BROILER_MMIO_BASE + BROILER_MMIO_LEN,
	.flags	= IORESOURCE_MEM,
};
static void __iomem *mmio;

static int __init BiscuitOS_init(void)
{
	unsigned long *val;
	int r;

	r = request_resource(&iomem_resource, &Broiler_mmio_res);
	if (r < 0)
		return r;

	mmio = ioremap(BROILER_MMIO_BASE, BROILER_MMIO_LEN);
	if (!mmio) {
		printk("IOREMAP MMIO failed.\n");
		remove_resource(&Broiler_mmio_res);
	}
	
	/* MMIO Read and Write */
	val = (unsigned long *)mmio;
	*val = 0x88520;
	printk("MMIO: Phys %#lx - %#lx\n",
			BROILER_MMIO_BASE, BROILER_MMIO_BASE + BROILER_MMIO_LEN);
	printk("      Virt %#lx - %#lx\n", (unsigned long)mmio, 
				(unsigned long)mmio + BROILER_MMIO_LEN);
	printk("      Value: %#lx\n", *val);

	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	iounmap(mmio);
	remove_resource(&Broiler_mmio_res);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Paging Project");
