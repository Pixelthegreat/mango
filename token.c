/*
 *
 * Copyright 2021, Elliot Kohlmyer
 *
 * This file is part of Mango.
 *
 * Mango is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mango is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mango.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

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