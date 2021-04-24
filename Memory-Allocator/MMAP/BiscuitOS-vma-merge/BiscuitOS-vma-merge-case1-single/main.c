/*
 * VMA Merge (Merge Case1): Triple to single
 *
 * (C) 2021.04.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

/*
 * +------------------------+                    +----------------------+
 * |          prev          |                    |         NEXT         |
 * +------------------------+                    +----------------------+
 *                          |                    |
 *                          +--------------------+
 *                          |      new  vma      |
 *                          +--------------------+
 *
 * +--------------------------------------------------------------------+
 * |                                 prev                               |
 * +--------------------------------------------------------------------+
 */

#ifdef __i386__ /* Intel i386 */
#define AREA_PREV_BASE		0x60000000
#define AREA_NEXT_BASE		0x60001000
#define AREA_BASE		0x60002000
#else           /* AMD x86_64 */
#define AREA_PREV_BASE		0x600000000000
#define AREA_NEXT_BASE		0x600000002000
#define AREA_BASE		0x600000001000
#endif
#define PAGE_SIZE		4096

static void *BiscuitOS_anonymous_mmap(unsigned long addr)
{
	return mmap((void *)addr, PAGE_SIZE, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static void BiscuitOS_anonymous_munmap(unsigned long addr)
{
	munmap((void *)addr, PAGE_SIZE);
}

int main()
{
	char *base[3];

	/* mmap default region */
	base[0] = BiscuitOS_anonymous_mmap(AREA_PREV_BASE);
	base[1] = BiscuitOS_anonymous_mmap(AREA_NEXT_BASE);

	/* Trigger page fault and bind physical page */
	base[0][0] = 'B';
	base[1][0] = 'i';

	/* mmap target region */
	base[2] = BiscuitOS_anonymous_mmap(AREA_BASE);

	/* Trigger page fault */
	base[2][0] = 's';

	/* Unmapp */
	BiscuitOS_anonymous_munmap(AREA_BASE);
	BiscuitOS_anonymous_munmap(AREA_NEXT_BASE);
	BiscuitOS_anonymous_munmap(AREA_PREV_BASE);

	return 0;
}
