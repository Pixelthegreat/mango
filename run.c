#include "run.h" /* header */
#include <stdlib.h> /* memory */
#include <stdio.h> /* printf, fprintf, ... */

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

		/* figure out what to do with bytecode */
		bytecodeFinish(bc);
		
		/* print bc data */
		bytecodePrintf(bc);
		
		/* we can now free lexer and parser */
		node *pn = p->pn; /* save node */
		parserFree(p);
		lexerFree(l);
		if (pn != NULL) nodeFree(pn); /* free node */

		/* for the moment, just free bytecode */
		bytecodeFree(bc);
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
