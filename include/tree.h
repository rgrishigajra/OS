typedef enum colorType
{
	red = 1,
	black = 0
} nodeColor;

typedef int nodeKey;

typedef struct treeNode
{
	nodeColor color;
	nodeKey key;
	struct treeNode *left;
	struct treeNode *right;
	struct treeNode *parent;
} node;

typedef struct Tree
{
	node *nil;
	node *root;
} tree;
void tr_insert(tree *T, nodeKey key);
void display(int k, node *x);
void show(tree *T, node *x, int i);
void inorder_traversal(tree *T, node *x);
node *tr_search(tree *T, node *x, int k);
uint rbt();
tree *newtree();