#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdlib.h>
#include <string.h>
#include "macro.h"

char* toArray(const char*);
#define isSyntax(node, token) \
	((node != NULL)&&(strcmp((node)->name, str(token)) == 0))

#endif
