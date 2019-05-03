#include "wottocc.h"

void gen_lval(Node *node) {
	if (node->node_ty == ND_DEREF) {
		gen(node->lhs);
		return;
	}
	if (node->node_ty != ND_IDENT && node->node_ty != ND_G_IDENT) {
		printf("%d\n", node->node_ty);
		error("lvalue of the substitution is not variable.\n");
	}

	if (node->node_ty == ND_IDENT) {
		int offset = get_stackpos(node->env, node->name);
		printf("  mov rax, rbp\n");
		printf("  sub rax, %d\n", offset);	
		printf("  push rax\n");
	} else if (node->node_ty == ND_G_IDENT) {
		//printf("  mov rax, qword ptr [rip + %s@GOTPCREL]\n", node->name);
		printf("  lea rax, %s[rip]\n", node->name);
		printf("  push rax\n");
	}
}

// emurate stack machine
void gen(Node *node) {
	if (node->node_ty == ND_FUNCDEF) {
		gen(node->lhs);
		return;
	}

	if (node->node_ty == ND_GVARDEC) {
		gen(node->lhs);
		return;
	}
	
	if (node->node_ty == ND_CASE) {
		printf(".LC%dbegin%d:\n", now_switch_cnt, node->val);
		gen(node->lhs);
		return;
	}

	if (node->node_ty == ND_COMPOUND_STMT) {
		gen(node->lhs);
		return;
	}

	if (node->node_ty == ND_BLOCKITEMLIST) {
		for (int i = 0; i < node->args->len; i++) {
			gen(vec_get(node->args, i));
		}
		return;
	}
	
	if (node->node_ty == ND_EXPRESSION_STMT) {
		// exclude " ;"
		if (node->lhs != NULL) {
			gen(node->lhs);
			// there must not be a value at stack resister after statement
			// (not expression)
			printf("  pop rax\n");
		}
		return;
	}
	
	if (node->node_ty == ND_IF) {
		int now_if_cnt = if_cnt;
		if_cnt++;
		Node *arg = vec_get(node->args, 0);
		gen(arg);
		// result must be on top of the stack
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .ILend%d\n", now_if_cnt);
		gen(node->lhs);
		if (node->rhs != NULL) {
			printf("  jmp .IELend%d\n", now_if_cnt);
		}
		printf(".ILend%d:\n", now_if_cnt);
		if (node->rhs != NULL) {
			// else
			gen(node->rhs);
			printf(".IELend%d:\n", now_if_cnt);
		}
		return;
	}
	
	if (node->node_ty == ND_SWITCH) {
		int past_switch_cnt = now_switch_cnt;
		now_switch_cnt = loop_cnt;
		loop_cnt++;
		gen(node->lhs);
		// switch value must be on top of the stack
		for (int i = node->env->cases->len - 1; i >= 0; i--) {
			Node *tmp = vec_get(node->env->cases, i);
			gen(tmp);
			printf("  pop rdi\n");
			printf("  pop rax\n");
			printf("  cmp rax, rdi\n");
			printf("  je .LC%dbegin%d\n", now_switch_cnt, i);
			printf("  push rax\n");
		}

		printf("  pop rax\n");
		gen(node->rhs);
		printf(".Lend%d:\n", now_switch_cnt);

		now_switch_cnt = past_switch_cnt;
		return;
	}
	
	if (node->node_ty == ND_WHILE) {
		int past_while_cnt = now_while_cnt;
		int past_switch_cnt = now_switch_cnt;
		now_while_cnt = loop_cnt;
		now_switch_cnt = loop_cnt;
		loop_cnt++;
		
		printf(".Lbegin%d:\n", now_while_cnt);
		gen(node->lhs);
		// result must be on top of the stack
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .Lend%d\n", now_while_cnt);
		
		gen(node->rhs);
		printf(".contin%d:\n", now_while_cnt);
		printf("  jmp .Lbegin%d\n", now_while_cnt);
		printf(".Lend%d:\n", now_while_cnt);

		now_while_cnt = past_while_cnt;
		now_switch_cnt = past_switch_cnt;
		return;
	}	
	
	if (node->node_ty == ND_FOR) {
		int past_while_cnt = now_while_cnt;
		int past_switch_cnt = now_switch_cnt;
		now_while_cnt = loop_cnt;
		now_switch_cnt = loop_cnt;
		loop_cnt++;
		
		Node *arg = vec_get(node->args, 0);
		if (arg != NULL) gen(arg);
		
		printf(".Lbegin%d:\n", now_while_cnt);
		arg = vec_get(node->args, 1);
		if (arg != NULL) {
			gen(arg);
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je .Lend%d\n", now_while_cnt);
		}
	
		gen(node->lhs);
		printf(".contin%d:\n", now_while_cnt);
		arg = vec_get(node->args, 2);
		if (arg != NULL) gen(arg);
		printf("  jmp .Lbegin%d\n", now_while_cnt);
		printf(".Lend%d:\n", now_while_cnt);
		
		now_while_cnt = past_while_cnt;
		now_switch_cnt = past_switch_cnt;
		return;
	}
	
	if (node->node_ty == ND_CONTINUE) {
		printf("  jmp .contin%d\n", now_while_cnt);
		return;
	}
	
	if (node->node_ty == ND_BREAK) {
		printf("  jmp .Lend%d\n", now_switch_cnt);
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

	if (node->node_ty == ND_DECLARATION) {
		gen(node->lhs);
		printf("  pop rax\n"); // statement must not be one value on stack
		return;
	}

	if (node->node_ty == ND_INIT_DECLARATOR_LIST || node->node_ty == ND_INIT_G_DECLARATOR_LIST) {
		gen(node->lhs);
		printf("  pop rax\n");
		gen(node->rhs);	
		return;
	}

	if (node->node_ty == ND_INIT_DECLARATOR) {
		// int,char,ptrのみ
		if (node->value_ty->ty != TY_ARRAY) {
			gen_lval(node->lhs);
			gen(node->rhs);

			printf("  pop rdi\n");
			printf("  pop rax\n");
			if (node->value_ty->ty == TY_INT)
				printf("  mov [rax], edi\n");
			else if (node->value_ty->ty == TY_CHAR)
				printf("  mov byte ptr [rax], dil\n");
			else
				printf("  mov [rax], rdi\n");
			printf("  push rdi\n");
		} else {
			// array a[10];
			gen_lval(node->lhs);
			// now the address of a is on stack
			for(size_t i = 0; i < node->rhs->value_ty->array_size; i++) {
				Node *arg = vec_get(node->rhs->args, i);
				gen(arg);
				
				printf("  pop rdi\n");
				printf("  pop rax\n");
				if (node->value_ty->ptrof->ty == TY_INT)
					printf("  mov [rax], edi\n");
				else if (node->value_ty->ptrof->ty == TY_CHAR)
					printf("  mov byte ptr [rax], dil\n");
				else
					printf("  mov [rax], rdi\n");
				printf("  add rax, %d\n", get_typesize(node->lhs->value_ty->ptrof));
				printf("  push rax\n");
			}
			printf("  push 0\n");
		}
		return;
	}	
	
	if (node->node_ty == ND_INIT_G_DECLARATOR) {
		// int,charのみ
		if (node->value_ty->ty == TY_INT || node->value_ty->ty == TY_CHAR) {
			printf(".data\n");
			printf("%s:\n", node->lhs->name);
			if (node->value_ty->ty == TY_INT) {
				printf("  .long %d\n", node->rhs->val);
			} else if (node->value_ty->ty == TY_CHAR) {
				printf("  .byte %d\n", node->rhs->val);
			}
			printf(".text\n");
		} else if (node->value_ty->ty == TY_ARRAY) {
			printf(".data\n");
			printf("%s:\n", node->lhs->name);
			for (size_t i = 0; i < node->rhs->value_ty->array_size; i++) {
				Node *arg = vec_get(node->rhs->args, i);
				if (node->value_ty->ptrof->ty == TY_INT) {
					printf("  .long %d\n", arg->val);
				} else if (node->value_ty->ptrof->ty == TY_CHAR) {
					printf("  .byte %d\n", arg->val);
				}
			}
			printf(".text\n");
		}
		return;
	}	

	if (node->node_ty == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	if (node->node_ty == ND_IDENT || node->node_ty == ND_G_IDENT) {
		gen_lval(node);
		Type *type;
		if (node->node_ty == ND_IDENT) {
			type = get_valuetype(node->env, node->name);
		} else {
			type = get_valuetype(g_env, node->name);
		}
		// int a[2]; a means the address of a[0],
		// not contents
		if (type->ty == TY_ARRAY) return;
		printf("  pop rax\n");
		if (type->ty == TY_INT)
			printf("  mov eax, [rax]\n");
		else if (type->ty == TY_CHAR)
			printf("  movzx eax, byte ptr [rax]\n");
		else 
			printf("  mov rax, [rax]\n");
		printf("  push rax\n");
	
		return;
	}

	if (node->node_ty == ND_G_DECLARATOR) {
		printf(".data\n");
		printf("%s:\n", node->name);
		printf("  .zero %d\n", get_typesize(node->value_ty));
		printf(".text\n");
		return;
	}


	if (node->node_ty == ND_FUNC_CALL) {
		char registers[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
		if (node->rhs != NULL) {
			gen(node->rhs);
			for (int i = node->rhs->length; i>0; i--) {
				printf("  pop %s\n", registers[i-1]);
			}
		}

		printf("  call %s\n", node->lhs->name);
		printf("  push rax\n");
		return;
	}

	if (node->node_ty == ND_ARG_EXP_LIST) {
		gen(node->lhs);
		gen(node->rhs);
		return;
	}

	if (node->node_ty == ND_EXP) {
		gen(node->lhs);
		printf("  pop rax\n");
		gen(node->rhs);
		return;
	}

	if (node->node_ty == '=') {
		gen_lval(node->lhs);
		gen(node->rhs);

		printf("  pop rdi\n");
		printf("  pop rax\n");
		if (node->value_ty->ty == TY_INT)
			printf("  mov [rax], edi\n");
		else if (node->value_ty->ty == TY_CHAR) 
			printf("  mov byte ptr [rax], dil\n");
	    else 
			printf("  mov [rax], rdi\n");
		printf("  push rdi\n");
		return;
	}

	if (node->node_ty == ND_PLUSEQ) {
		gen_lval(node->lhs);
		gen(node->rhs);

		printf("  pop rdi\n");
		printf("  pop rax\n");
		if (node->value_ty->ty == TY_INT)
			printf("  add [rax], edi\n");
		else if (node->value_ty->ty == TY_CHAR)
			printf("  add byte ptr [rax], dil\n");
		else 
			printf("  add [rax], rdi\n");
		printf("  push [rax]\n");
		return;
	}

	if (node->node_ty == ND_MINUSEQ) {
		gen_lval(node->lhs);
		gen(node->rhs);

		printf("  pop rdi\n");
		printf("  pop rax\n");
		if (node->value_ty->ty == TY_INT)
			printf("  sub [rax], edi\n");
		else if (node->value_ty->ty == TY_CHAR)
			printf("  sub byte ptr [rax], dil\n");
		else 
			printf("  sub [rax], rdi\n");
		printf("  push [rax]\n");
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

	if (node->node_ty == ND_POSTINC) {
		gen_lval(node->lhs);
		printf("  pop rax\n");
		printf("  push [rax]\n");
		printf("  mov rdi, 1\n");
		printf("  add [rax], rdi\n");
		return;
	}

	if (node->node_ty == ND_POSTDEC) {
		gen_lval(node->lhs);
		printf("  pop rax\n");
		printf("  push [rax]\n");
		printf("  mov rdi, 1\n");
		printf("  sub [rax], rdi\n");
		return;
	}

	if (node->node_ty == ND_DEREF) {
		gen(node->lhs);
		// address must be on the stack
		printf("  pop rax\n");
		if (node->value_ty->ty == TY_INT)
			printf("  mov eax, [rax]\n");
		else if (node->value_ty->ty == TY_CHAR)
			printf("  movzx eax, byte ptr [rax]\n");
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
		if (node->lhs->value_ty->ty == TY_PTR && (node->rhs->value_ty->ty == TY_INT || node->rhs->value_ty->ty == TY_CHAR)) {
			int ptr_offset = get_typesize(node->lhs->value_ty->ptrof);
			printf("  pop rax\n"); // rax = rhs
			printf("  mov rdi, %d\n", ptr_offset);
			printf("  mul rdi\n");
			printf("  push rax\n");
		} else if ((node->lhs->value_ty->ty == TY_INT || node->lhs->value_ty->ty == TY_CHAR) && node->rhs->value_ty->ty == TY_PTR) {
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
		if (node->lhs->value_ty->ty == TY_PTR && (node->rhs->value_ty->ty == TY_INT || node->rhs->value_ty->ty == TY_CHAR)) {
			printf("  pop rax\n"); // rax = rhs
			printf("  mov rdi, 4\n");
			printf("  mul rdi\n");
			printf("  push rax\n");
		} else if ((node->lhs->value_ty->ty == TY_CHAR || node->lhs->value_ty->ty == TY_INT) && node->rhs->value_ty->ty == TY_PTR) {
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
