#ifndef wottocc
#define wottocc

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// トークンの型を表す値
enum {
	TK_NUM = 256,	// token type of integer
	TK_IDENT,		// token type of identifier
	TK_EOF,			// token type of EOF
};

enum {
	ND_NUM = 256,	// node type of number
	ND_IDENT,		// node type of identifier
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
	char name;			// use if ty==ND_IDENT
} Node;

typedef struct {
	void **data;	// the data
	int capacity;	// buffer (the capacity of the length)
	int len;		// the length of the vector
} Vector;

typedef struct {
	Vector *keys;
	Vector *vals;
} Map;

Node *new_node(int, Node*, Node*);
Node *new_node_num(int);
Node *new_node_ident(char);
Map *new_map();
void map_put(Map*, char*, void*);
void *map_get(Map*, char*);
void test_map();
int consume(int);
void *program();
Node *stmt();
Node *assign();
Node *add();
Node *mul();
Node *term();
void tokenize(char*);
void gen_lval(Node*);
void gen(Node*);
void error(const char*, ...);
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void *vec_get(Vector *vec, int num);
void expect(int, int, int);
void runtest();
void test_vector();

#endif