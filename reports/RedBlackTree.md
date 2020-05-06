
**

# **Red Black Trees Implementation in Xinu**

**
**


***# The basic rules***
 - Every node is red or black.
 - Root node is always black
 - New insertions are always red.
 - Every path from root-leaf must have same number of Black nodes.
 - No path can have two consequtive Red Nodes.
 - Nulls are always black Nodes.

***# What if the current state does not follow the rules?***
 - We Rebalance based on the Aunt. The Aunt is the sibling of the parent node. Which means we look at the other child of the parent's parent node.
 - If the Aunt is Black Node, we rotate. After rotation the three nodes are Parent is black and children are red.
 - If we have a Red Aunt, We color flip. After color flip, the parent is Red and the children are black.
 
 ##
 ***# Implementation***
 **Structures**:
 I created a tree.h file that has all the structures and the function signatures that I used for the Red black tree assignment.
 

     typedef  enum  colorType
     {red = 1,
    black = 0
    } nodeColor;

A structure that is used as a color representation, for more human readibility
  

    typedef  struct  Tree
    {node *nil;
    node *root;
    } tree;

We will define a tree which will have a starting root node and nil to represent empty nodes and temporary nodes to perform operations.

      

    typedef  struct  treeNode
        {nodeColor color;
        pid32 key;
        struct  treeNode  *left;
        struct  treeNode  *right;
        struct  treeNode  *parent;
        } node;
As for a normal tree, we will have pointers to left and right children with a parent node
 **Functions**:

 - system/newtree.c/ tree  *newtree()
	 Declares a Tree and returns a root node that is empty. Note this node is made black even without value due to rule #1
	 
 - app/rbt.c/unit rbt()
Tests all the main functions, Does insert, Search, In order traversal and display that also shows levels of each node.
 
 - system/tr_search.c/node *tr_search(tree *T, node *x, int  k)
 Works just like a basic Tree search, goes left if value queried is smaller, right if its larger.
 
 
 - system/tr_search.c/void  inorder_traversal(tree *T, node *x)
This also works like a default BST inorder traversal. Also prints 1 for red and 0 for black while printing key value.
 - system/tr_search.c/void  display(int  k, node *x) and show()
 I took the logic for these functions from online since they are there just for testing purposes. A combination of both functions prints a well formated tree with colors.
 - system/tr_insert.c/void  tr_insert(tree *T, pid32 key)
This may be the main function but it works like the BST insert function, everything special about Red black trees happens in the tr_insert_fixup thats called at the end of the function. It adds a new node to one of the leaves.
 -   system/tr_insert.c/void  tr_insert_fixup(tree *T, pid32 key)
 We start with the parent node of the node thats just added. Iterate until z is not the root and z's parent color is red.
 If Z'parent is a left child, and aunt is red aswell, we do a color flip like in the rules. 
 If aunt is black, we do a rotate left.
 Similarly, if Z's parent is a right child and red aunt, we again do a color flip. We do a rotate right when Z is a left child istelf and left rotate when Z is a right child instead.
 
 -   system/tr_insert.c/void  rotate_left(tree *T, node *x)
We move the x nodes right childs left child to X's right child place. We proceed to then shift each of the nodes to the left circularly where initial parent moves to left child, X becomes parent the child comes at C place. The direction of the parent going depends on what X is, a left child or right child.
 -   system/tr_insert.c/rotate_right(tree *T, node *y)
Its pretty much rotate left but opposite.

