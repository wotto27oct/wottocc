#include "wottocc.h"

void gen_lval(Node *node) {
	if (node->node_ty == ND_DEREF) {
		gen(node->lhs);
		return;
	}
	if (node->node_ty != ND_IDENT)
		error("lvalue of the substitution is not variable.\n");

	Map *variables = vec_get(env, envnum);	
	//int offset = (variables->keys->len - map_get_ind(variables, node->name)) * 8;
	int offset = get_stackpos(variables, map_get_ind(variables, node->name));
	printf("  mov rax, rbp\n");
	printf("  sub rax, %d\n", offset);	
	printf("  push rax\n");
}

// emurate stack machine
void gen(Node *node) {
	if (node->node_ty == ND_FUNCDEF) {
		/*int i = 0;
		for (; i < node->stmts->len; i++) {
			gen(vec_get(node->stmts, i));
			// as a result of statement, there must be one value at stack register
			printf("  pop rax\n");
		}
		return;*/
		gen(node->lhs);
		return;
	}

	if (node->node_ty == ND_COMPOUND_STMT) {
		if (node->lhs != NULL) gen(node->lhs);
		return;
	}

	if (node->node_ty == ND_BLOCKITEMLIST) {
		int i = 0;
		for (; i < node->args->len; i++) {
			gen(vec_get(node->args, i));
			// as a result of statement, there must be one value at stack register
			// printf("  pop rax\n");
		}
		return;
	}

	if (node->node_ty == ND_RETURN) {
		gen(node->lhs);
		printf("  pop rax\n");
		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  ret\n");
		return;
	}

	if (node->node_ty == ND_IF) {
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

	if (node->node_ty == ND_WHILE) {
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

	if (node->node_ty == ND_FOR) {
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

	if (node->node_ty == ND_INT) {
		printf("  push 1\n"); // must be one value on stack register
		return;
	}

	if (node->node_ty == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	if (node->node_ty == ND_IDENT) {
		gen_lval(node);
		Map *variables = vec_get(env, envnum);
		Type *type = map_get_type(variables, node->name);
		if (type->ty == TY_ARRAY) return;
		printf("  pop rax\n");
		if (type->ty == TY_INT)
			printf("  mov eax, [rax]\n");
		else 
			printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return;
	}

	if (node->node_ty == ND_FUNC) {
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

	if (node->node_ty == '=') {
		gen_lval(node->lhs);
		gen(node->rhs);

		printf("  pop rdi\n");
		printf("  pop rax\n");
		if (node->value_ty->ty == TY_INT)
			printf("  mov [rax], edi\n");
	    else 
			printf("  mov [rax], rdi\n");
		printf("  push rdi\n");
		return;
	}

	if (node->node_ty == ND_PREINC) {
		gen_lval(node->lhs);
		printf("  pop rax\n");
		printf("  mov rdi, 1\n");
		printf("  add [rax], rdi\n");
		printf("  push [rax]\n");
		return;
	}

	if (node->node_ty == ND_PREDEC) {
		gen_lval(node->lhs);
		printf("  pop rax\n");
		printf("  mov rdi, 1\n");
		printf("  sub [rax], rdi\n");
		printf("  push [rax]\n");
		return;
	}

	if (node->node_ty == ND_DEREF) {
		gen(node->lhs);
		// address must be on the stack
		printf("  pop rax\n");
		if (node->value_ty->ty == TY_INT)
			printf("  mov eax, [rax]\n");
		else
			printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return;
	}

	if (node->node_ty == '&') {
		gen_lval(node->lhs);
		return;
	}

	if (node->node_ty == '+') {
		gen(node->lhs);
		gen(node->rhs);
		if (node->lhs->value_ty->ty == TY_PTR && node->rhs->value_ty->ty == TY_INT) {
			//int ptr_offset = 8;
			//if(node->lhs->value_ty->ptrof->ty == TY_INT) ptr_offset = 4;
			int ptr_offset = get_typesize(node->lhs->value_ty->ptrof);
			printf("  pop rax\n"); // rax = rhs
			printf("  mov rdi, %d\n", ptr_offset);
			printf("  mul rdi\n");
			printf("  push rax\n");
		} else if (node->lhs->value_ty->ty == TY_INT && node->rhs->value_ty->ty == TY_PTR) {
			//int ptr_offset = 8;
			//if(node->rhs->value_ty->ptrof->ty == TY_INT) ptr_offset = 4;
			int ptr_offset = get_typesize(node->rhs->value_ty->ptrof);
			printf("  pop rsi\n"); // rsi = rhs
			printf("  pop rax\n"); // rax = lhs
			printf("  mov rdi, %d\n", ptr_offset);
			printf("  mul rdi\n");
			printf("  push rax\n");
			printf("  push rsi\n");
		}

		printf("  pop rdi\n");
		printf("  pop rax\n");
		printf("  add rax, rdi\n");
		printf("  push rax\n");
		return;
	}
	
	if (node->node_ty == '-') {
		gen(node->lhs);
		gen(node->rhs);
		if (node->lhs->value_ty->ty == TY_PTR && node->rhs->value_ty->ty == TY_INT) {
			printf("  pop rax\n"); // rax = rhs
			printf("  mov rdi, 4\n");
			printf("  mul rdi\n");
			printf("  push rax\n");
		} else if (node->lhs->value_ty->ty == TY_INT && node->rhs->value_ty->ty == TY_PTR) {
			printf("  pop rsi\n"); // rsi = rhs
			printf("  pop rax\n"); // rax = lhs
			printf("  mov rdi, 4\n");
			printf("  mul rdi\n");
			printf("  push rax\n");
			printf("  push rsi\n");
		}

		printf("  pop rdi\n");
		printf("  pop rax\n");
		printf("  sub rax, rdi\n");
		printf("  push rax\n");
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->node_ty) {
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
