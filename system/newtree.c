#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <tree.h>
tree *newtree();
tree *newtree()
{
    tree *T = (tree *)getmem(sizeof(node));
    node *nil = (node *)getmem(sizeof(node));
    nil->color = black;
    node *root = nil;
    T->nil = nil;
    T->root = root;
    return T;
}

