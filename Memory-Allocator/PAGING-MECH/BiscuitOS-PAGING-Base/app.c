/*
 * BiscuitOS Paing Mechanism Base Application
 *
 * (C) 2020.10.06 <buddy.zhang@aliyun.com>
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

#define DEV_PATH		"/dev/BiscuitOS"
#define MMAP_SIZE		4096

int main()
{
	void *base;
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* mmap */
	base = mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE,
			  MAP_PRIVATE, fd, 0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmaping failed.\n");
		close(fd);
		return -1;
	}
	
	/* usage */
	sprintf((char *)base, "Hello %s", "BiscuitOS");
	printf("== %s ==\n", (char *)base);

	munmap(base, MMAP_SIZE);

	close(fd);
	return 0;
}
