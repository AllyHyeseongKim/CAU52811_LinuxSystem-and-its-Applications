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


int counter;
struct rw_semaphore counter_rwse;


struct my_type {
	struct rb_node node;
	int key;
	int value;
};

int rb_insert(struct rb_root *root, struct my_type *data)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out "where" to put new node */
	while (*new) {
		struct my_type *this = container_of(*new, struct my_type, node);
		parent = *new;
		if (this->key > data->key)
			new = &((*new)->rb_left);
		else if (this->key < data->key)
			new = &((*new)->rb_right);
		else
			return FALSE;
	}

	rb_link_node(&data->node, parent, new);		// relinking
	rb_insert_color(&data->node, root);		// recoloring & rebalancing

	return TRUE;
}

struct my_type *rb_search(struct rb_root *root, int key)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct my_type *data = container_of(node, struct my_type, node);

		if (data->key > key)
			node = node->rb_left;
		else if (data->key < key)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}

int rb_delete(struct rb_root *mytree, int key)
{
	struct my_type *data = rb_search(mytree, key);

	if (data) {
		rb_erase(&data->node, mytree);
		kfree(data);
	}
}

struct rb_root my_tree = RB_ROOT;
int num_nodes = 20;

static int insert_function(void *data)
{
	/* rb_node create and insert */
	int i = 0, ret;
	while (!kthread_should_stop) {
		down_write(&counter_rwse);
		counter++;
		printk("%s, writer_counter: %d, pid: %u\n", __func__, counter, current->pid);
		down_read(&counter_rwse);
		counter++;
		printk("%s, reader_counter: %d, pid: %u\n", __func__, counter, current->pid);
        	for (;i<num_nodes;i++) {
                	struct my_type *new = kmalloc(sizeof(struct my_type), GFP_KERNEL);
                	if(!new)
                	        return NULL;

                	new->value = i*10;
                	new->key = i;

                	ret = rb_insert(&my_tree, new);
        	printk("[insert] %d\n", i);
		}
	}
	printk("[Insert %d nodes] insert %d nodes in the reb black tree\n", num_nodes, i);
	up_write(&counter_rwse);
	up_read(&counter_rwse);
	msleep(500);
	
	do_exit(0);
}

static int search_function(void *data)
{
	/* rb_tree find node */
	int j = 0;
	while (!kthread_should_stop) {
		for (;j<num_nodes;j++) {
        		struct my_type *find_node = rb_search(&my_tree, j);
        		if(!find_node) {
				return NULL;
			}
		}
	}
	printk("[Search %d nodes] search %d nodes in the reb black tree\n", num_nodes, j);
	
	do_exit(0);
}

static int delete_function(void *data)
{
	/* rb_tree delete node */
	int k = 0;
	for (k;k<num_nodes;k++) {	
		rb_delete(&my_tree, k);
	}
	printk("[Delete %d nodes] delete %d nodes in the red black tree\n", num_nodes, k);
	
	do_exit(0);
}


struct task_struct *insert_thread, *delete_thread, *search_thread;

void struct_example(void)
{
	insert_thread = kthread_run(insert_function, NULL, "insert_function");
	search_thread = kthread_run(search_function, NULL, "search_function");
	delete_thread = kthread_run(delete_function, NULL, "delete_function");
	
	kthread_stop(insert_thread);
	kthread_stop(search_thread);
	kthread_stop(delete_thread);
}

int __init rbtree_module_init(void)
{
	counter = 0;
	init_rwsem(&counter_rwse);

	struct_example();
	//struct_example();
	//struct_example();

	printk("rbtree module init\n");
        return 0;
}

void __exit rbtree_module_cleanup(void)
{
	printk("Bye Module\n");
}

module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);

MODULE_LICENSE("GPL");
