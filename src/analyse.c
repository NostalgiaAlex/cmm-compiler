#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "syntax-tree.h"
#include "symbol.h"
#define semanticError(errorNo, lineNo, ...) \
do { \
	errorStatus = 2; \
	printf("Error type %d at Line %d: ", (errorNo), (lineNo)); \
	printf(SEMANTIC_ERROR[(errorNo)-1], __VA_ARGS__);\
	puts(".");\
} while (0)
extern int errorStatus;
const char* SEMANTIC_ERROR[] = {
		"Undefined variable \"%s\"",
		"Undefined function \"%s\"",
		"Redefined variable \"%s\"",
		"Redefined function \"%s\"",
		"Type mismatched for assignment",
		"The left-hand side of an assignment must be a variable",
		"Type mismatched for operands",
		"Type mismatched for return",
		"Function \"%s(%s)\" is not applicable for arguments \"(%s)\"",
		"\"%s\" is not an array",
		"\"%s\" is not a function",
		"\"%s\" is not an integer",
		"Illegal use of \".\"",
		"Non-existent field \"%s\"",
		"Redefined or initialized field \"%s\"",
		"Duplicated name \"%s\"",
		"Undefined structure \"%s\"",
		"Undefined function \"%s\"",
		"Inconsistent declaration of function \"%s\"",
};

typedef Field Dec;
static Type* retType;
static Type* analyseSpecifier(TreeNode*);
static void analyseExtDef(TreeNode*);
static void analyseExtDecList(TreeNode *, Type*);
static void analyseDefList(TreeNode*, ListHead*);
static void analyseDef(TreeNode*, ListHead*);
static void analyseDecList(TreeNode*, Type*, ListHead*);
static void analyseDec(TreeNode*, Type*, ListHead*);
static Dec* analyseVarDec(TreeNode*, Type*);
static Func* analyseFunDec(TreeNode*, Type*, bool);
static void analyseVarList(TreeNode*, ListHead*);
static Arg* analyseParamDec(TreeNode*);
static void analyseStmtList(TreeNode*);
static void analyseStmt(TreeNode*);
static void analyseArgs(TreeNode*, ListHead*);
static void analyseCompSt(TreeNode*, Func*);

typedef struct Val {
	Type* type;
	bool isVar;
} Val;
static Val analyseExp(TreeNode*);

static void analyseExtDefList(TreeNode* p) {
	assert(isSyntax(p, ExtDefList));
	analyseExtDef(treeFirstChild(p));
	TreeNode *rest = treeLastChild(p);
	if (isSyntax(rest, ExtDefList))
		analyseExtDefList(rest);
}

static void analyseExtDef(TreeNode *p) {
	assert(isSyntax(p, ExtDef));
	Type *type = analyseSpecifier(treeFirstChild(p));
	TreeNode *second = treeKthChild(p, 2);
	TreeNode *last = treeLastChild(p);
	if (isSyntax(second, ExtDecList)) {
		analyseExtDecList(second, type);
	} else if (isSyntax(second, FunDec)) {
		bool isDef = isSyntax(last, CompSt);
		Func* func = analyseFunDec(second, type, isDef);
		if (!func) return;
		retType = func->retType;
		if (isDef) {
			analyseCompSt(last, func);
			func->defined = true;
		}
	}
}

static void analyseExtDecList(TreeNode *p, Type *type) {
	assert(isSyntax(p, ExtDecList));
	TreeNode *first = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	Dec *varDec = analyseVarDec(first, type);
	Symbol *symbol = newVarSymbol(varDec->name, type);
	if (!symbolInsert(symbol))
		semanticError(3, first->lineNo, symbol->name);
	if (isSyntax(rest, ExtDecList))
		analyseExtDecList(rest, type);
}

static Type* analyseSpecifier(TreeNode* p) {
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
			if ((symbol == NULL)||(symbol->kind != STRUCT)) {
				semanticError(17, id->lineNo, id->text);
				return TYPE_INT;
			}
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
				Symbol *symbol = newStructSymbol(id->text, type);
				if (!symbolInsert(symbol))
					semanticError(16, id->lineNo, id->text);
			}
			return type;
		}
	}
	return NULL;
}

static void analyseDefList(TreeNode* p, ListHead* list) {
	assert(isSyntax(p, DefList));
	analyseDef(treeFirstChild(p), list);
	TreeNode *rest = treeLastChild(p);
	if (isSyntax(rest, DefList))
		analyseDefList(rest, list);
}

static void analyseDef(TreeNode* p, ListHead* list) {
	assert(isSyntax(p, Def));
	TreeNode *specifier = treeFirstChild(p);
	TreeNode *decList = treeKthChild(p, 2);
	Type *type = analyseSpecifier(specifier);
	analyseDecList(decList, type, list);
}

static void analyseDecList(TreeNode* p, Type* type, ListHead* list) {
	assert(isSyntax(p, DecList));
	TreeNode *dec = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	analyseDec(dec, type, list);
	if (isSyntax(rest, DecList))
		analyseDecList(rest, type, list);
}

static void analyseDec(TreeNode* p, Type* type, ListHead* list) {
	assert(isSyntax(p, Dec));
	TreeNode *first = treeFirstChild(p);
	TreeNode *last = treeLastChild(p);
	Dec *varDec = analyseVarDec(first, type);
	if (list) {
		if (fieldFind(list, varDec->name) != NULL) {
			semanticError(15, p->lineNo, varDec->name);
		} else {
			listAddBefore(list, &varDec->list);
		}
		if (isSyntax(last, Exp))
			semanticError(15, p->lineNo, varDec->name);
	} else {
		Symbol *symbol = newVarSymbol(varDec->name, varDec->type);
		if (!symbolInsert(symbol)) {
			semanticError(3, p->lineNo, symbol->name);
		} else if (isSyntax(last, Exp)) {
			Val val = analyseExp(last);
			if ((val.type != NULL)&&(!typeEqual(val.type, symbol->type)))
				semanticError(5, treeKthChild(p, 2)->lineNo, "");
		}
		free(varDec->name);
		free(varDec);
	}
}

static Dec* analyseVarDec(TreeNode* p, Type* type) {
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

typedef struct FunSymbol {
	Symbol *symbol;
	int lineNo;
	ListHead list;
} FunSymbol;
ListHead funSymbols;
static Func* analyseFunDec(TreeNode* p, Type* type, bool isDef) {
	assert(isSyntax(p, FunDec));
	assert(type != NULL);
	Func *func = (Func*)malloc(sizeof(Func));
	func->retType = type;
	func->defined = false;
	listInit(&func->args);
	TreeNode* id = treeFirstChild(p);
	assert(isSyntax(id, ID));
	Symbol *symbol = symbolFind(id->text);
	if ((symbol != NULL)&&((symbol->kind != FUNC)||(isDef&&symbol->func->defined))) {
		semanticError(4, id->lineNo, symbol->name);
	} else {
		TreeNode* varList = treeKthChild(p, 3);
		if (symbol == NULL) {
			symbol = newFuncSymbol(id->text, func);
			if (symbolInsert(symbol)) {
				FunSymbol *funSymbol = (FunSymbol*)malloc(sizeof(FunSymbol));
				funSymbol->symbol = symbol;
				funSymbol->lineNo = p->lineNo;
				listAddBefore(&funSymbols, &funSymbol->list);
			}
		}
		if (isSyntax(varList, VarList))
			analyseVarList(varList, &func->args);
		if (funcEqual(symbol->func, func)) {
			if (symbol->func != func) {
				func->defined = symbol->func->defined;
				funcRelease(symbol->func);
				symbol->func = func;
			}
			return symbol->func;
		} else {
			semanticError(19, p->lineNo, symbol->name);
		}
	}
	funcRelease(func);
	return NULL;
}

static void analyseVarList(TreeNode* p, ListHead* list) {
	assert(isSyntax(p, VarList));
	Arg *arg = analyseParamDec(treeFirstChild(p));
	listAddBefore(list, &arg->list);
	TreeNode *rest = treeLastChild(p);
	if (isSyntax(rest, VarList))
		analyseVarList(rest, list);
}

static Arg* analyseParamDec(TreeNode* p) {
	assert(isSyntax(p, ParamDec));
	Type *type = analyseSpecifier(treeFirstChild(p));
	return analyseVarDec(treeLastChild(p), type);
}

static void analyseCompSt(TreeNode* p, Func* func) {
	assert(isSyntax(p, CompSt));
	symbolsStackPush();
	ListHead *q;
	if (func != NULL) {
		listForeach(q, &func->args) {
			Arg *arg = listEntry(q, Arg, list);
			Symbol *symbol = newVarSymbol(arg->name, arg->type);
			symbolInsert(symbol);
		}
	}
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
	assert(isSyntax(p, StmtList));
	TreeNode *rest = treeLastChild(p);
	analyseStmt(treeFirstChild(p));
	if (isSyntax(rest, StmtList))
		analyseStmtList(rest);
}

static Val requireType(TreeNode *, Type *, int);
static void analyseStmt(TreeNode* p) {
	assert(isSyntax(p, Stmt));
	TreeNode *first = treeFirstChild(p);
	if (isSyntax(first, RETURN)) {
		Type *type = analyseExp(treeKthChild(p, 2)).type;
		if ((type != NULL)&&(!typeEqual(type, retType)))
			semanticError(8, p->lineNo, "");
	} else if (isSyntax(first, Exp)) {
		analyseExp(first);
	} else {
		ListHead *q;
		listForeach(q, &p->children) {
			TreeNode *r = listEntry(q, TreeNode, list);
			if (isSyntax(r, Stmt)) analyseStmt(r);
			else if (isSyntax(r, CompSt)) analyseCompSt(r, NULL);
			else if (isSyntax(r, Exp)) {
				requireType(r, TYPE_INT, 7);
			}
		}
	}
}

static Val makeVar(Type* type) {
	Val val;
	val.type = type;
	val.isVar = true;
	return val;
}
static Val makeVal(Type* type) {
	Val val;
	val.type = type;
	val.isVar = false;
	return val;
}
static Val requireBasic(TreeNode* p, int errorNo) {
	assert(isSyntax(p, Exp));
	Val val = analyseExp(p);
	if ((val.type != NULL)&&(val.type->kind != BASIC)) {
		semanticError(errorNo, p->lineNo, p->text);
	}
	return val;
}
static Val requireType(TreeNode *p, Type *type, int errorNo) {
	assert(isSyntax(p, Exp));
	Val val = analyseExp(p);
	if ((val.type != NULL)&&(!typeEqual(val.type, type))) {
		semanticError(errorNo, p->lineNo, p->text);
	}
	return val;
}
#define check(val) do { if (val.type == NULL) return makeVal(NULL); } while (0)
static Val analyseExp(TreeNode* p) {
	assert(isSyntax(p, Exp));
	TreeNode *first = treeFirstChild(p);
	TreeNode *second = treeKthChild(p, 2);
	TreeNode *last = treeLastChild(p);
	if (isSyntax(first, ID)) {
		if (isSyntax(last, RP)) { // ID LP Args RP | ID LP RP
			TreeNode *id = first;
			assert(isSyntax(id, ID));
			Symbol *symbol = symbolFind(id->text);
			if (!symbol) {
				semanticError(2, id->lineNo, id->text);
			} else if (symbol->kind != FUNC) {
				semanticError(11, id->lineNo, id->text);
			} else {
				ListHead list;
				listInit(&list);
				TreeNode *args = treeKthChild(p, 3);
				if (isSyntax(args, Args))
					analyseArgs(args, &list);
				if (!argsEqual(&list, &symbol->func->args)) {
					char paramsStr[32], argsStr[32];
					argsToStr(&symbol->func->args, paramsStr);
					argsToStr(&list, argsStr);
					semanticError(9, id->lineNo, symbol->name, paramsStr, argsStr);
				}
				argsRelease(&list);
				return makeVal(symbol->func->retType);
			}
		} else { // ID
			Symbol *symbol = symbolFind(first->text);
			if (!symbol) {
				semanticError(1, first->lineNo, first->text);
			} else {
				return makeVar(symbol->type);
			}
		}
	} else if (isSyntax(first, INT)) {
		return makeVal(TYPE_INT);
	} else if (isSyntax(first, FLOAT)) {
		return makeVal(TYPE_FLOAT);
	} else if (isSyntax(last, RB)) { // EXP LB EXP RB
		TreeNode *third = treeKthChild(p, 3);
		Val base = analyseExp(first);
		Val index = requireType(third, TYPE_INT, 12);
		check(base);
		check(index);
		if (base.type->kind != ARRAY) {
			semanticError(10, first->lineNo, first->text);
		} else {
			base.type = base.type->array.elem;
			return base;
		}
	} else if (isSyntax(last, ID)) { // EXP DOT ID
		assert(isSyntax(second, DOT));
		Val base = analyseExp(first);
		check(base);
		char *fieldName = last->text;
		if (base.type->kind != STRUCTURE) {
			semanticError(13, second->lineNo, "");
		} else {
			Field *field = fieldFind(&base.type->structure, fieldName);
			if (field == NULL) {
				semanticError(14, last->lineNo, fieldName);
			} else {
				base.type = field->type;
				return base;
			}
		}
	} else if (isSyntax(second, ASSIGNOP)) {
		Val left = analyseExp(first);
		check(left);
		if (!left.isVar) {
			semanticError(6, first->lineNo, "");
		} else {
			Val right = requireType(last, left.type, 5);
			check(right);
			return left;
		}
	} else if (isSyntax(first, LP) && isSyntax(last, RP)) {
		return analyseExp(second);
	} else if (last == second) {
		Val val;
		if (isSyntax(first, NOT)) {
			val = requireType(second, TYPE_INT, 7);
		} else {
			assert(isSyntax(first, MINUS));
			val = requireBasic(second, 7);
		}
		check(val);
		return val;
	} else if (isSyntax(second, AND)||isSyntax(second, OR)) {
		Val left = requireType(first, TYPE_INT, 7);
		Val right = requireType(last, TYPE_INT, 7);
		check(left);
		check(right);
		return makeVal(TYPE_INT);
	} else {
		Val left = requireBasic(first, 7);
		check(left);
		Val right = requireType(last, left.type, 7);
		check(right);
		return makeVal(left.type);
	}
	return makeVal(NULL);
}

static void analyseArgs(TreeNode* p, ListHead* list) {
	assert(list != NULL);
	assert(isSyntax(p, Args));
	TreeNode *exp = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	Arg *arg = (Arg*)malloc(sizeof(Arg));
	arg->type = analyseExp(exp).type;
	arg->name = NULL;
	listAddBefore(list, &arg->list);
	if (isSyntax(rest, Args))
		analyseArgs(rest, list);
}

void analyseProgram(TreeNode* p) {
	assert(isSyntax(p, Program));
	listInit(&funSymbols);
	TreeNode *extDefList = treeFirstChild(p);
	analyseExtDefList(extDefList);
	while (!listIsEmpty(&funSymbols)) {
		FunSymbol *funSymbol = listEntry(funSymbols.next, FunSymbol, list);
		assert(funSymbol->symbol->kind == FUNC);
		if (!funSymbol->symbol->func->defined)
			semanticError(18, funSymbol->lineNo, funSymbol->symbol->name);
		listDelete(funSymbols.next);
		free(funSymbol);
	}
}
