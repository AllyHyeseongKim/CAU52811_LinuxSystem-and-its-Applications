#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>	// for thread
#include <linux/slab.h>		// for kmalloc
#include <linux/delay.h>


int test_thread(void *_arg)
{
	int* arg = (int*)_arg;
	printk("argument: %d\n", *arg);
	kfree(arg);
	return 0;
}

void thread_create(void)
{
	int i;
	/* thread create */
	for (i=0; i<10; i++) {
		int* arg = (int*)kmalloc(sizeof(int), GFP_KERNEL);
		*arg=i;
		kthread_run(&test_thread, (void*)arg, "test_thread");
	}
}

int __init kernel_module_init(void)
{
	thread_create();
	printk(KERN_EMERG "Hello Kernel Module\n");
	return 0;
}

void __exit kernel_module_cleanup(void)
{
	printk("Bye Kernel Module\n");
}

module_init(kernel_module_init);
module_exit(kernel_module_cleanup);
MODULE_LICENSE("GPL");
