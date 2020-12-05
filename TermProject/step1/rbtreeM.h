/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
  Red Black Trees
  (C) 1999  Andrea Arcangeli <andrea@suse.de>
  
  linux/include/linux/rbtree.h
  To use rbtrees you'll have to implement your own insert and search cores.
  This will avoid us to use callbacks and to drop drammatically performances.
  I know it's not the cleaner way,  but in C (not in C++) to get
  performances and genericity...
  See Documentation/core-api/rbtree.rst for documentation and samples.
*/

#ifndef	_LINUX_RBTREE_MUK_H
#define	_LINUX_RBTREE_MUK_H

#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/rcupdate.h>

#define FALSE 0
#define TRUE 1

//-----------modify---------------
struct rw_semaphore rwse;
struct rb_root tree = RB_ROOT;
struct rb_root* tree_root = &tree;
unsigned long long UNDELETED;
//--------------------------------

/*
struct rb_node {
	unsigned long  __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));
*/    /* The alignment might seem pointless, but allegedly CRIS needs it */

/*
struct rb_root {
	struct rb_node *rb_node;
};
*/
#define rb_parent(r)   ((struct rb_node *)((r)->__rb_parent_color & ~3))

#define RB_ROOT	(struct rb_root) { NULL, }
#define	rb_entry(ptr, type, member) container_of(ptr, type, member)

#define RB_EMPTY_ROOT(root)  (READ_ONCE((root)->rb_node) == NULL)

/* 'empty' nodes are nodes that are known not to be inserted in an rbtree */
#define RB_EMPTY_NODE(node)  \
	((node)->__rb_parent_color == (unsigned long)(node))
#define RB_CLEAR_NODE(node)  \
	((node)->__rb_parent_color = (unsigned long)(node))


extern void rb_insert_color(struct rb_node *, struct rb_root *);
extern void rb_erase(struct rb_node *, struct rb_root *);


/* Find logical next and previous nodes in a tree */
extern struct rb_node *rb_next(const struct rb_node *);
extern struct rb_node *rb_prev(const struct rb_node *);
extern struct rb_node *rb_first(const struct rb_root *);
extern struct rb_node *rb_last(const struct rb_root *);

/* Postorder iteration - always visit the parent after its children */
extern struct rb_node *rb_first_postorder(const struct rb_root *);
extern struct rb_node *rb_next_postorder(const struct rb_node *);

/* Fast replacement of a single node without remove/rebalance/add/rebalance */
extern void rb_replace_node(struct rb_node *victim, struct rb_node *new,
			    struct rb_root *root);
extern void rb_replace_node_rcu(struct rb_node *victim, struct rb_node *new,
				struct rb_root *root);
/*
static inline void rb_link_node(struct rb_node *node, struct rb_node *parent,
				struct rb_node **rb_link)
{
	down_write(&rwse);
	node->__rb_parent_color = (unsigned long)parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
	up_write(&rwse);
}
*/
/*
static inline void rb_link_node_rcu(struct rb_node *node, struct rb_node *parent,
				    struct rb_node **rb_link)
{
	node->__rb_parent_color = (unsigned long)parent;
	node->rb_left = node->rb_right = NULL;

	rcu_assign_pointer(*rb_link, node);
}
*/
#define rb_entry_safe(ptr, type, member) \
	({ typeof(ptr) ____ptr = (ptr); \
	   ____ptr ? rb_entry(____ptr, type, member) : NULL; \
	})

/**
 * rbtree_postorder_for_each_entry_safe - iterate in post-order over rb_root of
 * given type allowing the backing memory of @pos to be invalidated
 *
 * @pos:	the 'type *' to use as a loop cursor.
 * @n:		another 'type *' to use as temporary storage
 * @root:	'rb_root *' of the rbtree.
 * @field:	the name of the rb_node field within 'type'.
 *
 * rbtree_postorder_for_each_entry_safe() provides a similar guarantee as
 * list_for_each_entry_safe() and allows the iteration to continue independent
 * of changes to @pos by the body of the loop.
 *
 * Note, however, that it cannot handle other modifications that re-order the
 * rbtree it is iterating over. This includes calling rb_erase() on @pos, as
 * rb_erase() may rebalance the tree, causing us to miss some nodes.
 */
#define rbtree_postorder_for_each_entry_safe(pos, n, root, field) \
	for (pos = rb_entry_safe(rb_first_postorder(root), typeof(*pos), field); \
	     pos && ({ n = rb_entry_safe(rb_next_postorder(&pos->field), \
			typeof(*pos), field); 1; }); \
	     pos = n)

/*
 * Leftmost-cached rbtrees.
 *
 * We do not cache the rightmost node based on footprint
 * size vs number of potential users that could benefit
 * from O(1) rb_last(). Just not worth it, users that want
 * this feature can always implement the logic explicitly.
 * Furthermore, users that want to cache both pointers may
 * find it a bit asymmetric, but that's ok.
 */
/*
struct rb_root_cached {
	struct rb_root rb_root;
	struct rb_node *rb_leftmost;
};
*/
#define RB_ROOT_CACHED (struct rb_root_cached) { {NULL, }, NULL }

/* Same as rb_first(), but O(1) */
#define rb_first_cached(root) (root)->rb_leftmost
/*
static inline void rb_insert_color_cached(struct rb_node *node,
					  struct rb_root_cached *root,
					  bool leftmost)
{
	if (leftmost)
		root->rb_leftmost = node;
	rb_insert_color(node, &root->rb_root);
}
*/
/*
static inline void rb_erase_cached(struct rb_node *node,
				   struct rb_root_cached *root)
{
	if (root->rb_leftmost == node)
		root->rb_leftmost = rb_next(node);
	rb_erase(node, &root->rb_root);
}
*/
/*
static inline void rb_replace_node_cached(struct rb_node *victim,
					  struct rb_node *new,
					  struct rb_root_cached *root)
{
	if (root->rb_leftmost == victim)
		root->rb_leftmost = new;
	rb_replace_node(victim, new, &root->rb_root);
}
*/

//-----------modify---------------
void initiate_rbtree(void)
{
	init_rwsem(&rwse);
}
//EXPORT_SYMBOL(initiate_rbtree);

struct my_type {
	struct rb_node node;
	int key;
	int value;
};
int rb_insert(struct my_type *data)
{
	struct rb_node **newNode = &(tree_root->rb_node), *parent = NULL;
	/* Figure out "where" to put new node */
	while (*newNode) {
		struct my_type *thisC = container_of(*newNode, struct my_type, node);

		parent = *newNode;
		if (thisC->key > data->key)
			newNode = &((*newNode)->rb_left);
		else if (thisC->key < data->key)
			newNode = &((*newNode)->rb_right);
		else
		{
			return FALSE;
		}
	}
	rb_link_node(&data->node, parent, newNode);		// relinking
	rb_insert_color(&data->node, tree_root);		// recoloring & rebalancing
	

	return TRUE;
}
//EXPORT_SYMBOL(rb_insert);

struct my_type *rb_search(int key)
{
	struct rb_node *node = tree_root->rb_node;

	while (node) {
		struct my_type *data = container_of(node, struct my_type, node);

		if (data->key > key)
			node = node->rb_left;
		else if (data->key < key)
			node = node->rb_right;
		else
		{
			return data;
		}

	}
	return NULL;
}
//EXPORT_SYMBOL(rb_search);

void rb_delete(int key)
{
	struct my_type *data;
	data = rb_search(key);

	if (data) {
		rb_erase(&data->node, tree_root);
		kfree(data);
	}
	else
		__sync_fetch_and_add(&UNDELETED,1);
	
}
//EXPORT_SYMBOL(rb_delete);

//--------------------------------

#endif	/* _LINUX_RBTREE_H */
