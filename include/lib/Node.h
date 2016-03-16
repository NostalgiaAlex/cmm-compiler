#ifndef __LIB_NODE_H__
#define __LIB_NODE_H__

#include "lib/List.h"

typedef struct Node {
	int lineNo, token, intVal;
	float floatVal;
	char* text;
	ListHead list, children;
} Node;

Node* newNode();
Node* createNode(int, ...);

#endif
