#include "error.h"

#include <stdio.h>

/* error values and stuff */
unsigned int error_type = 0;
const char *error_message = NULL;
unsigned int error_lineno = 0;
unsigned int error_colno = 0;
unsigned int is_error = 0;
unsigned int error_code = 0;
char *error_fname = NULL;

/* set the error type and value */
extern void errorSet(unsigned int err_type,
					 unsigned int err_code,
					 const char *err_msg) {

	error_type = err_type;
	error_message = err_msg;
	error_code = err_code;
	is_error = 1;
}

/* set the position of error */
extern void errorSetPos(unsigned int lineno,
						unsigned int colno, char *fname) {

	is_error = 1;

	error_lineno = lineno;
	error_colno = colno;
	error_fname = fname;
}

/* check if the error is set */
extern unsigned int errorIsSet() {

	return is_error;
}

/* print the error */
extern void errorPrint() {

	/* we can't print an error if it hasn't been set */
	if (!is_error)
		return;

	/* calculate error name */
	char *error_name = "Error";

	if (error_type == ERROR_TYPE_SYNTAX)
		error_name = "Syntax Error";
	else if (error_type = ERROR_TYPE_RUNTIME)
		error_name = "Runtime Error";

	/* get file name */
	char *fname = error_fname;

	if (fname == NULL)
		fname = (char*)"<stdin>";

	/* print the error */
	printf("%s (File \'%s\', Line %u, Column %u):\n  %s\n", error_name, fname, error_lineno, error_colno, error_message);
}

/* clear the error status */
extern void errorClear() {

	/* clear all values */
	is_error = 0;
	error_message = NULL;
	error_lineno = 0;
	error_colno = 0;
	error_type = 0;
	error_code = 0;
}