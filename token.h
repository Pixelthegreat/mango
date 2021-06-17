/* token.h -- lexer tokens. */
#ifndef _TOKEN_H
#define _TOKEN_H

/* token types */
#define TOKEN_INT			0
#define TOKEN_STRING		1
#define TOKEN_IDENT			2
#define TOKEN_EOL			3
#define TOKEN_EOF			4
#define TOKEN_COMMA			5
#define TOKEN_PLUS			6
#define TOKEN_MINUS			7
#define TOKEN_MUL			8
#define TOKEN_DIV			9
#define TOKEN_OPENBRACKET	10
#define TOKEN_CLOSEBRACKET	11
#define TOKEN_OPENBRACE		12
#define TOKEN_CLOSEBRACE	13
#define TOKEN_ARROW			14
#define TOKEN_KEYWORD		15
#define TOKEN_VARWORD		16
#define TOKEN_LPAREN		17
#define TOKEN_RPAREN		18
#define TOKEN_MOD			19
#define TOKEN_EQ			20
#define TOKEN_EE			21
#define TOKEN_NE			22
#define TOKEN_LT			23
#define TOKEN_GT			24
#define TOKEN_LTE			25
#define TOKEN_GTE			26

/* token struct */
typedef struct {
	unsigned int t_type; /* type of token */
	char *t_value; /* value of token */
	unsigned int lineno; /* for errors */
	unsigned int colno; /* also for errors */
	char *fname; /* also for errors */
} token;

/* functions */
extern token *tokenNew(unsigned int t_type, char *t_value, unsigned int lineno, unsigned int colno, char *fname); /* create a new token */
extern void tokenFree(token *t); /* free a token */

#endif /* _TOKEN_H */