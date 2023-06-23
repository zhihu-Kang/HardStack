// SPDX-License-Identifier: GPL-2.0
/*
 * LOCKING Mechanism on BiscuitOS: RW Semaphore
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>

static struct task_struct *tsk1;
static struct task_struct *tsk2;
static struct task_struct *tsk3;

static struct rw_semaphore rw_sema;

static int kthread_read(void *data)
{
	printk("%s Invoking.\n", (char *)data);
	down_read(&rw_sema);
	printk("%s RLOCK\n", (char *)data);
	while (!kthread_should_stop()) {
		; /* Do nothing */
	}
	up_read(&rw_sema);
	printk("%s RUNLOCK\n", (char *)data);
	return 0;
}

static int kthread_write(void *data)
{
	printk("%s Invoking.\n", (char *)data);
	down_write(&rw_sema);
	printk("%s WLOCK\n", (char *)data);
	while (!kthread_should_stop()) {
		; /* Do nothing */
	}
	up_write(&rw_sema);
	printk("%s WUNLOCK\n", (char *)data);
	return 0;
}

static int __init BiscuitOS_init(void)
{
	init_rwsem(&rw_sema);

	tsk1 = kthread_create(kthread_write, "Kthread1", "Kthread1");
	tsk2 = kthread_create(kthread_read, "Kthread2", "Kthread2");
	tsk3 = kthread_create(kthread_read, "Kthread3", "Kthread3");

	wake_up_process(tsk1);
	mdelay(2000); /* Force Kthread1 Running */
	wake_up_process(tsk2);
	wake_up_process(tsk3);

	printk("Wakeup finish...\n");
	/* STOP Kthread */
	mdelay(3000);
	kthread_stop(tsk1);
	mdelay(2000);
	kthread_stop(tsk2);
	mdelay(1000);
	kthread_stop(tsk3);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common LOCKING on BiscuitOS");
