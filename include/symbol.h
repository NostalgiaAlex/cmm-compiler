#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

typedef struct Symbol {
	char* name;
} Symbol;

void symbolsInit();
void symbolsStackPush();
void symbolsStackPop();

void symbolInsert(Symbol*);
Symbol* symbolFind(const char*);

#endif
