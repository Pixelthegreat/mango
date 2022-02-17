/*
 *
 * Copyright 2021, 2022 Elliot Kohlmyer
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

#ifndef _RUN_H
#define _RUN_H

#include <stdlib.h> /* stdlib */
#include "mango.h" /* all headers */

/* mango context */
typedef struct {
	char **fnames; /* file names */
	int flen; /* number of file names */
	int bc_mode; /* mode for bytecode */

	/* extra stuff */
	lexer *lex; /* lexer instance */
	parser *parse; /* parser instance */
	int e; /* resulting error code */
} mango_ctx;

/* functions */
extern mango_ctx *mango_ctx_new(char **, int, int); /* create a new mango context */
extern void mango_ctx_free(mango_ctx *); /* free a mango context */
extern int mango_ctx_run(mango_ctx *); /* run from context */

extern int run(char *, int); /* run a single file */
extern mango_ctx runlp(char *, char *, unsigned int, unsigned int); /* lex and parse a single file, return parsed node */

/* run directly instead of creating context */
static inline int run_all(char **fnames, int flen, int bc_mode) {

	/* create context */
	mango_ctx *mc = mango_ctx_new(fnames, flen, bc_mode);

	/* failed to allocate */
	if (mc == NULL)
		return -1;

	/* run each file */
	int res = mango_ctx_run(mc);

	/* free context and return result */
	mango_ctx_free(mc);
	return res;
}

#endif /* _RUN_H */