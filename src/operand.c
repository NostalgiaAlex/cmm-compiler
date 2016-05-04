#include <stdlib.h>
#include <stdio.h>
#include "inter-code.h"

#define SIZE 10005
static Operand operandsPool[SIZE];

static Operand zero, one;
Operand* const CONST_ZERO = &zero;
Operand* const CONST_ONE = &one;

void operandInit() {
	zero.kind = CONSTANT;
	zero.value = 0;
	one.kind = CONSTANT;
	one.value = 1;
}

Operand* newOperand(OperandKind kind) {
	static int cnt = 0;
	Operand *p = &operandsPool[cnt++];
	p->kind = kind;
	p->name = p->text = NULL;
	return p;
}

int newVarOperandId() {
	static int cnt = 0;
	return ++cnt;
}
int newTempOperandId() {
	static int cnt = 0;
	return ++cnt;
}
int newLabelOperandId() {
	static int cnt = 0;
	return ++cnt;
}

Operand* newVarOperand() {
	return varOperand(newVarOperandId());
}
Operand* newTempOperand() {
	return tempOperand(newTempOperandId());
}
Operand* newLabelOperand() {
	return labelOperand(newLabelOperandId());
}
Operand* newFunctionOperand(char* s) {
	Operand *p = newOperand(FUNCTION);
	p->name = s;
	return p;
}

Operand* constOperand(int val) {
	Operand *p = newOperand(CONSTANT);
	p->value = val;
	return p;
}
Operand* varOperand(int id) {
	Operand *p = newOperand(VARIABLE);
	p->id = id;
	return p;
}
Operand* tempOperand(int id) {
	Operand *p = newOperand(TEMP);
	p->id = id;
	return p;
}
Operand* addressOperand(int id) {
	Operand *p = newOperand(ADDRESS);
	p->id = id;
	return p;
}
Operand* deRefOperand(int id) {
	Operand *p = newOperand(DEREF);
	p->id = id;
	return p;
}
Operand* labelOperand(int id) {
	Operand *p = newOperand(LABEL);
	p->id = id;
	return p;
}

#define getStr(str, format, ...) do { \
	static char buf[30]; \
	sprintf(buf, format, __VA_ARGS__); \
	str = malloc(strlen(buf)+1); \
	strcpy(str, buf); \
	return str; \
} while (0)

char* operandToStr(Operand* p) {
	if (p == NULL) return NULL;
	if (p->text != NULL) return p->text;
	switch (p->kind) {
		case TEMP: getStr(p->text, "t%d", p->id);
		case VARIABLE: getStr(p->text, "v%d", p->id);
		case CONSTANT: getStr(p->text, "#%d", p->value);
		case ADDRESS: getStr(p->text, "&t%d", p->id);
		case DEREF: getStr(p->text, "*t%d", p->id);
		case LABEL: getStr(p->text, "label%d", p->id);
		case FUNCTION: getStr(p->text, "%s", p->name);
	}
	return NULL;
}
