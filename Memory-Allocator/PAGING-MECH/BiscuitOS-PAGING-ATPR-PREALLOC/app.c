// SPDX-License-Identifier: GPL-2.0
/*
 * ATPR(Apply To Page Range): PreAlloc
 *
 * (C) 2023.10.26 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAP_SIZE	(4096)
#define FILE_PATH	"/dev/BiscuitOS-ATPR"

int main()
{
	void *addr;
	int fd;

	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", FILE_PATH);
		return -1;
	}

	/* Prealloc Virtual and Physical Memory */
	addr = mmap(NULL, MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd, 
			0);
	if (!addr) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -1;
	}

	/* Write Ops, Don't trigger #PF */
	*(char *)addr = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("%#lx => %c\n", (unsigned long)addr, *(char *)addr);

	munmap(addr, MAP_SIZE);
	close(fd);

	return 0;
}
