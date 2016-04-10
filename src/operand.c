#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "inter-code.h"

Operand* newOperand(OperandKind kind) {
	Operand *p = (Operand*)malloc(sizeof(Operand));
	p->kind = kind;
	p->name = p->text = NULL;
	return p;
}
Operand* newVarOperand() {
	static int cnt = 0;
	Operand *p = newOperand(VARIABLE);
	p->id = ++cnt;
	return p;
}
Operand* newLabelOperand() {
	static int cnt = 0;
	Operand *p = newOperand(LABEL);
	p->id = ++cnt;
	return p;
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
		case VARIABLE: getStr(p->text, "t%d", p->id);
		case CONSTANT: getStr(p->text, "#%d", p->value);
		case ADDRESS: getStr(p->text, "&t%d", p->id);
		case DEREF: getStr(p->text, "*t%d", p->id);
		case LABEL: getStr(p->text, "label%d", p->id);
		case FUNCTION: getStr(p->text, "%s", p->name);
	}
	return NULL;
}

void operandRelease(Operand* p) {
	free(p->text);
	free(p);
}
