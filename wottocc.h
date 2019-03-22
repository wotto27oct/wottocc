#ifndef wottocc
#define wottocc

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
void *vec_get(Vector *vec, int num);
void expect(int, int, int);
void runtest();

#endif
