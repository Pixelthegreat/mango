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

	/* we can't print an error if it hasn't been set or if it's type is ERROR_TYPE_BREAK */
	if (!is_error || error_type == ERROR_TYPE_BREAK)
		return;

	/* calculate error name */
	char *error_name = "Error";

	if (error_type == ERROR_TYPE_SYNTAX)
		error_name = "Syntax Error";
	else if (error_type == ERROR_TYPE_RUNTIME)
		error_name = "Runtime Error";
	else if (error_type == ERROR_TYPE_BYTECODE)
		error_name = "Bytecode Error";
	else if (error_type == ERROR_TYPE_INTERNAL)
		error_name = "Internal Error";

	/* get file name */
	char *fname = error_fname;

	if (fname == NULL)
		fname = (char*)"(null)";

	/* print the error */
	fprintf(stderr, "%s (File \'%s\', Line %u, Column %u):\n  %s\n", error_name, fname, error_lineno, error_colno, error_message);
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