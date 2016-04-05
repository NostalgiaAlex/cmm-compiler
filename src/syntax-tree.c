#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "lib/Tree.h"

TreeNode* createTree(int airty, ...) {
	va_list ap;
	va_start(ap, airty);
	TreeNode *root = newNode();
	root->isToken = false;
	size_t i, len = 0;
	for (i = 0; i < airty; i++) {
		TreeNode *p = va_arg(ap, TreeNode*);
		if (i == 0) root->lineNo = p->lineNo;
		if (p != NULL) {
			treeAddChild(root, p);
			len += 1+strlen(p->text);
		}
	}
	ListHead *q;
	char *s = root->text = malloc(len);
	listForeach(q, &root->children) {
		TreeNode *p = listEntry(q, TreeNode, list);
		if (q != root->children.next) *(s++) = ' ';
		strcpy(s, p->text);
		s += strlen(s);
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
