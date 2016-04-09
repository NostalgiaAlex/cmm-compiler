#ifndef COMPILER_INTER_CODE_H
#define COMPILER_INTER_CODE_H

#include "lib/List.h"

typedef enum { VARIABLE, CONSTANT, ADDRESS, LABEL, FUNCTION } OperandKind;
typedef struct Operand {
	OperandKind kind;
	union {
		int id;
		int value;
		char* name;
	};
	char* text;
} Operand;
Operand* newOperand(OperandKind);
char* operandToStr(Operand*);
void operandRelease(Operand*);

typedef enum {
	DEF_LABEL, DEF_FUNCTION,
	ASSIGN, ADD, SUB, MUL, DIV, GET_ADDRESS,
	GOTO, GOTO_WITH_COND,
	RETURN, DEC, ARG, CALL, PARAM,
	READ, WRITE,
} InterCodeKind;
typedef struct InterCode {
	InterCodeKind kind;
	Operand *res, *op1, *op2;
	union {
		char* relop;
		int size;
	};
	ListHead list;
} InterCode;

void interCodeInit();
InterCode* newInterCode(InterCodeKind);
void interCodeInsert(InterCode*);

#endif
