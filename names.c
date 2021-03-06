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

#include "names.h" /* header */
#include "error.h" /* error handling */
#include "arrayobject.h" /* array object */
#include "pointerobject.h" /* pointers */
#include "structobject.h" /* structs */
#include <stdlib.h> /* malloc, realloc, free */
#include <string.h> /* strcmp */
#include <stdio.h> /* printf */

/* forward declarations */
extern object *objectCopy(object *);
extern int DEBUG;
extern FILE *debug_file;

int is_at_end = 0;
static int id = 0;

/* create a name table */
extern nameTable *namesNew() {

	/* malloc a table */
	nameTable *nt = (nameTable *)malloc(sizeof(nameTable));

	/* failed once again :(O */
	if (nt == NULL)
		return NULL;

	/* create lists */
	nt->names = (char **)malloc(sizeof(char *) * 8);
	nt->values = (object **)malloc(sizeof(object *) * 8);
	memset(nt->names, 0, sizeof(char *) * 8);

	/* assign values */
	nt->n_of_names = 0;
	nt->cap_names = 8;
	nt->parent = NULL;
	nt->id = id++;

	/* return */
	return nt;
}

extern nameTable *namesCopy(nameTable *nt) {

	/* create a new name table */
	nameTable *nt2 = namesNew();

	/* set values */
	nt2->n_of_names = nt->n_of_names;
	nt2->cap_names = nt->cap_names;
	nt2->parent = nt->parent;

	/* create new name and value lists */
	nt2->names = (char **)malloc(sizeof(char *) * nt2->cap_names);
	nt2->values = (object **)malloc(sizeof(object *) * nt2->cap_names);
	memset(nt2->names, 0, sizeof(char *) * nt2->cap_names);

	/* copy names and values */
	for (int i = 0; i < nt2->n_of_names; i++) {

		/* create a new name */
		//char *name = (char *)malloc(strlen(nt->names[i]) + 1);
		//strcpy(name, nt->names[i]);
		char *name = nt->names[i];

		/* create a new value */
		object *o = objectCopy(nt->values[i]);

		/* set name and value */
		nt2->names[i] = name;
		nt2->values[i] = o;

		/* reference tracking */
		XINCREF(o);
		if (o->type & OBJECT_POINTER)
			XINCREF(O_OBJ(O_PTR(o)->val));

		if (DEBUG) fprintf(debug_file, "[names] Copied %p[name=%s,type=%d] to %p[type=%d]\n", nt->values[i], nt->names[i], nt->values[i]->type, o, o->type);
	}

	if (DEBUG) fprintf(debug_file, "[names] Copied %p(%d) to %p(%d)\n", nt, nt->id, nt2, nt2->id);

	/* return table */
	return nt2;
}

/* set a value to a name */
extern void namesSet(nameTable *nt, char *name, object *value) {

	/* resize lists if neccessary */
	if (nt->n_of_names >= nt->cap_names) {

		nt->cap_names *= 2;
		nt->names = (char **)realloc(nt->names, sizeof(char *) * nt->cap_names);
		nt->values = (object **)realloc(nt->values, sizeof(object *) * nt->cap_names);
	}

	/* get the index and set values in list */
	unsigned int idx = namesIndex(nt, name);

	/* new seperate string */
	//char *new_name = (char *)malloc(strlen(name) + 2);
	//strcpy(new_name, name);
	//name = new_name;

	/* decrement reference if value is found */
	if (idx < nt->n_of_names) {
		XDECREF(nt->values[idx]);

		/* for pointers */
		if (nt->values[idx]->type & OBJECT_POINTER && !(nt->values[idx]->type & OBJECT_ARRAY))
			XDECREF(O_OBJ(O_PTR(nt->values[idx])->val));
	}

	/* increase reference for new value */
	XINCREF(value);

	/* for pointers */
	if (value->type & OBJECT_POINTER && !(value->type & OBJECT_ARRAY)) {

		XINCREF(O_OBJ(O_PTR(value)->val));
	}

	/* store stuff */
	nt->names[idx] = name;
	nt->values[idx] = value;

	/* advance number of values if needed */
	if (idx >= nt->n_of_names) nt->n_of_names++;
}

/* get the index of a name if it exists */
extern unsigned int namesIndex(nameTable *nt, char *name) {

	/* loop through list and compare values */
	int i;
	for (i = 0; i < nt->n_of_names; i++) {

		if (!strcmp(nt->names[i], name))
			break; /* exit out of loop to get correct value */
	}

	return i; /* return the final index */
}

/* get the value of a name */
extern object *namesGet(nameTable *nt, char *name) {

	return namesGetN(nt, name, 1);
}

/* get the value of a name */
extern object *namesGetN(nameTable *nt, char *name, int n) {

	/* get index of name */
	unsigned int idx = namesIndex(nt, name);
	object *o = NULL;

	/* error */
	if (idx == nt->n_of_names) {

		/* try and get parent value */
		if ((nt->parent != NULL) && (n != 0)) {

			o = namesGetN(nt->parent, name, 1);
		}

		/* no success */
		if (o == NULL)
			return NULL;
	}
	else o = nt->values[idx];

	/* return value */
	return o;
}

/* get a value from a string object */
extern object *namesGetFromString(nameTable *nt, object *obj) {

	/* get object */
	object *o = namesGet(nt, O_ARRAY(obj)->n_start);

	/* create appropriate error if needed */
	if (o == NULL) {

		if (!errorIsSet()) {

			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_UNDEFINEDNAME,
					 "Undefined name/reference");
			errorSetPos(obj->lineno, obj->colno, obj->fname);
		}
		return NULL;
	}

	/* return object */
	return o;
}

/* free a name table */
extern void namesFree(nameTable *nt) {

	if (DEBUG) fprintf(debug_file, "[names] Freeing %p\n", nt);

	/* decrease references for each object */
	for (int i = 0; i < nt->n_of_names; i++) {

		if (!is_at_end) {

			/* debug info */
			if (DEBUG) fprintf(debug_file, "[names] Dereffing '%s'...\n", nt->names[i]);
	
			XDECREF(nt->values[i]);
	
			/* for pointers */
			if (nt->values[i]->type & (1 << 3)) {
	
				XDECREF(O_OBJ(O_PTR(nt->values[i])->val));
			}
	
			if (DEBUG) fprintf(debug_file, "[names] Dereffed '%s'.\n", nt->names[i]);
		}
	}

	if (DEBUG) fprintf(debug_file, "[names] Dereffed all values for nt %p\n", nt);

	/* free lists */
	free(nt->names);
	free(nt->values);

	/* free table */
	free(nt);

	if (DEBUG) fprintf(debug_file, "[names] Freed nt %p\n", nt);
}