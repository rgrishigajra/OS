
#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <tree.h>
#define RED 0
#define BLACK 1

struct node{
	int key;
	int color;
	struct node *parent;
	struct node *left;
	struct node *right;
};

struct node *ROOT;
struct node *NILL;

void left_rotate(struct node *x);
void right_rotate(struct node *x);
void tree_print(struct node *x);
void red_black_insert(int key);
void red_black_insert_fixup(struct node *z);
struct node *tree_search(int key);
struct node *tree_minimum(struct node *x);
void red_black_transplant(struct node *u, struct node *v);
void red_black_delete(struct node *z);
void red_black_delete_fixup(struct node *x);

