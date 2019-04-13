#include "wottocc.h"

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
	if (node->ty == ND_RETURN) {
		gen(node->lhs);
		printf("  pop rax\n");
		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  ret\n");
		return;
	}

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

	if (node->ty == ND_FUNC) {
		if (node->lhs != NULL) {
			gen(node->lhs);
			printf("pop rdi\n");
		}
		printf("  call %s\n", node->name);
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
		break;
	case ND_EQUAL:
		printf("  cmp rax, rdi\n");
		printf("  sete al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_NEQUAL:
		printf("  cmp rax, rdi\n");
		printf("  setne al\n");
		printf("  movzb rax, al\n");
		break;
	}
	
	printf("  push rax\n");
}
