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

#include "argparse.h" /* header */
#include "bytecode.h" /* bytecode types */
#include "object.h" /* DEBUGging */
#include <string.h> /* string functions */
#include <stdio.h> /* printf, fprintf */
#include <stdlib.h> /* malloc, realloc, free */

/* main status flag */
unsigned int status_flags = 0x00000000;
char **filename_list = NULL; /* list of file names */
int filename_len = 0; /* length of filename list */
int program_arg_idx = -1; /* program argument index */
char **argparse_argv = NULL; /* argv */
int argparse_argc = 0; /* argc */

/* help information */
static char *hlp_inf = "usage: %s [filename] [options]\n\noptions:\n    -cl       compile library\n    -cm       compile bytecode executable\n    -i        idata mode\n    -h        display help\n    --help    same as '-h'\n    -l [lib]  specify a library to run with\n    -d        print debug info\n    -df [f]   specify an output file for the debug log\n    --        pass following arguments to program\n\n";
extern char *prog_name;
extern FILE *debug_file;

/* set flag */
extern int argparse_set_flag(unsigned int flag) {

	/* flag has already been set */
	if (argparse_get_flag(flag)) {

		/* print error */
		fprintf(stderr, "ERROR: Option already chosen\n");

		return -1; /* error */
	}

	/* set flag */
	status_flags |= (1 << flag);
}

/* get a flag */
extern int argparse_get_flag(unsigned int flag) {

	/* return flag */
	return (int)((status_flags & (1 << flag)) >> flag);
}

/* get bytecode mode from flag values */
extern int argparse_flag2bcm() {

	/* no file names */
	if (filename_list == NULL) {

		/* print error and return */
		fprintf(stderr, "No input file(s) specified\n");
		return -1;
	}

	/* invalid combinations */
	if (argparse_get_flag(FLAG_COMP_LIB) &&
		argparse_get_flag(FLAG_COMP_BIN)) {

		/* print error and return */
		fprintf(stderr, "Invalid combination of options 'cl' and 'cm'\n");
		return -1;
	}

	/* compile a library */
	else if (argparse_get_flag(FLAG_COMP_LIB))
		return BYTECODE_CLIB;

	/* compile regular bytecode executable */
	else if (argparse_get_flag(FLAG_COMP_BIN))
		return BYTECODE_CMP;

	/* find file extension */
	int dot_pos = strlen(filename_list[0]); /* position where file extension starts */
	for (int i = 0; i < strlen(filename_list[0]); i++) {

		/* found a dot char */
		if (filename_list[0][i] == '.')
			dot_pos = i;
	}

	/* get pointer to string */
	char *pext = (char *)&filename_list[0][dot_pos];

	/* if the extension is 'mc', then execute it */
	if (!strcmp(pext, "mc"))
		return BYTECODE_BC;

	/* otherwise, compile and execute */
	return BYTECODE_EX;
}

/* parse arguments */
extern int argparse(int argc, char **argv) {

	argparse_argv = argv;
	argparse_argc = argc;

	/* parse args */
	int argidx = 1; /* index in list */
	while (argidx < argc) {

		/* redirect arguments */
		if (!strcmp(argv[argidx], "--")) {

			program_arg_idx = ++argidx;
			break;
		}

		/* one argument */
		if (!strcmp(argv[argidx], "-cl") ||
			!strcmp(argv[argidx], "-cm") ||
			!strcmp(argv[argidx], "-i")  ||
			!strcmp(argv[argidx], "-h")  ||
			!strcmp(argv[argidx], "-d")  ||
			!strcmp(argv[argidx], "--help")) {

			int agp_res; /* result of argparse_one function */

			/* call one arg function */
			if ((agp_res = argparse_one(argv[argidx++])) <= -1)
				return -1; /* return error code */

			/* leave immediately with no error */
			else if (agp_res == 1) {

				return 1;
			}
		}

		/* two arguments */
		else if (!strcmp(argv[argidx], "-l") || !strcmp(argv[argidx], "-df")) {

			/* not enough arguments */
			if ((argidx + 1) >= argc) {

				/* print error */
				fprintf(stderr, "Expected argument value (%s)\n", argv[argidx]);
				return -1; /* return error code */
			}

			/* call two arg function */
			if (argparse_two(argv[argidx], argv[argidx + 1]) <= -1)
				return -1; /* return error code */

			/* advance argidx value */
			argidx += 2;
		}

		/* unknown argument */
		else if (argv[argidx][0] == '-') {

			/* unrecognized option */
			fprintf(stderr, "Unrecognized option '%s'\n", argv[argidx]);

			return -1; /* error */
		}

		/* filename */
		else {

			/* invalid number of files (to be changed) */
			if (filename_len >= 1) {

				/* print error and return */
				fprintf(stderr, "Cannot accept multiple files\n");
				return -1;
			}

			/* create filename list if necessary */
			if (filename_list == NULL)
				filename_list = (char **)malloc(sizeof(char *));

			/* reallocate list */
			else
				filename_list = (char **)realloc(filename_list, sizeof(char *) * (filename_len + 1));

			/* add item */
			filename_list[filename_len++] = argv[argidx];

			argidx++; /* advance to next argument */
		}
	}

	/* no error */
	return 0;
}

/* argparse one arg */
extern int argparse_one(char *a) {

	/* idata flag */
	if (!strcmp(a, "-i")) {

		/* set flag */
		if (argparse_set_flag(FLAG_IDATA) <= -1)
			return -1;
	}

	/* compile library */
	else if (!strcmp(a, "-cl")) {

		/* set flag */
		if (argparse_set_flag(FLAG_COMP_LIB) <= -1)
			return -1;
	}

	/* compile bytecode file */
	else if (!strcmp(a, "-cm")) {

		/* set flag */
		if (argparse_set_flag(FLAG_COMP_BIN) <= -1)
			return -1;
	}

	/* print help info and exit */
	else if (!strcmp(a, "-h") || !strcmp(a, "--help")) {

		/* print string */
		printf(hlp_inf, prog_name);

		/* return 1 to signify that nothing has gone wrong, but we want to exit anyway */
		return 1;
	}

	/* print debug info for interpreter */
	else if (!strcmp(a, "-d")) {

		/* set value */
		DEBUG = 1;
	}

	return 0;
}

/* argparse two args */
extern int argparse_two(char *a, char *b) {

	/* library */
	if (!strcmp(a, "-l")) {

		/* create filename list if necessary */
		if (filename_list == NULL)
			filename_list = (char **)malloc(sizeof(char *));

		/* reallocate list */
		else
			filename_list = (char **)realloc(filename_list, sizeof(char *) * (filename_len + 1));

		/* add item */
		filename_list[filename_len++] = b;
	}

	/* debug log file */
	else if (!strcmp(a, "-df")) {

		/* open file */
		debug_file = fopen(b, "w");

		if (debug_file == NULL) {

			/* error */
			fprintf(stderr, "Could not open file '%s' for logging!\n", b);
			return -1;
		}
	}

	return 0;
}

/* return pointer to argparse file name list */
extern char **argparse_filenames() {

	/* return */
	return filename_list;
}

/* return length of filename list */
extern int argparse_fileslen() {

	return filename_len;
}

/* free filename list */
extern void argparse_free() {

	/* free list if it is active */
	if (filename_list == NULL) free(filename_list);
}

/* close file */
extern void argparse_close_debug_file() {

	if (debug_file != stdout && debug_file != NULL) fclose(debug_file);
}