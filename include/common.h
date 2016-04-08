#ifndef COMPILER_COMMON_H
#define COMPILER_COMMON_H
#include <stdlib.h>
#include <string.h>
#include "macro.h"

char* toArray(const char*);
#define isSyntax(node, token) \
	(strcmp((node)->name, str(token)) == 0)

#endif
