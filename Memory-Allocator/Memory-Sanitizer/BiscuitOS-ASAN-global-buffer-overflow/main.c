/*
 * ASAN: Global-buffer-overflow
 *
 * (C) 2022.02.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int array[10] = { 0 };

int main()
{
	/* BOOM: Global buffer overflow */
	array[10] = 0x20;
	return 0;
}
