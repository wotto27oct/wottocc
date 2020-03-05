#include "wottocc.h"

Vector *tokens;
Vector *genv;
Vector *strings;
int envnum;
Vector *toplevels;
Node *now_switch_node;

// position of tokens
int pos = 0;

// number of loop_label
int if_cnt = 0;
int loop_cnt = 0;
int now_while_cnt = 0;
int now_switch_cnt = 0;

void runtest() {
	test_vector();
	test_map();
}

Env *g_env;
Map *g_funcs;

int main(int argc, char **argv) {

	if (argc > 3) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	if (strcmp(argv[1], "-test") == 0){
		runtest();
		return 0;
	}

	tokens = new_vector();
	genv = new_vector();
	strings = new_vector();
	envnum = 0;
	toplevels = new_vector();
	g_env = new_env(NULL);
	g_funcs = new_map(NULL);

	// tokenize and parse
	if (strcmp(argv[1], "-f") == 0) {
		FILE *fp;
		fp = fopen(argv[2], "r");
		if (!fp) {
			error("file not found\n");
		}
		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char *buf = malloc(sizeof(char) * (length + 1));
		fread(buf, length + 1, sizeof(char), fp);
		fclose (fp);
		tokenize(buf);
	} else {
		tokenize(argv[1]);
	}


	printf("#tokenized\n");
	program();
	printf("#parsed\n");
	
	envnum = 0;

	printf(".intel_syntax noprefix\n");

	for (int i = 0; i < toplevels->len; i++) {
		Node *tmp = vec_get(toplevels, i);
		if (tmp->node_ty == ND_GVARDEC) continue;
		printf(".global %s\n", tmp->fname);
	}

	gen_string_address();

	// analyze
	for (int i = 0; i < toplevels->len; i++) {
		Node *tmp = vec_get(toplevels, i);
		analyze(tmp, NULL);
	}
	printf("#analyzed\n");


	char r_registers[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
	char e_registers[6][4] = {"edi", "esi", "edx", "ecx", "e8", "e9"};

	// generate assembly in order
	for (int i = 0; i < toplevels->len; i++) {
		Node *tmp = vec_get(toplevels, i);
		if (tmp->node_ty == ND_GVARDEC) {
			gen(tmp);
			continue;
		}
		int variable_stack = gen_stackpos(tmp->env, 0);
		// alignment
		variable_stack = (variable_stack + 16 - 1) & -16;
		//Map *variables = vec_get(genv, i);
		// output the first half part of assembly
		printf("%s:\n", tmp->fname);

		// secure the range of variable
		printf("  push rbp\n");
		printf("  mov rbp, rsp\n");
		// must remove
		// variable_stack = get_stackpos(tmp->env->variables, tmp->env->variables->keys->len-1);
		printf("  sub rsp, %d\n", variable_stack);

		// apply values to args
		for (int j = 0; j < tmp->args->len; j++) {
			Node *arg = vec_get(tmp->args, j);
			//int offset = (variables->keys->len - map_get_ind(variables, arg->name)) * 8;
			//int offset = get_stackpos(tmp->env->variables, map_get_ind(tmp->env->variables, arg->name));
			int offset = get_stackpos(tmp->env, arg->name);
			printf("  mov rax, rbp\n");
			printf("  sub rax, %d\n", offset);
			if (arg->value_ty->ty == TY_INT)
				printf("  mov [rax], %s\n", e_registers[j]);	
			else 
				printf("  mov [rax], %s\n", r_registers[j]);	
		}
		gen(tmp);

		// the whole value of formula should be at the top of stack
		// pop it into RAX.
		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  ret\n");
	}

	return 0;
}
