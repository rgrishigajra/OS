#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <tree.h>

void display(int k, node *x);
void show(tree *T, node *x, int i);
void inorder_traversal(tree *T, node *x);
node *tr_search(tree *T, node *x, int k);

node *tr_search(tree *T, node *x, int k)
{
    if (x == T->nil || k == x->key)
        return x;
    if (k < x->key)
        return tr_search(T, x->left, k);
    else
        return tr_search(T, x->right, k);
}
void inorder_traversal(tree *T, node *x)
{
    if (x != T->nil)
    {
        inorder_traversal(T, x->left);
        printf("%d %d\t", x->key, x->color);
        inorder_traversal(T, x->right);
    }
}

// I took the logic for the functions from online, I dont take any credit for the functions below, i used them simply to test and show my RB tree
void display(int k, node *x)
{
    int i = 0;
    for (; i < k - 1; i++)
        printf("      ");
    if (k != 0)
        printf("|---");
    printf("%d", x->key);
    if (x->color == 1)
        printf("(r)\n");
    else
        printf("(b)\n");
}

void show(tree *T, node *x, int i)
{
    if (x != T->nil)
    {
        i++;
        show(T, x->right, i);
        i--;
        display(i, x);
        i++;
        show(T, x->left, i);
        i--;
    }
}
