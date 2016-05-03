#include <assert.h>
#include "syntax-tree.h"
#include "translate.h"

static InterCodes head;

#define SIZE 1005
static ListHead stack[SIZE];
static int top;
typedef struct ListNode {
	ListHead list;
	InterCodes *head;
} ListNode;

void interCodesInit() {
	listInit(&head);
	listInit(stack);
	top = 0;
}

void interCodeStackPush() {
	top++;
	assert(top < SIZE);
	listInit(stack+top);
}

void interCodeStackPop() {
	assert(top > 0);
	top--;
}

void interCodeStackInsert(InterCodes *head) {
	ListNode *listNode = (ListNode*)malloc(sizeof(ListNode));
	listAddBefore(stack+top, &listNode->list);
	listNode->head = head;
}

InterCodes* interCodeStackGet() {
	assert(!listIsEmpty(&stack[top]));
	ListHead *p = stack[top].next;
	ListNode *listNode = listEntry(p, ListNode, list);
	listDelete(p);
	InterCodes *head = listNode->head;
	free(listNode);
	return head;
}


void defineFunc(char *name, InterCodes *irs) {
	assert(name != NULL);
	InterCode* ir = newInterCode(DEF_FUNCTION);
	ir->res = newFunctionOperand(name);
	interCodeInsert(&head, ir);
	interCodesBind(&head, irs);
}

InterCodes* interCodesGet() {
	return &head;
}

static void translateStmtList(TreeNode*, InterCodes*);
static void translateArgs(TreeNode*, InterCodes*, ListHead*);
static InterCodes* translateStmt(TreeNode*);

typedef struct OperandNode {
	Operand *op;
	ListHead list;
} OperandNode;

InterCodes* translateExp(TreeNode *p, Operand *res) {
	assert(isSyntax(p, Exp));
	InterCodes *irs = newInterCodes();
	TreeNode *first = treeFirstChild(p);
	TreeNode *second = treeKthChild(p, 2);
	TreeNode *last = treeLastChild(p);
	if (isSyntax(first, ID)) {
		if (isSyntax(last, RP)) { // ID LP Args RP | ID LP RP
			TreeNode *argsNode = treeKthChild(p, 3);
			ListHead args;
			listInit(&args);
			if (isSyntax(argsNode, Args))
				translateArgs(argsNode, irs, &args);
			if (strcmp(first->text, "read") == 0) {
				InterCode *ir = newInterCode1(READ, res);
				interCodeInsert(irs, ir);
			} else if (strcmp(first->text, "write") == 0) {
				Operand *op = listEntry(args.next, OperandNode, list)->op;
				InterCode *ir = newInterCode1(WRITE, op);
				interCodeInsert(irs, ir);
			} else {
				ListHead *q;
				listForeach(q, &args) {
					Operand *op = listEntry(q, OperandNode, list)->op;
					InterCode *ir = newInterCode1(ARG, op);
					interCodeInsert(irs, ir);
				}
				Operand *op = newFunctionOperand(first->text);
				InterCode *ir = newInterCode2(CALL, res, op);
				interCodeInsert(irs, ir);
			}
			while (!listIsEmpty(&args)) {
				ListHead *q = args.next;
				OperandNode *operandNode = listEntry(q, OperandNode, list);
				listDelete(q);
				free(operandNode);
			}
		} else { // ID
			if (res == NULL) return irs;
			Symbol *symbol = symbolFind(first->text);
			if (symbol->id < 0)
				symbol->id = newVarOperandId();
			*res = *varOperand(symbol->id);
		}
	} else if (isSyntax(first, INT)) {
		if (res == NULL) return irs;
		*res = *constOperand(first->intVal);
	} else {
		Operand *op1 = newTempOperand();
		Operand *op2 = newTempOperand();
		InterCodes* irs1 = translateExp(first, op1);
		InterCodes* irs2 = translateExp(last, op2);
		interCodesBind(irs, irs1);
		interCodesBind(irs, irs2);
		InterCodeKind kind = ADD;
		if (isSyntax(second, ASSIGNOP)) {
			InterCode *ir = newInterCode2(ASSIGN, op1, op2);
			interCodeInsert(irs, ir);
			if (res == NULL) return irs;
			ir = newInterCode2(ASSIGN, res, op1);
			interCodeInsert(irs, ir);
		} else {
			if (res == NULL) return irs;
			if (isSyntax(second, PLUS)) {
				kind = ADD;
			} else if (isSyntax(second, MINUS)) {
				kind = SUB;
			} else if (isSyntax(second, STAR)) {
				kind = MUL;
			} else if (isSyntax(second, DIV)) {
				kind = DIV;
			}
			InterCode *ir = newInterCode3(kind, res, op1, op2);
			interCodeInsert(irs, ir);
		}
	}
	return irs;
}

void translateArgs(TreeNode *p, InterCodes *irs, ListHead *args) {
	assert(isSyntax(p, Args));
	TreeNode *first = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	Operand *op = newTempOperand();
	InterCodes *expIRs = translateExp(first, op);
	OperandNode *operandNode = (OperandNode*)malloc(sizeof(OperandNode));
	operandNode->op = op;
	listAddAfter(args, &operandNode->list);
	interCodesBind(irs, expIRs);
	if (isSyntax(rest, Args))
		translateArgs(rest, irs, args);
}

InterCodes* translateCompSt(TreeNode *p, Func* func) {
	assert(isSyntax(p, CompSt));
	InterCodes *irs = newInterCodes();
	if (func) {
		ListHead *q;
		listForeach(q, &func->args) {
			char *name = listEntry(q, Arg, list)->name;
			InterCode *ir = newInterCode(PARAM);
			ir->res = newVarOperand();
			interCodeInsert(irs, ir);
			symbolFind(name)->id = ir->res->id;
		}
	}

//	TreeNode *defList = treeKthChild(p, 2);
	TreeNode *stmtList = treeLastKthChild(p, 2);
//	if (isSyntax(defList, DefList))
//		analyseDefList(defList, NULL);
	if (isSyntax(stmtList, StmtList))
		translateStmtList(stmtList, irs);
	return irs;
}

static void translateStmtList(TreeNode *p, InterCodes *irs) {
	assert(isSyntax(p, StmtList));
	TreeNode *first = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	InterCodes *firstIRs = translateStmt(first);
	interCodesBind(irs, firstIRs);
	if (isSyntax(rest, StmtList))
		translateStmtList(rest, irs);
}

static InterCodes* translateStmt(TreeNode *p) {
	assert(isSyntax(p, Stmt));
	TreeNode *first = treeFirstChild(p);
	if (isSyntax(first, Exp)) {
		InterCodes *irs = translateExp(first, NULL);
		return irs;
	} else if (isSyntax(first, CompSt)) {
		InterCodes *compSt = interCodeStackGet();
		return compSt;
	} else if (isSyntax(first, RETURN)) {
		TreeNode *second = treeKthChild(p, 2);
		Operand *op = newTempOperand();
		InterCodes *irs = translateExp(second, op);
		InterCode *ir = newInterCode1(RETURN, op);
		interCodeInsert(irs, ir);
		return irs;
	} else {
		return NULL;
	}
}
