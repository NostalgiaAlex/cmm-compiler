#include "lib/Tree.h"
#include "lib/List.h"

TreeNode* newNode() {
	TreeNode* p = (TreeNode*)malloc(sizeof(TreeNode));
	listInit(&p->list);
	listInit(&p->children);
	return p;
}

void treeAddChild(TreeNode* root, TreeNode* p) {
	listAddBefore(&root->children, &p->list);
}
