#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mango.h"

int main(int argc, char **argv) {

	/* file name */
	char *fname = (char*)"test.m";

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
		}

		/* free parser and node */
		parserFree(p);
		if (p->pn != NULL) nodeFree(p->pn);
	}
	else {
		errorPrint();
	}

	free(text);

	/* close and free all files */
	fileFreeAll();

	/* free lexer */
	lexerFree(l);

	return 0;
}