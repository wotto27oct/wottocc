#include "wottocc.h"

void analyze_lval(Node *node) {
	if (node->node_ty == ND_DEREF) {
		analyze(node->lhs);
		return;
	}
	return;
}

Type *get_ptr(Node *node, Type *sp_type) {
	if (node->node_ty == ND_PTR) {
		Type *newtype = new_type(TY_PTR);
		newtype->ptrof = sp_type;
		if (node->lhs != NULL) {
			newtype = get_ptr(node->lhs, newtype);
		}
		return newtype;
	} else {
		return sp_type;
	}
}	


void analyze_dec(Node *node, Type *sp_type) {
	if (node->node_ty == ND_INIT_DECLARATOR_LIST || node->node_ty == ND_INIT_G_DECLARATOR_LIST) {
		analyze_dec(node->lhs, sp_type);
		analyze_dec(node->rhs, sp_type);	
		return;
	}
	
	if (node->node_ty == ND_INIT_DECLARATOR) {
		analyze_dec(node->lhs, sp_type);
		if (node->rhs != NULL) {
			analyze(node->rhs);
			if (node->rhs->node_ty != ND_INITIALIZER_LIST) {
				assignment_check(node->lhs->value_ty, node->rhs->value_ty);
				// ban x[3] = "abcde"
				if (node->lhs->value_ty->ty == TY_ARRAY && node->rhs->value_ty->ty == TY_PTR) {
					if (node->lhs->value_ty->array_size < node->rhs->length) {
						error("too much initialization at array");
					}
				}
			} else {
				// a[3] = {1,2,3};
				
				if (node->lhs->value_ty->ty != TY_ARRAY) {
					error("lhs must be array at init_declarator\n");
				}
				if (node->lhs->value_ty->array_size < node->rhs->value_ty->array_size) {
					error("too much initializer\n");
				}
				assignment_check(node->lhs->value_ty->ptrof, node->rhs->value_ty->ptrof);
			}
		}
		node->value_ty = node->lhs->value_ty;
		return;
	}	
	
	if (node->node_ty == ND_INIT_G_DECLARATOR) {
		analyze_dec(node->lhs, sp_type);
		analyze(node->rhs);
		if (node->rhs->node_ty != ND_INITIALIZER_LIST) {
			assignment_check(node->lhs->value_ty, node->rhs->value_ty);
		} else {
			// a[3] = {1,2,3};
			if (node->lhs->value_ty->ty != TY_ARRAY) {
				error("lhs must be array at init_declarator\n");
			}
			if (node->lhs->value_ty->array_size < node->rhs->value_ty->array_size) {
				error("too much initializer\n");
			}
			assignment_check(node->lhs->value_ty->ptrof, node->rhs->value_ty->ptrof);
		}
		node->value_ty = node->lhs->value_ty;
		return;
	}	
	
	if (node->node_ty == ND_DECLARATOR) {
		Type *type = sp_type;
		if (node->lhs != NULL) {
			type = get_ptr(node->lhs, type);
		}
		
		if (node->rhs != NULL) {
			// a[10]
			Type *newtype = new_type(TY_ARRAY);
			newtype->ptrof = type;
			newtype->array_size = node->rhs->val;
			type = newtype;
		}
		map_put(node->env->variables, node->name, 0, type);
		node->node_ty = ND_IDENT;
		node->value_ty = type;
		return;
	}	

	if (node->node_ty == ND_PTR) {
	   Type *type = new_type(TY_PTR);
	   type->ptrof = sp_type;
	   if (node->lhs != NULL) analyze_dec(node->lhs, type); 
	   sp_type = type;
	   return;
	}	   

	if (node->node_ty == ND_G_DECLARATOR) {
		Type *type = sp_type;
		if (node->lhs != NULL) {
			type = get_ptr(node->lhs, type);
		}
		
		if (node->rhs != NULL) {
			// a[10]
			Type *newtype = new_type(TY_ARRAY);
			newtype->ptrof = type;
			newtype->array_size = node->rhs->val;
			type = newtype;
		}
		map_put(node->env->variables, node->name, 0, type);
		node->value_ty = type;
		return;
	}
}


void analyze(Node *node) {
	if (node->node_ty == ND_FUNCDEF) {
		analyze(node->lhs); // int
		Type *type = node->lhs->value_ty;
		if (node->lhs->lhs != NULL) {
			type = get_ptr(node->lhs->lhs, type);
		}
		map_put(g_funcs, node->fname, 0, type);
		for (int i = 0; i < node->args->len; i++) {
			Node *tmp = vec_get(node->args, i);
			analyze(tmp);
		}
		analyze(node->rhs);
		return;
	}
	
	if (node->node_ty == ND_ARG) {
		analyze(node->lhs); // int
		Type *type = node->lhs->value_ty;
		if (node->lhs->lhs != NULL) {
			type = get_ptr(node->lhs->lhs, type);
		}
		map_put(node->env->variables, node->name, 0, type);
		node->value_ty = type;
		return;
	}
	
	if (node->node_ty == ND_GVARDEC) {
		analyze(node->lhs); // int
		Type *type = node->lhs->value_ty;
		if (node->lhs->lhs != NULL) {
			type = get_ptr(node->lhs->lhs, type);
		}
		analyze_dec(node->rhs, type);
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
		analyze_dec(node->rhs, node->lhs->value_ty);
		return;
	}

	if (node->node_ty == ND_INT) {
		node->value_ty = new_type(TY_INT);
		return;
	}

	if (node->node_ty == ND_CHAR) {
		node->value_ty = new_type(TY_CHAR);
		return;
	}

	if (node->node_ty == ND_INITIALIZER_LIST) {
		Type *arr_type = NULL;
		for (int i = 0; i < node->args->len; i++) {
			Node *tmp = vec_get(node->args, i);
			if (arr_type == NULL) arr_type = tmp->value_ty;
			if (arr_type->ty != tmp->value_ty->ty){
				error("different array initializer type\n");
			}
		}
		Type *type = new_type(TY_ARRAY);
		type->ptrof = arr_type;
		node->value_ty = type;
		node->value_ty->array_size = node->args->len;
		return;
	}


	
	if (node->node_ty == ND_NUM) {
		return;
	}

	if (node->node_ty == ND_IDENT) {
		//node->value_ty = node_ident_type(node->name, node->env);
		Type *value_ty = get_valuetype(node->env, node->name);
		if (value_ty == NULL || node->env == g_env) {
			value_ty = get_valuetype(g_env, node->name);
			if (value_ty == NULL) {
				// may be function call c.f. foo(3)
				return;
			} else {
				// global variable
				if (value_ty->ty == TY_ARRAY) {
					// read array a as if pointer a
					// cf. int a[10]; a[0]=1; *a => 1
					Type *newtype = new_type(TY_PTR);
					newtype->ptrof = value_ty->ptrof;
					newtype->array_size = value_ty->array_size;
					value_ty = newtype;
				}
				node->value_ty = value_ty;
				node->node_ty = ND_G_IDENT;
				return;
			}
		}

		if (value_ty->ty == TY_ARRAY) {
			// read array a as if pointer a
			// cf. int a[10]; a[0]=1; *a => 1
			Type *newtype = new_type(TY_PTR);
			newtype->ptrof = value_ty->ptrof;
			newtype->array_size = value_ty->array_size;
			value_ty = newtype;
		}
		node->value_ty = value_ty;
		return;
	}
	
	if (node->node_ty == ND_STR) {
		return;
	}
	
	if (node->node_ty == ND_FUNC_CALL) {
		// foo(1, 2, 3)
		analyze(node->lhs); // ident (foo)
		node->value_ty = map_get_type(g_funcs, node->lhs->name);
		node->name = node->lhs->name;
		
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

	if (node->node_ty == '=' || node->node_ty == ND_PLUSEQ || node->node_ty == ND_MINUSEQ) {
		analyze(node->lhs);
		analyze(node->rhs);
		node->value_ty = assignment_check(node->lhs->value_ty, node->rhs->value_ty);
		return;
	}
	
	if (node->node_ty == ND_PREINC) {
		analyze(node->lhs);
		node->value_ty = new_type(TY_INT);
		return;
	}
	
	if (node->node_ty == ND_PREDEC) {
		analyze(node->lhs);
		node->value_ty = new_type(TY_INT);
		return;
	}

	if (node->node_ty == ND_POSTINC) {
		analyze(node->lhs);
		node->value_ty = new_type(TY_INT);
		return;
	}

	if (node->node_ty == ND_POSTDEC) {
		analyze(node->lhs);
		node->value_ty = new_type(TY_INT);
		return;
	}

	if (node->node_ty == ND_DEREF) {
		analyze(node->lhs);
		if (node->lhs->value_ty->ty == TY_INT) {
			error("illegal deref: %s\n", ((Token *)vec_get(tokens, pos))->input);
		}
		node->value_ty = node->lhs->value_ty->ptrof;
		return;
	}

	if (node->node_ty == '&') {
		analyze(node->lhs);
		node->value_ty = new_type(TY_PTR);
		node->value_ty->ptrof = node->lhs->value_ty;
		return;
	}
	
	if (node->node_ty == ND_SIZEOF) {
		analyze(node->lhs);
		if (node->lhs->value_ty->ty == TY_PTR && node->lhs->value_ty->array_size != 0) {
			//node = new_node_num(node->lhs->value_ty->array_size * get_typesize(node->lhs->value_ty->ptrof), node->env);
			node->node_ty = ND_NUM;
			node->val = node->lhs->value_ty->array_size * get_typesize(node->lhs->value_ty->ptrof);
			node->value_ty = new_type(TY_INT);
		} else {
			//node = new_node_num(get_typesize(node->lhs->value_ty), node->env);
			node->node_ty = ND_NUM;
			node->val = get_typesize(node->lhs->value_ty);
			node->value_ty = new_type(TY_INT);
		}
		return;
	}

	
	if (node->node_ty == '+') {
		analyze(node->lhs);
		analyze(node->rhs);
		node->value_ty = plus_check(node->lhs->value_ty, node->rhs->value_ty);
		return;
	}
	
	if (node->node_ty == '-') {
		analyze(node->lhs);
		analyze(node->rhs);
		node->value_ty = plus_check(node->lhs->value_ty, node->rhs->value_ty);
		return;
	}

	if (node->node_ty == '*') {
		analyze(node->lhs);
		analyze(node->rhs);
		// TODO:need to change
		node->value_ty = new_type(TY_INT);
	}
	
	if (node->node_ty == '/') {
		analyze(node->lhs);
		analyze(node->rhs);
		// TODO:need to change
		node->value_ty = new_type(TY_INT);
	}

	if (node->node_ty == '<' || node->node_ty == '>' || node->node_ty == ND_LEQ || node->node_ty == ND_GEQ
			|| node->node_ty == ND_EQUAL || node->node_ty == ND_NEQUAL) {
		analyze(node->lhs);
		analyze(node->rhs);
		node->value_ty = new_type(TY_INT);
	}

	
	analyze(node->lhs);
	analyze(node->rhs);
	return;
}
