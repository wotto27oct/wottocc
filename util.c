#include "wottocc.h"

char *new_str(const char *src){
	// aqcc
	char *ret = malloc(strlen(src) + 1);
	strcpy(ret, src);
	return ret;
}

// report errors
void error(const char *str, ...) {
	va_list ap;
	va_start(ap, str);
	vprintf(str, ap);
	va_end(ap);
	exit(1);
}

int get_stackpos(Map *variables, int ind) {
	int variable_stack = 0;
	for (int i = 0; i <= ind; i++) {
		Type *type = variables->types->data[i];
		variable_stack += get_typesize(type);
	}
	return variable_stack;
}

int get_typesize(Type *type) {
	if (type->ty == TY_INT) return 4;
	else if (type->ty == TY_PTR) return 8;
	else if (type->ty == TY_ARRAY) return get_typesize(type->ptrof) * type->array_size;
	return 0;
}


// create new Node
Node *new_node(int node_ty, Type *value_ty, Node *lhs, Node *rhs) {
	Node *node = malloc(sizeof(Node));
	node->node_ty = node_ty;
	node->value_ty = value_ty;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = new_node(ND_NUM, new_type(TY_INT), NULL, NULL);
	node->val = val;
	return node;
}

Node *new_node_ident(char *name) {
	
	// decide value_ty
	Map *variables = vec_get(env, envnum);
	Type *value_ty = map_get_type(variables, name);
	if (value_ty->ty == TY_ARRAY) {
		// read array a as if pointer a
		// cf. int a[10]; a[0]=1; *a => 1
		Type *newtype = new_type(TY_PTR);
		newtype->ptrof = value_ty->ptrof;
		value_ty = newtype;
	}

	Node *node = new_node(ND_IDENT, value_ty, NULL, NULL);
	node->name = name;
	return node;
}

Node *new_node_func(char *name, Vector *args) {
	Node *node = new_node(ND_FUNC, NULL, NULL, NULL);
	node->name = name;
	node->args = args;
	return node;
}

Type *new_type(int ty) {
	Type *type = malloc(sizeof(Type));
	type->ty = ty;
	return type;
}

// consume tokens if the next token is as expected.
int consume(int ty) {
	if (((Token *)vec_get(tokens, pos))->ty != ty)
		return 0;
	pos++;
	return 1;
}

// if cannot consume tokens, put error
int err_consume(int ty, const char *str) {
	if (!consume(ty)) {
		printf("error! : %s\n", ((Token *)vec_get(tokens, pos))->input);
		error(str);
		return 0;
	}
	return 1;
}
