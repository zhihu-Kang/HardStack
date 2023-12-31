// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Anonymous on WP(Write-Protection)
 *
 * (C) 2023.09.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base, ch;

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Read Ops, Trigger #PF, and ZERO Page */
	ch = *base;
	/* Write Ops, Trigger #PF, WriteProtect */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("ANON-WP %#lx => %c\n", (unsigned long)base, *base);

	munmap(base, MAP_SIZE);

	return 0;
}
