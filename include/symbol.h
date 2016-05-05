#ifndef COMPILER_SYMBOL_H
#define COMPILER_SYMBOL_H
#include <stdbool.h>
#include "lib/List.h"
#include "syntax-tree.h"
#include "inter-code.h"

typedef enum { BASIC, ARRAY, STRUCTURE } TypeKind;
typedef enum { VAR, STRUCT, FUNC } SymbolKind;
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
	ListHead args;
	bool defined;
} Func;
typedef Field Arg;

extern Type* const TYPE_INT;
extern Type* const TYPE_FLOAT;
void typesInit();
bool typeEqual(Type*, Type*);
bool argsEqual(ListHead*, ListHead*);
bool funcEqual(Func*, Func*);
void argsRelease(ListHead *args);
void funcRelease(Func *);
void typeRelease(Type *);
void typeToStr(Type*, char*);
void argsToStr(ListHead*, char*);
Field* fieldFind(ListHead*, const char*);
int fieldOffset(ListHead*, const char*);
int typeSize(Type *);

typedef struct Symbol {
	char* name;
	SymbolKind kind;
	union {
		Type* type;
		Func* func;
	};
	int id;
	int depth;
} Symbol;
Symbol* newVarSymbol(const char*, Type*);
Symbol* newStructSymbol(const char*, Type*);
Symbol* newFuncSymbol(const char*, Func*);
Func* newFunc(Type*);
Operand* symbolGetOperand(Symbol*);

void symbolRelease(Symbol*);
void symbolTableInit();
void symbolsStackPush();
void symbolsStackPop();
bool symbolAtStackTop(const char*);

bool symbolInsert(Symbol*);
Symbol* symbolFind(const char*);

void analyseProgram(TreeNode*);

#endif
