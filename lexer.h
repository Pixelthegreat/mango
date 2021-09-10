/* lexer.h -- lexes code i guess */
#ifndef _LEXER_H
#define _LEXER_H

#include "token.h" /* token system */

/* lexer struct */
typedef struct {
	token **tokens; /* list of tokens lexed */
	unsigned int n_of_tokens; /* number of tokens */
	unsigned int cap_tokens; /* capacity of tokens list */
	char *text; /* source code */
	unsigned int text_index; /* index of text */
	char c_char; /* current character */
	unsigned int text_len; /* length of text (pre-calculated to save time) */
	unsigned int lineno; /* line number */
	unsigned int colno; /* column number */
	char *fname; /* file name */
} lexer;

/* functions */
extern lexer *lexerNew(char *text, char *fname); /* create a new lexer */
extern void lexerString(lexer *l); /* generate a string token */
extern void lexerInt(lexer *l); /* generate an int token */
extern void lexerArrow(lexer *l); /* generate an arrow token */
extern void lexerIdent(lexer *l); /* generate an identifier token */
extern void lexerLex(lexer *l); /* lex tokens */
extern void lexerAdvance(lexer *l); /* advance lexer */
extern void lexerFree(lexer *l); /* free lexer */
extern void lexerAddToken(lexer *l, token *t); /* add a token */

/* macros */
#define LEXER_IS_INT(c) ((c >= '0') &&\
						 (c <= '9'))
#define LEXER_IS_STR_START(c) ((c == '"') ||\
							   (c == '\''))
#define LEXER_IS_IDT(c) (((c >= 'a') &&\
						  (c <= 'z')) ||\
						 ((c >= 'A') &&\
						  (c <= 'Z')) ||\
						 (c == '_') ||\
						 ((c >= '0') &&\
						  (c <= '9')))
#define LEXER_IS_KWD(s) (!strcmp(s, "include") ||\
						 !strcmp(s, "extern") ||\
						 !strcmp(s, "fun") ||\
						 !strcmp(s, "for") ||\
						 !strcmp(s, "if") ||\
						 !strcmp(s, "return") ||\
						 !strcmp(s, "typedef") ||\
						 !strcmp(s, "as") ||\
						 !strcmp(s, "struct") ||\
						 !strcmp(s, "while") ||\
						 !strcmp(s, "unsigned") ||\
						 !strcmp(s, "const"))

#endif /* _LEXER_H */