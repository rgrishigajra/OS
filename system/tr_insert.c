#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <tree.h>

void rotate_left(tree *T, node *x);
void rotate_right(tree *T, node *y);
void tr_insert_fixup(tree *T, node *z);
void tr_insert(tree *T, pid32 key);

void tr_insert(tree *T, pid32 key)
{
    node *new_node = (node *)getmem(sizeof(node));
    new_node->key = key;
    node *parent_node = T->nil;
    node *current_node = T->root;
    while (current_node != T->nil)
    {
        parent_node = current_node;
        if (new_node->key < current_node->key)
            current_node = current_node->left;
        else
            current_node = current_node->right;
    }
    new_node->parent = parent_node;
    if (parent_node == T->nil)
        T->root = new_node;
    else if (new_node->key < parent_node->key)
        parent_node->left = parent_node;
    else
        parent_node->right = parent_node;
    new_node->left = T->nil;
    new_node->right = T->nil;
    new_node->color = red;
    tr_insert_fixup(T, parent_node);
}
