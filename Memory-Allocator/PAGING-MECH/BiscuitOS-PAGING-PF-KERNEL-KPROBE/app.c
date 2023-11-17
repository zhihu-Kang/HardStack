// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault on Kernel: KPROBE-PF
 *
 * (C) 2023.11.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base;

	/* ALLOC ANONYMOUS MEMORY */
	base = mmap((void *)MAP_VADDR, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF and SET _PAGE_PRESENT */
	*base = 'B'; // do_anonymous_page do_user_addr_fault do_mmap
	/* Read Ops, Don't Trigger #PF */
	printf("KPROBE-PF: %#lx => %c\n", (unsigned long)base, *base);

	munmap(base, PAGE_SIZE);

	return 0;
}
