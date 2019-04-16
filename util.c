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
		switch(type->ty) {
		case TY_INT:
			variable_stack += 4;
			break;
		case TY_PTR:
			variable_stack += 8;
			break;
		case TY_ARRAY:
			if (type->ptrof->ty == TY_INT){
				variable_stack += 8 * type->array_size;
			} else {
				variable_stack += 8 * type->array_size;
			}
			break;
		}
	}
	return variable_stack;
}

