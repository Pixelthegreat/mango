#include "names.h" /* header */
#include "object.h" /* object */
#include "error.h" /* error handling */
#include <stdlib.h> /* malloc, realloc, free */
#include <string.h> /* strcmp */
#include <stdio.h> /* printf */

/* create a name table */
extern nameTable *namesNew() {

	/* malloc a table */
	nameTable *nt = (nameTable *)malloc(sizeof(nameTable));

	/* failed once again :(O */
	if (nt == NULL)
		return NULL;

	/* create lists */
	nt->names = (char **)malloc(sizeof(char *) * 8);
	nt->values = (object **)malloc(sizeof(char *) * 8);

	/* assign values */
	nt->n_of_names = 0;
	nt->cap_names = 8;
	nt->parent = NULL;

	/* return */
	return nt;
}

/* set a value to a name */
extern void namesSet(nameTable *nt, char *name, object *value) {

	/* resize lists if neccessary */
	if (nt->n_of_names >= nt->cap_names) {

		nt->cap_names *= 2;
		nt->names = (char **)realloc(nt->names, sizeof(char *) * nt->cap_names);
		nt->values = (object **)realloc(nt->values, sizeof(char *) * nt->cap_names);
	}

	/* get the index and set values in list */
	unsigned int idx = namesIndex(nt, name);

	/* new seperate string */
	char *new_name = (char *)malloc(strlen(name) + 2);
	strcpy(new_name, name);
	name = new_name;

	/* decrement reference if value is found */
	if (idx < nt->n_of_names)
		XDECREF(nt->values[idx]);

	/* increase reference for new value */
	XINCREF(value);

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
	for (i = 0; i < nt->n_of_names; i++)
		if (!strcmp(nt->names[i], name))
			break; /* exit out of loop to get correct value */

	return i; /* return the final index */
}

/* get the value of a name */
extern object *namesGet(nameTable *nt, char *name) {

	/* get index of name */
	unsigned int idx = namesIndex(nt, name);

	/* error */
	if (idx == nt->n_of_names) {

		/* try and get parent value */
		object *o = NULL;
		if (nt->parent != NULL)
			o = namesGet(nt->parent, name);

		/* no success */
		if (o == NULL) {
			return NULL;

			/* get outta here */
			return NULL;
		}
	}

	/* return value */
	return nt->values[idx];
}

/* get a value from a string object */
extern object *namesGetFromString(nameTable *nt, object *obj) {

	/* get object */
	object *o = namesGet(nt, (char *)obj->value);

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

	/* free each name */
	for (int i = 0; i < nt->n_of_names; i++)
		free(nt->names[i]);

	/* free lists */
	free(nt->names);
	free(nt->values);

	/* free table */
	free(nt);
}