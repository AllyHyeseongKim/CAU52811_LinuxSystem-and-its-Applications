#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/slab.h>		// for kmalloc
#include <linux/time.h>
#include "rbtreeM.h"

#define NUM_OF_NODE 100000
#define NUM_OF_THREAD 16


#define BILLION 1000000000


void calclock(struct timespec *myclock,unsigned long long *total_time, unsigned long long *total_count) {
	long temp,temp_n;
	unsigned long long timedelay=0;
	if(myclock[1].tv_nsec >=myclock[0].tv_nsec){
		temp=myclock[1].tv_sec - myclock[0].tv_sec;
		temp_n=myclock[1].tv_nsec - myclock[0].tv_nsec;
		timedelay = temp*BILLION+temp_n;

	}else{
		temp=myclock[1].tv_sec - myclock[0].tv_sec -1;
		temp_n= BILLION + myclock[1].tv_nsec - myclock[0].tv_nsec;
		timedelay = temp*BILLION+temp_n;
	}
	__sync_fetch_and_add(total_time,timedelay);
	__sync_fetch_and_add(total_count,1);
}

unsigned long long insertTime,searchTime,deleteTime,insertCount,searchCount,deleteCount;

static int insert_function(void *_data)
{
	struct timespec ck[2];
	int* data =(int*)_data;
	/* rb_node create and insert */
	int i = 0, ret;
	int chunkSize = NUM_OF_NODE/NUM_OF_THREAD;
	getnstimeofday(&ck[0]);

//	down_write(&rwse);

	for (;i<chunkSize;i++) {
		struct my_type *new = kmalloc(sizeof(struct my_type), GFP_KERNEL);
               	if(!new)
               	        return -1;
               	new->value = i*10;
               	new->key = i+(*data)*chunkSize;
               	ret = rb_insert(new);
	}
	printk("(%d)[Insert %d ~ %d nodes] insert %d nodes in the reb black tree\n",*data, (*data)*chunkSize, ((*data)+1)*chunkSize-1, i);

//	up_write(&rwse);

	getnstimeofday(&ck[1]);
	calclock(ck,&insertTime,&insertCount);

	do_exit(0);
}

static int search_function(void *_data)
{
	struct timespec ck[2];
	int* data = (int*)_data;
	/* rb_tree find node */
	int j = 0;
	int chunkSize = NUM_OF_NODE/NUM_OF_THREAD;
	getnstimeofday(&ck[0]);
	for (;j<chunkSize;j++) {
		struct my_type *find_node;
		find_node = rb_search(j+(*data)*chunkSize);
        	if(!find_node) {
			return -1;
		}
	}

	getnstimeofday(&ck[1]);
	calclock(ck,&searchTime,&searchCount);
	
	do_exit(0);
}

static int delete_function(void *_data)
{
	struct timespec ck[2];
	int* data = (int*)_data;
	/* rb_tree delete node */
	int k = 0;
	int chunkSize = NUM_OF_NODE/NUM_OF_THREAD;
	getnstimeofday(&ck[0]);

	for (;k<chunkSize;k++) {
		rb_delete(k+(*data)*chunkSize);
	}
	printk("(%d)[Delete %d ~ %d nodes] delete %d nodes in the red black tree\n",*data, (*data)*chunkSize, ((*data)+1)*chunkSize-1, k);

	getnstimeofday(&ck[1]);
	calclock(ck,&deleteTime,&deleteCount);
	
	do_exit(0);
}


struct task_struct *insert_thread, *delete_thread, *search_thread;

void struct_example(void)
{
	//int* data = (int*)kmalloc(sizeof(int), GFP_KERNEL);
	int i=0;
	for(;i<NUM_OF_THREAD;i++){
		insert_thread = kthread_run(insert_function,&i, "insert_function");
		msleep(500);
		search_thread = kthread_run(search_function,&i, "search_function");
		msleep(500);
		delete_thread = kthread_run(delete_function,&i, "delete_function");
		msleep(500);
	}
}

int __init rbtree_module_init(void)
{
	printk("--------------------------Red Black Tree improvement--------------------------\n");
	
	initiate_rbtree();

	struct_example();
	
	msleep(5000);

	printk("rb_tree insert time : %llu | count : %llu\n",insertTime,insertCount);
	printk("rb_tree search time : %llu | count : %llu\n",searchTime,searchCount);
	printk("rb_tree delete time : %llu | count : %llu\n",deleteTime,deleteCount);
	printk("UNDELETED : %llu\n",UNDELETED);
        return 0;
}

void __exit rbtree_module_cleanup(void)
{
	printk("Bye Module\n");

	printk("-------------------------------------Done-------------------------------------\n");
}

MODULE_LICENSE("GPL");
module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);

