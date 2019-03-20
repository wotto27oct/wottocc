#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
	TK_NUM = 256,	// 整数トークン
	TK_EOF,			// 入力の終わりを表すトークン
};

// トークンの型
typedef struct {
	int ty;		// トークンの型
	int val;	// tyがTK_NUMの場合，その数値
	char *input; // トークン文字列（error massage)
} Token;

// preserve tokenized token string at this array
// assume that the number of token string is less than 100.
Token tokens[100];

// divide strings which p points into token and preserve at tokens.
void tokenize(char *p) {
	int i = 0;
	while (*p) {
		//skip spaces
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-') {
			tokens[i].ty = *p;
			tokens[i].input = p;
			i++;
			p++;
			continue;
		}

		if (isdigit(*p)) {
			tokens[i].ty = TK_NUM;
			tokens[i].input = p;
			tokens[i].val = strtol(p, &p, 10);
			i++;
			continue;
		}

		fprintf(stderr, "cannot tokenize: %s\n", p);
		exit(1);
	}

	tokens[i].ty = TK_EOF;
	tokens[i].input = p;
}

// report errors
void error(int i) {
	fprintf(stderr, "unexpected token: %s\n", tokens[i].input);
	exit(1);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	// tokenize
	tokenize(argv[1]);
	
	// output the first half part of assembli
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// output first mov command
	// the first token of the formula must be TK_NUM.
	if (tokens[0].ty != TK_NUM)
		error(0);
	printf("  mov rax, %d\n", tokens[0].val);

	// output assembli
	// processing tokens of '+ <NUM>' or '- <NUM>'
	int i = 1;

	while (tokens[i].ty != TK_EOF) {
		if (tokens[i].ty == '+') {
			i++;
			if (tokens[i].ty != TK_NUM)
				error(i);
			printf("  add rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		if (tokens[i].ty == '-') {
			i++;
			if (tokens[i].ty != TK_NUM)
				error(i);
			printf("  sub rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		error(i);
		return 1;
	}

	printf("  ret\n");
	return 0;
}
