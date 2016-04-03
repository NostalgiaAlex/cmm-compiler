#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "lib/Tree.h"
#include "lib/List.h"
#include "common.h"

TreeNode* createTree(int airty, ...) {
	va_list ap;
	va_start(ap, airty);
	TreeNode *root = newNode();
	root->isToken = false;
	int i;
	for (i = 0; i < airty; i++) {
		TreeNode *p = va_arg(ap, TreeNode*);
		if (i == 0) root->lineNo = p->lineNo;
		if (p != NULL) treeAddChild(root, p);
	}
	va_end(ap);
	return root;
}

void print(TreeNode* root, int stop) {
	printf("%*s%s", stop*2, "", root->name);
	if (root->isToken) {
		if (isSyntax(root, INT)) {
			printf(": %d", root->intVal);
		} else if (isSyntax(root, FLOAT)) {
			printf(": %.10lf", root->floatVal);
		} else if (isSyntax(root, ID) || isSyntax(root, TYPE)) {
			printf(": %s", root->text);
		}
	} else {
		printf(" (%d)", root->lineNo);
	}
	puts("");
	ListHead *p;
	listForeach(p, &root->children) {
		print(listEntry(p, TreeNode, list), stop+1);
	}
}
