#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// preserve tokenized token string at this array
// assume that the number of token string is less than 100.
Token tokens[100];

// トークンの型を表す値
enum {
	TK_NUM = 256,	// token type of integer
	TK_EOF,			// token type of EOF
};

enum {
	ND_NUM = 256,	// node type of number
}

// トークンの型
typedef struct {
	int ty;		// トークンの型
	int val;	// tyがTK_NUMの場合，その数値
	char *input; // トークン文字列（error massage)
} Token;

typedef struct Node {
	int ty;				// is operator or ND_NUM
	struct Node *lhs;	// LHS
	struct Node *rhs;	// RHS
	int val;			// use if ty==ND_NUM
} Node;

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

// consume tokens if the next token is as expected.
int consume(int ty) {
	if (tokens[pos].ty != ty)
		return 0;
	pos++;
	return 1;
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
		if (!consume(')'))
			error("there isn't right-parenthesis: %s", token[pos].input);
		return node;
	}
	
	if (tokens[pos].ty == TK_NUM)
		return new_node_num(tokes[pos++].val);
	
	error("the token is neither number nor left-parenthesis: %s", tokens[pos].input);
}

// divide strings which p points into token and preserve at tokens.
void tokenize(char *p) {
	int i = 0;
	while (*p) {
		//skip spaces
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-') {
			tokens[i].ty = *p;
			tokens[i].input = p;
			i++;
			p++;
			continue;
		}

		if (isdigit(*p)) {
			tokens[i].ty = TK_NUM;
			tokens[i].input = p;
			tokens[i].val = strtol(p, &p, 10);
			i++;
			continue;
		}

		fprintf(stderr, "cannot tokenize: %s\n", p);
		exit(1);
	}

	tokens[i].ty = TK_EOF;
	tokens[i].input = p;
}

// emurate stack machine
void gen(Node *node) {
	if (node->ty == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->ty)
	case '+':
		printf("  add rax, rdi\n");
		break;
	case '-':
		printf("  sub rax, rdi\n");
		break;
	case '*':
		printf("  mul rdi\n");
		break;
	case '/':
		printf("  mov rdx, 0\n");
		printf("  div rdi\n");
	
	printf("  push rax\n");
}

// report errors
void error(int i) {
	fprintf(stderr, "unexpected token: %s\n", tokens[i].input);
	exit(1);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	// tokenize
	tokenize(argv[1]);
	
	// output the first half part of assembli
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// output first mov command
	// the first token of the formula must be TK_NUM.
	if (tokens[0].ty != TK_NUM)
		error(0);
	printf("  mov rax, %d\n", tokens[0].val);

	// output assembli
	// processing tokens of '+ <NUM>' or '- <NUM>'
	int i = 1;

	while (tokens[i].ty != TK_EOF) {
		if (tokens[i].ty == '+') {
			i++;
			if (tokens[i].ty != TK_NUM)
				error(i);
			printf("  add rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		if (tokens[i].ty == '-') {
			i++;
			if (tokens[i].ty != TK_NUM)
				error(i);
			printf("  sub rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		error(i);
		return 1;
	}

	printf("  ret\n");
	return 0;
}
