#include <xinu.h>
#include <tree.h>

uint rbt()
{
	tree *T = newtree();
	tr_insert(T, 3);
	tr_insert(T, 1);
	tr_insert(T, 5);
	tr_insert(T, 7);
	tr_insert(T, 6);
	tr_insert(T, 8);
	tr_insert(T, 9);
	tr_insert(T, 10);
	printf("Inorder Traversal: shows node, color code (red=1,black=0)\n:");
	inorder_traversal(T, T->root);
	node *search_result = tr_search(T, T->root, 9);
	printf("searched this:%d,\t found at: %p\n", search_result->key, search_result);
	printf("\nSpecial printf function that prints with levels to show a balanced tree with colors of nodes:\n");
	show(T, T->root, 0);
	return 0;
}