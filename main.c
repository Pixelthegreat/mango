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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h> /* signals */

//#define TEST_MODE /* for API testing purposes */

#include "mango.h"

char *prog_name; /* program name */

int main(int argc, char **argv) {

	prog_name = argv[0]; /* get program name */

	/* set functions to execute at program exit (objectFreeAll, vmFreeAll, argparse_free) */
	atexit(objectFreeAll);
	atexit(vmFreeAll);
	atexit(argparse_free);

	/* set signal handlers */
	signal(SIGINT, objectIntHandler); /* Ctrl+C keyboard combination */
	signal(SIGSEGV, objectSegvHandler); /* segmentation faults; setup so that the program can free any dynamically allocated memory */

	#ifndef TEST_MODE

	int agp_res; /* arg parse result */

	/* parse args */
	if ((agp_res = argparse(argc, argv)) <= -1) {

		return 0; /* error code */
	}

	/* leave immediately with no error */
	else if (agp_res == 1) {

		return 0; /* no error */
	}

	/* grab filename data from argparse */
	char **argpfv = argparse_filenames();
	int argpfc = argparse_fileslen();

	/* grab bytecode mode */
	int bc_mode = argparse_flag2bcm();

	/* error */
	if (bc_mode <= -1) {

		return 0; /* error code */
	}

	/* no files */
	if (argpfc < 1) {

		return 0; /* default code */
	}

	/* set bytecode lib list */
	lib_fnames = &(argpfv[1]);
	lib_fnames_len = (argpfc - 1);

	/* run file */
	run(argpfv[0], bc_mode);

	#else

	#endif

	return 0;
}
