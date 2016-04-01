#include <string.h>
#include <stdlib.h>
#include "lib/List.h"
#include "symbol.h"
#include "stdio.h"

#define MASK     0x3FFF
#define SIZE     (MASK+1)

typedef struct SymbolNode {
	Symbol *symbol;
	ListHead list, depth;
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
}

static unsigned hashPJW(const char* name) {
	unsigned val = 0;
	for (; *name; name++) {
		val = (val<<2)+*name;
		unsigned i = val&~MASK;
		if (i) val = (val^(i>>12))&MASK;
	}
	return val;
}

void symbolsStackPush() {
	stackTop++;
}
void symbolsStackPop() {
	ListHead* top = stack+stackTop;
	while (!listIsEmpty(top)) {
		SymbolNode *p = listEntry(top->next, SymbolNode, depth);
		listDelete(&p->list);
		listDelete(&p->depth);
		free(p);
	}
	stackTop--;
}

void symbolInsert(Symbol* symbol) {
	SymbolNode *p = (SymbolNode*)malloc(sizeof(SymbolNode));
	p->symbol = symbol;
	listInit(&p->list);
	listInit(&p->depth);
	unsigned hashVal = hashPJW(symbol->name);
	listAddBefore(symbolTable+hashVal, &p->list);
	listAddBefore(stack+stackTop, &p->depth);
}
Symbol* symbolFind(const char* name) {
	unsigned hash = hashPJW(name);
	ListHead *p;
	listForeach(p, symbolTable+hash) {
		Symbol* symbol = listEntry(p, SymbolNode, list)->symbol;
		if (strcmp(symbol->name, name) == 0)
			return symbol;
	}
	return NULL;
}
