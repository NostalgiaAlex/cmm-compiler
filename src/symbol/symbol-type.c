#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <symbol.h>
#include <stdio.h>

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
			while ((p != &a->structure)&&(q != &b->structure)) {
				Type * typeP = listEntry(p, Field, list)->type;
				Type * typeQ = listEntry(q, Field, list)->type;
				if (!typeEqual(typeP, typeQ)) return false;
				p = p->next; q = q->next;
			}
			return (p == &a->structure)&&(q == &b->structure);
	}
	return false;
}
bool argsEqual(ListHead* a, ListHead* b) {
	assert(a != NULL);
	assert(b != NULL);
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
bool funcEqual(Func* a, Func* b) {
	assert(a != NULL);
	assert(b != NULL);
	return (typeEqual(a->retType, b->retType) &&
			argsEqual(&a->args, &b->args));
}

void argsRelease(ListHead *args) {
	assert(args != NULL);
	while (!listIsEmpty(args)) {
		Arg* arg = listEntry(args->next, Arg, list);
		listDelete(&arg->list);
		if (arg->name != NULL) free(arg->name);
		free(arg);
	}
}
void funcRelease(Func *func) {
	assert(func != NULL);
	argsRelease(&func->args);
	free(func);
}
void typeRelease(Type *type) {
	assert(type != NULL);
	if (type->kind == ARRAY) {
		Type *baseType = type->array.elem;
		if (baseType->kind == ARRAY) typeRelease(baseType);
		free(type);
	} else if (type->kind == STRUCTURE) {
		ListHead *p;
		listForeach(p, &type->structure) {
			Field *field = listEntry(p, Field, list);
			typeRelease(field->type);
			free(field->name);
			free(field);
		}
		free(type);
	}
}
static void typeArrayToStr(Type* type, char* s) {
	assert(type != NULL);
	assert(s != NULL);
	if (type->kind == ARRAY) {
		sprintf(s, "[%d]", type->array.size);
		s += strlen(s);
		typeArrayToStr(type->array.elem, s);
	}
}
static Type* baseType(Type* type) {
	assert(type != NULL);
	if (type->kind != ARRAY) return type;
	return baseType(type->array.elem);
}
void typeToStr(Type* type, char* s) {
	assert(type != NULL);
	assert(s != NULL);
	if (typeEqual(type, TYPE_INT)) {
		strcpy(s, "int");
	} else if (typeEqual(type, TYPE_FLOAT)) {
		strcpy(s, "float");
	} else if (type->kind == ARRAY) {
		typeToStr(baseType(type), s);
		s += strlen(s);
		typeArrayToStr(type, s);
	} else if (type->kind == STRUCTURE) {
		strcpy(s, "struct");
	}
}
void argsToStr(ListHead* list, char* s) {
	assert(list != NULL);
	assert(s != NULL);
	ListHead *p;
	listForeach(p, list) {
		Arg *arg = listEntry(p, Arg, list);
		if (p != list->next) {
			strcpy(s, ", ");
			s += 2;
		}
		typeToStr(arg->type, s);
		s += strlen(s);
	}
}

Field* fieldFind(ListHead* structure, const char* fieldName) {
	assert(structure != NULL);
	assert(fieldName != NULL);
	ListHead *p;
	listForeach(p, structure) {
		Field* field = listEntry(p, Field, list);
		if (strcmp(field->name, fieldName) == 0)
			return field;
	}
	return NULL;
}