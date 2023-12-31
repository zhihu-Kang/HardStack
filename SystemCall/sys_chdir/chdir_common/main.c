/*
 * sys_chdir in C
 *
 * (C) 2020.03.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
/* __NR_chdir */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_chdir
#define __NR_chdir	12
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_chdir helper\n");
	printf("Usage:\n");
	printf("      %s <-p dir_name>\n", program_name);
	printf("\n");
	printf("\t-p\t--path\tThe dirent name for changing.\n");
	printf("\ne.g:\n");
	printf("%s -p /tmp\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *path = NULL;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hp:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "path", required_argument, NULL, 'p'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'p': /* Path */
			path = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !path) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_chdir
	 *
	 *    SYSCALL_DEFINE1(chdir,
	 *                    const char __user *, filename)
	 */
	syscall(__NR_chdir, path);
	printf("Current directory: %s\n", getcwd(NULL, 0));
	return 0;
}
