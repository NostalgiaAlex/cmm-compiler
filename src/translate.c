#include <assert.h>
#include "syntax-tree.h"
#include "translate.h"
#include "symbol.h"

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

void interCodeStackInsert(InterCodes* head) {
	ListNode *listNode = (ListNode*)malloc(sizeof(ListNode));
	listAddBefore(stack+top, &listNode->list);
	listNode->head = head;
}

InterCodes* interCodeStackGet() {
	assert(!listIsEmpty(&stack[top]));
	ListHead *p = stack[top].next;
	ListNode *listNode = listEntry(p, ListNode, list);
	listDelete(p);
	return listNode->head;
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

void translateExp(TreeNode *p) {
	assert(p != NULL);
	assert(isSyntax(p, Exp));
}

InterCodes* translateCompSt(TreeNode *p, Func* func) {
	assert(p != NULL);
	assert(isSyntax(p, CompSt));
	static InterCodes irs;
	listInit(&irs);
	if (func) {
		ListHead *q;
		listForeach(q, &func->args) {
			char *name = listEntry(q, Arg, list)->name;
			InterCode *ir = newInterCode(PARAM);
			ir->res = newVarOperand();
			interCodeInsert(&irs, ir);
			symbolFind(name)->id = ir->res->id;
		}
	}
	return &irs;
}
