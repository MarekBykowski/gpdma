#include <linux/module.h>
#include <linux/kthread.h>

#define CREATE_TRACE_POINTS
#include "netboot-trace.h"

static void silly_thread_func(void)
{
	static unsigned long count;
	unsigned long long value = 0x1111111111111111;

	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(HZ);
	printk("hello! %lu\n", count);
	trace_netboot_kernel_rw(        
           21/*NCP_HWIO_READ64*/,      
           0/*devNum*/,               
           123,             
           0x40000,               
           (unsigned char *) &value,
           sizeof(unsigned long long), 
           1, /* count */        
           0,                    
           0 /* flags */         
          );                     

	count++;
}

static int silly_thread(void *arg)
{
	while (!kthread_should_stop())
		silly_thread_func();

	return 0;
}

static struct task_struct *silly_tsk;

static int __init silly_init(void)
{
	silly_tsk = kthread_run(silly_thread, NULL, "silly-thread");
	if (IS_ERR(silly_tsk))
		return -1;

	return 0;
}

static void __exit silly_exit(void)
{
	kthread_stop(silly_tsk);
}

module_init(silly_init);
module_exit(silly_exit);

MODULE_AUTHOR("Steven Rostedt");
MODULE_DESCRIPTION("silly-module");
MODULE_LICENSE("GPL");
