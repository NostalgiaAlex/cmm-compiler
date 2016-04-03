#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__
#include "lib/List.h"
#include "lib/Tree.h"

typedef struct Type {
	enum { BASIC, ARRAY, STRUCTURE } kind;
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

extern Type *INT, *FLOAT;
void typesInit();
bool typeEqual(Type*, Type*);

typedef struct Symbol {
	char* name;
	enum { VAR, STRUCT, FUNC } kind;
	union {
		Type* type;
		Func* func;
	};
	int depth;
} Symbol;
void symbolRelease(Symbol*);
void symbolGenerate(TreeNode* p);

void symbolsInit();
void symbolsStackPush();
void symbolsStackPop();

void symbolInsert(Symbol*);
Symbol* symbolFind(const char*);

#endif
