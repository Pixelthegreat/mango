#include "token.h" /* header */
#include <stdlib.h> /* malloc, realloc, free */

/* create a token */
extern token *tokenNew(unsigned int t_type,
					   char *t_value,
					   unsigned int lineno,
					   unsigned int colno,
					   char *fname) {

	/* malloc new token */
	token *t = (token *)malloc(sizeof(token));

	/* :( */
	if (t == NULL)
		return NULL;

	/* values */
	t->t_type = t_type;
	t->t_value = t_value;
	t->lineno = lineno;
	t->colno = colno;
	t->fname = fname;

	/* return */
	return t;
}

/* free a token */
extern void tokenFree(token *t) {

	/* free value if neccessary */
	if (t->t_type == TOKEN_INT ||
		t->t_type == TOKEN_STRING ||
		t->t_type == TOKEN_IDENT ||
		t->t_type == TOKEN_KEYWORD ||
		t->t_type == TOKEN_VARWORD) {

		free(t->t_value);
	}

	/* free token */
	free(t);
}