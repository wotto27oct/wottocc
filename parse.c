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

	err_consume(TK_INT, "It's not suitable format for function");
	err_consume(TK_IDENT, "It's not suitable format for function");
	err_consume('(', "no left-parenthesis at declaration_list");

	node = new_node(ND_FUNCDEF, NULL, new_env(NULL), NULL, NULL);
	node->fname = ((Token *)vec_get(tokens, pos-2))->input;
	Vector *args = new_vector();
	// int foo(int *x, int y){ ... }
	//Map *variables = vec_get(genv, envnum);
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
		map_put(node->env->variables, vname, 0, type);

		Node *arg = new_node(ND_INT, type, NULL, NULL, NULL);
		arg->name = vname;
		vec_push(args, arg);

		if (!consume(',')) {
			err_consume(')', "there's no right-parenthesis at args");
			break;
		}

	}
	node->args = args;

	node->lhs = compound_statement(node->env);
	/*consume('{');
	node->stmts = new_vector();
	while (!consume('}')) {
		vec_push(node->stmts, statement());
	}*/
	return node;
}

/*Node *declaration_list(Env *env)
{
	Node *node = new_node(ND_DECLARATIONLIST, NULL, env, NULL, NULL);
	err_consume('(', "no left-parenthesis at declaration_list");
	Vector *args = new_vector();
	while (1) {
		Node *arg = declaration(env);
		if (arg == NULL) break;
		else vec_push(args, arg);
	}
	node->args = args;
	return node;
}*/

Node *compound_statement(Env *env) {
	Node *node = new_node(ND_COMPOUND_STMT, NULL, env, NULL, NULL);
	err_consume('{', "no left-brace at compound_statement");
	if(!consume('}')){
		Env *inner_env = new_env(env);
		node->lhs = block_item_list(inner_env);
		vec_push(env->inner, inner_env);
	} else {
		return node;
	}
	err_consume('}', "no right-brace at compound_statement");
	return node;
}

Node *block_item_list(Env *env) {
	Node *node = new_node(ND_BLOCKITEMLIST, NULL, env, NULL, NULL);
	Vector *args = new_vector();
	while (1) {
		Node *arg = block_item(env);
		if (arg == NULL) break;
		else vec_push(args, arg);
	}
	node->args = args;
	return node;
}

Node *block_item(Env *env) {
	Node *node = declaration(env);
	if (node == NULL){
		node = statement(env);
	}
	return node;
}

Node *declaration(Env *env) {
	Node *node = NULL;
	if (consume(TK_INT)) {
		node = new_node(ND_DECLARATION, NULL, env, NULL, NULL);
		Type *type = new_type(TY_INT); 

		if (read_nextToken(';') == 0) {
			node->lhs = init_declarator_list(env, type);
		}
			

		/*while (consume('*')) {
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
		
		//Map *variables = vec_get(genv, envnum);
		map_put(env->variables, vname, 0, type);
		
		node = new_node(ND_INT, type, env, NULL, NULL);
		node->name = vname;*/

		err_consume(';', "no ';' at declaration");
	}
	return node;
}

Node *init_declarator_list(Env *env, Type *sp_type) {
	Node *node = new_node(ND_INITDECLARATORLIST, NULL, env, NULL, NULL);
	Vector *args = new_vector();
	Node *arg = init_declarator(env, sp_type);
	vec_push(args, arg);
	while(1) {
		if (!consume(',')) break;
		Node *arg = init_declarator(env, sp_type);
		vec_push(args, arg);
	}
	node->args = args;
	return node;
}	

Node *init_declarator(Env *env, Type *sp_type) {
	Node *node = declarator(env, sp_type);
	return node;
}

Node *declarator(Env *env, Type *sp_type) {
	Type *type = sp_type;

	while (consume('*')) {
		Type *newtype = new_type(TY_PTR);
		newtype->ptrof = type;
		type = newtype;
	}

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
	
	//Map *variables = vec_get(genv, envnum);
	map_put(env->variables, vname, 0, type);
	
	Node *node = new_node(ND_DECLARATOR, type, env, NULL, NULL);
	node->name = vname;

	return node;
}



Node *statement(Env *env) {
	Node *node = NULL;
	if (read_nextToken(TK_RETURN)) {
		node = jump_statement(env);
	} else if (read_nextToken(TK_IF)) {
		node = selection_statement(env);
	} else if (read_nextToken(TK_WHILE) || read_nextToken(TK_FOR)) {
		node = iteration_statement(env);
	} else if (read_nextToken('{')) {
		node = compound_statement(env);
	} else {
		node = expression_statement(env);
	}
	return node;
}

Node *jump_statement(Env *env) {
	consume(TK_RETURN);
	Node *node = new_node(ND_RETURN, NULL, env, assign(env), NULL);
	err_consume(';', "no ';' at return");
	return node;
}

Node *expression_statement(Env *env) {
	Node *node = assign(env);
	if (node != NULL) err_consume(';', "no ';' at end of the expression_statement\n");
	return node;
}

Node *selection_statement(Env *env) {
	Node *node;
	if (consume(TK_IF)) {
		node = new_node(ND_IF, NULL, env, NULL, NULL);
		err_consume('(', "no left-parenthesis at if");
		Vector *arg = new_vector();
		vec_push(arg, assign(env));
		node->args = arg;
		err_consume(')', "no right-parenthesis at if");
		node->lhs = statement(env);
		if (consume(TK_ELSE)) {
			node->rhs = statement(env);
		} else {
			node->rhs = NULL;
		}
	}
	return node;
}

Node *iteration_statement(Env *env) {
	Node *node;
	if (consume(TK_WHILE)) {
		node = new_node(ND_WHILE, NULL, env, NULL, NULL);
		err_consume('(', "no left-parenthesis at while");
		Vector *arg = new_vector();
		vec_push(arg, assign(env));
		node->args = arg;
		err_consume(')', "no right-parenthesis at while");
		node->lhs = statement(env);

	} else if (consume(TK_FOR)) {
		node = new_node(ND_FOR, NULL, env, NULL, NULL);
		err_consume('(', "no left-parenthesis at for");
		Vector *arg = new_vector();
		vec_push(arg, assign(env));
		err_consume(';', "no ';' at while");
		vec_push(arg, equal(env));
		err_consume(';', "no ';' at while");
		vec_push(arg, assign(env));
		node->args = arg;
		err_consume(')', "no right-parenthesis at for");
		node->lhs = statement(env);
	}
	return node;
}


Node *assign(Env *env) {
	Node *node = equal(env);

	for (;;) {
		if (consume('=')){
			Node *lhs = node;
			Node *rhs = assign(env);
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
			node = new_node('=', value_ty, env, lhs, rhs);
		} else {
			return node;
		}
	}
}

Node *equal(Env *env) {
	Node *node = compare(env);

	for (;;) {
		if (consume(TK_EQUAL))
			node = new_node(ND_EQUAL, new_type(TY_INT), env, node, equal(env));
		else if (consume(TK_NEQUAL))
			node = new_node(ND_NEQUAL, new_type(TY_INT), env, node, equal(env));
		else
			return node;
	}
}

Node *compare(Env *env) {
	Node *node = add(env);
	
	for (;;) {
		if (consume('<'))
			node = new_node('<', new_type(TY_INT), env, node, add(env));
		else if (consume('>'))
			node = new_node('>', new_type(TY_INT), env, node, add(env));
		else if (consume(TK_LEQ))
			node = new_node(ND_LEQ, new_type(TY_INT), env, node, add(env));
		else if (consume(TK_GEQ))
			node = new_node(ND_GEQ, new_type(TY_INT), env, node, add(env));
		else 
			return node;
	}
}

Node *add(Env *env) {
	Node *node = mul(env);

	for (;;) {
		if (consume('+')){
			Type *value_ty;
			Node *lhs = node;
			Node *rhs = mul(env);
			if (lhs->value_ty->ty != TY_INT || rhs->value_ty->ty != TY_INT) {
				// ptr
				if (lhs->value_ty->ty != TY_INT)
					value_ty = lhs->value_ty;
				else 
					value_ty = rhs->value_ty;
			} else {
				value_ty = new_type(TY_INT);
			}
			node = new_node('+', value_ty, env, lhs, rhs);
		} else if (consume('-')) {
			Node *lhs = node;
			Node *rhs = mul(env);
			Type *value_ty;
			if (lhs->value_ty->ty != TY_INT || rhs->value_ty->ty != TY_INT) {
				// ptr
				value_ty = new_type(TY_PTR);
			} else {
				value_ty = new_type(TY_INT);
			}
			node = new_node('-', value_ty, env, lhs, rhs);
		} else {
			return node;
		}
	}
}

Node *mul(Env *env) {
	Node *node = monomial(env);

	for (;;) {
		if (consume('*'))
			node = new_node('*', new_type(TY_INT), env, node, term(env));
		else if (consume('/'))
			node = new_node('/', new_type(TY_INT), env, node, term(env));
		else
			return node;
	}
}

Node *monomial(Env *env) {
	Node *node = NULL;

	// distinct repeatable and unrepeatable
	for (;;) {
		if (consume(TK_PREINC)) {
			node = new_node(ND_PREINC, new_type(TY_INT), env, term(env), NULL);
			return node;
		} else if (consume(TK_PREDEC)) {
			node = new_node(ND_PREDEC, new_type(TY_INT), env, term(env), NULL);
			return node;
		} else if (consume('*')) {
			Node *lhs = monomial(env);
			if (lhs->value_ty->ty == TY_INT) {
				error("illegal deref: %s\n", ((Token *)vec_get(tokens, pos))->input);
			}
			node = new_node(ND_DEREF, new_type(lhs->value_ty->ptrof->ty), env, lhs, NULL);
		} else if (consume('&')) {
			node = new_node('&', new_type(TY_PTR), env, term(env), NULL);
			return node;
		} else {
			if (node == NULL) node = term(env);
			return node;
		}
	}
}

Node *term(Env *env) {
	if (consume('(')) {
		Node *node = add(env);
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
		return new_node_num(((Token *)vec_get(tokens, pos++))->val, env);
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
				vec_push(args, assign(env));
				if (!consume(',')) {
					err_consume(')', "no right-parenthesis at func_call");
					break;
				}
			}
			return new_node_func(t_name, args, env);
		} else if (consume('[')) {
			// a[3]; -> *(a + 3);			
			Node *rhs = add(env);
			Node *lhs = new_node_ident(t_name, env);
			Node *plus = new_node('+', new_type(TY_PTR), env, lhs, rhs);
			Node *node = new_node(ND_DEREF, new_type(lhs->value_ty->ptrof->ty), env, plus, NULL);
			err_consume(']', "no right-bracket at array_suffix");
			return node;
		} else {
			Node *node = new_node_ident(t_name, env);
			return node;
		}
		break;
	}
	
	//error("the token is neither number nor left-parenthesis: %s\n", 
			//((Token *)vec_get(tokens, pos))->input);
	//exit(1);
	return NULL;
}
