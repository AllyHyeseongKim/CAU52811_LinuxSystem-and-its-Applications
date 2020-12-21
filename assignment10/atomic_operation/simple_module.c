#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>	// for thread
#include <linux/slab.h>		// for kmalloc
#include <linux/delay.h>


#define NUM_OF_THREAD 4
int i;

int foo(void *_data)
{
	int data = *(int*)_data;
	int prev = __sync_fetch_and_add(&i, 1);
	printk("(%d) [__sync_fetch_and_add] i: %d -> %d\n", data, prev, i);

	prev = __sync_lock_test_and_set(&i, 1);
	printk("(%d) [__sync_lock_test_and_set] i: %d -> %d\n", data, prev, i);

	prev = __sync_val_compare_and_swap(&i, 1, 2);
	printk("(%d) [__val_sync_compare_and_swap] i: %d -> %d\n", data, prev, i);
	return 0;
}

int __init simple_module_init(void)
{
	printk("------------------------------------Atomic Operation------------------------------------\n");
	i = 0;
	int l=0;
	for (; l<NUM_OF_THREAD; l++) {
		kthread_run(&foo, &l, "foo");
		msleep(10);
	}
	msleep(500);
	return 0;
}

void __exit simple_module_cleanup(void)
{
	printk("----------------------------------------------------------------------------------------\n");
}

module_init(simple_module_init);
module_exit(simple_module_cleanup);

MODULE_LICENSE("GPL");
