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

void tr_insert_fixup(tree *T, node *z)
{
    node *y = (node *)getmem(sizeof(node));
    while (z->parent->color == red)
    {
        if (z->parent == z->parent->parent->left)
        {
            y = z->parent->parent->right;
            if (y->color == red)
            {
                z->parent->color = black;
                y->color = black;
                z->parent->parent->color = red;
                z = z->parent->parent;
            }
            else if (z == z->parent->right)
            {
                z = z->parent;
                rotate_left(T, z);
            }
            else
            {
                z->parent->color = black;
                z->parent->parent->color = red;
                rotate_right(T, z->parent->parent);
            }
        }
        else
        {
            y = z->parent->parent->left;
            if (y->color == red)
            {
                z->parent->color = black;
                y->color = black;
                z->parent->parent->color = red;
                z = z->parent->parent;
            }
            else if (z == z->parent->left)
            {
                z = z->parent;
                rotate_right(T, z);
            }
            else
            {
                z->parent->color = black;
                z->parent->parent->color = red;
                rotate_left(T, z->parent->parent);
            }
        }
    }
    T->root->color = black;
}

void rotate_left(tree *T, node *x)
{
    node *y = (node *)getmem(sizeof(node));
    y = x->right;
    x->right = y->left;
    if (y->left != T->nil)
        y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == T->nil)
        T->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x;
    x->parent = y;
}

void rotate_right(tree *T, node *y)
{
    node *x = (node *)getmem(sizeof(node));
    x = y->left;
    y->left = x->right;
    if (x->right != T->nil)
        x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == T->nil)
        T->root = x;
    else if (y == y->parent->right)
        y->parent->right = x;
    else
        y->parent->left = x;
    x->right = y;
    y->parent = x;
}
