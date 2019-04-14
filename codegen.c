#include "wottocc.h"

void gen_lval(Node *node) {
	if (node->ty != ND_IDENT)
		error("lvalue of the substitution is not variable.");

	Map *variables = vec_get(env, envnum);	
	int offset = (variables->keys->len - map_get_ind(variables, node->name) + 1) * 8;
	printf("  mov rax, rbp\n");
	printf("  sub rax, %d\n", offset);	
	printf("  push rax\n");
}

// emurate stack machine
void gen(Node *node) {
	if (node->ty == ND_FUNCDEF) {
		int i = 0;
		for (; i < node->stmts->len; i++) {
			gen(vec_get(node->stmts, i));
			// as a result of statement, there must be one value at stack register
			printf("  pop rax\n");
		}
		return;
	}

	if (node->ty == ND_RETURN) {
		gen(node->lhs);
		printf("  pop rax\n");
		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  ret\n");
		return;
	}

	if (node->ty == ND_IF) {
		int now_loop_cnt = loop_cnt;
		loop_cnt++;
		Node *arg = vec_get(node->args, 0);
		gen(arg);
		// result must be on top of the stack
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .Lend%d\n", now_loop_cnt);
		gen(node->lhs);
		if (node->rhs != NULL) {
			printf("  jmp .ELend%d\n", now_loop_cnt);
		}
		printf(".Lend%d:\n", now_loop_cnt);
		if (node->rhs != NULL) {
			// else
			gen(node->rhs);
			printf(".ELend%d:\n", now_loop_cnt);
		}
		return;
	}

	if (node->ty == ND_WHILE) {
		int now_loop_cnt = loop_cnt;
		loop_cnt++;
		printf(".Lbegin%d:\n", now_loop_cnt);
		Node *arg = vec_get(node->args, 0);
		gen(arg);
		// result must be on top of the stack
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .Lend%d\n", now_loop_cnt);
		gen(node->lhs);
		printf("  jmp .Lbegin%d\n", now_loop_cnt);
		printf(".Lend%d:\n", now_loop_cnt);
		return;
	}	

	if (node->ty == ND_FOR) {
		int now_loop_cnt = loop_cnt;
		loop_cnt++;
		Node *arg = vec_get(node->args, 0);
		gen(arg);
		printf(".Lbegin%d:\n", now_loop_cnt);
		arg = vec_get(node->args, 1);
		gen(arg);
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .Lend%d\n", now_loop_cnt);
		gen(node->lhs);
		arg = vec_get(node->args, 2);
		gen(arg);
		printf("  jmp .Lbegin%d\n", now_loop_cnt);
		printf(".Lend%d:\n", now_loop_cnt);
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
		char registers[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
		int i = 0;
		for (; i < node->args->len; i++) {
			gen(vec_get(node->args, i));
		}
		i--;
		for (; i>=0; i--) {
			printf("  pop %s\n", registers[i]);
		}

		printf("  call %s\n", node->name);
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
		break;
	case '<':
		printf("  cmp rdi, rax\n");
		printf("  setg al\n");
		printf("  movzb rax, al\n");
		break;
	case '>':
		printf("  cmp rdi, rax\n");
		printf("  setb al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LEQ:
		printf("  cmp rdi, rax\n");
		printf("  setge al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_GEQ:
		printf("  cmp rdi, rax\n");
		printf("  setbe al\n");
		printf("  movzb rax, al\n");
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
