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

#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "names.h" /* variable name table */

/* context types */
#define CONTEXT_MAIN   0
#define CONTEXT_FUNC   1
#define CONTEXT_STRUCT 2

/* context type: provide a scope for variables */
typedef struct {
	nameTable *nt; /* table of names */
	char *fn; /* file name of scope */
	char *sn; /* name of scope */
	unsigned char tp; /* type of scope (main, function, struct) */
	object *rt; /* return value for functions */
} context;

/* functions */
extern context *contextNew(char *, char *); /* create a new context */
extern void contextFree(context *); /* free a context */

#endif /* _CONTEXT_H */