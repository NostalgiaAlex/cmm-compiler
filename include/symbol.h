#ifndef __SYMBOL_H__
#define __SYMBOL_H__
#include <stdbool.h>
#include "lib/List.h"
#include "lib/Tree.h"

typedef enum { BASIC, ARRAY, STRUCTURE } TypeKind;
typedef enum { VAR, STRUCT, FUNC} SymbolKind;
typedef struct Type {
	TypeKind kind;
	union {
		int basic;
		struct { struct Type *elem; int size; } array;
		ListHead structure;
	};
} Type;
typedef struct Field {
	char* name;
	Type* type;
	ListHead list;
} Field;
typedef struct Func {
	Type* retType;
	int argc;
	ListHead args;
} Func;
typedef Field Arg;

extern Type *TYPE_INT, *TYPE_FLOAT;
void typesInit();
bool typeEqual(Type*, Type*);

typedef struct Symbol {
	char* name;
	SymbolKind kind;
	union {
		Type* type;
		Func* func;
	};
	int depth;
} Symbol;

void symbolTableInit();
void symbolsStackPush();
void symbolsStackPop();
bool symbolAtStackTop(const char*);

bool symbolInsert(Symbol*);
Symbol* symbolFind(const char*);

#endif
