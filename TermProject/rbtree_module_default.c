#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/slab.h>		// for kmalloc
#include "rbtree.h"

#define FALSE 0
#define TRUE 1


static int insert_function(void *_data)
{
	int* data = (int*)_data;
	/* rb_node create and insert */
	int i = 0, ret;
	while (!kthread_should_stop()) {
		down_write(&counter_rwse);
		counter++;
		printk("%s, writer_counter: %d, pid: %u\n", __func__, counter, current->pid);

		for (;i<num_nodes;i++) {
			struct my_type *new = kmalloc(sizeof(struct my_type), GFP_KERNEL);
                	if(!new)
                	        return NULL;

                	new->value = i*10;
                	new->key = i+*data;

                	ret = rb_insert(&my_tree, new);
		}
		printk("[Insert %d ~ %d nodes] insert %d nodes in the reb black tree\n", *data, num_nodes+*data, i);

		up_write(&counter_rwse);
	}
	do_exit(0);
}

static int search_function(void *_data)
{
	int* data = (int*)_data;
	/* rb_tree find node */
	int j = 0;
	while (!kthread_should_stop()) {
                down_read(&counter_rwse);
                counter++;
                printk("%s, reader_counter: %d, pid: %u\n", __func__, counter, current->pid);

		for (;j<num_nodes;j++) {
			struct my_type *find_node = rb_search(&my_tree, j+*data);
        		if(!find_node) {
				return NULL;
			}
		}
		printk("[Search %d ~ %d nodes] search %d nodes in the reb black tree\n", *data, num_nodes+*data, j);

        	//up_write(&counter_rwse);
        	up_read(&counter_rwse);
        	//msleep(500);
	}
	do_exit(0);
}

static int delete_function(void *_data)
{
	int* data = (int*)_data;
	/* rb_tree delete node */
	int k = 0;
	while (!kthread_should_stop()) {
		down_write(&counter_rwse);
                counter++;
                printk("%s, writer_counter: %d, pid: %u\n", __func__, counter, current->pid);

		for (k;k<num_nodes;k++) {	
			rb_delete(&my_tree, k+*data);
		}
		printk("[Delete %d ~ %d nodes] delete %d nodes in the red black tree\n", *data, num_nodes+*data, k);

        	up_write(&counter_rwse);
	}
	do_exit(0);
}


struct task_struct *insert_thread, *delete_thread, *search_thread;

void struct_example(int num)
{
	int* data = (int*)kmalloc(sizeof(int), GFP_KERNEL);
	*data = num;
	insert_thread = kthread_run(insert_function, (void*)data, "insert_function");
	search_thread = kthread_run(search_function, (void*)data, "search_function");
	delete_thread = kthread_run(delete_function, (void*)data, "delete_function");

}

int __init rbtree_module_init(void)
{
	printk("--------------------------Red Black Tree improvement--------------------------\n");
	initiate_rbtree();		

	struct_example(0);
	struct_example(10);
	struct_example(20);
	
	printk("rbtree module init\n");
        return 0;
}

void __exit rbtree_module_cleanup(void)
{
	printk("Bye Module\n");

	printk("-------------------------------------Done-------------------------------------\n");
}

module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);

MODULE_LICENSE("GPL");
