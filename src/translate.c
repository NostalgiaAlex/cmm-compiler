#include <assert.h>
#include <symbol.h>
#include "syntax-tree.h"
#include "translate.h"

static InterCodes head;

#define SIZE 1005
static ListHead stack[SIZE];
static int top;
typedef struct ListNode {
	ListHead list;
	InterCodes *head;
} ListNode;

void interCodesInit() {
	listInit(&head);
	listInit(stack);
	top = 0;
}

void interCodeStackPush() {
	top++;
	assert(top < SIZE);
	listInit(stack+top);
}

void interCodeStackPop() {
	assert(top > 0);
	top--;
}

void interCodeStackInsert(InterCodes *head) {
	ListNode *listNode = (ListNode*)malloc(sizeof(ListNode));
	listAddBefore(stack+top, &listNode->list);
	listNode->head = head;
}

InterCodes* interCodeStackGet() {
	assert(!listIsEmpty(&stack[top]));
	ListHead *p = stack[top].next;
	ListNode *listNode = listEntry(p, ListNode, list);
	listDelete(p);
	InterCodes *head = listNode->head;
	free(listNode);
	return head;
}


InterCodes* interCodesGet() {
	return &head;
}

void defineFunc(char *name, InterCodes *irs) {
	assert(name != NULL);
	Operand *op = newFunctionOperand(name);
	InterCode* ir = newInterCode1(DEF_FUNCTION, op);
	interCodeInsert(&head, ir);
	interCodesBind(&head, irs);
}

static InterCodes* translateCond(TreeNode*, Operand*, Operand*);
static InterCodes* translateArgs(TreeNode*, ListHead*);
static InterCodes* translateStmtList(TreeNode*);
static InterCodes* translateStmt(TreeNode*);
static InterCodes* translateDefList(TreeNode*);
static InterCodes* translateDecList(TreeNode*);
static InterCodes* translateDec(TreeNode*);

InterCodes* translateCompSt(TreeNode *p, Func* func) {
	assert(isSyntax(p, CompSt));
	InterCodes *irs = newInterCodes();
	if (func) {
		ListHead *q;
		listForeach(q, &func->args) {
			char *name = listEntry(q, Arg, list)->name;
			InterCode *ir = newInterCode1(PARAM, newVarOperand());
			interCodeInsert(irs, ir);
			symbolFind(name)->id = ir->res->id;
		}
	}

	TreeNode *defList = treeKthChild(p, 2);
	TreeNode *stmtList = treeLastKthChild(p, 2);
	if (isSyntax(defList, DefList))
		interCodesBind(irs, translateDefList(defList));
	if (isSyntax(stmtList, StmtList))
		interCodesBind(irs, translateStmtList(stmtList));
	return irs;
}

typedef struct OperandNode {
	Operand *op;
	ListHead list;
} OperandNode;

#define checkRes() do { if (!res) return irs; } while (0)
InterCodes* translateExp(TreeNode *p, Operand *res) {
	assert(isSyntax(p, Exp));
	static Type *type = NULL;
	InterCodes *irs = newInterCodes();
	TreeNode *first = treeFirstChild(p);
	TreeNode *second = treeKthChild(p, 2);
	TreeNode *last = treeLastChild(p);
	if (isSyntax(first, ID)) {
		if (isSyntax(last, RP)) { // ID LP Args RP | ID LP RP
			TreeNode *argsNode = treeKthChild(p, 3);
			ListHead args;
			listInit(&args);
			if (isSyntax(argsNode, Args))
				interCodesBind(irs, translateArgs(argsNode, &args));
			if (strcmp(first->text, "read") == 0) {
				if (!res) res = newTempOperand();
				InterCode *ir = newInterCode1(READ, res);
				interCodeInsert(irs, ir);
			} else if (strcmp(first->text, "write") == 0) {
				Operand *op = listEntry(args.next, OperandNode, list)->op;
				InterCode *ir = newInterCode1(WRITE, op);
				interCodeInsert(irs, ir);
				if (res) {
					op = constOperand(0);
					ir = newInterCode2(ASSIGN, res, op);
					interCodeInsert(irs, ir);
				}
			} else {
				ListHead *q;
				listForeach(q, &args) {
					Operand *op = listEntry(q, OperandNode, list)->op;
					InterCode *ir = newInterCode1(ARG, op);
					interCodeInsert(irs, ir);
				}
				if (!res) res = newTempOperand();
				Operand *op = newFunctionOperand(first->text);
				InterCode *ir = newInterCode2(CALL, res, op);
				interCodeInsert(irs, ir);
			}
			while (!listIsEmpty(&args)) {
				ListHead *q = args.next;
				OperandNode *operandNode = listEntry(q, OperandNode, list);
				listDelete(q);
				free(operandNode);
			}
		} else { // ID
			checkRes();
			Symbol *symbol = symbolFind(first->text);
			type = symbol->type;
			Operand *op = symbolGetOperand(symbol);
			if ((res->id < 0)&&(type->kind != BASIC)) {
				res->id = newTempOperandId();
				InterCode *ir = newInterCode2(GET_REF, res, op);
				interCodeInsert(irs, ir);
			} else {
				*res = *op;
			}
		}
	} else if (isSyntax(first, INT)) {
		checkRes();
		*res = *constOperand(first->intVal);
	} else if (isSyntax(second, LB)) {
		TreeNode *indexNode = treeLastKthChild(p, 2);
		Operand *index = newTempOperand();
		interCodesBind(irs, translateExp(indexNode, index));
		Operand *base = tempOperand(-1);
		interCodesBind(irs, translateExp(first, base));
		assert(type->kind == ARRAY);
		type = type->array.elem;
		Operand *offset = newTempOperand();
		Operand *size = constOperand(typeSize(type));
		interCodeInsert(irs, newInterCode3(MUL, offset, index, size));
		if (res->id < 0) {
			res->id = newTempOperandId();
			interCodeInsert(irs, newInterCode3(ADD, res, base, offset));
		} else {
			Operand *tmp = newTempOperand();
			interCodeInsert(irs, newInterCode3(ADD, tmp, base, offset));
			interCodeInsert(irs, newInterCode2(GET_ADDR, res, tmp));
		}
	} else if (isSyntax(second, DOT)) {
		char *id = last->text;
		Operand *base = tempOperand(-1);
		interCodesBind(irs, translateExp(first, base));
		assert(type->kind == STRUCTURE);
		Field *field = fieldFind(&type->structure, id);
		assert(field != NULL);
		Operand *offset = constOperand(fieldOffset(&type->structure, id));
		type = field->type;
		if (res->id < 0) {
			res->id = newTempOperandId();
			interCodeInsert(irs, newInterCode3(ADD, res, base, offset));
		} else {
			Operand *tmp = newTempOperand();
			interCodeInsert(irs, newInterCode3(ADD, tmp, base, offset));
			interCodeInsert(irs, newInterCode2(GET_ADDR, res, tmp));
		}
	} else if (isSyntax(first, LP)) {
		return translateExp(second, res);
	} else if (isSyntax(first, NOT)||isSyntax(second, RELOP)||
			   isSyntax(second, AND)||isSyntax(second, OR)) {
		Operand *label1 = newLabelOperand();
		Operand *label2 = newLabelOperand();
		InterCodes *condIRs = translateCond(p, label1, label2);
		if (!res) res = newTempOperand();
		interCodeInsert(irs, newInterCode2(ASSIGN, res, CONST_ZERO));
		interCodesBind(irs, condIRs);
		interCodeInsert(irs, newInterCode1(DEF_LABEL, label1));
		interCodeInsert(irs, newInterCode2(ASSIGN, res, CONST_ONE));
		interCodeInsert(irs, newInterCode1(DEF_LABEL, label2));
	} else if (isSyntax(first, MINUS)) {
		Operand *op = newTempOperand();
		InterCodes *expIRs = translateExp(second, op);
		interCodesBind(irs, expIRs);
		checkRes();
		InterCode *ir = newInterCode3(SUB, res, CONST_ZERO, op);
		interCodeInsert(irs, ir);
	} else if (isSyntax(second, ASSIGNOP)) {
		Operand *op1 = tempOperand(-1);
		Operand *op2 = newTempOperand();
		InterCodes* irs1 = translateExp(first, op1);
		InterCodes* irs2 = translateExp(last, op2);
		interCodesBind(irs, irs1);
		interCodesBind(irs, irs2);
		InterCode *ir = (op1->kind == VARIABLE)?
						newInterCode2(ASSIGN, op1, op2):
						newInterCode2(SET_ADDR, op1, op2);
		interCodeInsert(irs, ir);
		checkRes();
		if (res->id < 0) {
			*res = *op1;
		} else {
			interCodeInsert(irs, newInterCode2(GET_ADDR, res, op1));
		}
	} else {
		Operand *op1 = newTempOperand();
		Operand *op2 = newTempOperand();
		InterCodes* irs1 = translateExp(first, op1);
		InterCodes* irs2 = translateExp(last, op2);
		interCodesBind(irs, irs1);
		interCodesBind(irs, irs2);
		InterCodeKind kind = ADD;
		checkRes();
		if (isSyntax(second, PLUS)) {
			kind = ADD;
		} else if (isSyntax(second, MINUS)) {
			kind = SUB;
		} else if (isSyntax(second, STAR)) {
			kind = MUL;
		} else if (isSyntax(second, DIV)) {
			kind = DIV;
		}
		InterCode *ir = newInterCode3(kind, res, op1, op2);
		interCodeInsert(irs, ir);
	}
	return irs;
}

#define checkLabel(label) assert((label != NULL)&&(label->kind == LABEL))
static InterCodes* translateCond(TreeNode *p, Operand *labelTrue, Operand *labelFalse) {
	assert(isSyntax(p, Exp));
	checkLabel(labelTrue);
	checkLabel(labelFalse);
	TreeNode *first = treeFirstChild(p);
	TreeNode *second = treeKthChild(p, 2);
	TreeNode *last = treeLastChild(p);
	if (isSyntax(first, NOT)) {
		return translateCond(second, labelFalse, labelTrue);
	} else if (isSyntax(second, RELOP)) {
		Operand *op1 = newTempOperand();
		Operand *op2 = newTempOperand();
		InterCodes *irs = translateExp(first, op1);
		InterCodes *irs2 = translateExp(last, op2);
		interCodesBind(irs, irs2);
		InterCode *ir = newInterCode3(GOTO_WITH_COND, labelTrue, op1, op2);
		ir->relop = second->text;
		interCodeInsert(irs, ir);
		return interCodeInsert(irs, newInterCode1(GOTO, labelFalse));
	} else if (isSyntax(second, AND)) {
		Operand *label = newLabelOperand();
		InterCodes *irs =  translateCond(first, label, labelFalse);
		interCodeInsert(irs, newInterCode1(DEF_LABEL, label));
		return interCodesBind(irs, translateCond(last, labelTrue, labelFalse));
	} else if (isSyntax(second, OR)) {
		Operand *label = newLabelOperand();
		InterCodes *irs =  translateCond(first, labelTrue, label);
		interCodeInsert(irs, newInterCode1(DEF_LABEL, label));
		return interCodesBind(irs, translateCond(last, labelTrue, labelFalse));
	} else {
		Operand *op = newTempOperand();
		InterCodes *irs = translateExp(p, op);
		InterCode *ir = newInterCode3(GOTO_WITH_COND, labelTrue, op, CONST_ZERO);
		ir->relop = toArray("!=");
		interCodeInsert(irs, ir);
		ir = newInterCode1(GOTO, labelFalse);
		return interCodeInsert(irs, ir);
	}
}

static InterCodes* translateArgs(TreeNode *p, ListHead *args) {
	assert(isSyntax(p, Args));
	TreeNode *first = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	Operand *op = newTempOperand();
	InterCodes *irs = translateExp(first, op);
	OperandNode *operandNode = (OperandNode*)malloc(sizeof(OperandNode));
	operandNode->op = op;
	listAddAfter(args, &operandNode->list);
	if (isSyntax(rest, Args))
		interCodesBind(irs, translateArgs(rest, args));
	return irs;
}

static InterCodes* translateStmtList(TreeNode *p) {
	assert(isSyntax(p, StmtList));
	TreeNode *first = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	InterCodes *irs = translateStmt(first);
	if (isSyntax(rest, StmtList))
		interCodesBind(irs, translateStmtList(rest));
	return irs;
}

static InterCodes* translateStmt(TreeNode *p) {
	assert(isSyntax(p, Stmt));
	TreeNode *first = treeFirstChild(p);
	if (isSyntax(first, Exp)) {
		InterCodes *irs = translateExp(first, NULL);
		return irs;
	} else if (isSyntax(first, CompSt)) {
		InterCodes *compSt = interCodeStackGet();
		return compSt;
	} else if (isSyntax(first, RETURN)) {
		TreeNode *second = treeKthChild(p, 2);
		Operand *op = newTempOperand();
		InterCodes *irs = translateExp(second, op);
		InterCode *ir = newInterCode1(RETURN, op);
		interCodeInsert(irs, ir);
		return irs;
	} else if (isSyntax(first, IF)) {
		TreeNode *exp = treeKthChild(p, 3);
		bool withElse = isSyntax(treeLastKthChild(p, 2), ELSE);
		TreeNode *stmt1, *stmt2;
		if (withElse) {
			stmt1 = treeLastKthChild(p, 3);
			stmt2 = treeLastChild(p);
		} else {
			stmt1 = treeLastChild(p);
			stmt2 = NULL;
		}
		Operand *label1 = newLabelOperand();
		Operand *label2 = newLabelOperand();
		Operand *label3 = NULL;
		InterCodes *irs = translateCond(exp, label1, label2);
		interCodeInsert(irs, newInterCode1(DEF_LABEL, label1));
		interCodesBind(irs, translateStmt(stmt1));
		if (stmt2) {
			label3 = newLabelOperand();
			interCodeInsert(irs, newInterCode1(GOTO, label3));
		}
		interCodeInsert(irs, newInterCode1(DEF_LABEL, label2));
		if (stmt2) {
			interCodesBind(irs, translateStmt(stmt2));
			interCodeInsert(irs, newInterCode1(DEF_LABEL, label3));
		}
		return irs;
	} else {
		TreeNode *exp = treeKthChild(p, 3);
		TreeNode *stmt = treeLastChild(p);
		Operand *label1 = newLabelOperand();
		Operand *label2 = newLabelOperand();
		Operand *label3 = newLabelOperand();
		InterCodes *irs = newInterCodes();
		interCodeInsert(irs, newInterCode1(DEF_LABEL, label1));
		interCodesBind(irs, translateCond(exp, label2, label3));
		interCodeInsert(irs, newInterCode1(DEF_LABEL, label2));
		interCodesBind(irs, translateStmt(stmt));
		interCodeInsert(irs, newInterCode1(GOTO, label1));
		return interCodeInsert(irs, newInterCode1(DEF_LABEL, label3));
	}
}

static InterCodes* translateDefList(TreeNode *p) {
	assert(isSyntax(p, DefList));
	TreeNode *first = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	TreeNode *decList = treeKthChild(first, 2);
	InterCodes *irs = translateDecList(decList);
	if (isSyntax(rest, DefList))
		interCodesBind(irs, translateDefList(rest));
	return irs;
}

static InterCodes* translateDecList(TreeNode *p) {
	assert(isSyntax(p, DecList));
	TreeNode *first = treeFirstChild(p);
	TreeNode *rest = treeLastChild(p);
	InterCodes *irs = translateDec(first);
	if (isSyntax(rest, DecList))
		interCodesBind(irs, translateDecList(rest));
	return irs;
}

char *varDecToStr(TreeNode *p) {
	assert(isSyntax(p, VarDec));
	TreeNode *first = treeFirstChild(p);
	if (isSyntax(first, ID)) return first->text;
	return varDecToStr(first);
}
static InterCodes* translateDec(TreeNode *p) {
	assert(isSyntax(p, Dec));
	TreeNode *first = treeFirstChild(p);
	TreeNode *last = treeLastChild(p);
	Symbol *symbol = symbolFind(varDecToStr(first));
	InterCodes *irs = newInterCodes();
	Operand *var = symbolGetOperand(symbol);
	if (symbol->type->kind != BASIC) {
		InterCode *ir = newInterCode1(DEC, var);
		ir->size = typeSize(symbol->type);
		interCodeInsert(irs, ir);
	}
	if (isSyntax(last, Exp)) {
		Operand *op = newTempOperand();
		interCodesBind(irs, translateExp(last, op));
		interCodeInsert(irs, newInterCode2(ASSIGN, var, op));
	}
	return irs;
}