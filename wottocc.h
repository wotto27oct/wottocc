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
	TK_RETURN,		// token type of return
	TK_EQUAL,		// token type of ==
	TK_NEQUAL,		// token type of !=
	TK_FUNC,
	TK_IF,			// token type of if
	TK_EOF,			// token type of EOF
};

enum {
	ND_NUM = 256,	// node type of number
	ND_IDENT,		// node type of identifier
	ND_RETURN,		// node type of return
	ND_EQUAL,		// node type of ==
	ND_NEQUAL,		// node type of !=
	ND_FUNC,
	ND_FUNCDEF,
	ND_IF
};

// トークンの型
typedef struct Token {
	int ty;		// トークンの型
	int val;	// tyがTK_NUMの場合，その数値
	char *input; // トークン文字列（error massage)
} Token;

typedef struct {
	void **data;	// the data
	int capacity;	// buffer (the capacity of the length)
	int len;		// the length of the vector
} Vector;

typedef struct {
	Vector *keys;	// variable names
	Vector *vals;	// varialle values
	int len;		// the length of the map
} Map;

typedef struct Node {
	int ty;				// is operator or ND_NUM
	struct Node *lhs;	// LHS
	struct Node *rhs;	// RHS
	int val;			// use if ty==ND_NUM
	char *name;			// use if ty==ND_IDENT
	Vector *args;		// use if ty==ND_FUNC
	Vector *stmts;		// use if ty==ND_FUNCDEF
	char *fname;		// use if ty==ND_FUNCDEF
} Node;

// util.c
void runtest();

// parse.c 
Node *new_node(int, Node*, Node*);
Node *new_node_num(int);
Node *new_node_ident(char*);
int consume(int);
void program();
Node *function();
Node *stmt();
Node *assign();
Node *equal();
Node *add();
Node *mul();
Node *term();

// tokenize.c
void tokenize(char*);

// codegen.c
void gen_lval(Node*);
void gen(Node*);

// vector.c
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void *vec_get(Vector *vec, int num);
void expect(int, int, int);
void runtest();
void test_vector();

// map.c
Map *new_map();
void map_put(Map*, char*, void*);
void *map_get(Map*, char*);
int map_get_ind(Map*, char*);
void test_map();

// util.c
char *new_str(const char*);
void error(const char*, ...);

extern Vector *tokens;
extern Vector *env;
extern int pos;
extern int if_cnt;
extern Vector *functions;
extern int envnum;

#endif
