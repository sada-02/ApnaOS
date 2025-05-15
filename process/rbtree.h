#ifndef RBTREE_H
#define RBTREE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

struct rb_node {
    struct rb_node *rb_left;
    struct rb_node *rb_right;
    struct rb_node *rb_parent;
    bool            rb_red;    /* red = true, black = false */
};

/* The tree root */
struct rb_root {
    struct rb_node *rb_node;
};

/* initialize */
#define RB_ROOT (struct rb_root){ .rb_node = NULL }

#define rb_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

void rb_rotate_left(struct rb_node *node, struct rb_root *root);
void rb_rotate_right(struct rb_node *node, struct rb_root *root);

void rb_insert_color(struct rb_node *node, struct rb_root *root);

/* Remove a node and rebalance */
void rb_erase(struct rb_node *node, struct rb_root *root);


static inline void rb_link_node(struct rb_node *node,
                                struct rb_node *parent,
                                struct rb_node **link)
{
    node->rb_parent = parent;
    node->rb_left   = node->rb_right = NULL;
    node->rb_red    = true;
    *link = node;
}

static inline struct rb_node *rb_first(const struct rb_root *root)
{
    struct rb_node *n = root->rb_node;
    while (n && n->rb_left)
        n = n->rb_left;
    return n;
}

#endif /* RBTREE_H */
