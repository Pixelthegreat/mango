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

/* names.h -- variable naming system */
#ifndef _NAMES_H
#define _NAMES_H

#include <stdlib.h>

#include "obhead.h"

/* name table for storing names */
typedef struct _nameTable {
	struct _nameTable *parent; /* for accessing "outside" values */
	char **names; /* list of names */
	object **values; /* list of values */
	unsigned int n_of_names; /* number of names */
	unsigned int cap_names; /* capacity of lists */
} nameTable;

/* functions */
extern nameTable *namesNew(); /* create a new nameTable for storing names */
extern void namesSet(nameTable *nt, char *name, object *value); /* add a value to the list or set a value to the list */
extern unsigned int namesIndex(nameTable *nt, char *name); /* get the index of a name if it has been found */
extern object *namesGet(nameTable *nt, char *name); /* get a name's value if the name is found */
extern object *namesGetFromString(nameTable *nt, object *obj); /* get a value from a string name */
extern object *namesGetN(nameTable *nt, char *name, int n); /* same as namesGet, but n controls whether or not to check the parent name table as well */
extern void namesFree(nameTable *nt); /* free a table */
extern nameTable *namesCopy(nameTable *nt); /* copy a table entirely */

#endif /* _NAMES_H */