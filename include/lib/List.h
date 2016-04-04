#ifndef __LIB_LIST_H__
#define __LIB_LIST_H__

#include <stdbool.h>
#include "common.h"

typedef struct ListHead {
	struct ListHead *prev, *next;
} ListHead;

#define listEntry(ptr, type, member) \
	((type*)((char*)(ptr) - (long)(&((type*)0)->member)))

#define listForeach(ptr, head) \
	for ((ptr) = (head)->next; (ptr) != (head); (ptr) = (ptr)->next)

void listAddBefore(ListHead*, ListHead*);
void listAddAfter(ListHead*, ListHead*);
void listDelete(ListHead*);
void listInit(ListHead*);
bool listIsEmpty(ListHead*);

#endif
