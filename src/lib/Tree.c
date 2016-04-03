#include <assert.h>
#include "lib/Tree.h"
#include "lib/List.h"

TreeNode* newNode() {
	TreeNode* p = (TreeNode*)malloc(sizeof(TreeNode));
	listInit(&p->list);
	listInit(&p->children);
	return p;
}

void treeAddChild(TreeNode* root, TreeNode* p) {
	assert(root != NULL);
	assert(p != NULL);
	listAddBefore(&root->children, &p->list);
}

bool treeIsLeaf(TreeNode* p) {
	assert(p != NULL);
	return listIsEmpty(&p->children);
}

TreeNode* treeFirstChild(TreeNode* p) {
	assert(p != NULL);
	assert(!listIsEmpty(&p->children));
	return listEntry(p->children.next, TreeNode, list);
}

TreeNode* treeLastChild(TreeNode* p) {
	assert(p != NULL);
	assert(!listIsEmpty(&p->children));
	return listEntry(p->children.prev, TreeNode, list);
}

TreeNode* treeKthChild(TreeNode* root, int k) {
	assert(root != NULL);
	assert(!listIsEmpty(&root->children));
	ListHead *p = &root->children;
	for (; k > 0; k--) p = p->next;
	return listEntry(p, TreeNode, list);
}
