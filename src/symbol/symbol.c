#include <assert.h>
#include <stdlib.h>
#include "lib/Tree.h"
#include "common.h"
#include "symbol.h"

void symbolRelease(Symbol* symbol) {
	free(symbol->name);
	free(symbol);
}

typedef Field Dec;
static Type* analyseSpecifier(TreeNode*);
static void analyseDefList(TreeNode*, ListHead*);
static void analyseDef(TreeNode*, ListHead*);
static void analyseDecList(TreeNode*, Type*, ListHead*);
static Dec* analyseVarDec(TreeNode*, Type*);
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
		free(dec);
	}
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
			type->kind = STRUCT;
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

static void analyseDefList(TreeNode *p, ListHead* list) {
	assert(p != NULL);
	assert(isSyntax(p, DefList));
	if (treeIsLeaf(p)) return;
	analyseDef(treeFirstChild(p), list);
	analyseDefList(treeLastChild(p), list);
}

static void analyseDef(TreeNode* p, ListHead* list) {
	assert(p != NULL);
	assert(isSyntax(p, Def));
	TreeNode *specifier = treeFirstChild(p);
	TreeNode *decList = treeKthChild(p, 2);
	Type *type = analyseSpecifier(specifier);
	analyseDecList(decList, type, list);
}

static void analyseDecList(TreeNode *p, Type* type, ListHead* list) {
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

