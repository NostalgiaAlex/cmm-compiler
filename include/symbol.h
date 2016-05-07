#ifndef COMPILER_SYMBOL_H
#define COMPILER_SYMBOL_H
#include <stdbool.h>
#include "lib/List.h"
#include "syntax-tree.h"
#include "inter-code.h"

typedef enum { BASIC, ARRAY, STRUCTURE } TypeKind;
typedef enum { VAR, STRUCT, FUNC } SymbolKind;
typedef ListHead Fields;
typedef struct Type {
	TypeKind kind;
	union {
		int basic;
		struct { struct Type *elem; int size; } array;
		Fields structure;
	};
} Type;
typedef struct Field {
	char* name;
	Type* type;
	ListHead list;
} Field;
typedef Fields Args;
typedef Field Arg;
typedef struct Func {
	Type* retType;
	Args args;
	bool defined;
} Func;

extern Type* const TYPE_INT;
extern Type* const TYPE_FLOAT;
void typesInit();
bool typeEqual(Type*, Type*);
bool argsEqual(Args*, Args*);
bool funcEqual(Func*, Func*);
void argsRelease(Args*);
void funcRelease(Func *);
void typeRelease(Type *);
void typeToStr(Type*, char*);
void argsToStr(Args*, char*);
Field* fieldFind(Fields*, const char*);
int fieldOffset(Fields*, const char*);
int typeSize(Type *);

typedef struct Symbol {
	char* name;
	SymbolKind kind;
	union {
		Type* type;
		Func* func;
	};
	int id;
	bool isRef;
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
