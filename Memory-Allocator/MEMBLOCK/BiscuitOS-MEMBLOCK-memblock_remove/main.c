/*
 * MEMBLOCK Memory Allocator: memblock_remove
 *
 * (C) 2022.10.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>

#define MEMBLOCK_FAKE_BASE	0x800000000
#define MEMBLOCK_FAKE_SIZE	0x100000

int __init BiscuitOS_Running(void)
{
	phys_addr_t start, end;
	u64 idx;

	/* Add new range */
	memblock_add(MEMBLOCK_FAKE_BASE, MEMBLOCK_FAKE_SIZE);

	printk("==== Before Remove ==============\n");
	for_each_mem_range(idx, &start, &end)
		printk("Range %lld: %#llx - %#llx\n", idx, start, end);

	/* Only Test: Remove range */
	memblock_remove(MEMBLOCK_FAKE_BASE, MEMBLOCK_FAKE_SIZE);

	printk("==== After Remove ==============\n");
	for_each_mem_range(idx, &start, &end)
		printk("Range %lld: %#llx - %#llx\n", idx, start, end);

	return 0;
}
