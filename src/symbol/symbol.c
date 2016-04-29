#include <stdlib.h>
#include "common.h"
#include "symbol.h"

static Symbol* newSymbol(SymbolKind kind, const char * name) {
	Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
	symbol->kind = kind;
	symbol->name = toArray(name);
	return symbol;
}

Symbol* newVarSymbol(const char* name, Type* type) {
	Symbol *symbol = newSymbol(VAR, name);
	symbol->type = type;
	return symbol;
}

Symbol* newFuncSymbol(const char* name, Func* func) {
	Symbol *symbol = newSymbol(FUNC, name);
	symbol->func = func;
	return symbol;
}

Symbol* newStructSymbol(const char* name, Type* type) {
	Symbol *symbol = newSymbol(STRUCT, name);
	symbol->type = type;
	return symbol;
}

Func* newFunc(Type* retType) {
	Func *func = (Func*)malloc(sizeof(Func));
	listInit(&func->args);
	func->retType = retType;
	return func;
}