// SPDX-License-Identifier: GPL-2.0
/*
 * BiscuitOS MISC DD on Userspace
 *
 * (C) 2020.10.06 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

/* PATH for device */
#define DEV_PATH		"/dev/BiscuitOS"
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_SET		_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_GET		_IO(BISCUITOS_IO, 0x01)

/* Exchange data between kernel and userland */
struct BiscuitOS_pdata {
	int num;
};

int main()
{
	struct BiscuitOS_pdata pdata;
	int rvl = 0;
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* Read information from Kernel */
	rvl = read(fd, &pdata, sizeof(pdata));
	if (rvl != sizeof(pdata)) {
		printf("ERROR: BAD read.\n");
		rvl = -1;
		goto out;
	}
	printf("Kernel information: %d\n", pdata.num);

	/* Write informtion to kernel */
	pdata.num = 0x28;
	rvl = write(fd, &pdata, sizeof(pdata));
	if (rvl != sizeof(pdata)) {
		printf("ERROR: BAD write!\n");
		rvl = -1;
		goto out;
	}
	
	/* ioctl */
	rvl = ioctl(fd, BISCUITOS_SET, (unsigned long)0);
	if (fd < 0) {
		printf("ERROR: IOCTL BISCUITOS_SET.\n");
		goto out;
	}

	/* Normal ending */
	rvl = 0;
out:
	close(fd);
	return rvl;
}
