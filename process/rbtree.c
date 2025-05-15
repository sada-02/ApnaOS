#include "rbtree.h"
void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *parent, *gparent;

    while ((parent = node->rb_parent) && parent->rb_red) {
        gparent = parent->rb_parent;
        if (parent == gparent->rb_left) {
            struct rb_node *uncle = gparent->rb_right;
            if (uncle && uncle->rb_red) {
                parent->rb_red = uncle->rb_red = false;
                gparent->rb_red = true;
                node = gparent;
                continue;
            }
            if (parent->rb_right == node) {
                rb_rotate_left(parent, root);
                struct rb_node *tmp = parent;
                parent = node;
                node = tmp;
            }
            parent->rb_red = false;
            gparent->rb_red = true;
            rb_rotate_right(gparent, root);
        } else {
            struct rb_node *uncle = gparent->rb_left;
            if (uncle && uncle->rb_red) {
                parent->rb_red = uncle->rb_red = false;
                gparent->rb_red = true;
                node = gparent;
                continue;
            }
            if (parent->rb_left == node) {
                rb_rotate_right(parent, root);
                struct rb_node *tmp = parent;
                parent = node;
                node = tmp;
            }
            parent->rb_red = false;
            gparent->rb_red = true;
            rb_rotate_left(gparent, root);
        }
    }
    /* ensure root is always black */
    root->rb_node->rb_red = false;
}
void rb_erase(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *child, *parent;
    bool color;

    if (!node->rb_left)
        child = node->rb_right;
    else if (!node->rb_right)
        child = node->rb_left;
    else {
        struct rb_node *old = node, *left;
        node = node->rb_right;
        while ((left = node->rb_left) != NULL)
            node = left;
        if (old->rb_parent) {
            if (old->rb_parent->rb_left == old)
                old->rb_parent->rb_left = node;
            else
                old->rb_parent->rb_right = node;
        } else {
            root->rb_node = node;
        }
        child = node->rb_right;
        parent = node->rb_parent;
        color = node->rb_red;
        if (parent == old) {
            parent = node;
        } else {
            if (child) child->rb_parent = parent;
            parent->rb_left = child;
            node->rb_right = old->rb_right;
            old->rb_right->rb_parent = node;
        }
        node->rb_parent = old->rb_parent;
        node->rb_red = old->rb_red;
        node->rb_left = old->rb_left;
        old->rb_left->rb_parent = node;
        goto color_fixup;
    }

    parent = node->rb_parent;
    color  = node->rb_red;
    if (child) child->rb_parent = parent;

    if (parent) {
        if (parent->rb_left == node)
            parent->rb_left = child;
        else
            parent->rb_right = child;
    } else {
        root->rb_node = child;
    }

color_fixup:
    if (color == false) {
        struct rb_node *sibling;
        while ((child != root->rb_node) &&
               (child == NULL || child->rb_red == false)) {
            if (parent->rb_left == child) {
                sibling = parent->rb_right;
                if (sibling->rb_red) {
                    sibling->rb_red = false;
                    parent->rb_red = true;
                    rb_rotate_left(parent, root);
                    sibling = parent->rb_right;
                }
                if ((!sibling->rb_left || !sibling->rb_left->rb_red) &&
                    (!sibling->rb_right || !sibling->rb_right->rb_red)) {
                    sibling->rb_red = true;
                    child = parent;
                    parent = child->rb_parent;
                } else {
                    if (!sibling->rb_right || !sibling->rb_right->rb_red) {
                        if (sibling->rb_left) sibling->rb_left->rb_red = false;
                        sibling->rb_red = true;
                        rb_rotate_right(sibling, root);
                        sibling = parent->rb_right;
                    }
                    sibling->rb_red = parent->rb_red;
                    parent->rb_red = false;
                    if (sibling->rb_right) sibling->rb_right->rb_red = false;
                    rb_rotate_left(parent, root);
                    child = root->rb_node;
                }
            } else {
                /* symmetric code for right child */
                sibling = parent->rb_left;
                if (sibling->rb_red) {
                    sibling->rb_red = false;
                    parent->rb_red = true;
                    rb_rotate_right(parent, root);
                    sibling = parent->rb_left;
                }
                if ((!sibling->rb_left || !sibling->rb_left->rb_red) &&
                    (!sibling->rb_right || !sibling->rb_right->rb_red)) {
                    sibling->rb_red = true;
                    child = parent;
                    parent = child->rb_parent;
                } else {
                    if (!sibling->rb_left || !sibling->rb_left->rb_red) {
                        if (sibling->rb_right) sibling->rb_right->rb_red = false;
                        sibling->rb_red = true;
                        rb_rotate_left(sibling, root);
                        sibling = parent->rb_left;
                    }
                    sibling->rb_red = parent->rb_red;
                    parent->rb_red = false;
                    if (sibling->rb_left) sibling->rb_left->rb_red = false;
                    rb_rotate_right(parent, root);
                    child = root->rb_node;
                }
            }
        }
        if (child) child->rb_red = false;
    }
}

void rb_rotate_left(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *right = node->rb_right;
    struct rb_node *parent = node->rb_parent;

    if ((node->rb_right = right->rb_left))
        right->rb_left->rb_parent = node;
    right->rb_left = node;
    node->rb_parent = right;

    if ((right->rb_parent = parent)) {
        if (parent->rb_left == node)
            parent->rb_left = right;
        else
            parent->rb_right = right;
    } else {
        root->rb_node = right;
    }
}

void rb_rotate_right(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *left = node->rb_left;
    struct rb_node *parent = node->rb_parent;

    if ((node->rb_left = left->rb_right))
        left->rb_right->rb_parent = node;
    left->rb_right = node;
    node->rb_parent = left;

    if ((left->rb_parent = parent)) {
        if (parent->rb_right == node)
            parent->rb_right = left;
        else
            parent->rb_left = left;
    } else {
        root->rb_node = left;
    }
}
