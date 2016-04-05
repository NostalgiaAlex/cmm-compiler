#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib/Tree.h"
#include "symbol.h"
#define semanticError(errorNo, lineNo, ...) \
do { \
	printf("Error type %d at Line %d: ", (errorNo), (lineNo)); \
	printf(str[errorNo], __VA_ARGS__);\
	puts(".");\
} while (0)
const char* str[] = {
		"",
		"Undefined variable \"%s\"",
		"Undefined function \"%s\"",
		"Redefined variable \"%s\"",
		"Redefined function \"%s\"",
		"",
		"",
		"",
		"",
		"",
		"",
		"\"%s\" is not a function",
		"",
};

typedef Field Dec;

static Type* analyseSpecifier(TreeNode*);
static void analyseExtDecList(TreeNode *, Type*);
static void analyseExtDef(TreeNode*);
static void analyseDefList(TreeNode*, ListHead*);
static void analyseDef(TreeNode*, ListHead*);
static void analyseDecList(TreeNode*, Type*, ListHead*);
static Dec* analyseVarDec(TreeNode*, Type*);
static void analyseFunDec(TreeNode*, Type*);
static void analyseVarList(TreeNode*, Func*);
static Arg* analyseParamDec(TreeNode*);
static void analyseStmtList(TreeNode*);
static void analyseStmt(TreeNode*);
static Type* analyseExp(TreeNode*);

static void analyseCompSt(TreeNode* p) ;

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
	if (!symbolInsert(symbol))
		semanticError(3, extDec->lineNo, symbol->name);
	if (isSyntax(rest, DecList))
		analyseExtDecList(rest, type);
}

static Type* analyseSpecifier(TreeNode* p) {
	assert(p != NULL);
	assert(isSyntax(p, Specifier));
	TreeNode* first = treeFirstChild(p);
	if (isSyntax(first, TYPE)) {
		if (strcmp(first->text, "int") == 0) return TYPE_INT;
		else return TYPE_FLOAT;
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
			int defListIndex = (isSyntax(tag, OptTag))? 4: 3;
			Type *type = (Type*)malloc(sizeof(Type));
			type->kind = STRUCTURE;
			listInit(&type->structure);
			analyseDefList(treeKthChild(first, defListIndex), &type->structure);
			if (defListIndex == 4) {
				TreeNode* id = treeFirstChild(tag);
				assert(isSyntax(id, ID));
				Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
				symbol->name = toArray(id->text);
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
	if (list) {
		listAddBefore(list, &varDec->list);
		if (isSyntax(rest, DecList))
			analyseDecList(rest, type, list);
	} else {
		Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
		symbol->name = varDec->name;
		symbol->kind = VAR;
		symbol->type = varDec->type;
		if (!symbolInsert(symbol))
			semanticError(3, p->lineNo, symbol->name);
		free(varDec);
	}
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
	TreeNode* id = treeFirstChild(p);
	assert(isSyntax(id, ID));
	TreeNode* varList = treeKthChild(p, 3);
	if (isSyntax(varList, VarList))
		analyseVarList(varList, func);
	Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
	symbol->name = toArray(id->text);
	symbol->kind = FUNC;
	symbol->func = func;
	if (!symbolInsert(symbol))
		semanticError(4, id->lineNo, symbol->name);
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
	TreeNode *next = treeKthChild(p, 2);
	if (isSyntax(next, DefList)) {
		analyseDefList(next, NULL);
		next = treeKthChild(p, 3);
	}
	if (isSyntax(next, StmtList))
		analyseStmtList(next);
	symbolsStackPop();
}

static void analyseStmtList(TreeNode* p) {
	assert(p != NULL);
	assert(isSyntax(p, StmtList));
	TreeNode *rest = treeLastChild(p);
	analyseStmt(treeFirstChild(p));
	if (isSyntax(rest, StmtList))
		analyseStmtList(rest);
}

static void analyseStmt(TreeNode* p) {
	assert(p != NULL);
	assert(isSyntax(p, Stmt));
	ListHead *q;
	listForeach(q, &p->children) {
		TreeNode *r = listEntry(q, TreeNode, list);
		if (isSyntax(r, Stmt)) analyseStmt(r);
		else if (isSyntax(r, CompSt)) analyseCompSt(r);
		else if (isSyntax(r, Exp)) analyseExp(r);
	}
}

static Type* analyseExp(TreeNode* p) {
	assert(p != NULL);
	assert(isSyntax(p, Exp));
	if (isSyntax(treeLastChild(p), RP)) {
		TreeNode *id = treeFirstChild(p);
		assert(isSyntax(id, ID));
		Symbol *symbol = symbolFind(id->text);
		if (!symbol) {
			semanticError(2, id->lineNo, id->text);
		} else if (symbol->kind != FUNC) {
			semanticError(11, id->lineNo, id->text);
		} else {
			return symbol->func->retType;
		}
	} else {
		ListHead *q;
		listForeach(q, &p->children) {
			TreeNode *r = listEntry(q, TreeNode, list);
			if (isSyntax(r, Exp)) analyseExp(r);
			else if (isSyntax(r, ID)) {
				Symbol* symbol = symbolFind(r->text);
				if (!symbol) {
					semanticError(1, r->lineNo, r->text);
				} else {
					return symbol->type;
				}
			}
		}
	}
	return NULL;
}

void analyseProgram(TreeNode* p) {
	assert(p != NULL);
	assert(isSyntax(p, Program));
	analyseExtDefList(treeFirstChild(p));
}
