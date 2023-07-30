// SPDX-License-Identifier: GPL-2.0
/*
 * Page Table Attribute
 *
 * (C) 2023.07.25 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define DEV_PATH		"/dev/BiscuitOS-PageTable"
#define PAGE_SIZE		4096

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

	base = mmap(NULL, PAGE_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_SHARED,
		   fd, 0);
	if (base == MAP_FAILED) {
		printf("ERROR: No free Memory\n");
		close(fd);
		return -1;
	}

	/* Read Ops np Trigger #GP */
	printf("%#lx => %c\n", (unsigned long)base, *(char *)base);
	sleep(1);
	/* Write Ops Trigger #GP */
	*(char *)base = 'C';

	printf("Case Finish.\n");
	munmap(base, PAGE_SIZE);
	close(fd);

	return 0;
}