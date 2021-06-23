#include "lexer.h"
#include "token.h" /* tokens */
#include "error.h" /* error handling */

#include <stdlib.h> /* malloc, realloc, free */
#include <string.h> /* strlen */
#include <stdio.h> /* printf */

/* create a new lexer */
extern lexer *lexerNew(char *text, char *fname) {

	/* malloc new lexer */
	lexer *l = (lexer *)malloc(sizeof(lexer));

	/* assign values */
	l->n_of_tokens = 0;
	l->cap_tokens = 8;
	l->text = text;
	l->text_index = 0;
	l->text_len = strlen(text);
	l->c_char = 0;
	l->lineno = 0;
	l->colno = 0;
	l->fname = fname;

	/* list */
	l->tokens = (token **)malloc(sizeof(token *) * 8);

	/* return lexer */
	return l;
}

/* generate a string token */
extern void lexerString(lexer *l) {

	/* buffer */
	char *buf = (char *)malloc(128);
	unsigned int buf_sz = 128;
	unsigned int buf_len = 0;

	/* store line and column */
	unsigned int lineno = l->lineno;
	unsigned int colno = l->colno;

	/* stuff */
	int escape_char = 0; /* boolean */
	char cs = l->c_char; /* string char */

	lexerAdvance(l); /* advance */

	/* loop */
	while (l->c_char != 0 && l->c_char != cs) {

		/* resize buffer */
		if (buf_len >= buf_sz) {

			buf_sz *= 2;
			buf = (char *)realloc(buf, buf_sz);
		}

		/* escape character */
		if (l->c_char == '\\' && !escape_char)
			escape_char = 1;

		else if (l->c_char != '\\' && !escape_char)
			buf[buf_len++] = l->c_char;

		else {

			/* escape char */
			if (l->c_char == 'n')
				buf[buf_len++] = '\n';
			else if (l->c_char == 't')
				buf[buf_len++] = '\t';
			else if (l->c_char == '0')
				buf[buf_len++] = '\0';
			else
				buf[buf_len++] = l->c_char;
		}

		/* advance */
		lexerAdvance(l);
	}

	/* advance if needed */
	if (l->c_char == cs)
		lexerAdvance(l);

	/* add null term char */
	if (buf_len >= buf_sz) {

		buf_sz *= 2;
		buf = (char *)realloc(buf, buf_sz);
	}

	buf[buf_len] = '\0';

	/* create token */
	token *t = tokenNew(TOKEN_STRING, buf, lineno, colno, l->fname);
	lexerAddToken(l, t);
}

/* generate an int token */
extern void lexerInt(lexer *l) {

	/* buffer */
	char *buf = (char *)malloc(128);
	unsigned int buf_sz = 128;
	unsigned int buf_len = 0;

	/* store line and column */
	unsigned int lineno = l->lineno;
	unsigned int colno = l->colno;

	/* loop */
	while (l->c_char != 0 && LEXER_IS_INT(l->c_char)) {

		/* resize buffer */
		if (buf_len >= buf_sz) {

			buf_sz *= 2;
			buf = (char *)realloc(buf, buf_sz);
		}

		/* add char */
		buf[buf_len++] = l->c_char;

		/* advance */
		lexerAdvance(l);
	}

	/* add null term char */
	if (buf_len >= buf_sz) {

		buf_sz *= 2;
		buf = (char *)realloc(buf, buf_sz);
	}

	buf[buf_len] = '\0';

	/* create token */
	token *t = tokenNew(TOKEN_INT, buf, lineno, colno, l->fname);
	lexerAddToken(l, t);
}

/* generate an arrow (->) token */
extern void lexerArrow(lexer *l) {

	/* advance */
	lexerAdvance(l);

	/* line and column */
	unsigned int lineno = l->lineno;
	unsigned int colno = l->colno;

	/* arrow */
	if (l->c_char == '>') {

		lexerAdvance(l);
		token *t = tokenNew(TOKEN_ARROW, "->", lineno, colno, l->fname);
		lexerAddToken(l, t);
	}

	/* decrement (--) */
	else if (l->c_char == '-') {

		lexerAdvance(l);
		token *t = tokenNew(TOKEN_DEC, "--", lineno, colno, l->fname);
		lexerAddToken(l, t);
	}

	/* minus */
	else {

		token *t = tokenNew(TOKEN_MINUS, "-", lineno, colno, l->fname);
		lexerAddToken(l, t);
	}
}

/* generate an ident token */
extern void lexerIdent(lexer *l) {

	/* buffer */
	char *buf = (char *)malloc(128);
	unsigned int buf_sz = 128;
	unsigned int buf_len = 0;

	/* store line and column */
	unsigned int lineno = l->lineno;
	unsigned int colno = l->colno;

	/* loop */
	while (l->c_char != 0 && LEXER_IS_IDT(l->c_char)) {

		/* resize buffer */
		if (buf_len >= buf_sz) {

			buf_sz *= 2;
			buf = (char *)realloc(buf, buf_sz);
		}

		/* add char */
		buf[buf_len++] = l->c_char;

		/* advance */
		lexerAdvance(l);
	}

	/* add null term char */
	if (buf_len >= buf_sz) {

		buf_sz *= 2;
		buf = (char *)realloc(buf, buf_sz);
	}

	buf[buf_len] = '\0';

	/* token type defaulting to IDENT */
	unsigned int t_type = TOKEN_IDENT;

	/* keyword */
	if (LEXER_IS_KWD(buf))
		t_type = TOKEN_KEYWORD;

	/* create token */
	token *t = tokenNew(t_type, buf, lineno, colno, l->fname);
	lexerAddToken(l, t);
}

extern void lexerLex(lexer *l) {

	/* get first char */
	if (l->text_len > l->text_index)
		l->c_char = l->text[l->text_index];
	else
		l->c_char = '\0';

	/* loop through chars */
	while (l->c_char != '\0') {

		/* string */
		if (LEXER_IS_STR_START(l->c_char))
			lexerString(l);

		/* int */
		else if (LEXER_IS_INT(l->c_char))
			lexerInt(l);

		/* '-' */
		else if (l->c_char == '-')
			lexerArrow(l);

		/* ident */
		else if (LEXER_IS_IDT(l->c_char))
			lexerIdent(l);

		/* '+' */
		else if (l->c_char == '+') {

			/* error info */
			unsigned int lineno = l->lineno;
			unsigned int colno = l->colno;

			/* advance */
			lexerAdvance(l);

			/* type */
			unsigned int type = TOKEN_PLUS;
			const char *value = "+";

			/* '++' */
			if (l->c_char == '+') {

				type = TOKEN_INC;
				value = "++";
			
				/* advance */
				lexerAdvance(l);
			}

			/* add token */
			token *t = tokenNew(type, (char *)value, lineno, colno, l->fname);
			lexerAddToken(l, t);
		}

		/* '*' */
		else if (l->c_char == '*')  {

			token *t = tokenNew(TOKEN_MUL, "*", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* '/' */
		else if (l->c_char == '/') {

			/* keep line and column */
			unsigned int lineno = l->lineno;
			unsigned int colno = l->colno;

			/* comment */
			lexerAdvance(l);

			if (l->c_char == '/') {

				/* advance to newline */
				while (l->c_char != '\0' && l->c_char != '\n')
					lexerAdvance(l);
			}

			else if (l->c_char == '*') {

				/* advance */
				while (l->c_char != 0) {

					lexerAdvance(l); /* advance */

					if (l->c_char == '*') {

						/* advance */
						lexerAdvance(l);

						/* exit */
						if (l->c_char == '/') {
							lexerAdvance(l); /* advance */
							break;
						}
					}
				}
			}

			/* otherwise, add token */
			else {

				token *t = tokenNew(TOKEN_DIV, "/", lineno, colno, l->fname);
				lexerAddToken(l, t);
			}
		}

		/* ',' */
		else if (l->c_char == ',')  {

			token *t = tokenNew(TOKEN_COMMA, ",", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* ';' */
		else if (l->c_char == ';') {

			token *t = tokenNew(TOKEN_EOL, ";", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* '%' */
		else if (l->c_char == '%')  {

			token *t = tokenNew(TOKEN_MOD, "%", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* '{' */
		else if (l->c_char == '{') {

			token *t = tokenNew(TOKEN_OPENBRACE, "{", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* '}' */
		else if (l->c_char == '}')  {

			token *t = tokenNew(TOKEN_CLOSEBRACE, "}", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* '(' */
		else if (l->c_char == '(') {

			token *t = tokenNew(TOKEN_LPAREN, "(", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* ')' */
		else if (l->c_char == ')')  {

			token *t = tokenNew(TOKEN_RPAREN, ")", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* '[' */
		else if (l->c_char == '[') {

			token *t = tokenNew(TOKEN_OPENBRACKET, "[", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* ']' */
		else if (l->c_char == ']')  {

			token *t = tokenNew(TOKEN_CLOSEBRACKET, "]", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* '&' */
		else if (l->c_char == '&')  {

			token *t = tokenNew(TOKEN_AMP, "&", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
			lexerAdvance(l);
		}

		/* '=', '==' */
		else if (l->c_char == '=') {

			/* advance */
			lexerAdvance(l);

			unsigned int t_type = TOKEN_EQ; /* token type */
			char *t_val = "="; /* token value */

			/* '=='? */
			if (l->c_char == '=') {

				t_type = TOKEN_EE;
				t_val = "==";
				lexerAdvance(l);
			}

			/* add token */
			token *t = tokenNew(t_type, t_val, l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
		}

		/* '<', '<=' */
		else if (l->c_char == '<') {

			/* advance */
			lexerAdvance(l);

			unsigned int t_type = TOKEN_LT; /* token type */
			char *t_val = "<"; /* token value */

			/* '=='? */
			if (l->c_char == '=') {

				t_type = TOKEN_LTE;
				t_val = "<=";
				lexerAdvance(l);
			}

			/* add token */
			token *t = tokenNew(t_type, t_val, l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
		}

		/* '>', '>=' */
		else if (l->c_char == '>') {

			/* advance */
			lexerAdvance(l);

			unsigned int t_type = TOKEN_GT; /* token type */
			char *t_val = ">"; /* token value */

			/* '=='? */
			if (l->c_char == '=') {

				t_type = TOKEN_GTE;
				t_val = ">=";
				lexerAdvance(l);
			}

			/* add token */
			token *t = tokenNew(t_type, t_val, l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
		}

		/* '!=' */
		else if (l->c_char == '!') {

			/* advance */
			lexerAdvance(l);

			/* expects '=' */
			if (l->c_char != '=') {

				/* set error and exit */
				errorSet(ERROR_TYPE_SYNTAX,
						 ERROR_CODE_EXPECTEDCHAR,
						 "Expected '!'");
				errorSetPos(l->lineno, l->colno, l->fname);
				return;
			}

			/* advance */
			lexerAdvance(l);

			/* add token */
			token *t = tokenNew(TOKEN_NE, "!=", l->lineno, l->colno, l->fname);
			lexerAddToken(l, t);
		}

		/* whitespace */
		else if (l->c_char == '\t' ||
				 l->c_char == '\n' ||
				 l->c_char == '\r' ||
				 l->c_char == ' ') {

			/* advance */
			lexerAdvance(l);
		}

		else {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_UNKNOWNCHAR,
					 "Unknown character");
			errorSetPos(l->lineno, l->colno, l->fname);
			return;
		}
	}

	/* add eof token */
	token *t = tokenNew(TOKEN_EOF, "EOF", l->lineno,
										  l->colno,
										  l->fname);
	lexerAddToken(l, t);
}

/* advance lexer */
extern void lexerAdvance(lexer *l) {

	/* advance column and index */
	l->text_index++;
	l->colno++;

	/* get next char */
	if (l->text_len > l->text_index)
		l->c_char = l->text[l->text_index];

	else
		l->c_char = 0;

	/* advance line */
	if (l->c_char == '\n') {

		l->colno = 0;
		l->lineno++;
	}
}

/* free lexer */
extern void lexerFree(lexer *l) {

	/* free tokens */
	for (int i = 0; i < l->n_of_tokens; i++)
		tokenFree(l->tokens[i]);

	free(l); /* free lexer */
}

/* add a token */
extern void lexerAddToken(lexer *l, token *t) {

	/* resize list */
	if (l->n_of_tokens >= l->cap_tokens) {

		l->cap_tokens *= 2;
		l->tokens = (token **)realloc(l->tokens, l->cap_tokens * sizeof(token *));
	}

	/* add token */
	l->tokens[l->n_of_tokens++] = t;
}