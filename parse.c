#include "wottocc.h"

// create new Node
Node *new_node(int ty, Node *lhs, Node *rhs) {
	Node *node = malloc(sizeof(Node));
	node->ty = ty;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}
Node *new_node_num(int val) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_NUM;
	node->val = val;
	return node;
}

Node *new_node_ident(char *name) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_IDENT;
	node->name = name;
	return node;
}


// consume tokens if the next token is as expected.
int consume(int ty) {
	if (((Token *)vec_get(tokens, pos))->ty != ty)
		return 0;
	pos++;
	return 1;
}

void program() {
	int i = 0;
	while (((Token *)vec_get(tokens, pos))->ty != TK_EOF)
		code[i++] = stmt();
	code[i] = NULL;
	return;
}

Node *stmt() {
	Node *node;

	if (consume(TK_RETURN)) {
		node = malloc(sizeof(Node));
		node->ty = ND_RETURN;
		node->lhs = assign();
	} else {
		node = assign();
	}
	if (!consume(';'))
		error("It's not the token ';': %s\n", ((Token *)vec_get(tokens, pos++))->input);
	return node;
}

Node *assign() {
	Node *node = equal();

	for (;;) {
		if (consume('='))
			node = new_node('=', node, assign());
		else
			return node;
	}
}

Node *equal() {
	Node *node = add();

	for (;;) {
		if (consume(TK_EQUAL))
			node = new_node(ND_EQUAL, node, equal());
		else if (consume(TK_NEQUAL))
			node = new_node(ND_NEQUAL, node, equal());
		else
			return node;
	}
}

Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume('+'))
			node = new_node('+', node, mul());
		else if (consume('-'))
			node = new_node('-', node, mul());
		else
			return node;
	}
}

Node *mul() {
	Node *node = term();

	for (;;) {
		if (consume('*'))
			node = new_node('*', node, term());
		else if (consume('/'))
			node = new_node('/', node, term());
		else
			return node;
	}
}

Node *term() {
	if (consume('(')) {
		Node *node = add();
		if (!consume(')')) {
			//error("there isn't right-parenthesis: %s", tokens[pos].input);
			error("there isn't right-parenthesis: %s\n", 
					((Token *)vec_get(tokens, pos))->input);
		}
		return node;
	}
	
	switch (((Token *)vec_get(tokens, pos))->ty){
	case TK_NUM:
		return new_node_num(((Token *)vec_get(tokens, pos++))->val);
		break;
	case TK_IDENT:
		return new_node_ident(((Token *)vec_get(tokens, pos++))->input);
		break;
	}
	
	error("the token is neither number nor left-parenthesis: %s\n", 
			((Token *)vec_get(tokens, pos))->input);
	exit(1);
}
