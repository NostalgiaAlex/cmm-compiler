#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "lib/Tree.h"
#include "symbol.h"

Type *TYPE_INT, *TYPE_FLOAT;
void typesInit() {
	TYPE_INT = (Type*)malloc(sizeof(Type));
	TYPE_INT->basic = 0;
	TYPE_FLOAT = (Type*)malloc(sizeof(Type));
	TYPE_FLOAT->basic = 1;
}

bool typeEqual(Type* a, Type* b) {
	assert(a != NULL);
	assert(b != NULL);
	if (a == b) return true;
	if (a->kind != b->kind) return false;
	ListHead *p, *q;
	switch (a->kind) {
		case BASIC:
			return (a->basic == b->basic);
		case ARRAY:
			return typeEqual(a->array.elem, b->array.elem)
				&& (a->array.size == b->array.size);
		case STRUCTURE:
			p = a->structure.next;
			q = b->structure.next;
			while ((p != &a->structure)&&(q != &a->structure)) {
				if (!typeEqual(listEntry(p, Field, list)->type,
							listEntry(q, Field, list)->type)) return false;
			}
			return (p == &a->structure)&&(q == &b->structure);
	}
	return false;
}
