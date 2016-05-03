#ifndef COMPILER_INTER_CODE_H
#define COMPILER_INTER_CODE_H

#include <stdio.h>
#include "lib/List.h"

typedef enum { VARIABLE, TEMP, CONSTANT, DEREF, ADDRESS, LABEL, FUNCTION } OperandKind;
typedef struct Operand {
	OperandKind kind;
	union {
		int id;
		int value;
		char* name;
	};
	char* text;
} Operand;
extern Operand* const CONST_ZERO;
extern Operand* const CONST_ONE;
void operandInit();
Operand* newOperand(OperandKind);
int newVarOperandId();
int newTempOperandId();
int newLabelOperandId();
Operand* newVarOperand();
Operand* newTempOperand();
Operand* newLabelOperand();
Operand* newFunctionOperand(char*);
Operand* constOperand(int);
Operand* varOperand(int);
Operand* tempOperand(int);
Operand* addressOperand(int);
Operand* deRefOperand(int);
Operand* labelOperand(int);
char* operandToStr(Operand*);
void operandRelease(Operand*);

typedef enum {
	DEF_LABEL, DEF_FUNCTION,
	ASSIGN, ADD, SUB, MUL, DIV,
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
typedef ListHead InterCodes;

InterCode* newInterCode3(InterCodeKind, Operand*, Operand*, Operand*);
InterCode* newInterCode2(InterCodeKind, Operand*, Operand*);
InterCode* newInterCode1(InterCodeKind, Operand*);
InterCode* newInterCode(InterCodeKind);
void interCodeToStr(InterCode*, char*);
void interCodesPrint(FILE*, InterCodes*);

InterCodes* newInterCodes();
InterCodes* interCodeInsert(InterCodes*, InterCode*);
InterCodes* interCodesBind(InterCodes*, InterCodes*);
#endif
