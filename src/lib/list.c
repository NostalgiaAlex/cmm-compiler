#include "common.h"
#include "lib/list.h"
#include <assert.h>

static void listAdd(ListHead *prev, ListHead *next, ListHead *data) {
	assert(data != NULL);
	data->prev = prev;
	data->next = next;
	if (prev != NULL) prev->next = data;
	if (next != NULL) next->prev = data;
}

void listAddBefore(ListHead *list, ListHead *data) {
	assert(list != NULL);
	assert(list->prev->next == list);
	assert(list->next->prev == list);
	listAdd(list->prev, list, data);
}

void listAddAfter(ListHead *list, ListHead *data) {
	assert(list != NULL);
	assert(list->prev->next == list);
	assert(list->next->prev == list);
	listAdd(list, list->next, data);
}

void listDelete(ListHead *data) {
	assert(data != NULL);
	ListHead *prev = data->prev;
	ListHead *next = data->next;
	if (prev != NULL) prev->next = next;
	if (next != NULL) next->prev = prev;
}

void listMerge(ListHead *dest, ListHead *src) {
	assert(dest != NULL);
	assert(src != NULL);
	assert(dest != src);
	if (listIsEmpty(src)) return;
	ListHead *srcTail = src->prev;
	ListHead *srcHead = src->next;
	ListHead *destTail = dest->prev;
	destTail->next = srcHead;
	srcHead->prev = destTail;
	dest->prev = srcTail;
	srcTail->next = dest;
}

void listInit(ListHead *list) {
	assert(list != NULL);
	list->prev = list->next = list;
}

bool listIsEmpty(ListHead *list) {
	assert(list != NULL);
	return list == list->next;
}
