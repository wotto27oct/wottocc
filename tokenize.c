#include "wottocc.h"

// divide strings which p points into token and preserve at tokens.
void tokenize(char *p) {
	while (*p) {
		//skip spaces
		if (isspace(*p)) {
			p++;
			continue;
		}


		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' 
			|| *p == ';' || *p == ',' || *p == '{' || *p == '}') {
			Token *tmp = malloc(sizeof(Token));
			tmp->ty = *p;
			tmp->input = p;
			vec_push(tokens, (void *)tmp);
			p++;
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
			} else if (strcmp(buf_str, "while") == 0) {
				tmp->ty = TK_WHILE;
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

