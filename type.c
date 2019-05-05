#include "wottocc.h"

Type *new_type(int ty) {
	Type *type = malloc(sizeof(Type));
	type->ty = ty;
	return type;
}

int get_typesize(Type *type) {
	if (type->ty == TY_CHAR) return 1;
	else if (type->ty == TY_INT) return 4;
	else if (type->ty == TY_PTR) return 8;
	else if (type->ty == TY_ARRAY) return get_typesize(type->ptrof) * type->array_size;
	return 0;
}

Node *read_type() {
	Node *node = NULL;
	if (consume(TK_INT)) {
		node = new_node(ND_INT, NULL, NULL);
	} else if (consume(TK_CHAR)) {
		node = new_node(ND_CHAR, NULL, NULL);
	}
	return node;
}

Node *err_read_type() {
	Node *type = read_type();
	if (type == NULL) error("no specifier\n");
	return type;
}

Node *read_ptr() {
	Node *tmp = NULL;
	while(consume('*')) {
		tmp = new_node(ND_PTR, tmp, NULL);
	}
	return tmp;
}

Type *assignment_check(Type *lhs, Type *rhs) {
	Type *value_ty;
	switch(lhs->ty){
	case TY_INT:
		if (rhs->ty == TY_INT) {
			value_ty = new_type(TY_INT);
		} else {
			error("illegal assignment\n");
		}
		break;
	case TY_CHAR:
		if (rhs->ty == TY_CHAR || rhs->ty == TY_INT) {
			value_ty = new_type(TY_CHAR);
		} else {
			error("illegal assignment\n");
		}
		break;
	case TY_PTR:
	case TY_ARRAY:
		if (rhs->ty == TY_PTR || rhs->ty == TY_ARRAY) {
			// 本当はまずい(何重ポインタか考慮していない)
			value_ty = lhs;
		} else {
			error("illegal assignment\n");
		}
		break;
	}
	return value_ty;
}

Type *plus_check(Type *lhs, Type *rhs) {
	Type *value_ty;
	switch(lhs->ty) {
	case TY_INT:
		if (rhs->ty == TY_INT || rhs->ty == TY_CHAR) {
			value_ty = new_type(TY_INT);
		} else if (rhs->ty == TY_PTR || rhs->ty == TY_ARRAY) {
			// (1 + a)
			value_ty = rhs;
		}
		break;
	case TY_CHAR:
		if (rhs->ty == TY_CHAR) {
			value_ty = new_type(TY_CHAR);
		} else if (rhs->ty == TY_INT) {
			value_ty = new_type(TY_INT);
		} else if (rhs->ty == TY_PTR || rhs->ty == TY_ARRAY) {
			// (1 + a)
			value_ty = rhs;
		}
		break;
	case TY_PTR:
	case TY_ARRAY:
		if (rhs->ty == TY_INT || rhs->ty == TY_CHAR) {
			value_ty = lhs;
		} else if (rhs->ty == TY_PTR || rhs->ty == TY_ARRAY) {
			value_ty = lhs;
		}
		break;
	}
	return value_ty;
}

