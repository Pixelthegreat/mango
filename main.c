#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define TEST_MODE /* for API testing purposes */

#include "mango.h"

int main(int argc, char **argv) {

	#ifndef TEST_MODE

	int agp_res; /* arg parse result */

	/* parse args */
	if ((agp_res = argparse(argc, argv)) <= -1) {

		/* free list */
		argparse_free();
		return -1; /* error code */
	}

	/* leave immediately with no error */
	else if (agp_res == 1) {

		/* free list */
		argparse_free();
		return 0; /* no error */
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

	#endif
	return 0;
}
