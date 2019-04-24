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

int get_stackpos(Env *env, char *name) {
	int variable_stack = env->stackpos;
	int ind = map_get_ind(env->variables, name);
	if (ind == -1) {
		if (env->outer == NULL) return -1;
		else return get_stackpos(env->outer, name);
	}
	for (int i = 0; i <= ind; i++) {
		Type *type = env->variables->types->data[i];
		variable_stack += get_typesize(type);
	}
	return variable_stack;
}

// allocate stackpos and return total stack range of variables
int gen_stackpos(Env *env, int startnum) {
	env->stackpos = startnum;
	int variable_stack = startnum;
	for (int i = 0; i < env->variables->keys->len; i++) {
		variable_stack += get_typesize(env->variables->types->data[i]);
	}
	for (int i = 0; i < env->inner->len; i++) {
		variable_stack = gen_stackpos(vec_get(env->inner, i), variable_stack);
	}
	return variable_stack;
}

Type* get_valuetype(Env *env, char* name) {
	Type *value_ty = map_get_type(env->variables, name);
	if (value_ty == NULL) {
		if (env->outer == NULL) value_ty = NULL;
		else value_ty = get_valuetype(env->outer, name);
	}
	return value_ty;
}


int get_typesize(Type *type) {
	if (type->ty == TY_INT) return 4;
	else if (type->ty == TY_PTR) return 8;
	else if (type->ty == TY_ARRAY) return get_typesize(type->ptrof) * type->array_size;
	return 0;
}


// create new Node
Node *new_node(int node_ty, Type *value_ty, Env *env, Node *lhs, Node *rhs) {
	Node *node = malloc(sizeof(Node));
	node->node_ty = node_ty;
	node->value_ty = value_ty;
	node->env = env;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val, Env *env) {
	Node *node = new_node(ND_NUM, new_type(TY_INT), env, NULL, NULL);
	node->val = val;
	return node;
}

Node *new_node_ident(char *name, Env *env) {
	
	// decide value_ty
	//Map *variables = vec_get(genv, envnum);
	//Type *value_ty = map_get_type(env->variables, name);
	Type *value_ty = get_valuetype(env, name);
	if (value_ty->ty == TY_ARRAY) {
		// read array a as if pointer a
		// cf. int a[10]; a[0]=1; *a => 1
		Type *newtype = new_type(TY_PTR);
		newtype->ptrof = value_ty->ptrof;
		value_ty = newtype;
	}

	Node *node = new_node(ND_IDENT, value_ty, env, NULL, NULL);
	node->name = name;
	return node;
}

Node *new_node_func(char *name, Vector *args, Env *env) {
	Node *node = new_node(ND_FUNC, NULL, env, NULL, NULL);
	node->name = name;
	node->args = args;
	return node;
}

Type *new_type(int ty) {
	Type *type = malloc(sizeof(Type));
	type->ty = ty;
	return type;
}

Env *new_env(Env *outer) {
	Env *env = malloc(sizeof(Env));
	env->variables = new_map();
	env->outer = outer;
	env->inner = new_vector();
	env->stackpos = 0;
	env->cases = new_vector();
	return env;
}

int read_nextToken(int ty) {
	if (((Token *)vec_get(tokens, pos))->ty == ty) return 1;
	return 0;
}

// consume tokens if the next token is as expected.
int consume(int ty) {
	if (read_nextToken(ty) != 1)
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

