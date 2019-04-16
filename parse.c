#include "wottocc.h"

// create new Node
Node *new_node(int ty, Type *value_ty, Node *lhs, Node *rhs) {
	Node *node = malloc(sizeof(Node));
	node->ty = ty;
	node->value_ty = value_ty;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_NUM;
	node->val = val;
	Type *value_ty = new_vtype(TY_INT);
	node->value_ty = value_ty;

	return node;
}

Node *new_node_ident(char *name) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_IDENT;
	node->name = name;

	// ptr or other
	Map *variables = vec_get(env, envnum);
	Type *value_ty = map_get_type(variables, name);
	node->value_ty = value_ty;
	if (value_ty->ty == TY_ARRAY) {
		// read array a as if pointer a
		// cf. int a[10]; a[0]=1; *a => 1
		Type *newtype = new_vtype(TY_PTR);
		newtype->ptrof = value_ty->ptrof;
		node->value_ty = newtype;
	}
	return node;
}

Node *new_node_func(char *name, Vector *args) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_FUNC;
	node->name = name;
	node->args = args;
	return node;
}

Type *new_vtype(int ty) {
	Type *value_ty = malloc(sizeof(Type));
	value_ty->ty = ty;
	return value_ty;
}


// consume tokens if the next token is as expected.
int consume(int ty) {
	if (((Token *)vec_get(tokens, pos))->ty != ty)
		return 0;
	pos++;
	return 1;
}

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

	if (consume(TK_INT) && consume(TK_IDENT) && consume('(') ){
		node = malloc(sizeof(Node));
		node->ty = ND_FUNCDEF;
		node->fname = ((Token *)vec_get(tokens, pos-2))->input;
		Vector *args = new_vector();
		// int foo(int *x, int y){ ... }
		while (1) {
			if (consume(')')) {
				break;
			}
			if (!consume(TK_INT)) {
				error("no type at args: %s\n", ((Token *)vec_get(tokens, pos))->input);
			}
			Type *type = malloc(sizeof(Type)); 
			type->ty = TY_INT;
			type->ptrof = NULL;

			while (consume('*')) {
				Type *newtype = malloc(sizeof(Type));
				newtype->ty = TY_PTR;
				newtype->ptrof = type;
				type = newtype;
			}
			if (!consume(TK_IDENT)){
				error("args is not variable: %s\n", ((Token *)vec_get(tokens, pos))->input);
			}
			char *vname = ((Token *)vec_get(tokens,pos-1))->input;	
			Map *variables = vec_get(env, envnum);
			map_put(variables, vname, 0, type);

			Node *arg = malloc(sizeof(Node));
		   	arg->ty = ND_INT;
			arg->name = vname;
			arg->value_ty = type;
			vec_push(args, arg);

			if (!consume(',')) {
				if (!consume(')')) {
					error("there's no right-parenthesis: %s\n", ((Token *)vec_get(tokens, pos))->input);
				} else {
					break;
				}
			}

		}
		node->args = args;
		consume('{');
		node->stmts = new_vector();
		while (!consume('}')) {
			vec_push(node->stmts, stmt());
		}
		return node;
	} else {
		error("It's not function: %s\n", ((Token *)vec_get(tokens, pos))->input);
		exit(1);
	}
}

Node *stmt() {
	Node *node;

	if (consume(TK_RETURN)) {
		node = malloc(sizeof(Node));
		node->ty = ND_RETURN;
		node->lhs = assign();
		if (!consume(';'))
			error("It's not the token ';': %s\n", ((Token *)vec_get(tokens, pos++))->input);
	} else if (consume(TK_IF)) {
		node = malloc(sizeof(Node));
		node->ty = ND_IF;
		if (!consume('(')) {
			error("no left-parenthesis at if: %s\n", ((Token *)vec_get(tokens,pos))->input);
		}
		Vector *arg = new_vector();
		vec_push(arg, assign());
		node->args = arg;
		if (!consume(')')) {
			error("no right-parenthesis at if: %s\n", ((Token *)vec_get(tokens,pos))->input);
		}
		node->lhs = stmt();
		if (consume(TK_ELSE)) {
			node->rhs = stmt();
		} else {
			node->rhs = NULL;
		}
		
	} else if (consume(TK_WHILE)) {
		node = malloc(sizeof(Node));
		node->ty = ND_WHILE;
		if (!consume('(')) {
			error("no left-parenthesis at while: %s\n", ((Token *)vec_get(tokens,pos))->input);
		}
		Vector *arg = new_vector();
		vec_push(arg, assign());
		node->args = arg;
		if (!consume(')')) {
			error("no right-parenthesis at while: %s\n", ((Token *)vec_get(tokens,pos))->input);
		}
		node->lhs = stmt();

	} else if (consume(TK_FOR)) {
		node = malloc(sizeof(Node));
		node->ty = ND_FOR;
		if (!consume('(')) {
			error("no left-parenthesis at for: %s\n", ((Token *)vec_get(tokens,pos))->input);
		}
		Vector *arg = new_vector();
		vec_push(arg, assign());
		if (!consume(';')) {
			error("no ';' at while: %s\n", ((Token *)vec_get(tokens,pos))->input);
		}
		vec_push(arg, equal());
		if (!consume(';')) {
			error("no ';' at while: %s\n", ((Token *)vec_get(tokens,pos))->input);
		}
		vec_push(arg, assign());
		node->args = arg;
		if (!consume(')')) {
			error("no right-parenthesis at for: %s\n", ((Token *)vec_get(tokens,pos))->input);
		}
		node->lhs = stmt();

	} else if (consume(TK_INT)) {
		node = malloc(sizeof(Node));
		node->ty = ND_INT;
		// int x[10];
		Type *type = malloc(sizeof(Type)); 
		type->ty = TY_INT;
		type->ptrof = NULL;

		while (consume('*')) {
			Type *newtype = malloc(sizeof(Type));
			newtype->ty = TY_PTR;
			newtype->ptrof = type;
			type = newtype;
		}

		if(!consume(TK_IDENT)) {
			error("it's not identifier: %s\n", ((Token *)vec_get(tokens,pos))->input);
		}
		char *vname = ((Token *)vec_get(tokens,pos-1))->input;	
		node->name = vname;

		if (consume('[')) {
			if (!consume(TK_NUM)) {
				error("array_initializer must be number: %s\n", ((Token *)vec_get(tokens,pos))->input);
			}
			Type *newtype = malloc(sizeof(Type));
			newtype->ty = TY_ARRAY;
			newtype->ptrof = type;
			newtype->array_size = ((Token *)vec_get(tokens,pos-1))->val;
			type = newtype;
			if (!consume(']')){
				error("no ']' at array-def: %s\n", ((Token *)vec_get(tokens,pos))->input);
			}
		}
		
		Map *variables = vec_get(env, envnum);
		map_put(variables, vname, 0, type);

		if (!consume(';')) {
			error("no ';' at variable-def: %s\n", ((Token *)vec_get(tokens,pos))->input);
		}

	} else {
		node = assign();
		if (!consume(';'))
			error("It's not the token ';': %s\n", ((Token *)vec_get(tokens, pos++))->input);
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
				value_ty = new_vtype(TY_INT);
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
			node = new_node(ND_EQUAL, new_vtype(TY_INT), node, equal());
		else if (consume(TK_NEQUAL))
			node = new_node(ND_NEQUAL, new_vtype(TY_INT), node, equal());
		else
			return node;
	}
}

Node *compare() {
	Node *node = add();
	
	for (;;) {
		if (consume('<'))
			node = new_node('<', new_vtype(TY_INT), node, add());
		else if (consume('>'))
			node = new_node('>', new_vtype(TY_INT), node, add());
		else if (consume(TK_LEQ))
			node = new_node(ND_LEQ, new_vtype(TY_INT), node, add());
		else if (consume(TK_GEQ))
			node = new_node(ND_GEQ, new_vtype(TY_INT), node, add());
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
				value_ty = new_vtype(TY_INT);
			}
			node = new_node('+', value_ty, lhs, rhs);
		} else if (consume('-')) {
			Node *lhs = node;
			Node *rhs = mul();
			Type *value_ty;
			if (lhs->value_ty->ty != TY_INT || rhs->value_ty->ty != TY_INT) {
				// ptr
				value_ty = new_vtype(TY_PTR);
			} else {
				value_ty = new_vtype(TY_INT);
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
			node = new_node('*', new_vtype(TY_INT), node, term());
		else if (consume('/'))
			node = new_node('/', new_vtype(TY_INT), node, term());
		else
			return node;
	}
}

Node *monomial() {
	Node *node = NULL;

	// distinct repeatable and unrepeatable
	for (;;) {
		if (consume(TK_PREINC)) {
			node = new_node(ND_PREINC, new_vtype(TY_INT), term(), NULL);
			return node;
		} else if (consume(TK_PREDEC)) {
			node = new_node(ND_PREDEC, new_vtype(TY_INT), term(), NULL);
			return node;
		} else if (consume('*')) {
			Node *lhs = monomial();
			if (lhs->value_ty->ty == TY_INT) {
				error("illegal deref: %s\n", ((Token *)vec_get(tokens, pos))->input);
			}
			node = new_node(ND_DEREF, new_vtype(lhs->value_ty->ptrof->ty), lhs, NULL);
		} else if (consume('&')) {
			node = new_node('&', new_vtype(TY_PTR), term(), NULL);
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
					if (!consume(')')) {
						error("there's no right-parenthesis: %s\n", ((Token *)vec_get(tokens, pos))->input);
						exit(1);
					} else {
						break;
					}
				}
			}
			return new_node_func(t_name, args);
		} else if (consume('[')) {
			// a[3]; -> *(a + 3);			
			Node *rhs = add();
			Node *lhs = new_node_ident(t_name);
			Node *plus = new_node('+', new_vtype(TY_PTR), lhs, rhs);
			Node *node = new_node(ND_DEREF, new_vtype(lhs->value_ty->ptrof->ty), plus, NULL);
			consume(']');
			return node;
		} else {
			return new_node_ident(t_name);
		}
		break;
	}
	
	error("the token is neither number nor left-parenthesis: %s\n", 
			((Token *)vec_get(tokens, pos))->input);
	exit(1);
}
