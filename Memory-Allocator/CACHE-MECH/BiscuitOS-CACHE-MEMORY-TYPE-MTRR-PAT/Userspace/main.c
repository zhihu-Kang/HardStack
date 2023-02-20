/*
 * CostTime for different memory type
 *
 * (C) 2023.02.19 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#define DEV_PATH		"/dev/BiscuitOS-CACHE"
#define PAGE_SIZE		(4 * 1024)

int main()
{
	unsigned long count = 100000; 
	struct timeval tv, ntv;
	void *base;
	char *val;
	int fd, i;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* mmap */
	base = mmap(NULL, PAGE_SIZE,
			  PROT_READ | PROT_WRITE,
			  MAP_PRIVATE,
			  fd,
			  0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmaping failed.\n");
		close(fd);
		return -1;
	}
	/* CACHE Line Fill or Prefetch */
	memset(base, 0x00, PAGE_SIZE);
	val = (char *)base;
	
	/* Cost Time for Special Memory Type */
	gettimeofday(&tv, NULL);
	while (count--) {
		for (i = 0; i < PAGE_SIZE; i++)
			/* WRITE */
			val[i] = i;
	}
	gettimeofday(&ntv, NULL);
	printf("Cost Time: %ld nsec\n", ntv.tv_sec * 1000000 +
			ntv.tv_usec - tv.tv_sec * 1000000 - tv.tv_usec);

	munmap(base, PAGE_SIZE);
	close(fd);

	return 0;
}
