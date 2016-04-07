#include <stdio.h>
#include "syntax-tree.h"
#include "symbol.h"

void yyrestart(FILE*);
void yyparse();
void print(TreeNode*, int);

int main(int argc, char* argv[]) {
    symbolTableInit();
	typesInit();
	if (argc <= 1) return 1;
	FILE *f = fopen(argv[1], "r");
	if (!f) return 1;
	yyrestart(f);
	yyparse();
	if (root != NULL) {
//		print(root, 0);
		analyseProgram(root);
	}
	return 0;
}
