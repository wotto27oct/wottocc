#include "wottocc.h"

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


void analyze_dec(Node *node, Env *env, Type *sp_type) {
	if (node->node_ty == ND_INIT_DECLARATOR_LIST || node->node_ty == ND_INIT_G_DECLARATOR_LIST) {
		node->env = env;
		analyze_dec(node->lhs, env, sp_type);
		analyze_dec(node->rhs, env, sp_type);	
		return;
	}
	
	if (node->node_ty == ND_INIT_DECLARATOR) {
		node->env = env;
		analyze_dec(node->lhs, env, sp_type);
		// TODO: may change
		if (node->rhs != NULL) {
			analyze(node->rhs, env);
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
		node->env = env;
		analyze_dec(node->lhs, env, sp_type);
		analyze(node->rhs, env);
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
		node->env = env;
		Type *type = sp_type;
		if (node->lhs != NULL) {
			type = get_ptr(node->lhs, type);
		}
		
		if (node->rhs != NULL) {
			// a[10]
			analyze(node->rhs, env);
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

	if (node->node_ty == ND_G_DECLARATOR) {
		node->env = env;
		Type *type = sp_type;
		if (node->lhs != NULL) {
			type = get_ptr(node->lhs, type);
		}
		
		if (node->rhs != NULL) {
			// a[10]
			analyze(node->rhs, env);
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


void analyze(Node *node, Env *env) {
	if (node->node_ty == ND_FUNCDEF) {
		Env *newenv = new_env(NULL);
		node->env = newenv;
		analyze(node->lhs, newenv); // int
		Type *type = node->lhs->value_ty;
		if (node->lhs->lhs != NULL) {
			type = get_ptr(node->lhs->lhs, type);
		}
		map_put(g_funcs, node->fname, 0, type);
		newenv->returntype = type;
		for (int i = 0; i < node->args->len; i++) {
			Node *tmp = vec_get(node->args, i);
			analyze(tmp, newenv);
		}
		analyze(node->rhs, newenv);
		return;
	}
	
	if (node->node_ty == ND_ARG) {
		node->env = env;
		analyze(node->lhs, env); // int
		Type *type = node->lhs->value_ty;
		if (node->lhs->lhs != NULL) {
			type = get_ptr(node->lhs->lhs, type);
		}
		map_put(node->env->variables, node->name, 0, type);
		node->value_ty = type;
		return;
	}
	
	if (node->node_ty == ND_GVARDEC) {
		analyze(node->lhs, g_env); // int
		Type *type = node->lhs->value_ty;
		if (node->lhs->lhs != NULL) {
			type = get_ptr(node->lhs->lhs, type);
		}
		analyze_dec(node->rhs, g_env, type);
		return;
	}
	
	if (node->node_ty == ND_CASE) {
		node->env = env;
		if (now_switch_node == NULL) error("case must be in switch\n");
		node->val = now_switch_node->env->cases->len; // the order of this case
		vec_push(now_switch_node->env->cases, node->rhs);
		analyze(node->lhs, env);
		return;
	}
	
	if (node->node_ty == ND_COMPOUND_STMT) {
		// update env
		Env *inner_env = new_env(env);
		vec_push(env->inner, inner_env);
		analyze(node->lhs, inner_env);
		return;
	}
	
	if (node->node_ty == ND_BLOCKITEMLIST) {
		node->env = env;
		for (int i = 0; i < node->args->len; i++) {
			analyze(vec_get(node->args, i), env);
		}
		return;
	}
	
	if (node->node_ty == ND_EXPRESSION_STMT) {
		node->env = env;
		// exclude " ;"
		if (node->lhs != NULL) {
			analyze(node->lhs, env);
		}
		return;
	}
	
	if (node->node_ty == ND_IF) {
		// update env
		Env *inner_env = new_env(env);
		vec_push(env->inner, inner_env);
		node->env = inner_env;
		
		Node *arg = vec_get(node->args, 0);
		analyze(arg, inner_env);
		analyze(node->lhs, inner_env);
		if (node->rhs != NULL) {
			// else
			analyze(node->rhs, inner_env);
		}
		return;
	}
	
	if (node->node_ty == ND_SWITCH) {
		// update env
		Env *inner_env = new_env(env);
		vec_push(env->inner, inner_env);
		node->env = inner_env;
		
		analyze(node->lhs, env);

		// switch value must be on top of the stack
		for (int i = node->env->cases->len - 1; i >= 0; i--) {
			Node *tmp = vec_get(node->env->cases, i);
			analyze(tmp, inner_env);
		}

		Node *old_switch_node = now_switch_node;
		now_switch_node = node;
		analyze(node->rhs, inner_env);
		now_switch_node = old_switch_node;
		return;
	}
	
	if (node->node_ty == ND_WHILE) {
		// update env
		Env *inner_env = new_env(env);
		vec_push(env->inner, inner_env);
		node->env = inner_env;
		
		analyze(node->lhs, inner_env);
		
		analyze(node->rhs, inner_env);
		return;
	}	
	
	if (node->node_ty == ND_FOR) {
		Env *inner_env = new_env(env);
		vec_push(env->inner, inner_env);
		node->env = inner_env;

		Node *arg = vec_get(node->args, 0);
		if (arg != NULL) analyze(arg, inner_env);
		
		arg = vec_get(node->args, 1);
		if (arg != NULL) {
			analyze(arg, inner_env);
		}
	
		analyze(node->lhs, inner_env);
		arg = vec_get(node->args, 2);
		if (arg != NULL) analyze(arg, inner_env);
		return;
	}
	
	if (node->node_ty == ND_CONTINUE) {
		node->env = env;
		return;
	}
	
	if (node->node_ty == ND_BREAK) {
		node->env = env;
		return;
	}

	if (node->node_ty == ND_RETURN) {
		// change later (support void)
		node->env = env;
		// void
		if (node->env->returntype->ty == TY_VOID) {
			if (node->lhs != NULL) {
				error("return value must be NULL\n");
			}
		} else {
			analyze(node->lhs, env);
			//printf("%d, %d\n", node->lhs->value_ty->ty, node->env->returntype->ty);
			if (node->lhs->value_ty != NULL && node->lhs->value_ty->ty != node->env->returntype->ty) {
				error("the type of return value is wrong\n");
			}
		}

		return;
	}
	
	if (node->node_ty == ND_DECLARATION) {
		node->env = env;
		analyze(node->lhs, env);
		if (node->rhs != NULL) analyze_dec(node->rhs, env, node->lhs->value_ty);
		return;
	}

	if (node->node_ty == ND_INT) {
		node->env = env;
		node->value_ty = new_type(TY_INT);
		return;
	}

	if (node->node_ty == ND_CHAR) {
		node->env = env;
		node->value_ty = new_type(TY_CHAR);
		return;
	}

	if (node->node_ty == ND_DOUBLE) {
		node->env = env;
		node->value_ty = new_type(TY_DOUBLE);
		return;
	}

	if (node->node_ty == ND_VOID) {
		node->env = env;
		node->value_ty = new_type(TY_VOID);
		return;
	}

	if (node->node_ty == ND_INITIALIZER_LIST) {
		node->env = env;
		Type *arr_type = NULL;
		for (int i = 0; i < node->args->len; i++) {
			Node *tmp = vec_get(node->args, i);
			analyze(tmp, env);
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


	if (node->node_ty == ND_EXP) {
		node->env = env;
		// a = 3, b = 3;
		analyze(node->lhs, env);
		analyze(node->rhs, env);
		return;
	}

	if (node->node_ty == '=' || node->node_ty == ND_PLUSEQ || node->node_ty == ND_MINUSEQ) {
		node->env = env;
		analyze(node->lhs, env);
		analyze(node->rhs, env);
		node->value_ty = assignment_check(node->lhs->value_ty, node->rhs->value_ty);
		return;
	}
	
	if (node->node_ty == '<' || node->node_ty == '>' || node->node_ty == ND_LEQ || node->node_ty == ND_GEQ
			|| node->node_ty == ND_EQUAL || node->node_ty == ND_NEQUAL) {
		node->env = env;
		analyze(node->lhs, env);
		analyze(node->rhs, env);
		node->value_ty = new_type(TY_INT);
	}
	
	if (node->node_ty == '+' || node->node_ty == '-') {
		node->env = env;
		analyze(node->lhs, env);
		analyze(node->rhs, env);
		node->value_ty = plus_check(node->lhs->value_ty, node->rhs->value_ty);
		return;
	}
	
	if (node->node_ty == '*' || node->node_ty == '-') {
		node->env = env;
		analyze(node->lhs, env);
		analyze(node->rhs, env);
		// TODO:need to change
		node->value_ty = new_type(TY_INT);
	}
	
	if (node->node_ty == ND_PREINC || node->node_ty == ND_PREDEC) {
		node->env = env;
		analyze(node->lhs, env);
		node->value_ty = new_type(TY_INT);
		return;
	}
	
	if (node->node_ty == ND_DEREF) {
		node->env = env;
		analyze(node->lhs, env);
		if (node->lhs->value_ty->ty == TY_INT) {
			error("illegal deref: %s\n", ((Token *)vec_get(tokens, pos))->input);
		}
		node->value_ty = node->lhs->value_ty->ptrof;
		return;
	}

	if (node->node_ty == '&') {
		node->env = env;
		analyze(node->lhs, env);
		node->value_ty = new_type(TY_PTR);
		node->value_ty->ptrof = node->lhs->value_ty;
		return;
	}
	
	if (node->node_ty == ND_SIZEOF) {
		node->env = env;
		analyze(node->lhs, env);
		if (node->lhs->value_ty->ty == TY_PTR && node->lhs->value_ty->array_size != 0) {
			node->node_ty = ND_NUM;
			node->val = node->lhs->value_ty->array_size * get_typesize(node->lhs->value_ty->ptrof);
			node->value_ty = new_type(TY_INT);
		} else {
			node->node_ty = ND_NUM;
			node->val = get_typesize(node->lhs->value_ty);
			node->value_ty = new_type(TY_INT);
		}
		return;
	}
	
	if (node->node_ty == ND_FUNC_CALL) {
		node->env = env;
		// foo(1, 2, 3)
		analyze(node->lhs, env); // ident (foo)
		node->value_ty = map_get_type(g_funcs, node->lhs->name);
		node->name = node->lhs->name;
		
		if (node->rhs != NULL) {
			// 1, 2, 3
			analyze(node->rhs, env);
		}
		return;
	}

	if (node->node_ty == ND_POSTINC || node->node_ty == ND_POSTDEC) {
		node->env = env;
		analyze(node->lhs, env);
		node->value_ty = new_type(TY_INT);
		return;
	}
	
	if (node->node_ty == ND_ARG_EXP_LIST) {
		node->env = env;
		// 1, 2, 3 (args)
		analyze(node->lhs, env);
		analyze(node->rhs, env);
		return;
	}

	
	if (node->node_ty == ND_NUM) {
		node->env = env;	
		node->value_ty = new_type(TY_INT);
		return;
	}

	if (node->node_ty == ND_IDENT) {
		node->env = env;
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
		node->env = env;
		Type *value_ty = new_type(TY_PTR);
		value_ty->ptrof = new_type(TY_CHAR);
		node->value_ty = value_ty;
		return;
	}
	
	return;
}
