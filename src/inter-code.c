#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib/List.h"
#include "inter-code.h"

static ListHead head;
static const char *interCodeStr[] = {
		"LABEL %s :",
		"FUNCTION %s :",
		"%s := %s",
		"%s := %s + %s",
		"%s := %s - %s",
		"%s := %s * %s",
		"%s := %s / %s",
		"%s := &%s",
		"GOTO %s",
		"",
		"RETURN %s",
		"",
		"ARG %s",
		"%s := CALL %s",
		"PARAM %s",
		"READ %s",
		"WRITE %s",
};

void interCodeInit() {
	listInit(&head);
}

InterCode* newInterCode(InterCodeKind kind) {
	InterCode* p = (InterCode*)malloc(sizeof(InterCode));
	p->kind = kind;
	p->res = p->op1 = p->op2 = NULL;
	return p;
}

void interCodeInsert(InterCode* p) {
	assert(p != NULL);
	listAddBefore(&head, &p->list);
}

#define op2s(op) (operandToStr(op))
void interCodePrint(InterCode* p) {
	assert(p != NULL);
	if (p->kind == GOTO_WITH_COND) {
		printf("IF %s %s %s GOTO %s", op2s(p->op1), p->relop, op2s(p->op2), op2s(p->res));
	} else if (p->kind == DEC) {
		printf("DEC %s %d", op2s(p->res), p->size);
	} else {
		printf(interCodeStr[p->kind], op2s(p->res), op2s(p->op1), op2s(p->op2));
	}
	puts("");
}
#undef op2s

void test() {
	Operand *op1 = newOperand(VARIABLE);
	op1->id = 1;
	Operand *op2 = newOperand(VARIABLE);
	op2->id = 2;
	Operand *op3 = newOperand(ADDRESS);
	op3->id = 3;
	InterCode *ir = newInterCode(ADD);
	ir->res = op3;
	ir->op1 = op1;
	ir->op2 = op2;
	interCodePrint(ir);
}