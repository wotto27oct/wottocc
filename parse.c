#include "wottocc.h"

void program() {
	while (((Token *)vec_get(tokens, pos))->ty != TK_EOF) {
		Map *variables = new_map();
		vec_push(env, variables);
		vec_push(functions, function());
		envnum++;
	}
	return;
}

Node *function() {
	Node *node;

	err_consume(TK_INT, "It's not suitable format for function");
	err_consume(TK_IDENT, "It's not suitable format for function");
	err_consume('(', "It's not suitable format for function");

	node = new_node(ND_FUNCDEF, NULL, NULL, NULL);
	node->fname = ((Token *)vec_get(tokens, pos-2))->input;
	Vector *args = new_vector();
	// int foo(int *x, int y){ ... }
	Map *variables = vec_get(env, envnum);
	while (1) {
		if (consume(')')) {
			break;
		}
		err_consume(TK_INT, "no type at args");
		Type *type = new_type(TY_INT);

		while (consume('*')) {
			Type *newtype = new_type(TY_PTR);
			newtype->ptrof = type;
			type = newtype;
		}
		err_consume(TK_IDENT, "args is not variable");

		char *vname = ((Token *)vec_get(tokens,pos-1))->input;	
		map_put(variables, vname, 0, type);

		Node *arg = new_node(ND_INT, type, NULL, NULL);
		arg->name = vname;
		vec_push(args, arg);

		if (!consume(',')) {
			err_consume(')', "there's no right-parenthesis at args");
			break;
		}

	}
	node->args = args;
	node->lhs = compound_statement();
	/*consume('{');
	node->stmts = new_vector();
	while (!consume('}')) {
		vec_push(node->stmts, statement());
	}*/
	return node;
}

Node *compound_statement() {
	Node *node = new_node(ND_COMPOUND_STMT, NULL, NULL, NULL);
	err_consume('{', "no left-brace at compound_statement");
	if(!consume('}')){
		node->lhs = block_item_list();
	}
	err_consume('}', "no right-brace at compound_statement");
	return node;
}

Node *block_item_list() {
	Node *node = new_node(ND_BLOCKITEMLIST, NULL, NULL, NULL);
	Vector *args = new_vector();
	while (1) {
		Node *arg = block_item();
		if (arg == NULL) break;
		else vec_push(args, arg);
	}
	node->args = args;
	return node;
}

Node *block_item() {
	Node *node = declaration();
	if (node == NULL){
		node = statement();
	}
	return node;
}

Node *declaration() {
	Node *node = NULL;
	if (consume(TK_INT)) {
		Type *type = new_type(TY_INT); 

		while (consume('*')) {
			Type *newtype = new_type(TY_PTR);
			newtype->ptrof = type;
			type = newtype;
		}

		err_consume(TK_IDENT, "it's not suitable format for declaration");
		char *vname = ((Token *)vec_get(tokens,pos-1))->input;	

		if (consume('[')) {
			err_consume(TK_NUM, "array initializer must be number");
			Type *newtype = new_type(TY_ARRAY);
			newtype->ptrof = type;
			newtype->array_size = ((Token *)vec_get(tokens,pos-1))->val;
			type = newtype;
			err_consume(']', "no ']' at array-def");
		}
		
		Map *variables = vec_get(env, envnum);
		map_put(variables, vname, 0, type);
		
		node = new_node(ND_INT, type, NULL, NULL);
		node->name = vname;

		err_consume(';', "no ';' at declaration");
	}
	return node;
}


Node *statement() {
	Node *node = NULL;

	if (read_nextToken(TK_RETURN)) {
		node = jump_statement();
	} else if (read_nextToken(TK_IF)) {
		node = selection_statement();
	} else if (read_nextToken(TK_WHILE) || read_nextToken(TK_FOR)) {
		node = iteration_statement();
	} else if (read_nextToken('{')) {
		node = compound_statement();
	} else {
		node = expression_statement();
	}
	return node;
}

Node *jump_statement() {
	consume(TK_RETURN);
	Node *node = new_node(ND_RETURN, NULL, assign(), NULL);
	err_consume(';', "no ';' at return");
	return node;
}

Node *expression_statement() {
	Node *node = assign();
	if (node != NULL) err_consume(';', "no ';' at end of the expression_statement\n");
	return node;
}

Node *selection_statement() {
	Node *node;
	if (consume(TK_IF)) {
		node = new_node(ND_IF, NULL, NULL, NULL);
		err_consume('(', "no left-parenthesis at if");
		Vector *arg = new_vector();
		vec_push(arg, assign());
		node->args = arg;
		err_consume(')', "no right-parenthesis at if");
		node->lhs = statement();
		if (consume(TK_ELSE)) {
			node->rhs = statement();
		} else {
			node->rhs = NULL;
		}
	}
	return node;
}

Node *iteration_statement() {
	Node *node;
	if (consume(TK_WHILE)) {
		node = new_node(ND_WHILE, NULL, NULL, NULL);
		err_consume('(', "no left-parenthesis at while");
		Vector *arg = new_vector();
		vec_push(arg, assign());
		node->args = arg;
		err_consume(')', "no right-parenthesis at while");
		node->lhs = statement();

	} else if (consume(TK_FOR)) {
		node = new_node(ND_FOR, NULL, NULL, NULL);
		err_consume('(', "no left-parenthesis at for");
		Vector *arg = new_vector();
		vec_push(arg, assign());
		err_consume(';', "no ';' at while");
		vec_push(arg, equal());
		err_consume(';', "no ';' at while");
		vec_push(arg, assign());
		node->args = arg;
		err_consume(')', "no right-parenthesis at for");
		node->lhs = statement();
	}
	return node;
}


Node *assign() {
	Node *node = equal();

	for (;;) {
		if (consume('=')){
			Node *lhs = node;
			Node *rhs = assign();
			Type *value_ty;
			if (lhs->value_ty->ty == TY_INT){
				if (rhs->value_ty->ty != TY_INT) {
					error("substitution from ptr to int: %s\n", ((Token *)vec_get(tokens, pos++))->input);
				}
				value_ty = new_type(TY_INT);
			} else {
				// ptr or array
				if (rhs->value_ty->ty == TY_INT) {
					error("substitution from int to ptr: %s\n", ((Token *)vec_get(tokens, pos++))->input);
				}
				value_ty = rhs->value_ty;
			}
			node = new_node('=', value_ty, lhs, rhs);
		} else {
			return node;
		}
	}
}

Node *equal() {
	Node *node = compare();

	for (;;) {
		if (consume(TK_EQUAL))
			node = new_node(ND_EQUAL, new_type(TY_INT), node, equal());
		else if (consume(TK_NEQUAL))
			node = new_node(ND_NEQUAL, new_type(TY_INT), node, equal());
		else
			return node;
	}
}

Node *compare() {
	Node *node = add();
	
	for (;;) {
		if (consume('<'))
			node = new_node('<', new_type(TY_INT), node, add());
		else if (consume('>'))
			node = new_node('>', new_type(TY_INT), node, add());
		else if (consume(TK_LEQ))
			node = new_node(ND_LEQ, new_type(TY_INT), node, add());
		else if (consume(TK_GEQ))
			node = new_node(ND_GEQ, new_type(TY_INT), node, add());
		else 
			return node;
	}
}

Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume('+')){
			Type *value_ty;
			Node *lhs = node;
			Node *rhs = mul();
			if (lhs->value_ty->ty != TY_INT || rhs->value_ty->ty != TY_INT) {
				// ptr
				if (lhs->value_ty->ty != TY_INT)
					value_ty = lhs->value_ty;
				else 
					value_ty = rhs->value_ty;
			} else {
				value_ty = new_type(TY_INT);
			}
			node = new_node('+', value_ty, lhs, rhs);
		} else if (consume('-')) {
			Node *lhs = node;
			Node *rhs = mul();
			Type *value_ty;
			if (lhs->value_ty->ty != TY_INT || rhs->value_ty->ty != TY_INT) {
				// ptr
				value_ty = new_type(TY_PTR);
			} else {
				value_ty = new_type(TY_INT);
			}
			node = new_node('-', value_ty, lhs, rhs);
		} else {
			return node;
		}
	}
}

Node *mul() {
	Node *node = monomial();

	for (;;) {
		if (consume('*'))
			node = new_node('*', new_type(TY_INT), node, term());
		else if (consume('/'))
			node = new_node('/', new_type(TY_INT), node, term());
		else
			return node;
	}
}

Node *monomial() {
	Node *node = NULL;

	// distinct repeatable and unrepeatable
	for (;;) {
		if (consume(TK_PREINC)) {
			node = new_node(ND_PREINC, new_type(TY_INT), term(), NULL);
			return node;
		} else if (consume(TK_PREDEC)) {
			node = new_node(ND_PREDEC, new_type(TY_INT), term(), NULL);
			return node;
		} else if (consume('*')) {
			Node *lhs = monomial();
			if (lhs->value_ty->ty == TY_INT) {
				error("illegal deref: %s\n", ((Token *)vec_get(tokens, pos))->input);
			}
			node = new_node(ND_DEREF, new_type(lhs->value_ty->ptrof->ty), lhs, NULL);
		} else if (consume('&')) {
			node = new_node('&', new_type(TY_PTR), term(), NULL);
			return node;
		} else {
			if (node == NULL) node = term();
			return node;
		}
	}
}

Node *term() {
	if (consume('(')) {
		Node *node = add();
		if (!consume(')')) {
			//error("there isn't right-parenthesis: %s", tokens[pos].input);
			error("there isn't right-parenthesis: %s\n", 
					((Token *)vec_get(tokens, pos))->input);
			exit(1);
		}
		return node;
	}

	char *t_name;

	switch (((Token *)vec_get(tokens, pos))->ty){
	case TK_NUM:
		return new_node_num(((Token *)vec_get(tokens, pos++))->val);
		break;
	case TK_IDENT:
		t_name = ((Token *)vec_get(tokens, pos++))->input;
		if (consume('(')) {
			// foo(x, y);
			Vector *args = new_vector();
			while (1) {
				if (consume(')')) {
					break;
				}
				vec_push(args, assign());
				if (!consume(',')) {
					err_consume(')', "no right-parenthesis at func_call");
					break;
				}
			}
			return new_node_func(t_name, args);
		} else if (consume('[')) {
			// a[3]; -> *(a + 3);			
			Node *rhs = add();
			Node *lhs = new_node_ident(t_name);
			Node *plus = new_node('+', new_type(TY_PTR), lhs, rhs);
			Node *node = new_node(ND_DEREF, new_type(lhs->value_ty->ptrof->ty), plus, NULL);
			err_consume(']', "no right-bracket at array_suffix");
			return node;
		} else {
			return new_node_ident(t_name);
		}
		break;
	}
	
	//error("the token is neither number nor left-parenthesis: %s\n", 
			//((Token *)vec_get(tokens, pos))->input);
	//exit(1);
	return NULL;
}
