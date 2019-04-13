#include "wottocc.h"

Vector *tokens;
Map *variables;

// position of tokens
int pos = 0;

// nodes
Node *code[100];


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
