#include <assert.h>
#include "syntax-tree.h"
#include "translate.h"

void translateExp(TreeNode *p) {
	assert(p != NULL);
	assert(isSyntax(p, Exp));
}
