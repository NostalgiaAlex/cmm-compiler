#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "lib/list.h"
#include "symbol.h"

#define MASK     0x3FFF
#define SIZE     (MASK+1)

typedef struct SymbolNode {
	Symbol *symbol;
	ListHead list, stack;
} SymbolNode;

static ListHead symbolTable[SIZE], stack[SIZE];
unsigned stackTop;

void symbolTableInit() {
	int i;
	stackTop = 0;
	for (i = 0; i < SIZE; i++) {
		listInit(symbolTable+i);
		listInit(stack+i);
	}
	Func *readFunc = newFunc(TYPE_INT);
	Symbol *read = newFuncSymbol("read", readFunc);
	symbolInsert(read);
	Func *writeFunc = newFunc(TYPE_INT);
	Arg *param = (Arg*)malloc(sizeof(Arg));
	param->type = TYPE_INT;
	listAddBefore(&writeFunc->args, &param->list);
	Symbol *write = newFuncSymbol("write", writeFunc);
	symbolInsert(write);
}

static unsigned hashPJW(const char* name) {
	assert(name != NULL);
	unsigned val = 0;
	for (; *name; name++) {
		val = (val<<2)+*name;
		unsigned i = val&~MASK;
		if (i) val = (val^(i>>12))&MASK;
	}
	return val;
}

void symbolRelease(Symbol *symbol) {
	assert(symbol!= NULL);
	SymbolKind kind = symbol->kind;
	Type *type = symbol->type;
	if (((kind == STRUCT)&&(type->kind == ARRAY))||(kind == STRUCT)) {
		typeRelease(type);
	}
	free(symbol->name);
	free(symbol);
}
void symbolsStackPush() {
	stackTop++;
}
void symbolsStackPop() {
	assert(stackTop >= 0);
	ListHead* top = stack+stackTop;
	while (!listIsEmpty(top)) {
		SymbolNode *p = listEntry(top->next, SymbolNode, stack);
		listDelete(&p->list);
		listDelete(&p->stack);
		symbolRelease(p->symbol);
		free(p);
	}
	stackTop--;
}
bool symbolAtStackTop(const char* name) {
	Symbol *symbol = symbolFind(name);
	return (symbol != NULL)&& (symbol->depth == stackTop);
}

bool symbolInsert(Symbol* symbol) {
	assert(symbol != NULL);
	assert(symbol->name != NULL);
	if (symbolAtStackTop(symbol->name)) return false;
	SymbolNode *p = (SymbolNode*)malloc(sizeof(SymbolNode));
	symbol->depth = stackTop;
	p->symbol = symbol;
	listInit(&p->list);
	listInit(&p->stack);
	unsigned hashVal = hashPJW(symbol->name);
	listAddAfter(symbolTable+hashVal, &p->list);
	listAddBefore(stack+stackTop, &p->stack);
	return true;
}
Symbol* symbolFind(const char* name) {
	assert(name != NULL);
	unsigned hash = hashPJW(name);
	ListHead *p;
	listForeach(p, symbolTable+hash) {
		Symbol* symbol = listEntry(p, SymbolNode, list)->symbol;
		if (strcmp(symbol->name, name) == 0)
			return symbol;
	}
	return NULL;
}
