/*
 * Hugetlb: Anonymous Shared-mapping for NO Reserved using Hugepage
 *
 * (C) 2021.11.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifdef __i386__
#error "Process doesn't support I386 Architecture"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman.h>

#define HPAGE_SIZE		(1 * 1024 * 1024 * 1024)
#define BISCUITOS_MAP_SIZE	(1 * HPAGE_SIZE)

#ifndef MAP_HUGE_1GB
#define HUGETLB_FLAG_ENCODE_SHIFT	26
#define MAP_HUGE_1GB			(30 << HUGETLB_FLAG_ENCODE_SHIFT)
#endif

int main()
{
	char *base;

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED | MAP_ANONYMOUS | 
			    MAP_HUGETLB | MAP_HUGE_1GB |
			    MAP_NORESERVE,
			    -1,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	/* Just for debug */
	sleep(10);

	/* Trigger page fault */
	base[0] = 'B';
	printf("%#lx => %c\n", (unsigned long)base, base[0]);

	sleep(-1);
	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
