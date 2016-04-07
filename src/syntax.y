%{
#include "common.h"
#include <string.h>
#include <stdio.h>
#include "macro.h"
#include "lib/Tree.h"
#define YYSTYPE TreeNode*
int yylex();
void yyerror(char*);
TreeNode* createTree(int , ...);
void analyseProgram(TreeNode*);
void print(TreeNode*, int);
extern int yylineno;
extern int errorStatus;
#define handle(root, token, arity, ...) \
do { \
	root = createTree(arity, __VA_ARGS__); \
	root->name = toArray(str(token)); \
} while (0)
%}
%token INT FLOAT ID
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT NEG
%left LP RP DOT LB RB
%token LC RC
%token TYPE STRUCT RETURN IF WHILE
%token SEMI COMMA
%nonassoc SUB_ELSE
%nonassoc ELSE
%%
/* High-level Definitions */
Program: ExtDefList {
	   handle($$, Program, 1, $1);
	   if (errorStatus == 0) {
		   // print($$, 0);
		   analyseProgram($$);
	   }
	   }
	   ;
ExtDefList: { $$ = NULL; }
		  | ExtDef ExtDefList { handle($$, ExtDefList, 2, $1, $2); }
		  ;
ExtDef: Specifier ExtDecList SEMI { handle($$, ExtDef, 3, $1, $2, $3); }
	  | Specifier SEMI { handle($$, ExtDef, 2, $1, $2); }
	  | Specifier FunDec CompSt { handle($$, ExtDef, 3, $1, $2, $3); }
	  | Specifier FunDec SEMI { handle($$, ExtDef, 3, $1, $2, $3); }
	  ;
ExtDecList: VarDec { handle($$, ExtDecList, 1, $1); }
		  | VarDec COMMA ExtDecList { handle($$, ExtDecList, 3, $1, $2, $3); }
		  ;

/* Specifiers */
Specifier: TYPE { handle($$, Specifier, 1, $1); }
		 | StructSpecifier { handle($$, Specifier, 1, $1); }
		 ;
StructSpecifier: STRUCT Tag { handle($$, StructSpecifier, 2, $1, $2); }
			   | STRUCT OptTag LC DefList RC { handle($$, StructSpecifier, 5, $1, $2, $3, $4, $5); }
			   ;
OptTag: { $$ = NULL; }
	  | ID { handle($$, OptTag, 1, $1); }
	  ;
Tag: ID { handle($$, Tag, 1, $1); }
   ;

/* Declarators */
VarDec: ID { handle($$, VarDec, 1, $1); }
	  | VarDec LB INT RB { handle($$, VarDec, 4, $1, $2, $3, $4); }
	  ;
FunDec: ID LP VarList RP { handle($$, FunDec, 4, $1, $2, $3, $4); }
	  | ID LP RP { handle($$, FunDec, 3, $1, $2, $3); }
	  ;
VarList: ParamDec COMMA VarList { handle($$, VarList, 3, $1, $2, $3); }
	   | ParamDec { handle($$, VarList, 1, $1); }
	   ;
ParamDec: Specifier VarDec { handle($$, ParamDec, 2, $1, $2); }
		;

/* Statements */
CompSt: LC DefList StmtList RC { handle($$, CompSt, 4, $1, $2, $3, $4); }
	  ;
StmtList: { $$ = NULL; }
		| Stmt StmtList { handle($$, StmtList, 2, $1, $2); }
		;
Stmt: Exp SEMI { handle($$, Stmt, 2, $1, $2); }
	| CompSt { handle($$, Stmt, 1, $1); }
	| RETURN Exp SEMI { handle($$, Stmt, 3, $1, $2, $3); }
	| IF LP Exp RP Stmt %prec SUB_ELSE { handle($$, Stmt, 5, $1, $2, $3, $4, $5); }
	| IF LP Exp RP Stmt ELSE Stmt { handle($$, Stmt, 7, $1, $2, $3, $4, $5, $6, $7); }
	| WHILE LP Exp RP Stmt { handle($$, Stmt, 5, $1, $2, $3, $4, $5); }
	;

/* Local Definitions */
DefList: { $$ = NULL; }
	   | Def DefList { handle($$, DefList, 2, $1, $2); }
	   ;
Def: Specifier DecList SEMI { handle($$, Def, 3, $1, $2, $3); }
   ;
DecList: Dec { handle($$, DecList, 1, $1); }
	   | Dec COMMA DecList { handle($$, DecList, 3, $1, $2, $3); }
Dec: VarDec { handle($$, Dec, 1, $1); }
   | VarDec ASSIGNOP Exp { handle($$, Dec, 3, $1, $2, $3); }
   ;

/* Expressions */
Exp: Exp ASSIGNOP Exp { handle($$, Exp, 3, $1, $2, $3); }
   | Exp AND Exp { handle($$, Exp, 3, $1, $2, $3); }
   | Exp OR Exp { handle($$, Exp, 3, $1, $2, $3); }
   | Exp RELOP Exp { handle($$, Exp, 3, $1, $2, $3); }
   | Exp PLUS Exp { handle($$, Exp, 3, $1, $2, $3); }
   | Exp MINUS Exp { handle($$, Exp, 3, $1, $2, $3); }
   | Exp STAR Exp { handle($$, Exp, 3, $1, $2, $3); }
   | Exp DIV Exp { handle($$, Exp, 3, $1, $2, $3); }
   | Exp LB Exp RB { handle($$, Exp, 4, $1, $2, $3, $4); }
   | Exp DOT ID { handle($$, Exp, 3, $1, $2, $3); }
   | LP Exp RP { handle($$, Exp, 3, $1, $2, $3); }
   | MINUS Exp %prec NEG { handle($$, Exp, 2, $1, $2); }
   | NOT Exp { handle($$, Exp, 2, $1, $2); }
   | ID LP Args RP { handle($$, Exp, 4, $1, $2, $3, $4); }
   | ID LP RP { handle($$, Exp, 3, $1, $2, $3); }
   | ID { handle($$, Exp, 1, $1); }
   | INT { handle($$, Exp, 1, $1); }
   | FLOAT { handle($$, Exp, 1, $1); }
   ;
Args: Exp COMMA Args { handle($$, Args, 3, $1, $2, $3); }
	| Exp { handle($$, Args, 1, $1); }
	;
%%
void yyerror(char* msg) {
	printf("Error type B at Line %d: %s.\n", yylineno, msg);
}
