#include "wottocc.h"

void program() {
	while (((Token *)vec_get(tokens, pos))->ty != TK_EOF) {
		//Map *variables = new_map();
		//vec_push(genv, variables);
		vec_push(functions, function());
		envnum++;
	}
	return;
}

Node *function() {
	Node *node;

	Type *toptype = err_read_type();
	toptype = read_ptr(toptype);

	err_consume(TK_IDENT, "It's not suitable format for function");
	char *name = ((Token *)vec_get(tokens, pos - 1))->input;
	//err_consume('(', "no left-parenthesis at declaration_list");

	if (consume('(')) {
		// function_definition
		node = new_node(ND_FUNCDEF, toptype, new_env(NULL), NULL, NULL);
		node->fname = name;
		map_put(g_funcs, name, 0, toptype);
		Vector *args = new_vector();

		// int foo(int *x, int y){ ... }
		while (1) {
			if (consume(')')) {
				break;
			}
			Type *type = err_read_type();
			type = read_ptr(type);
			err_consume(TK_IDENT, "args is not variable");

			char *vname = ((Token *)vec_get(tokens,pos-1))->input; //variable name
			map_put(node->env->variables, vname, 0, type);

			Node *arg = new_node(ND_INT, type, NULL, NULL, NULL);
			arg->name = vname;
			vec_push(args, arg);

			if (!consume(',')) {
				err_consume(')', "there's no right-parenthesis at args");
				break;
			}

		}
		node->args = args; // set of arguments of function


		node->lhs = compound_statement(node->env); // { ... }
	} else {
		// def of global variable
		node = new_node(ND_GVARDEC, NULL, NULL, NULL, NULL);
		node->name = name;
		pos -= 1; // read from the identifier
		node->lhs = init_g_declarator_list(g_env, toptype);
		err_consume(';', "no ; at def of global variable");
	}

	return node;
}

// 6.8
Node *statement(Env *env) {
	Node *node = NULL;
	if (read_nextToken(TK_CASE)) {
		node = labeled_statement(env);
	} else if (read_nextToken('{')) {
		node = compound_statement(env);
	} else if (read_nextToken(TK_IF) || read_nextToken(TK_SWITCH)) {
		node = selection_statement(env);
	} else if (read_nextToken(TK_RETURN) || read_nextToken(TK_BREAK) || read_nextToken(TK_CONTINUE)) {
		node = jump_statement(env);
	} else if (read_nextToken(TK_WHILE) || read_nextToken(TK_FOR)) {
		node = iteration_statement(env);
	} else {
		node = expression_statement(env);
	}
	return node;
}

// 6.8.1
Node *labeled_statement(Env *env) {
	Node *node = NULL;
	if (consume(TK_CASE)) {
		// case env->cases : node->lhs
		if (now_switch_node == NULL) error("case must be in switch\n");
		node = new_node(ND_CASE, NULL, env, NULL, NULL);
		node->val = now_switch_node->env->cases->len; // the order of this case
		vec_push(now_switch_node->env->cases, constant_expression(env));
		err_consume(':', "no colon at case");
		node->lhs = statement(env);
	}
	return node;
}

// 6.8.2
Node *compound_statement(Env *env) {
	// { node->lhs }
	Node *node = new_node(ND_COMPOUND_STMT, NULL, env, NULL, NULL);
	err_consume('{', "no left-brace at compound_statement");

	// update env
	Env *inner_env = new_env(env);
	vec_push(env->inner, inner_env);

	// node->lhs can't be NULL
	node->lhs = block_item_list(inner_env);

	err_consume('}', "no right-brace at compound_statement");
	return node;
}

Node *block_item_list(Env *env) {
	// node->args has the vector of block_item
	Node *node = new_node(ND_BLOCKITEMLIST, NULL, env, NULL, NULL);
	Vector *args = new_vector();
	while (1) {
		Node *arg = block_item(env);
		if (arg == NULL) break;
		else vec_push(args, arg);
	}
	node->args = args; // args can have nothing
	return node;
}

Node *block_item(Env *env) {
	Node *node = declaration(env);
	if (node == NULL){
		node = statement(env); // it can be NULL
	}
	return node;
}

// 6.8.3
Node *expression_statement(Env *env) {
	Node *node = new_node(ND_EXPRESSION_STMT, NULL, env, expression(env), NULL);
	// only ";" is the expression, but " " is illegal.
	// but here " " returns NULL for block_item_list.
	if (node->lhs == NULL) {
		if (!consume(';')) {
			return NULL;
		}
	} else {
		err_consume(';', "no ; at end of the expression_statement\n");
	}
	return node;
}

// 6.8.4
Node *selection_statement(Env *env) {
	// it's guaranteed that the next token is TK_IF or TK_SWITCH
	Node *node;
	if (consume(TK_IF)) {
		// if (node->args) node->lhs else node->rhs

		// update env
		Env *inner_env = new_env(env);
		vec_push(env->inner, inner_env);

		node = new_node(ND_IF, NULL, inner_env, NULL, NULL);
		err_consume('(', "no left-parenthesis at if");
		Vector *arg = new_vector();
		vec_push(arg, expression(inner_env));
		node->args = arg;
		err_consume(')', "no right-parenthesis at if");
		node->lhs = statement(inner_env);
		if (consume(TK_ELSE)) {
			node->rhs = statement(inner_env);
		} else {
			node->rhs = NULL;
		}
	} else if (consume(TK_SWITCH)) {
		// switch (node->lhs) node->rhs

		// update env
		Env *inner_env = new_env(env);
		vec_push(env->inner, inner_env);

		
		err_consume('(', "no left-parenthesis at switch");
		node = new_node(ND_SWITCH, NULL, inner_env, expression(env), NULL);
		Node *old_switch_node = now_switch_node;
		now_switch_node = node;
		err_consume(')', "no right-parenthesis at switch");
		node->rhs = statement(inner_env);
		now_switch_node = old_switch_node;
		
	}
	return node;
}

// 6.8.5
Node *iteration_statement(Env *env) {
	Node *node;
	if (consume(TK_WHILE)) {
		// while(node->lhs) node->rhs

		// update env
		Env *inner_env = new_env(env);
		vec_push(env->inner, inner_env);
		
		node = new_node(ND_WHILE, NULL, inner_env, NULL, NULL);
		err_consume('(', "no left-parenthesis at while");
		node->lhs = expression(inner_env);
		err_consume(')', "no right-parenthesis at while");
		node->rhs = statement(inner_env);

	} else if (consume(TK_FOR)) {
		Env *for_env = new_env(env);
		vec_push(env->inner, for_env);
		node = new_node(ND_FOR, NULL, for_env, NULL, NULL);
		err_consume('(', "no left-parenthesis at for");
		Vector *arg = new_vector();

		Node *tmp = expression(for_env);
		if (tmp != NULL) { 
			vec_push(arg, tmp);
			err_consume(';', "no ';' at for");
		} else { 
			tmp = declaration(for_env);
			if (tmp != NULL) {
				vec_push(arg, tmp);
			} else {
				err_consume(';', "no ';' at for");
				vec_push(arg, NULL);
			}
		}
		vec_push(arg, expression(for_env));
		err_consume(';', "no ';' at while");
		vec_push(arg, expression(for_env));
		node->args = arg;
		err_consume(')', "no right-parenthesis at for");
		node->lhs = statement(for_env);
	}
	return node;
}

// 6.8.6
Node *jump_statement(Env *env) {
	Node *node = NULL;
	if (consume(TK_CONTINUE)) {
		node = new_node(ND_CONTINUE, NULL, env, NULL, NULL);
		err_consume(';', "no ';' at continue");
	} else if (consume(TK_BREAK)) {
		node = new_node(ND_BREAK, NULL, env, NULL, NULL);
		err_consume(';', "no ';' at break");
	} else if (consume(TK_RETURN)) {
		node = new_node(ND_RETURN, NULL, env, expression(env), NULL);
		err_consume(';', "no ';' at return");
	}
	return node;
}


// 6.7
Node *declaration(Env *env) {
	Node *node = NULL;
	// allow NULL
	Type *type = read_type();
	if (type != NULL) {
		node = new_node(ND_DECLARATION, NULL, env, NULL, NULL);

		// allow "int;"
		if (read_nextToken(';') == 0) {
			node->lhs = init_declarator_list(env, type);
		}

		err_consume(';', "no ';' at declaration");
	}
	return node;
}

Node *init_declarator_list(Env *env, Type *sp_type) {
	Node *node = init_declarator(env, sp_type);

	for (;;) {
		if (consume(',')) {
			node = new_node(ND_INIT_DECLARATOR_LIST, NULL, env, node, init_declarator(env, sp_type));
		} else {
			return node;
		}
	}
}	

Node *init_g_declarator_list(Env *env, Type *sp_type) {
	Node *node = init_g_declarator(env, sp_type);
	for (;;) {
		if (consume(',')) {
			node = new_node(ND_INIT_G_DECLARATOR_LIST, NULL, env, node,init_g_declarator(env, sp_type));
		} else {
			return node;
		}
	}
}

Node *init_declarator(Env *env, Type *sp_type) {
	Node *node = declarator(env, sp_type);

	if (consume('=')) {
		Node *rhs = initializer(env);
		if (rhs->node_ty != ND_INITIALIZER_LIST) {
			assignment_check(node->value_ty, rhs->value_ty);
			node = new_node(ND_INIT_DECLARATOR, node->value_ty, env, node, rhs);
		} else {
			// a[3] = {1,2,3};
			if (node->value_ty->ty != TY_ARRAY) {
				error("lhs must be array at init_declarator\n");
			}
			if (node->value_ty->array_size < rhs->value_ty->array_size) {
				error("too much initializer\n");
			}
			assignment_check(node->value_ty->ptrof, rhs->value_ty->ptrof);
			node = new_node(ND_INIT_DECLARATOR, node->value_ty, env, node, rhs);
		}
	}

	return node;
}

Node *init_g_declarator(Env *env, Type *sp_type) {
	Node *node = g_declarator(env, sp_type);

	if (consume('=')) {
		if (sp_type->ty == TY_INT || sp_type->ty == TY_CHAR) {
			err_consume(TK_NUM, "initialization must be number\n");
			Node *rhs = new_node_num(((Token *)vec_get(tokens, pos-1))->val, env);
			node = new_node(ND_INIT_G_DECLARATOR, sp_type, env, node, rhs);
		}
	}

	return node;
}

// 6.7.6
Node *declarator(Env *env, Type *sp_type) {
	Type *type = sp_type;
	type = read_ptr(type);

	err_consume(TK_IDENT, "no identifier at declarator");
	char *vname = ((Token *)vec_get(tokens,pos-1))->input;	

	if (consume('[')) {
		err_consume(TK_NUM, "array initializer must be number");
		Type *newtype = new_type(TY_ARRAY);
		newtype->ptrof = type;
		newtype->array_size = ((Token *)vec_get(tokens,pos-1))->val;
		type = newtype;
		err_consume(']', "no ']' at array-def");
	}
	
	map_put(env->variables, vname, 0, type);
	
	Node *node = new_node_ident(vname, env);
	node->value_ty = type;
	
	return node;
}

Node *g_declarator(Env *env, Type *sp_type) {
	Type *type = sp_type;
	type = read_ptr(type);

	err_consume(TK_IDENT, "no identifier at declarator");
	char *vname = ((Token *)vec_get(tokens,pos-1))->input;	

	if (consume('[')) {
		err_consume(TK_NUM, "array initializer must be number");
		Type *newtype = new_type(TY_ARRAY);
		newtype->ptrof = type;
		newtype->array_size = ((Token *)vec_get(tokens,pos-1))->val;
		type = newtype;
		err_consume(']', "no ']' at array-def");
	}
	
	map_put(env->variables, vname, 0, type);
	
	Node *node = new_node(ND_G_DECLARATOR, type, env, NULL, NULL);
	node->name = vname;
	return node;
}

// 6.7.9
Node *initializer(Env *env) {
	Node *node;
	if (consume('{')) {
		Vector *args = new_vector();
		Type *arr_type = NULL;
		size_t arr_size = 0;
		while (!consume('}')) {
			Node *node = assignment_expression(env);
			if (arr_type == NULL) arr_type = node->value_ty;
			if (arr_type->ty != node->value_ty->ty){
				error("different array initializer type\n");
			}
			vec_push(args, node);
			arr_size++;
			if (consume('}')) break;
			err_consume(',', "no comma at array initialzation\n");
		}
		Type *type = new_type(TY_ARRAY);
		type->ptrof = arr_type;
		type->array_size = arr_size;
		node = new_node(ND_INITIALIZER_LIST, type, env, NULL, NULL);
		node->args = args;
	} else {
		node = assignment_expression(env);
	}
	return node;
}


Node *expression(Env *env) {
	Node *node = assignment_expression(env);

	for (;;) {
		if (consume(',')) {
			node = new_node(ND_EXP, NULL, env, node, assignment_expression(env));
		} else {
			return node;
		}
	}
}


Node *assignment_expression(Env *env) {
	Node *node = conditional_expression(env); // analyzeで=の場合は左辺がunary_expressionであることを確認
	//Node *node = NULL;

	if (consume('=')){
		Node *lhs = node;
		Node *rhs = assignment_expression(env);
		Type *value_ty;
		value_ty = assignment_check(lhs->value_ty, rhs->value_ty);
		node = new_node('=', value_ty, env, lhs, rhs);
	} else if (consume(TK_PLUSEQ)) {
		Node *lhs = node;
		Node *rhs = assignment_expression(env);
		Type *value_ty;
		value_ty = assignment_check(lhs->value_ty, rhs->value_ty);
		node = new_node(ND_PLUSEQ, value_ty, env, lhs, rhs);
	} else if (consume(TK_MINUSEQ)) {
		Node *lhs = node;
		Node *rhs = assignment_expression(env);
		Type *value_ty;
		value_ty = assignment_check(lhs->value_ty, rhs->value_ty);
		node = new_node(ND_MINUSEQ, value_ty, env, lhs, rhs);
	}
	return node;
}

Node *conditional_expression(Env *env) {
	Node *node = logical_OR_expression(env);

	return node;
}

Node *logical_OR_expression(Env *env) {
	Node *node = logical_AND_expression(env);

	return node;
}

Node *logical_AND_expression(Env *env) {
	Node *node = inclusive_OR_expression(env);

	return node;
}

Node *inclusive_OR_expression(Env *env) {
	Node *node = exclusive_OR_expression(env);

	return node;
}

Node *exclusive_OR_expression(Env *env) {
	Node *node = AND_expression(env);

	return node;
}

Node *AND_expression(Env *env) {
	Node *node = equality_expression(env);

	return node;
}

Node *equality_expression(Env *env) {
	Node *node = relational_expression(env);

	for (;;) {
		if (consume(TK_EQUAL)) {
			node = new_node(ND_EQUAL, new_type(TY_INT), env, node, relational_expression(env));
		} else if (consume(TK_NEQUAL)) {
			node = new_node(ND_NEQUAL, new_type(TY_INT), env, node, relational_expression(env));
		} else {
			return node;
		}
	}
}

Node *relational_expression(Env *env) {
	Node *node = shift_expression(env);
	
	for (;;) {
		if (consume('<'))
			node = new_node('<', new_type(TY_INT), env, node, shift_expression(env));
		else if (consume('>'))
			node = new_node('>', new_type(TY_INT), env, node, shift_expression(env));
		else if (consume(TK_LEQ))
			node = new_node(ND_LEQ, new_type(TY_INT), env, node, shift_expression(env));
		else if (consume(TK_GEQ))
			node = new_node(ND_GEQ, new_type(TY_INT), env, node, shift_expression(env));
		else 
			return node;
	}
}

Node *shift_expression(Env *env) {
	Node *node = additive_expression(env);
	return node;
}

Node *additive_expression(Env *env) {
	Node *node = multiplicative_expression(env);

	for (;;) {
		if (consume('+')){
			Type *value_ty;
			Node *lhs = node;
			Node *rhs = multiplicative_expression(env);
			value_ty = plus_check(lhs->value_ty, rhs->value_ty);
			node = new_node('+', value_ty, env, lhs, rhs);
		} else if (consume('-')) {
			Node *lhs = node;
			Node *rhs = multiplicative_expression(env);
			Type *value_ty;
			value_ty = plus_check(lhs->value_ty, rhs->value_ty);
			node = new_node('-', value_ty, env, lhs, rhs);
		} else {
			return node;
		}
	}
}

Node *multiplicative_expression(Env *env) {
	Node *node = cast_expression(env);

	for (;;) {
		if (consume('*'))
			node = new_node('*', new_type(TY_INT), env, node, cast_expression(env));
		else if (consume('/'))
			node = new_node('/', new_type(TY_INT), env, node, cast_expression(env));
		else
			return node;
	}
}

Node *cast_expression(Env *env) {
	Node *node = unary_expression(env);
	return node;
}

Node *unary_expression(Env *env) {
	Node *node = NULL;

	if (consume(TK_INC)) {
		node = new_node(ND_PREINC, new_type(TY_INT), env, unary_expression(env), NULL);
	} else if (consume(TK_DEC)) {
		node = new_node(ND_PREDEC, new_type(TY_INT), env, unary_expression(env), NULL);
	} else if (consume('*')) {
		Node *lhs = cast_expression(env);
		if (lhs->value_ty->ty == TY_INT) {
			error("illegal deref: %s\n", ((Token *)vec_get(tokens, pos))->input);
		}
		node = new_node(ND_DEREF, new_type(lhs->value_ty->ptrof->ty), env, lhs, NULL);
	} else if (consume('&')) {
		node = new_node('&', new_type(TY_PTR), env, cast_expression(env), NULL);
	} else if (consume(TK_SIZEOF)) {
		// sizeof(a)
		node = unary_expression(env);
		if (node->value_ty->ty == TY_PTR && node->value_ty->array_size != 0) {
			node = new_node_num(node->value_ty->array_size * get_typesize(node->value_ty->ptrof), env);
		} else {
			node = new_node_num(get_typesize(node->value_ty), env);
		}
		return node;

	} else {
		node = postfix_expression(env);
	}
	return node;
}

Node *postfix_expression(Env *env) {
	Node *node = primary_expression(env);

	for(;;) {
		if (consume('[')) {
			// a[3] -> *(a + 3)
			Node *rhs = expression(env);
			
			Node *plus = new_node('+', new_type(TY_PTR), env, node, rhs);
			node = new_node(ND_DEREF, new_type(node->value_ty->ptrof->ty), env, plus, NULL);
			err_consume(']', "no right_braket at array");
		} else if (consume('(')) {
			// foo(1 ,2)
			Type *type = map_get_type(g_funcs, node->name);
			node = new_node(ND_FUNC_CALL, type, env, node, argument_expression_list(env));
			err_consume(')', "no right-parenthesis at func_call");
		} else if (consume(TK_INC)) {
			node = new_node(ND_POSTINC, new_type(TY_INT), env, node, NULL);
		} else if (consume(TK_DEC)) {
			node = new_node(ND_POSTDEC, new_type(TY_INT), env, node, NULL);
		} else {
			return node;
		}
	}
}

Node *argument_expression_list(Env *env) {
	// 1, 2, 3
	Node *node = assignment_expression(env);
	int length = 1;
	if (node == NULL) return NULL;
	node->length = length;
	for (;;) {
		if (consume(',')) {
			node = new_node(ND_ARG_EXP_LIST, NULL, env, node, assignment_expression(env));
			node->length = ++length;
		} else {
			return node;
		}
	}
}


Node *primary_expression(Env *env) {
	if (consume('(')) {
		Node *node = expression(env);
		err_consume(')', "there isn't right-parenthesis at primary_expression");
		return node;
	}

	char *t_name;
			
	switch (((Token *)vec_get(tokens, pos))->ty){
	case TK_NUM:
		return new_node_num(((Token *)vec_get(tokens, pos++))->val, env);
		break;
	case TK_IDENT:
		t_name = ((Token *)vec_get(tokens, pos++))->input;
		Node *node = new_node_ident(t_name, env);
		return node;
		break;
	}
	
	return NULL;
}

Node *constant_expression(Env *env) {
	Node *node = conditional_expression(env);
	// TODO: need to change for analyze
	return node;
}
