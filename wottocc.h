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
	TK_LEQ,			// token type of <=
	TK_GEQ,			// token type of >=
	TK_PREINC,		// token type of ++ (pre)
	TK_PREDEC,		// token type of -- (pre)
	TK_FUNC,
	TK_IF,			// token type of if
	TK_ELSE,
	TK_WHILE,
	TK_FOR,
	TK_INT,			// token type of int
	TK_CHAR,
	TK_EOF,			// token type of EOF
};

enum {
	ND_NUM = 256,	// node type of number
	ND_IDENT,		// node type of identifier
	ND_RETURN,		// node type of return
	ND_EQUAL,		// node type of ==
	ND_NEQUAL,		// node type of !=
	ND_LEQ,			// node type of <=
	ND_GEQ,			// node type of >=
	ND_PREINC,		// node type of ++ (pre)
	ND_PREDEC,		// node type of -- (pre)
	ND_FUNC,
	ND_FUNCDEF,
	ND_IF,
	ND_ELSE,
	ND_WHILE,
	ND_FOR,
	ND_INT,
	ND_CHAR,
	ND_DEREF,
	ND_ADDRESS,
	ND_BLOCKITEMLIST,
	ND_COMPOUND_STMT,
};

enum {
	TY_INT = 512,
	TY_CHAR,
	TY_PTR,
	TY_ARRAY
};

// トークンの型
typedef struct Token {
	int ty;		// トークンの型
	int val;	// tyがTK_NUMの場合，その数値
	char *input; // トークン文字列（error massage)
} Token;

typedef struct Type{
	int ty;		// TY_INT or something
	struct Type *ptrof;
	size_t array_size;
} Type;

typedef struct {
	void **data;	// the data
	int capacity;	// buffer (the capacity of the length)
	int len;		// the length of the vector
} Vector;

typedef struct {
	Vector *keys;	// variable names
	Vector *vals;	// varialle values
	Vector *types;  // variable types
	int len;		// the length of the map
} Map;

typedef struct Env {
	Vector *variables;
	struct Env *outer;
} Env;

typedef struct Node {
	int node_ty;		// the type of Node
	Type *value_ty;		// the type of value of the tree under this Node
	struct Node *lhs;	// LHS
	struct Node *rhs;	// RHS
	struct Env *env;

	int val;			// use if ty==ND_NUM
	char *name;			// use if ty==ND_IDENT or ND_FUNC
	char *fname;		// use if ty==ND_FUNCDEF
	
	Vector *args;		// use if ty==ND_FUNC
	Vector *stmts;		// use if ty==ND_FUNCDEF
} Node;

// util.c
void runtest();

// parse.c 
void program();
Node *function();
Node *compound_statement();
Node *block_item_list();
Node *block_item();
Node *declaration();
Node *statement();
Node *jump_statement();
Node *expression_statement();
Node *selection_statement();
Node *iteration_statement();
Node *assign();
Node *equal();
Node *compare();
Node *add();
Node *mul();
Node *monomial();
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
void map_put(Map*, char*, void*, Type*);
void *map_get(Map*, char*);
int map_get_ind(Map*, char*);
void *map_get_type(Map*, char*);
void test_map();

// util.c
char *new_str(const char*);
void error(const char*, ...);
int get_stackpos(Map*, int);
int get_typesize(Type*);
Node *new_node(int, Type*, Env*, Node*, Node*);
Node *new_node_num(int, Env*);
Node *new_node_ident(char*, Env*);
Node *new_node_func(char*, Vector*, Env*);
Type *new_type(int);
Env *new_env(Vector*);
int read_nextToken(int);
int consume(int);
int err_consume(int, const char*);

extern Vector *tokens;
extern Vector *genv;
extern int pos;
extern int loop_cnt;
extern Vector *functions;
extern int envnum;

#endif
