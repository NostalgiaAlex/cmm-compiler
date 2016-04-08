#include <stdlib.h>
#include "lib/List.h"
#include "inter-code.h"

ListHead interCodes;

InterCode* newInterCode(InterCodeKind kind, Operand* res, Operand* op1, Operand* op2) {
	InterCode* interCode = (InterCode*)malloc(sizeof(InterCode));
	interCode->kind = kind;
	interCode->res = res;
	interCode->op1 = op1;
	interCode->op2 = op2;
	listAddBefore(&interCodes, &interCode->list);
	return interCode;
}
