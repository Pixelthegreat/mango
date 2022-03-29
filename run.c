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

#include "run.h" /* header */
#include "stringext.h" /* fngext */
#include <stdlib.h> /* memory */
#include <stdio.h> /* printf, fprintf, ... */
#include <string.h>

/* create a new context */
extern mango_ctx *mango_ctx_new(char **fnames, int flen, int bc_mode) {

	/* allocate new context */
	mango_ctx *mc = (mango_ctx *)malloc(sizeof(mango_ctx));

	/* failed to allocate */
	if (mc == NULL)
		return NULL;

	/* set values */
	mc->fnames = fnames;
	mc->flen = flen;
	mc->bc_mode = bc_mode;

	/* return context */
	return mc;
}

/* free context */
extern void mango_ctx_free(mango_ctx *mc) {

	free(mc); /* just does this */
}

/* run code from a context */
extern int mango_ctx_run(mango_ctx *mc) {

	/* go through each file, run each file until we get an error */
	for (int fi = 0; fi < mc->flen; fi++) {

		/* error */
		if (run(mc->fnames[fi], mc->bc_mode) <= -1)
			return -1;
	}

	/* default code */
	return 0;
}

/* lex and parse a single file, return context with */
extern mango_ctx runlp(char *fname, char *bfn, unsigned int lineno, unsigned int colno) {

	/* context */
	mango_ctx ctx;
	ctx.e = 0; /* no error */

	/* open a file */
	FILE *f = fopen(fname, "r");

	/* file was unable to open */
	if (f == NULL) {

		/* error and leave */
		fprintf(stderr, "[file: '%s', line: %d, col: %d] Unable to open file '%s'\n", bfn, lineno, colno, fname);
		ctx.e = 1;
		return ctx;
	}

	char *text = NULL; /* text buffer */

	/* get file length */
	fseek(f, 0, SEEK_END);
	int flen = ftell(f);
	fseek(f, 0, SEEK_SET);

	/* create buffer and read text */
	text = (char *)malloc(flen + 1);
	fread(text, 1, flen, f);
	text[flen] = 0;

	/* close file */
	fclose(f);

	/* create a lexer */
	lexer *l = lexerNew(text, fname);
	ctx.lex = l; /* lexer */

	/* run the lexer, free the text buffer */
	lexerLex(l);
	free(text);

	/* no error, can continue to parser */
	if (!errorIsSet()) {

		/* create parser */
		parser *p = parserNew(l->tokens, l->n_of_tokens);
		ctx.parse = p; /* parser */

		/* parse */
		parserParse(p);

		/* error */
		if (errorIsSet()) {

			/* free stuff */
			parserFree(p);
			lexerFree(l);

			/* set error in context */
			ctx.e = 1;

			errorPrint(); /* print error */

			/* return context */
			return ctx;
		}

		/* debug info */
		if (DEBUG) {

			printf("\nPARSER:\n");
			nodePrintTree(p->pn);
		}
	}
	/* error */
	else {

		/* free stuff */
		lexerFree(l);

		/* set error in context */
		ctx.e = 1;

		errorPrint(); /* print error */

		/* return context */
		return ctx;
	}

	/* success */
	return ctx;
}

/* run a single file */
extern int run(char *fname, int bc_mode) {

	/* get extension */
	char *ext = fngext(fname);

	/* ext = ml? */
	if (ext != NULL && !strcmp(ext, "ml")) {

		/* print an error */
		fprintf(stderr, "Cannot (directly) run MangoLib files (use the '-l' option)!\n");
		return -1;
	}

	/* ext = mc? */
	if (ext != NULL && !strcmp(ext, "mc")) {

		/* create a vm and execute code */
		vm *v = vmNewFromFile(fname);

		/* error */
		if (v == NULL)
			return -1;

		/* execute vm code */
		vmExec(v);

		/* check if the error is set */
		if (errorIsSet()) {

			/* print the error and exit */
			errorPrint();
			return -1;
		}

		return 0;
	}

	/* open a file */
	FILE *f = fopen(fname, "r");

	/* file was unable to open */
	if (f == NULL) {

		/* error and leave */
		fprintf(stderr, "Unable to open file '%s'\n", fname);
		return -1;
	}

	char *text = NULL; /* pointer to a text buffer that we will create */
	
	/* get file length */
	fseek(f, 0, SEEK_END);
	int flen = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	/* create buffer and read text */
	text = (char *)malloc(flen + 1);
	fread(text, 1, flen, f);
	text[flen] = 0; /* line added in 0.2.0 */
	
	/* close file */
	fclose(f);

	/* unable to read text */
	if (text == NULL)
		return -1;

	/* create a lexer */
	lexer *l = lexerNew(text, fname);

	/* run the lexer */
	lexerLex(l);
	free(text); /* free the text buffer, we don't need it anymore */

	/* no error */
	if (!errorIsSet()) {

		/* create parser */
		parser *p = parserNew(l->tokens, l->n_of_tokens);

		/* parse */
		parserParse(p);

		/* error */
		if (errorIsSet()) {

			/* free stuff */
			parserFree(p);
			lexerFree(l);

			/* print error and exit */
			errorPrint();
			return -1;
		}

		/* debug info */
		if (DEBUG) {

			printf("\nPARSER:\n");
			nodePrintTree(p->pn);
		}

		/* create bytecode */
		bytecode *bc = bytecodeNew(p->pn, bc_mode);
		bc->is_idat = argparse_get_flag(FLAG_IDATA);
		bytecodeComp(bc);

		/* error */
		if (errorIsSet()) {

			/* free stuff */
			bytecodeFree(bc);
			parserFree(p);
			lexerFree(l);

			/* print error and exit */
			errorPrint();
			return -1;
		}

		/* debug info */
		if (DEBUG) {

			printf("\nBYTECODE:\n");
			bytecodePrintf(bc);
		}

		/* figure out what to do with bytecode */
		bytecodeFinish(bc);
		
		/* we can now free lexer and parser */
		node *pn = p->pn; /* save node */
		parserFree(p);
		lexerFree(l);
		if (pn != NULL) nodeFree(pn); /* free node */

		/* if we are executing a file directly */
		if (bc_mode == BYTECODE_EX) {

			/* create a vm */
			vm *v = vmNew(bc);

			/* free bytecode */
			bytecodeFree(bc);

			/* debug info */
			if (DEBUG) {

				printf("\nVM:\n");
			}

			/* execute code */
			vmExec(v);

			/* check if error is set */
			if (errorIsSet()) {

				/* print and exit */
				errorPrint();
				return -1;
			}
		}
		else {

			bytecodeFree(bc);
		}
	}

	else {

		/* free stuff */
		lexerFree(l);

		/* error */
		errorPrint();
	}

	/* default return code */
	return 0;
}
