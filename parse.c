#include "wottocc.h"

void program() {
	while (((Token *)vec_get(tokens, pos))->ty != TK_EOF) {
		vec_push(toplevels, toplevel());
		envnum++;
	}
	return;
}

Node *toplevel() {
	Node *node;

	Node *toptype = err_read_type();
	Node *topptr = read_ptr();
	toptype->lhs = topptr;

	err_consume(TK_IDENT, "It's not suitable format for function");
	char *name = ((Token *)vec_get(tokens, pos - 1))->input;
	//err_consume('(', "no left-parenthesis at declaration_list");

	if (consume('(')) {
		// function_definition
		//node = new_node(ND_FUNCDEF, NULL, new_env(NULL), toptype, NULL);
		node = new_node(ND_FUNCDEF, NULL, NULL, toptype, NULL);
		node->fname = name;
		//map_put(g_funcs, name, 0, toptype);
		Vector *args = new_vector();

		// int foo(int *x, int y){ ... }
		while (1) {
			if (consume(')')) {
				break;
			}
			Node *type = err_read_type();
			Node *typeptr = read_ptr();
			type->lhs = typeptr;
			err_consume(TK_IDENT, "args is not variable");

			char *vname = ((Token *)vec_get(tokens,pos-1))->input; //variable name
			//map_put(node->env->variables, vname, 0, type);

			Node *arg = new_node(ND_ARG, NULL, NULL, type, NULL);
			arg->name = vname;
			vec_push(args, arg);

			if (!consume(',')) {
				err_consume(')', "there's no right-parenthesis at args");
				break;
			}

		}
		node->args = args; // set of arguments of function
		node->rhs = compound_statement(); // { ... }
	} else {
		// def of global variable
		node = new_node(ND_GVARDEC, NULL, NULL, toptype, NULL);
		node->name = name;
		pos -= 1; // read from the identifier
		//node->lhs = init_g_declarator_list(g_env, toptype);
		node->rhs = init_g_declarator_list();
		err_consume(';', "no ; at def of global variable");
	}

	return node;
}

// 6.8
Node *statement() {
	Node *node = NULL;
	if (read_nextToken(TK_CASE)) {
		node = labeled_statement();
	} else if (read_nextToken('{')) {
		node = compound_statement();
	} else if (read_nextToken(TK_IF) || read_nextToken(TK_SWITCH)) {
		node = selection_statement();
	} else if (read_nextToken(TK_RETURN) || read_nextToken(TK_BREAK) || read_nextToken(TK_CONTINUE)) {
		node = jump_statement();
	} else if (read_nextToken(TK_WHILE) || read_nextToken(TK_FOR)) {
		node = iteration_statement();
	} else {
		node = expression_statement();
	}
	return node;
}

// 6.8.1
Node *labeled_statement() {
	Node *node = NULL;
	if (consume(TK_CASE)) {
		// case env->cases : node->lhs
		node = new_node(ND_CASE, NULL, NULL, NULL, constant_expression());
		err_consume(':', "no colon at case");
		node->lhs = statement();
	}
	return node;
}

// 6.8.2
Node *compound_statement() {
	// { node->lhs }
	Node *node = new_node(ND_COMPOUND_STMT, NULL, NULL, NULL, NULL);
	err_consume('{', "no left-brace at compound_statement");

	// node->lhs can't be NULL
	node->lhs = block_item_list();

	err_consume('}', "no right-brace at compound_statement");
	return node;
}

Node *block_item_list() {
	// node->args has the vector of block_item
	Node *node = new_node(ND_BLOCKITEMLIST, NULL, NULL, NULL, NULL);
	Vector *args = new_vector();
	while (1) {
		Node *arg = block_item();
		if (arg == NULL) break;
		else vec_push(args, arg);
	}
	node->args = args; // args can have nothing
	return node;
}

Node *block_item() {
	Node *node = declaration();
	if (node == NULL){
		node = statement(); // it can be NULL
	}
	return node;
}

// 6.8.3
Node *expression_statement() {
	Node *node = new_node(ND_EXPRESSION_STMT, NULL, NULL, expression(), NULL);
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
Node *selection_statement() {
	// it's guaranteed that the next token is TK_IF or TK_SWITCH
	Node *node;
	if (consume(TK_IF)) {
		// if (node->args) node->lhs else node->rhs


		node = new_node(ND_IF, NULL, NULL, NULL, NULL);
		err_consume('(', "no left-parenthesis at if");
		Vector *arg = new_vector();
		vec_push(arg, expression());
		node->args = arg;
		err_consume(')', "no right-parenthesis at if");
		node->lhs = statement();
		if (consume(TK_ELSE)) {
			node->rhs = statement();
		} else {
			node->rhs = NULL;
		}
	} else if (consume(TK_SWITCH)) {
		// switch (node->lhs) node->rhs
		
		err_consume('(', "no left-parenthesis at switch");
		node = new_node(ND_SWITCH, NULL, NULL, expression(), NULL);
		err_consume(')', "no right-parenthesis at switch");
		node->rhs = statement();
	}
	return node;
}

// 6.8.5
Node *iteration_statement() {
	Node *node;
	if (consume(TK_WHILE)) {
		// while(node->lhs) node->rhs
		
		node = new_node(ND_WHILE, NULL, NULL, NULL, NULL);
		err_consume('(', "no left-parenthesis at while");
		node->lhs = expression();
		err_consume(')', "no right-parenthesis at while");
		node->rhs = statement();

	} else if (consume(TK_FOR)) {
		node = new_node(ND_FOR, NULL, NULL, NULL, NULL);
		err_consume('(', "no left-parenthesis at for");
		Vector *arg = new_vector();

		Node *tmp = expression();
		if (tmp != NULL) { 
			vec_push(arg, tmp);
			err_consume(';', "no ';' at for");
		} else { 
			tmp = declaration();
			if (tmp != NULL) {
				vec_push(arg, tmp);
			} else {
				err_consume(';', "no ';' at for");
				vec_push(arg, NULL);
			}
		}
		vec_push(arg, expression());
		err_consume(';', "no ';' at while");
		vec_push(arg, expression());
		node->args = arg;
		err_consume(')', "no right-parenthesis at for");
		node->lhs = statement();
	}
	return node;
}

// 6.8.6
Node *jump_statement() {
	Node *node = NULL;
	if (consume(TK_CONTINUE)) {
		node = new_node(ND_CONTINUE, NULL, NULL, NULL, NULL);
		err_consume(';', "no ';' at continue");
	} else if (consume(TK_BREAK)) {
		node = new_node(ND_BREAK, NULL, NULL, NULL, NULL);
		err_consume(';', "no ';' at break");
	} else if (consume(TK_RETURN)) {
		node = new_node(ND_RETURN, NULL, NULL, expression(), NULL);
		err_consume(';', "no ';' at return");
	}
	return node;
}


// 6.7
Node *declaration() {
	Node *node = NULL;
	// allow NULL
	Node *lhs = read_type();
	if (lhs != NULL) {
		node = new_node(ND_DECLARATION, NULL, NULL, lhs, NULL);

		// allow "int;"
		if (read_nextToken(';') == 0) {
			node->rhs = init_declarator_list();
		}

		err_consume(';', "no ';' at declaration");
	}
	return node;
}

Node *init_declarator_list() {
	Node *node = init_declarator();

	for (;;) {
		if (consume(',')) {
			node = new_node(ND_INIT_DECLARATOR_LIST, NULL, NULL, node, init_declarator());
		} else {
			return node;
		}
	}
}	

Node *init_g_declarator_list() {
	Node *node = init_g_declarator();
	for (;;) {
		if (consume(',')) {
			node = new_node(ND_INIT_G_DECLARATOR_LIST, NULL, NULL, node,init_g_declarator());
		} else {
			return node;
		}
	}
}

Node *init_declarator() {
	Node *node = declarator();

	if (consume('=')) {
		Node *rhs = initializer();
		node = new_node(ND_INIT_DECLARATOR, NULL, NULL, node, rhs);
	}

	return node;
}

Node *init_g_declarator() {
	Node *node = g_declarator();

	if (consume('=')) {
		// これだと変数も初期化に認めてるので本当はよくない
		// いい感じにanalyzeしたいけどとりあえずスルー
		Node *rhs = initializer();
		node = new_node(ND_INIT_G_DECLARATOR, NULL, NULL, node, rhs);
	}

	return node;
}

// 6.7.6
Node *declarator() {
	Node *lhs = read_ptr();

	err_consume(TK_IDENT, "no identifier at declarator");
	char *vname = ((Token *)vec_get(tokens,pos-1))->input;	

	Node *rhs = NULL;

	if (consume('[')) {
		rhs = primary_expression();
		err_consume(']', "no ']' at array-def");
	}
	
	Node *node = new_node(ND_DECLARATOR, NULL, NULL, lhs, rhs);
	node->name = vname;
	
	return node;
}

Node *g_declarator() {
	Node *lhs = read_ptr();

	err_consume(TK_IDENT, "no identifier at declarator");
	char *vname = ((Token *)vec_get(tokens,pos-1))->input;	

	Node *rhs = NULL;

	if (consume('[')) {
		rhs = primary_expression();
		err_consume(']', "no ']' at array-def");
	}
	Node *node = new_node(ND_G_DECLARATOR, NULL, NULL, lhs, rhs);
	node->name = vname;
	return node;
}

// 6.7.9
Node *initializer() {
	Node *node;
	if (consume('{')) {
		Vector *args = new_vector();
		while (!consume('}')) {
			Node *node = assignment_expression();
			vec_push(args, node);
			if (consume('}')) break;
			err_consume(',', "no comma at array initialzation\n");
		}
		node = new_node(ND_INITIALIZER_LIST, NULL, NULL, NULL, NULL);
		node->args = args;
	} else {
		node = assignment_expression();
	}
	return node;
}


Node *expression() {
	Node *node = assignment_expression();

	for (;;) {
		if (consume(',')) {
			node = new_node(ND_EXP, NULL, NULL, node, assignment_expression());
		} else {
			return node;
		}
	}
}


Node *assignment_expression() {
	Node *node = conditional_expression(); // analyzeで=の場合は左辺がunary_expressionであることを確認

	if (consume('=')){
		Node *lhs = node;
		Node *rhs = assignment_expression();
		node = new_node('=', NULL, NULL, lhs, rhs);
	} else if (consume(TK_PLUSEQ)) {
		Node *lhs = node;
		Node *rhs = assignment_expression();
		node = new_node(ND_PLUSEQ, NULL, NULL, lhs, rhs);
	} else if (consume(TK_MINUSEQ)) {
		Node *lhs = node;
		Node *rhs = assignment_expression();
		node = new_node(ND_MINUSEQ, NULL, NULL, lhs, rhs);
	}
	return node;
}

Node *conditional_expression() {
	Node *node = logical_OR_expression();

	return node;
}

Node *logical_OR_expression() {
	Node *node = logical_AND_expression();

	return node;
}

Node *logical_AND_expression() {
	Node *node = inclusive_OR_expression();

	return node;
}

Node *inclusive_OR_expression() {
	Node *node = exclusive_OR_expression();

	return node;
}

Node *exclusive_OR_expression() {
	Node *node = AND_expression();

	return node;
}

Node *AND_expression() {
	Node *node = equality_expression();

	return node;
}

Node *equality_expression() {
	Node *node = relational_expression();

	for (;;) {
		if (consume(TK_EQUAL)) {
			node = new_node(ND_EQUAL, NULL, NULL, node, relational_expression());
		} else if (consume(TK_NEQUAL)) {
			node = new_node(ND_NEQUAL, NULL, NULL, node, relational_expression());
		} else {
			return node;
		}
	}
}

Node *relational_expression() {
	Node *node = shift_expression();
	
	for (;;) {
		if (consume('<'))
			node = new_node('<', NULL, NULL, node, shift_expression());
		else if (consume('>'))
			node = new_node('>', NULL, NULL, node, shift_expression());
		else if (consume(TK_LEQ))
			node = new_node(ND_LEQ, NULL, NULL, node, shift_expression());
		else if (consume(TK_GEQ))
			node = new_node(ND_GEQ, NULL, NULL, node, shift_expression());
		else 
			return node;
	}
}

Node *shift_expression() {
	Node *node = additive_expression();
	return node;
}

Node *additive_expression() {
	Node *node = multiplicative_expression();

	for (;;) {
		if (consume('+')){
			Node *lhs = node;
			Node *rhs = multiplicative_expression();
			node = new_node('+', NULL, NULL, lhs, rhs);
		} else if (consume('-')) {
			Node *lhs = node;
			Node *rhs = multiplicative_expression();
			node = new_node('-', NULL, NULL, lhs, rhs);
		} else {
			return node;
		}
	}
}

Node *multiplicative_expression() {
	Node *node = cast_expression();

	for (;;) {
		if (consume('*'))
			node = new_node('*', NULL, NULL, node, cast_expression());
		else if (consume('/'))
			node = new_node('/', NULL, NULL, node, cast_expression());
		else
			return node;
	}
}

Node *cast_expression() {
	Node *node = unary_expression();
	return node;
}

Node *unary_expression() {
	Node *node = NULL;

	if (consume(TK_INC)) {
		node = new_node(ND_PREINC, NULL, NULL, unary_expression(), NULL);
	} else if (consume(TK_DEC)) {
		node = new_node(ND_PREDEC, NULL, NULL, unary_expression(), NULL);
	} else if (consume('*')) {
		node = new_node(ND_DEREF, NULL, NULL, cast_expression(), NULL);
	} else if (consume('&')) {
		node = new_node('&', NULL, NULL, cast_expression(), NULL);
	} else if (consume(TK_SIZEOF)) {
		// sizeof(a)
		node = new_node(ND_SIZEOF, NULL, NULL, unary_expression(), NULL);
	} else {
		node = postfix_expression();
	}
	return node;
}

Node *postfix_expression() {
	Node *node = primary_expression();

	for(;;) {
		if (consume('[')) {
			// a[3] -> *(a + 3)
			Node *rhs = expression();
			
			//Node *plus = new_node('+', new_type(TY_PTR), env, node, rhs);
			Node *plus = new_node('+', NULL, NULL, node, rhs);
			//node = new_node(ND_DEREF, new_type(node->value_ty->ptrof->ty), env, plus, NULL);
			node = new_node(ND_DEREF, NULL, NULL, plus, NULL);
			err_consume(']', "no right_braket at array");
		} else if (consume('(')) {
			// foo(1 ,2)
			node = new_node(ND_FUNC_CALL, NULL, NULL, node, argument_expression_list());
			err_consume(')', "no right-parenthesis at func_call");
		} else if (consume(TK_INC)) {
			node = new_node(ND_POSTINC, NULL, NULL, node, NULL);
		} else if (consume(TK_DEC)) {
			node = new_node(ND_POSTDEC, NULL, NULL, node, NULL);
		} else {
			return node;
		}
	}
}

Node *argument_expression_list() {
	// 1, 2, 3
	Node *node = assignment_expression();
	int length = 1;
	if (node == NULL) return NULL;
	node->length = length;
	for (;;) {
		if (consume(',')) {
			node = new_node(ND_ARG_EXP_LIST, NULL, NULL, node, assignment_expression());
			node->length = ++length;
		} else {
			return node;
		}
	}
}

Node *primary_expression() {
	if (consume('(')) {
		Node *node = expression();
		err_consume(')', "there isn't right-parenthesis at primary_expression");
		return node;
	}

	char *t_name;
	Node *node;
			
	switch (((Token *)vec_get(tokens, pos))->ty){
	case TK_NUM:
		return new_node_num(((Token *)vec_get(tokens, pos++))->val);
		break;
	case TK_IDENT:
		t_name = ((Token *)vec_get(tokens, pos++))->input;
		//node = new_node_ident(t_name, env);
		node = new_node(ND_IDENT, NULL, NULL, NULL, NULL);
		node->name = t_name;
		return node;
		break;
	case TK_STR:
		t_name = ((Token *)vec_get(tokens, pos))->input;
		int str_len = ((Token *)vec_get(tokens, pos++))->val;
		node = new_node_string(t_name, str_len);
		return node;
		break;
	}

	return NULL;
}

Node *constant_expression() {
	Node *node = conditional_expression();
	// TODO: need to change for analyze
	return node;
}
