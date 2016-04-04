#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib/Tree.h"
#include "symbol.h"

typedef Field Dec;
static void decRelease(Dec* dec) {
	free(dec->name);
	free(dec);
}

static Type* analyseSpecifier(TreeNode*);
static void analyseExtDef(TreeNode*);
static void analyseExtDecList(TreeNode *, Type*);
static void analyseDefList(TreeNode*, ListHead*);
static void analyseDef(TreeNode*, ListHead*);
static void analyseDecList(TreeNode*, Type*, ListHead*);
static Dec* analyseVarDec(TreeNode*, Type*);
static void analyseFunDec(TreeNode*, Type*);
static void analyseVarList(TreeNode*, Func*);
static Arg* analyseParamDec(TreeNode*);
static void analyseCompSt(TreeNode* p) ;

void decListToSymbols(ListHead* list) {
	ListHead *p;
	listForeach(p, list) {
		Dec *dec = listEntry(p, Dec, list);
		Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
		symbol->name = toArray(dec->name);
		symbol->type = dec->type;
		symbolInsert(symbol);
	}
	while (!listIsEmpty(list)) {
		Dec *dec = listEntry(list->next, Dec, list);
		listDelete(&dec->list);
		decRelease(dec);
	}
}

static void analyseExtDefList(TreeNode* p) {
	assert(p != NULL);
	assert(isSyntax(p, ExtDefList));
	analyseExtDef(treeFirstChild(p));
	TreeNode *rest = treeLastChild(p);
	if (isSyntax(rest, ExtDefList))
		analyseExtDefList(rest);
}

static void analyseExtDef(TreeNode *p) {
	assert(p != NULL);
	assert(isSyntax(p, ExtDef));
	Type *type = analyseSpecifier(treeFirstChild(p));
	TreeNode *second = treeKthChild(p, 2);
	if (isSyntax(second, ExtDecList)) {
		analyseExtDecList(second, type);
	} else if (isSyntax(second, FunDec)) {
		analyseFunDec(second, type);
		analyseCompSt(treeKthChild(p, 3));
	}
}

static void analyseExtDecList(TreeNode *p, Type *type) {
	assert(p != NULL);
	assert(isSyntax(p, ExtDecList));
	TreeNode *extDec = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	Dec *varDec = analyseVarDec(treeFirstChild(extDec), type);
	Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
	symbol->name = toArray(varDec->name);
	symbol->kind = VAR;
	symbol->type = type;
	symbolInsert(symbol);
	if (isSyntax(rest, DecList))
		analyseExtDecList(rest, type);
}

static Type* analyseSpecifier(TreeNode* p) {
	assert(p != NULL);
	assert(isSyntax(p, Specifier));
	TreeNode* first = treeFirstChild(p);
	if (isSyntax(first, TYPE)) {
		if (strcmp(first->text, "int") == 0) return INT;
		else return FLOAT;
	} else {
		assert(isSyntax(first, StructSpecifier));
		assert(isSyntax(treeFirstChild(first), STRUCT));
		TreeNode *tag = treeKthChild(first, 2);
		if (isSyntax(tag, Tag)) {
			TreeNode *id = treeFirstChild(tag);
			assert(isSyntax(id, ID));
			Symbol *symbol = symbolFind(id->text);
			assert(symbol->kind == STRUCT);
			return symbol->type;
		} else {
			assert(isSyntax(tag, OptTag));
			Type *type = (Type*)malloc(sizeof(Type));
			type->kind = STRUCTURE;
			listInit(&type->structure);
			analyseDefList(treeKthChild(first, 4), &type->structure);
			if (!treeIsLeaf(tag)) {
				Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
				symbol->kind = STRUCT;
				symbol->type = type;
				symbolInsert(symbol);
			}
			return type;
		}
	}
	return NULL;
}

static void analyseDefList(TreeNode* p, ListHead* list) {
	assert(p != NULL);
	assert(isSyntax(p, DefList));
	analyseDef(treeFirstChild(p), list);
	TreeNode *rest = treeLastChild(p);
	if (isSyntax(rest, DefList))
		analyseDefList(rest, list);
}

static void analyseDef(TreeNode* p, ListHead* list) {
	assert(p != NULL);
	assert(isSyntax(p, Def));
	TreeNode *specifier = treeFirstChild(p);
	TreeNode *decList = treeKthChild(p, 2);
	Type *type = analyseSpecifier(specifier);
	analyseDecList(decList, type, list);
}

static void analyseDecList(TreeNode* p, Type* type, ListHead* list) {
	assert(p != NULL);
	assert(isSyntax(p, DecList));
	TreeNode *dec = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	Dec *varDec = analyseVarDec(treeFirstChild(dec), type);
	listAddBefore(list, &varDec->list);
	if (isSyntax(rest, DecList))
		analyseDecList(rest, type, list);
}

static Dec* analyseVarDec(TreeNode* p, Type* type) {
	assert(p != NULL);
	assert(isSyntax(p, VarDec));
	TreeNode* first = treeFirstChild(p);
	if (isSyntax(first, ID)) {
		Dec *dec = (Dec*)malloc(sizeof(Dec));
		dec->name = toArray(first->text);
		dec->type = type;
		return dec;
	} else {
		TreeNode *size = treeKthChild(p, 3);
		assert(isSyntax(first, VarDec));
		assert(isSyntax(size, INT));
		Type *newType = (Type*)malloc(sizeof(Type));
		newType->kind = ARRAY;
		newType->array.elem = type;
		newType->array.size = size->intVal;
		return analyseVarDec(first, newType);
	}
}

static void analyseFunDec(TreeNode* p, Type* type) {
	assert(p != NULL);
	assert(isSyntax(p, FunDec));
	Func *func = (Func*)malloc(sizeof(Func));
	func->retType = type;
	func->argc = 0;
	listInit(&func->args);
	TreeNode* varList = treeKthChild(p, 3);
	if (isSyntax(varList, VarList))
		analyseVarList(varList, func);
	Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
	symbol->kind = FUNC;
	symbol->func = func;
}

static void analyseVarList(TreeNode* p, Func* func) {
	assert(p != NULL);
	assert(isSyntax(p, VarList));
	Arg *arg = analyseParamDec(treeFirstChild(p));
	listAddBefore(&func->args, &arg->list);
	func->argc++;
	TreeNode *rest = treeLastChild(p);
	if (isSyntax(rest, VarList))
		analyseVarList(rest, func);
}

static Arg* analyseParamDec(TreeNode* p) {
	assert(p != NULL);
	assert(isSyntax(p, ParamDec));
	Type *type = analyseSpecifier(treeFirstChild(p));
	return analyseVarDec(treeLastChild(p), type);
}

static void analyseCompSt(TreeNode* p) {
	assert(p != NULL);
	assert(isSyntax(p, CompSt));
	symbolsStackPush();
	ListHead list;
	listInit(&list);
	analyseDefList(treeKthChild(p, 2), &list);
	decListToSymbols(&list);
	// TODO: analyseStmtList
	symbolsStackPop();
}

void analyseProgram(TreeNode* p) {
	assert(p != NULL);
	assert(isSyntax(p, Program));
	analyseExtDefList(treeFirstChild(p));
}
