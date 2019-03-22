#include "wottocc.h"

// preserve tokenized token string at this array
// assume that the number of token string is less than 100.
//Token tokens[100];

Vector *tokens;
Map *variables;

// position of tokens
int pos = 0;

// nodes
Node *code[100];

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

Map *new_map() {
	Map *map = malloc(sizeof(Map));
	map->keys = new_vector();
	map->vals = new_vector();
	return map;
}

void map_put(Map *map, char *key, void *val) {
	vec_push(map->keys, key);
	vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
	for (int i = map->keys->len - 1; i >= 0; i--)
		if (strcmp(map->keys->data[i], key) == 0)
			return map->vals->data[i];
	return NULL;
}

int map_get_ind(Map *map, char *key) {
	for (int i = map->keys->len - 1; i >= 0; i--)
		if (strcmp(map->keys->data[i], key) == 0)
			return i;
	return NULL;
}


void test_map() {
	Map *map = new_map();
	expect(__LINE__, 0, (int)map_get(map, "foo"));

	map_put(map, "foo", (void *)2);
	expect(__LINE__, 2, (int)map_get(map, "foo"));

	map_put(map, "bar", (void *)4);
	expect(__LINE__, 4, (int)map_get(map, "bar"));

	map_put(map, "foo", (void *)6);
	expect(__LINE__, 6, (int)map_get(map, "foo"));

	printf("map OK\n");
}

void runtest(){
	test_vector();
	test_map();
}

// consume tokens if the next token is as expected.
int consume(int ty) {
	if (((Token *)vec_get(tokens, pos))->ty != ty)
		return 0;
	pos++;
	return 1;
}

void *program() {
	int i = 0;
	while (((Token *)vec_get(tokens, pos))->ty != TK_EOF)
		code[i++] = stmt();
	code[i] = NULL;
}

Node *stmt() {
	Node *node = assign();
	if (!consume(';'))
		error("It's not the token ';': %s\n", ((Token *)vec_get(tokens, pos++))->input);
	return node;
}

Node *assign() {
	Node *node = add();

	for (;;) {
		if (consume('='))
			node = new_node('=', node, assign());
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
}

char *new_str(const char *src){
	// aqcc
	char *ret = malloc(strlen(src) + 1);
	strcpy(ret, src);
	return ret;
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

		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' 
			|| *p == ';' || *p == '=') {
			Token tmp;
			tmp.ty = *p;
			tmp.input = p;
			Token *d = malloc(sizeof(Token));
			*d = tmp;
			vec_push(tokens, (void *)d);
			p++;
			continue;
		}

		if (isdigit(*p)) {
			Token tmp;
			tmp.ty = TK_NUM;
			tmp.input = p;
			tmp.val = strtol(p, &p, 10);
			Token *d = malloc(sizeof(Token));
			*d = tmp;
			vec_push(tokens, (void *)d);
			continue;
		}

		if (isalpha(*p) || *p == '_') {
			char buf[256];		// enough length?
			int bufind = 0;
			buf[bufind++] = *p;
			p++;
			while (1) {
				if(isalpha(*p) || *p == '_'){
					buf[bufind++] = *p;
					p++;
				} else {
					break;
				}
			}
			buf[bufind++] = '\0';
			Token tmp;
			tmp.ty = TK_IDENT;
			tmp.input = new_str(buf);
			Token *d = malloc(sizeof(Token));
			*d = tmp;
			vec_push(tokens, (void *)d);
			map_put(variables, new_str(buf), 0);
			continue;
		}

		error("cannot tokenize: %s\n", p);
		exit(1);
	}

	Token tmp;
	tmp.ty = TK_EOF;
	tmp.input = p;
	Token *d = malloc(sizeof(Token));
	*d = tmp;
	vec_push(tokens, (void *)d);
}

void gen_lval(Node *node) {
	if (node->ty != ND_IDENT)
		error("lvalue of the substitution is not variable.");
	
	int offset = (variables->keys->len - map_get_ind(variables, node->name) + 1) * 8;
	printf("  mov rax, rbp\n");
	printf("  sub rax, %d\n", offset);	
	printf("  push rax\n");
}

// emurate stack machine
void gen(Node *node) {
	if (node->ty == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	if (node->ty == ND_IDENT) {
		gen_lval(node);
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return;
	}

	if (node->ty == '=') {
		gen_lval(node->lhs);
		gen(node->rhs);

		printf("  pop rdi\n");
		printf("  pop rax\n");
		printf("  mov [rax], rdi\n");
		printf("  push rdi\n");
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
void error(const char *str, ...) {
	va_list ap;
	va_start(ap, str);
	vprintf(str, ap);
	va_end(ap);
	exit(1);
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

	tokens = new_vector();
	variables = new_map();

	// tokenize and parse
	tokenize(argv[1]);

	printf("#tokenized\n");
	program();
	
	// output the first half part of assembly
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// secure the range of variable 'a'~'z'
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, %d\n", (variables->keys->len) * 8);

	// generate assembly in order
	for (int i = 0; code[i]; i++) {
		gen(code[i]);

		// as a result of formula, there must be one value at stack register
		printf("  pop rax\n");
	}

	// the whole value of formula should be at the top of stack
	// pop it into RAX.
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");
	return 0;
}
