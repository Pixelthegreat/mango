#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define TEST_MODE /* for API testing purposes */

#include "mango.h"

int main(int argc, char **argv) {

	#ifndef TEST_MODE

	/* parse args */
	if (argparse(argc, argv) <= -1) {

		/* free list */
		argparse_free();
		return -1; /* error code */
	}

	/* grab filename data from argparse */
	char **argpfv = argparse_filenames();
	int argpfc = argparse_fileslen();

	/* grab bytecode mode */
	int bc_mode = argparse_flag2bcm();

	/* error */
	if (bc_mode <= -1) {

		/* free lists */
		argparse_free();
		return -1; /* error code */
	}

	/* no files */
	if (argpfc < 1) {

		/* free and return */
		argparse_free();
		return 0; /* default code */
	}

	/* run files */
	run_all(argpfv, argpfc, bc_mode);

	argparse_free(); /* free argument parser */

	/* free objects */
	objectFreeAll();

	#else
	/* file name */
	char *fname = (char *)"test.m";

	file *f = fileNew(fname, FILE_MODE_READ);

	/* read text */
	char *text = fileReadAll(f);

	lexer *l = lexerNew(text, fname);
	lexerLex(l);

	if (!errorIsSet()) {

		printf("Lexer Stage\n");

		/* print tokens */
		for (int i = 0; i < l->n_of_tokens; i++)
			printf("%d:%s\n", l->tokens[i]->t_type, l->tokens[i]->t_value);
	
		printf("Parser Stage\n");

		/* create parser */
		parser *p = parserNew(l->tokens, l->n_of_tokens);

		/* parse */
		parserParse(p);

		/* error */
		if (errorIsSet()) {

			errorPrint();
		}
		else {

			if (p->pn != NULL) nodePrintTree(p->pn);

			printf("Bytecode Compilation Stage\n");

			/* bytecode */
			bytecode *bc = bytecodeNew(p->pn, BYTECODE_EX);
			bc->is_idat = 1;
			bytecodeComp(bc);

			/* error */
			if (errorIsSet())
				errorPrint();

			else {

				bytecodePrintf(bc);

				FILE *fp = fopen("test.mc", "wb");

				fwrite((char*)bc->bytes, bc->len, 1, fp);
			
				fclose(fp);
			}

			/* free bytecode */
			bytecodeFree(bc);
		}

		node *pn = p->pn;

		/* free parser and node */
		parserFree(p);
		if (pn != NULL) nodeFree(pn);
	}
	else {
		errorPrint();
	}

	free(text);

	/* close and free all files */
	fileFreeAll();
	objectFreeAll();

	/* free lexer */
	lexerFree(l);

	#endif
	return 0;
}
