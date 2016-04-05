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
				Type * typeP = listEntry(p, Field, list)->type;
				Type * typeQ = listEntry(q, Field, list)->type;
				if (!typeEqual(typeP, typeQ)) return false;
			}
			return (p == &a->structure)&&(q == &b->structure);
	}
	return false;
}

bool argsEqual(ListHead* a, ListHead* b) {
	ListHead *p = a->next, *q = b->next;
	while ((p != a) && (q != b)) {
		Arg *argP = listEntry(p, Arg, list);
		Arg *argQ = listEntry(q, Arg, list);
		if (!typeEqual(argP->type, argQ->type))
			return false;
		p = p->next; q = q->next;
	}
	return (p == a) && (q == b);
}