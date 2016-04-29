#ifndef COMPILER_SYNTAX_TREE_H
#define COMPILER_SYNTAX_TREE_H

#include "lib/List.h"

typedef struct TreeNode {
	int lineNo, token;
	union {
		int intVal;
		float floatVal;
	};
	char *text, *name;
	ListHead list, children;
} TreeNode;

extern TreeNode *root;

TreeNode* newNode();
void treeAddChild(TreeNode*, TreeNode*);
bool treeIsLeaf(TreeNode*);
TreeNode* treeFirstChild(TreeNode*);
TreeNode* treeLastChild(TreeNode*);
TreeNode* treeKthChild(TreeNode*, int);
TreeNode* treeLastKthChild(TreeNode*, int);

#endif
