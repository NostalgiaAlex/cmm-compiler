%{
#include <string.h>
#include "macro.h"
#include "lib/Node.h"
#define YYSTYPE Node*
#include "syntax.tab.h"
#define handle(token) do { \
	yylval = newToken(token); \
	return token; \
} while(0)
Node* newToken(int token) {
	Node* p = newNode();
	p->lineNo = yylineno;
	p->token = token;
	p->text = (char*)malloc(strlen(yytext));
	strcpy(p->text, yytext);
	return p;
}
%}
%option yylineno
digit [0-9]
letter [_a-zA-Z]
numHex 0[xX][0-9a-fA-F]+
numDec 0|([1-9]{digit}*)
numOct 0[0-7]+
id {letter}({letter}|{digit})*
floatSci (({digit}*\.?{digit}+)|({digit}+\.))[eE][\+-]?{digit}+
floatNormal {digit}+\.{digit}+
float {floatSci}|{floatNormal}
space [ \t\n]
%%
>|<|>=|<=|==|!= { handle(RELOP); }
; { handle(SEMI); }
, { handle(COMMA); }
= { handle(ASSIGNOP); }
- { handle(MINUS); }
! { handle(NOT); }
&& { handle(AND); }
"||" { handle(OR); }
"+" { handle(PLUS); }
"*" { handle(STAR); }
"/" { handle(DIV); }
"." { handle(DOT); }
"(" { handle(LP); }
")" { handle(RP); }
"[" { handle(LB); }
"]" { handle(RB); }
"{" { handle(LC); }
"}" { handle(RC); }
int|float { handle(TYPE); }
struct { handle(STRUCT); }
return { handle(RETURN); }
if { handle(IF); }
else { handle(ELSE); }
while { handle(WHILE); }
{id} { handle(ID); }
{numDec} {
	yylval = newToken(INT);
	yylval->intVal = atoi(yytext);
	return INT;
}
{numHex} {
	yylval = newToken(INT);
	yylval->intVal = strtol(yytext, NULL, 16);
	return INT;
}
{numOct} {
	yylval = newToken(INT);
	yylval->intVal = strtol(yytext, NULL, 8);
	return INT;
}
{float} {
	yylval = newToken(FLOAT);
	yylval->floatVal = atol(yytext);
	return FLOAT;
}
{space} {}
"//".* {}
"/*" {
	char c = 0, d = input();
	while (!((c == '*')&&(d == '/'))) {
		c = d;
		d = input();
		if (d < 0) break;
	}
}
. {
	printf("Error type A at line %d: Mysterious characters \'%s\'\n",
			yylineno, yytext);
}
%%
