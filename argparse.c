#include "argparse.h" /* header */
#include "bytecode.h" /* bytecode types */
#include <string.h> /* string functions */
#include <stdio.h> /* printf, fprintf */
#include <stdlib.h> /* malloc, realloc, free */

/* main status flag */
unsigned int status_flags = 0x00000000;
char **filename_list = NULL; /* list of file names */
int filename_len = 0; /* length of filename list */

/* help information */
static char *hlp_inf = "usage: mango [filename] [options]\n\noptions:\n\t-cl\t\tcompile library\n\t-cm\t\tcompile bytecode executable\n\t-i\t\tidata mode\n\t-h\t\tdisplay help\n\t--help\t\tsame as '-h'\n\t-l [lib]\tspecify a library to run with\n\nbuild: development\n";

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

	/* invalid number of files (to be changed) */
	if (filename_len > 1) {

		/* print error and return */
		fprintf(stderr, "Cannot accept multiple files\n");
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

	/* parse args */
	int argidx = 1; /* index in list */
	while (argidx < argc) {

		/* one argument */
		if (!strcmp(argv[argidx], "-cl") ||
			!strcmp(argv[argidx], "-cm") ||
			!strcmp(argv[argidx], "-i")  ||
			!strcmp(argv[argidx], "-h")  ||
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
		else if (!strcmp(argv[argidx], "-l")) {

			/* not enough arguments */
			if ((argidx + 1) >= argc) {

				/* print error */
				fprintf(stderr, "Expected argument value\n");
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
		printf("%s\n", hlp_inf);

		/* return 1 to signify that nothing has gone wrong, but we want to exit anyway */
		return 1;
	}

	return 0;
}

/* argparse two args */
extern int argparse_two(char *a, char *b) {

	/* do nothing ... */

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