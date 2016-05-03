#ifndef COMPILER_TRANSLATE_H
#define COMPILER_TRANSLATE_H

#include "syntax-tree.h"
#include "inter-code.h"
#include "symbol.h"

void interCodesInit();
void interCodeStackPush();
void interCodeStackPop();
void interCodeStackInsert(InterCodes*);
InterCodes* interCodeStackGet();

InterCodes* interCodesGet();

InterCodes* translateExp(TreeNode*, Operand*);
InterCodes* translateCompSt(TreeNode*, Func*);
void defineFunc(char*, InterCodes*);


#endif
