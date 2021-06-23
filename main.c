#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define TEST_MODE

#include "mango.h"

int main(int argc, char **argv) {

	#ifdef TEST_MODE

	bytecode *bc = bytecodeNew(NULL, BYTECODE_EX);

	bytecodeWriteStr(bc, "This is the IRS. You have been arrested for violating many money stuffs and whatever. STUPID");

	bytecodePrintf(bc);

	bytecodeFree(bc);

	#else
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

			printf("Bytecode Compilation Stage\n");

			/* bytecode */
			bytecode *bc = bytecodeNew(p->pn, BYTECODE_EX);
			bytecodeComp(bc);

			/* error */
			if (errorIsSet())
				errorPrint();

			else {

				bytecodePrintf(bc);
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

	/* free lexer */
	lexerFree(l);

	#endif
	return 0;
}