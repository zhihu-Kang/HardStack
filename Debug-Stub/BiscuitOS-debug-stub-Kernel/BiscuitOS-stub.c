/*
 * BiscuitOS Kernel Debug Stub
 *
 * (C) 2020.03.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.04.01 BiscuitOS
 *                <https://biscuitos.github.io/blog/BiscuitOS_Catalogue/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sysctl.h>

int bs_debug_kernel_enable;
int bs_debug_kernel_enable_one;
unsigned long bs_debug_async_data;
EXPORT_SYMBOL_GPL(bs_debug_async_data);
EXPORT_SYMBOL_GPL(bs_debug_kernel_enable);
EXPORT_SYMBOL_GPL(bs_debug_kernel_enable_one);

SYSCALL_DEFINE1(debug_BiscuitOS, unsigned long, enable)
{
	if (enable == 1) {
		bs_debug_kernel_enable = 1;
		bs_debug_kernel_enable_one = 1;
	} else if (enable == 0) {
		bs_debug_kernel_enable = 0;
		bs_debug_kernel_enable_one = 0;
	} else
		bs_debug_async_data = enable;

	return 0;
}

static int BiscuitOS_bs_debug_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *length, loff_t *ppos)
{
	int ret;

	ret = proc_dointvec(table, write, buffer, length, ppos);
	if (bs_debug_kernel_enable) {
		bs_debug_kernel_enable_one = 1;
		bs_debug_kernel_enable = 1;
	} else {
		bs_debug_kernel_enable_one = 0;
		bs_debug_kernel_enable = 0;
	}

	return ret;
}

static struct ctl_table BiscuitOS_table[] = {
	{
		.procname	= "bs_debug_kernel_enable",
		.data		= &bs_debug_kernel_enable,
		.maxlen		= sizeof(unsigned long),
		.mode		= 0644,
		.proc_handler	= BiscuitOS_bs_debug_handler,
	},
	{ }
};

static struct ctl_table sysctl_BiscuitOS_table[] = {
	{
		.procname	= "BiscuitOS",
		.mode		= 0555,
		.child		= BiscuitOS_table,
	},
	{ }
};

static int __init BiscuitOS_debug_proc(void)
{
	register_sysctl_table(sysctl_BiscuitOS_table);
	return 0;
}
device_initcall(BiscuitOS_debug_proc);
