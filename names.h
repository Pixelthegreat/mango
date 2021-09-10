/* names.h -- variable naming system */
#ifndef _NAMES_H
#define _NAMES_H

#include "object.h"

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
extern void namesFree(nameTable *nt); /* free a table */

#endif /* _NAMES_H */