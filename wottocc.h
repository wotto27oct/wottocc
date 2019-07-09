#ifndef wottocc
#define wottocc

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// トークンの型を表す値
enum {
	TK_NUM = 0,	// token type of integer
	TK_IDENT,		// token type of identifier
	TK_RETURN,		// token type of return
	TK_EQUAL,		// token type of ==
	TK_NEQUAL,		// token type of !=
	TK_LEQ,			// token type of <=
	TK_GEQ,			// token type of >=
	TK_PLUSEQ,		// +=
	TK_MINUSEQ,		// -=
	TK_INC,		// token type of ++
	TK_SIZEOF,
	TK_DEC,		// token type of --
	TK_FUNC,
	TK_IF,			// token type of if
	TK_ELSE,
	TK_SWITCH,
	TK_CASE,
	TK_WHILE,
	TK_FOR,
	TK_BREAK,
	TK_CONTINUE,
	TK_INT,			// token type of int
	TK_CHAR,
	TK_STR,
	TK_DOUBLE,
	TK_VOID,
	TK_EOF,			// token type of EOF
};

enum {
	ND_NUM = 256,	// node type of number
	ND_IDENT,		// node type of identifier
	ND_STR,
	ND_RETURN,		// node type of return
	ND_EQUAL,		// node type of ==
	ND_NEQUAL,		// node type of !=
	ND_LEQ,			// node type of <=
	ND_GEQ,			// node type of >=
	ND_PLUSEQ,
	ND_MINUSEQ,
	ND_PREINC,		// node type of ++ (pre)
	ND_PREDEC,		// node type of -- (pre)
	ND_FUNC,
	ND_FUNCDEF,
	ND_IF,
	ND_ELSE,
	ND_SWITCH,
	ND_CASE,
	ND_WHILE,
	ND_FOR,
	ND_BREAK,
	ND_CONTINUE,
	ND_INT,
	ND_PTR,
	ND_CHAR,
	ND_DOUBLE,
	ND_VOID,
	ND_DEREF,
	ND_ADDRESS,
	ND_BLOCKITEMLIST,
	ND_COMPOUND_STMT,
	ND_DECLARATION_LIST,
	ND_INIT_DECLARATOR_LIST,
	ND_DECLARATOR,
	ND_DECLARATION,
	ND_EXPRESSION_STMT,
	ND_INIT_DECLARATOR,
	ND_ARG_EXP_LIST,
	ND_FUNC_CALL,
	ND_EXP,
	ND_POSTINC,
	ND_POSTDEC,
	ND_GVARDEC,
	ND_GVAR_DEF,
	ND_G_IDENT,
	ND_INIT_G_DECLARATOR_LIST,
	ND_INIT_G_DECLARATOR,
	ND_G_DECLARATOR,
	ND_ARG,
	ND_INITIALIZER_LIST,
	ND_SIZEOF,
};

enum {
	TY_INT = 512,
	TY_CHAR,
	TY_PTR,
	TY_ARRAY,
	TY_DOUBLE,
	TY_VOID
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
	Map *variables;
	struct Env *outer;
	Vector *inner;
	int stackpos;
	Vector *cases;
	Type *returntype;
} Env;

typedef struct Node {
	int node_ty;		// the type of Node
	Type *value_ty;		// the type of value of the tree under this Node
	struct Node *lhs;	// LHS
	struct Node *rhs;	// RHS
	struct Env *env;

	int val;			// use if ty==ND_NUM or ND_CASE
	char *name;			// use if ty==ND_IDENT or ND_FUNC
	char *fname;		// use if ty==ND_FUNCDEF
	int length;			// use if ty==ND_ARG_EXP_LIST
	
	Vector *args;		// use if ty==ND_FUNC
	Vector *stmts;		// use if ty==ND_FUNCDEF
} Node;

// util.c
void runtest();

// parse.c 
void program();
Node *toplevel();
Node *compound_statement();
Node *block_item_list();
Node *block_item();
Node *declaration();
Node *init_declarator_list();
Node *init_g_declarator_list();
Node *init_declarator();
Node *init_g_declarator();
Node *initializer();
Node *declarator();
Node *g_declarator();
Node *statement();
Node *jump_statement();
Node *expression_statement();
Node *selection_statement();
Node *labeled_statement();
Node *iteration_statement();
Node *expression();
Node *assignment_expression();
Node *conditional_expression();
Node *logical_OR_expression();
Node *logical_AND_expression();
Node *inclusive_OR_expression();
Node *exclusive_OR_expression();
Node *AND_expression();
Node *equality_expression();
Node *relational_expression();
Node *shift_expression();
Node *additive_expression();
Node *multiplicative_expression();
Node *cast_expression();
Node *unary_expression();
Node *postfix_expression();
Node *argument_expression_list();
Node *primary_expression();
Node *constant_expression();

// tokenize.c
void tokenize(char*);

// analyze.c
void analyze_dec(Node*, Env*, Type*);
void analyze(Node*, Env*);
Type *get_ptr(Node*, Type*);

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

// type.c
Type *new_type(int);
int get_typesize(Type*);
Node *read_type();
Node *err_read_type();
Node *read_ptr();
Type *assignment_check(Type*, Type*);
Type *plus_check(Type*, Type*);

// util.c
char *new_str(const char*);
void error(const char*, ...);
int get_stackpos(Env*, char*);
int gen_stackpos(Env*, int);
Type *get_valuetype(Env*, char*);
Node *new_node(int, Node*, Node*);
Node *new_node_num(int);
Node *new_node_string(char*, int);
Env *new_env(Env*);
int read_nextToken(int);
int consume(int);
int err_consume(int, const char*);
void gen_string_address();

extern Vector *tokens;
extern Vector *genv;
extern int pos;
extern int if_cnt;
extern int loop_cnt;
extern int now_while_cnt;
extern int now_switch_cnt;
extern Vector *toplevels;
extern int envnum;
extern Env *g_env;
extern Map *g_funcs;
extern Node *now_switch_node;
extern Vector *strings;

#endif
