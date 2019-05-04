#include "wottocc.h"

// divide strings which p points into token and preserve at tokens.
void tokenize(char *p) {
	while (*p) {
		//skip spaces
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (strncmp(p, "//", 2) == 0) {
			p += 2;
			while (*p != '\n' && *p != '\0')
				p++;
			continue;
		}

		if (strncmp(p, "/*", 2) == 0) {
			p = strstr(p + 2, "*/");
			if (!p) {
				error("no */ at block comment\n");
			}
			p += 2;
			continue;
		}

		if (*p == '*' || *p == '/' || *p == '(' || *p == ')' 
			|| *p == ';' || *p == ',' || *p == '{' || *p == '}'
			|| *p == '&' || *p == '[' || *p == ']' || *p == ':') {
			Token *tmp = malloc(sizeof(Token));
			tmp->ty = *p;
			tmp->input = p;
			vec_push(tokens, (void *)tmp);
			p++;
			continue;
		}

		if (*p == '+') {
			Token *tmp = malloc(sizeof(Token));
			if (*(p+1) == '+') {
				tmp->ty = TK_INC;
				tmp->input = "++";
				p += 2;
			} else if (*(p+1) == '=') {
				tmp->ty = TK_PLUSEQ;
				tmp->input = "+=";
				p += 2;
			} else {
				tmp->ty = '+';
				tmp->input = p;
				p++;
			}
			vec_push(tokens, (void *)tmp);
			continue;
		}
		
		if (*p == '-') {
			Token *tmp = malloc(sizeof(Token));
			if (isdigit(*(p+1))) {
				p += 1;
				tmp->ty = TK_NUM;
				tmp->input = p;
				tmp->val = strtol(p, &p, 10) * (-1);
			} else if (*(p+1) == '-') {
				tmp->ty = TK_DEC;
				tmp->input = "--";
				p += 2;
			} else if (*(p+1) == '=') {
				tmp->ty = TK_MINUSEQ;
				tmp->input = "-=";
				p += 2;
			} else {
				tmp->ty = '-';
				tmp->input = p;
				p++;
			}
			vec_push(tokens, (void *)tmp);
			continue;
		}

		if (*p == '=') {
			if (*(p+1) != '=') {
				Token *tmp = malloc(sizeof(Token));
				tmp->ty = '=';
				tmp->input = p;
				vec_push(tokens, (void *)tmp);
				p++;
				continue;
			} else {
				Token *tmp = malloc(sizeof(Token));
				tmp->ty = TK_EQUAL;
				tmp->input = "==";
				vec_push(tokens, (void *)tmp);
				p += 2;
				continue;
			}
		}	
		if (*p == '!' && *(p+1) == '=') {
			Token *tmp = malloc(sizeof(Token));
			tmp->ty = TK_NEQUAL;
			tmp->input = "!=";
			vec_push(tokens, (void *)tmp);
			p += 2;
			continue;
		}

		// <, <=
		if (*p == '<') {
			Token *tmp = malloc(sizeof(Token));
			if (*(p+1) != '=') {
				tmp->ty = '<';
				tmp->input = p;
				p++;
			} else {
				tmp->ty = TK_LEQ;
				tmp->input = "<=";
				p+=2;
			}
			vec_push(tokens, (void *)tmp);
			continue;
		}
		
		// >, >=
		if (*p == '>') {
			Token *tmp = malloc(sizeof(Token));
			if (*(p+1) != '=') {
				tmp->ty = '>';
				tmp->input = p;
				p++;
			} else {
				tmp->ty = TK_GEQ;
				tmp->input = ">=";
				p+=2;
			}
			vec_push(tokens, (void *)tmp);
			continue;
		}
		
		if (*p == '"') {
			char buf[256];
			int bufind = 0;
			p++;
			while(1) {
				if (*p == '"'){
					p++;
					break;
				} else {
					buf[bufind++] = *p;
					p++;
				}
			}
			buf[bufind] = '\0';
			char *buf_str = new_str(buf);

			Token *tmp = malloc(sizeof(Token));
			tmp->ty = TK_STR;
			tmp->input = buf_str;
			tmp->val = bufind;
			vec_push(tokens, (void *)tmp);
			continue;
		}


		if (isdigit(*p)) {
			Token *tmp = malloc(sizeof(Token));
			tmp->ty = TK_NUM;
			tmp->input = p;
			tmp->val = strtol(p, &p, 10);
			vec_push(tokens, (void *)tmp);
			continue;
		}

		if (isalpha(*p) || *p == '_') {
			char buf[256];		// enough length?
			int bufind = 0;
			buf[bufind++] = *p;
			p++;
			while (1) {
				if(isalpha(*p) || *p == '_'){
					buf[bufind++] = *p;
					p++;
				} else {
					break;
				}
			}
			buf[bufind++] = '\0';
			char *buf_str = new_str(buf);
			
			Token *tmp = malloc(sizeof(Token));
			tmp->input = buf_str;
			if (strcmp(buf_str, "return") == 0) {
				tmp->ty = TK_RETURN;
			} else if (strcmp(buf_str, "if") == 0) {
				tmp->ty = TK_IF;
			} else if (strcmp(buf_str, "else") == 0) {
				tmp->ty = TK_ELSE;
			} else if (strcmp(buf_str, "switch") == 0) {
				tmp->ty = TK_SWITCH;
			} else if (strcmp(buf_str, "case") == 0) {
				tmp->ty = TK_CASE;
			} else if (strcmp(buf_str, "while") == 0) {
				tmp->ty = TK_WHILE;
			} else if (strcmp(buf_str, "for") == 0) {
				tmp->ty = TK_FOR;
			} else if (strcmp(buf_str, "break") == 0) {
				tmp->ty = TK_BREAK;
			} else if (strcmp(buf_str, "continue") == 0) {
				tmp->ty = TK_CONTINUE;
			} else if (strcmp(buf_str, "int") == 0) {
				tmp->ty = TK_INT;
			} else if (strcmp(buf_str, "char") == 0) {
				tmp->ty = TK_CHAR;
			} else if (strcmp(buf_str, "sizeof") == 0) {
				tmp->ty = TK_SIZEOF;
			} else {
				tmp->ty = TK_IDENT;
			}
			vec_push(tokens, (void *)tmp);
			continue;
		}

		
		error("cannot tokenize: %s\n", p);
		exit(1);
	}

	Token *tmp = malloc(sizeof(Token));
	tmp->ty = TK_EOF;
	tmp->input = p;
	vec_push(tokens, (void *)tmp);
}

