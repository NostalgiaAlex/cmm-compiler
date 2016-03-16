#ifndef __LIB_NODE_H__
#define __LIB_NODE_H__

#include "lib/List.h"

typedef struct TreeNode {
	int lineNo, token, intVal;
	float floatVal;
	char *text, *name;
	ListHead list, children;
} TreeNode;

TreeNode* newNode();
void treeAddChild(TreeNode*, TreeNode*);

#endif
