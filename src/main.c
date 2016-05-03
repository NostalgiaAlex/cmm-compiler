#include <stdio.h>
#include "syntax-tree.h"
#include "translate.h"

void yyrestart(FILE*);
void yyparse();
void print(TreeNode*, int);

void init() {
	symbolTableInit();
	typesInit();
	operandInit();
	interCodesInit();
}

int main(int argc, char* argv[]) {
	if (argc <= 2) return 1;
	FILE *fin = fopen(argv[1], "r");
	FILE *fout = fopen(argv[2], "w");
	if (!fin) return 1;
	init();
	yyrestart(fin);
	yyparse();
	if (root != NULL) {
		analyseProgram(root);
		InterCodes *head = interCodesGet();
		interCodesPrint(fout, head);
	}
	return 0;
}
