#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// トークンの型を表す値
enum {
	TK_NUM = 256,	// token type of integer
	TK_EOF,			// token type of EOF
};

enum {
	ND_NUM = 256,	// node type of number
};

// トークンの型
typedef struct Token {
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

typedef struct {
	void **data;	// the data
	int capacity;	// buffer (the capacity of the length)
	int len;		// the length of the vector
} Vector;

// preserve tokenized token string at this array
// assume that the number of token string is less than 100.
Token tokens[100];

// position of tokens
int pos = 0;

Node *new_node(int, Node*, Node*);
Node *new_node_num(int);
int consume(int);
Node *add();
Node *mul();
Node *term();
void tokenize(char*);
void gen(Node*);
//void error(char*);
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void expect(int, int, int);
void runtest();

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
		if (!consume(')')) {
			//error("there isn't right-parenthesis: %s", tokens[pos].input);
			fprintf(stderr, "there isn't right-parenthesis: %s", tokens[pos].input);
			exit(1);
		}
		return node;
	}
	
	if (tokens[pos].ty == TK_NUM)
		return new_node_num(tokens[pos++].val);
	
	//error("the token is neither number nor left-parenthesis: %s", tokens[pos].input);
	fprintf(stderr, "the token is neither number nor left-parenthesis: %s", tokens[pos].input);
	exit(1);
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

		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
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

	switch (node->ty) {
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
	}
	
	printf("  push rax\n");
}

// report errors
/*void error(char *str) {
	printf("%s", str);
	exit(1);
}*/

Vector *new_vector() {
	Vector *vec = malloc(sizeof(Vector));
	vec->data = malloc(sizeof(void *) * 16);
	vec->capacity = 16;
	vec->len = 0;
	return vec;
}

void vec_push(Vector *vec, void *elem) {
	if (vec->capacity == vec->len) {
		vec->capacity *= 2;
		vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
	}
	vec->data[vec->len++] = elem;
}

void expect(int line, int expected, int actual) {
	if (expected == actual)
		return;
	fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
	exit(1);
}

void runtest() {
	Vector *vec = new_vector();
	expect(__LINE__, 0, vec->len);

	for (int i = 0; i < 100; i++)
		vec_push(vec, (void *)i);

	expect(__LINE__, 100, vec->len);
	expect(__LINE__, 0, (int)vec->data[0]);
	expect(__LINE__, 50, (int)vec->data[50]);
	expect(__LINE__, 99, (int)vec->data[99]);

	Vector *vec2 = new_vector();
	
	for (int i = 0; i < 100; i++){
		Token tmp;
		tmp.ty = 2*i;
		vec_push(vec2, (void *)&tmp);
	}
	printf("%d\n", ((Token *)(vec2->data[23]))->ty);

	printf("OK\n");
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	if (strcmp(argv[1], "-test") == 0){
		runtest();
		return 0;
	}

	// tokenize and parse
	tokenize(argv[1]);
	Node *node = add();
	
	// output the first half part of assembly
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// generate assembly analyzing Abstract Syntax Tree
	gen(node);

	// the whole value of formula should be at the top of stack
	// pop it into RAX.
	printf("  pop rax\n");
	printf("  ret\n");
	return 0;
}
