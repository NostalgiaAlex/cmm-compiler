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

#define getStr(str, format, ...) do { \
	char _s[30]; \
	sprintf(_s, format, __VA_ARGS__); \
	str = malloc(strlen(_s)+1); \
	strcpy(str, _s); \
	return str; \
} while (0)

char* operandToStr(Operand* p) {
	assert(p);
	if (p->text != NULL) return p->text;
	switch (p->kind) {
		case VARIABLE: getStr(p->text, "t%d", p->id);
		case CONSTANT: getStr(p->text, "#%d", p->value);
		case ADDRESS: getStr(p->text, "*t%d", p->id);
		case LABEL: getStr(p->text, "label%d", p->id);
		case FUNCTION: getStr(p->text, "%s", p->name);
	}
	return "";
}

void operandRelease(Operand* p) {
	free(p->text);
	free(p);
}
