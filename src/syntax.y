%{
#include <stdio.h>
#include "lib/Node.h"
#define YYSTYPE Node*
int yylex();
void yyerror(char*);
%}
%token INT FLOAT ID
%token PLUS MINUS STAR DIV
%token AND OR NOT
%token SEMI COMMA DOT ASSIGNOP RELOP
%token LP RP LB RB LC RC
%token TYPE STRUCT RETURN IF ELSE WHILE
%%
Program: ExtDefList { $$ = createNode(1, $1); }
	   ;
ExtDefList: ExtDef ExtDefList { $$ = createNode(2, $1, $2); }
		  |
		  ;

%%
int main() {
yyparse();
}
