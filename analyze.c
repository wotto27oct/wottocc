#include "wottocc.h"

void analyze_lval(Node *node) {
	if (node->node_ty == ND_DEREF) {
		analyze(node->lhs);
		return;
	}
	return;
}

void analyze(Node *node) {
	if (node->node_ty == ND_FUNCDEF) {
		analyze(node->lhs);
		return;
	}
	
	if (node->node_ty == ND_GVARDEC) {
		analyze(node->lhs);
		return;
	}
	
	if (node->node_ty == ND_CASE) {
		if (now_switch_node == NULL) error("case must be in switch\n");
		node->val = now_switch_node->env->cases->len; // the order of this case
		vec_push(now_switch_node->env->cases, node->rhs);
		analyze(node->lhs);
		return;
	}
	
	if (node->node_ty == ND_COMPOUND_STMT) {
		analyze(node->lhs);
		return;
	}
	
	if (node->node_ty == ND_BLOCKITEMLIST) {
		for (int i = 0; i < node->args->len; i++) {
			analyze(vec_get(node->args, i));
		}
		return;
	}
	
	if (node->node_ty == ND_EXPRESSION_STMT) {
		// exclude " ;"
		if (node->lhs != NULL) {
			analyze(node->lhs);
		}
		return;
	}
	
	if (node->node_ty == ND_IF) {
		Node *arg = vec_get(node->args, 0);
		analyze(arg);
		analyze(node->lhs);
		if (node->rhs != NULL) {
			// else
			analyze(node->rhs);
		}
		return;
	}
	
	if (node->node_ty == ND_SWITCH) {
		analyze(node->lhs);
		// switch value must be on top of the stack
		for (int i = node->env->cases->len - 1; i >= 0; i--) {
			Node *tmp = vec_get(node->env->cases, i);
			analyze(tmp);
		}

		Node *old_switch_node = now_switch_node;
		now_switch_node = node;
		analyze(node->rhs);
		now_switch_node = old_switch_node;
		return;
	}
	
	if (node->node_ty == ND_WHILE) {
		analyze(node->lhs);
		
		analyze(node->rhs);
		return;
	}	
	
	if (node->node_ty == ND_FOR) {
		Node *arg = vec_get(node->args, 0);
		if (arg != NULL) analyze(arg);
		
		arg = vec_get(node->args, 1);
		if (arg != NULL) {
			analyze(arg);
		}
	
		analyze(node->lhs);
		arg = vec_get(node->args, 2);
		if (arg != NULL) analyze(arg);
		return;
	}
	
	if (node->node_ty == ND_CONTINUE) {
		return;
	}
	
	if (node->node_ty == ND_BREAK) {
		return;
	}

	if (node->node_ty == ND_RETURN) {
		// change later (support void)
		analyze(node->lhs);
		return;
	}
	
	if (node->node_ty == ND_DECLARATION) {
		analyze(node->lhs);
		return;
	}

	if (node->node_ty == ND_INIT_DECLARATOR_LIST || node->node_ty == ND_INIT_G_DECLARATOR_LIST) {
		analyze(node->lhs);
		analyze(node->rhs);	
		return;
	}
	
	if (node->node_ty == ND_INIT_DECLARATOR) {
		if (node->rhs->node_ty != ND_INITIALIZER_LIST) {
			// int a = 10;
			analyze_lval(node->lhs);
			analyze(node->rhs);
		} else {
			// array a[10] = {1, 2, 3};
			analyze_lval(node->lhs);
			for(size_t i = 0; i < node->rhs->value_ty->array_size; i++) {
				Node *arg = vec_get(node->rhs->args, i);
				analyze(arg);
			}
		}
		return;
	}	
	
	if (node->node_ty == ND_INIT_G_DECLARATOR) {
		// int,char,ptr
		if (node->rhs->node_ty != ND_INITIALIZER_LIST) {
			// int a = 10;
			
		} else {
			// int a[10] = {1, 2, 3}
		}
		return;
	}	
	
	if (node->node_ty == ND_G_DECLARATOR) {
		return;
	}
	
	if (node->node_ty == ND_NUM) {
		return;
	}

	if (node->node_ty == ND_IDENT || node->node_ty == ND_G_IDENT) {
		analyze_lval(node);
		return;
	}
	
	if (node->node_ty == ND_STR) {
		return;
	}
	
	if (node->node_ty == ND_FUNC_CALL) {
		// foo(1, 2, 3)
		if (node->rhs != NULL) {
			// 1, 2, 3
			analyze(node->rhs);
		}
		return;
	}

	if (node->node_ty == ND_ARG_EXP_LIST) {
		// 1, 2, 3 (args)
		analyze(node->lhs);
		analyze(node->rhs);
		return;
	}

	if (node->node_ty == ND_EXP) {
		// a = 3, b = 3;
		analyze(node->lhs);
		analyze(node->rhs);
		return;
	}

	if (node->node_ty == '=') {
		analyze_lval(node->lhs);
		analyze(node->rhs);

		return;
	}
	
	if (node->node_ty == ND_PLUSEQ) {
		analyze_lval(node->lhs);
		analyze(node->rhs);

		return;
	}

	if (node->node_ty == ND_MINUSEQ) {
		analyze_lval(node->lhs);
		analyze(node->rhs);

		return;
	}

	if (node->node_ty == ND_PREINC) {
		analyze_lval(node->lhs);
		return;
	}
	
	if (node->node_ty == ND_PREDEC) {
		analyze_lval(node->lhs);
		return;
	}

	if (node->node_ty == ND_POSTINC) {
		analyze_lval(node->lhs);
		return;
	}

	if (node->node_ty == ND_POSTDEC) {
		analyze_lval(node->lhs);
		return;
	}

	if (node->node_ty == ND_DEREF) {
		analyze(node->lhs);
		return;
	}

	if (node->node_ty == '&') {
		analyze_lval(node->lhs);
		return;
	}
	
	if (node->node_ty == '+') {
		analyze(node->lhs);
		analyze(node->rhs);
		return;
	}
	
	if (node->node_ty == '-') {
		analyze(node->lhs);
		analyze(node->rhs);
		return;
	}
	
	analyze(node->lhs);
	analyze(node->rhs);
	return;
}
