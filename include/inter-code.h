#ifndef COMPILER_INTER_CODE_H
#define COMPILER_INTER_CODE_H

#include "lib/List.h"

typedef enum { VARIABLE, CONSTANT, ADDRESS } OperandKind;
typedef struct Operand {
	OperandKind kind;
	union {
		int varID;
		int value;
	};
} Operand;

typedef enum { ASSIGN, ADD, SUB, MUL, DIV } InterCodeKind;
typedef struct InterCode {
	InterCodeKind kind;
	Operand *res, *op1, *op2;
	ListHead list;
} InterCode;

extern ListHead interCodes;
void interCodeInit();
InterCode* newInterCode(InterCodeKind, Operand*, Operand*, Operand*);

#endif
