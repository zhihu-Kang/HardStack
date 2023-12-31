/*
 * Memory private mmap on Userspace
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

#define BISCUITOS_MEMDEV	"/dev/mem"
#define BISCUITOS_MAP_SIZE	4096

int main()
{
	unsigned long *val;
	char *base;
	int fd;

	/* Open */
	fd = open(BISCUITOS_MEMDEV, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Open %s failed.\n", BISCUITOS_MEMDEV);
		return -EBUSY;
	}

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_PRIVATE,
			    fd,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	val = (unsigned long *)base;
	/* Trigger page fault */
	*val = 88520;
	printf("%#lx => %ld\n", (unsigned long)val, *val);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);
	close(fd);

	return 0;
}
