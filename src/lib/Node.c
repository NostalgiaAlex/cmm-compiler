#include "common.h"
#include "lib/Node.h"
#include <assert.h>
#include <stdarg.h>

Node* newNode() {
	Node* p = (Node*)malloc(sizeof(Node));
	listInit(&p->list);
	listInit(&p->children);
	return p;
}

Node* createNode(int arity, ...) {
	Node *root = newNode();
	va_list ap;
	int i;
	va_start(ap, arity); 
	for (i = 0; i < arity; i++) {
		Node* p = va_arg(ap, Node*);
		if (p != NULL)
			listAddBefore(&root->children, &p->list);
	}
	va_end(ap);
	return root;
}
