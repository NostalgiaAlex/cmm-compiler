#include "lib/Tree.h"
#include "lib/List.h"
#include <stdarg.h>
#include <stdio.h>

TreeNode* createTree(int airty, ...) {
	va_list ap;
	va_start(ap, airty);
	TreeNode *root = newNode();
	int i;
	for (i = 0; i < airty; i++) {
		TreeNode *p = va_arg(ap, TreeNode*);
		if (i == 0) root->lineNo = p->lineNo;
		if (p != NULL) treeAddChild(root, p);
	}
	va_end(ap);
	return root;
}

void print(TreeNode* root) {
	puts(root->name);
	ListHead *p;
	listForeach(p, &root->children) {
		print(listEntry(p, TreeNode, list));
	}
}
