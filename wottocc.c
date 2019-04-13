#include "wottocc.h"

Vector *tokens;
Vector *env;
int envnum;
Vector *functions;

// position of tokens
int pos = 0;

void runtest() {
	test_vector();
	test_map();
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
	env = new_vector();
	envnum = 0;
	functions = new_vector();

	// tokenize and parse
	tokenize(argv[1]);

	printf("#tokenized\n");
	program();
	
	envnum = 0;

	printf(".intel_syntax noprefix\n");

	for (int i = 0; i < functions->len; i++) {
		Node *tmp = vec_get(functions, i);
		printf(".global %s\n", tmp->fname);
	}

	// generate assembly in order
	for (int i = 0; i < functions->len; i++) {
		Node *tmp = vec_get(functions, i);
		Map *variables = vec_get(env, i);
		// output the first half part of assembly
		//printf(".global %s\n", tmp->fname);
		printf("%s:\n", tmp->fname);

		// secure the range of variable
		printf("  push rbp\n");
		printf("  mov rbp, rsp\n");
		printf("  sub rsp, %d\n", (variables->keys->len) * 8);
		gen(tmp);

		// as a result of formula, there must be one value at stack register
		//printf("  pop rax\n");
		
		// the whole value of formula should be at the top of stack
		// pop it into RAX.
		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  ret\n");
	}

	return 0;
}
