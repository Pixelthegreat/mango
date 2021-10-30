#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "names.h" /* variable name table */

/* context type: provide a scope for variables */
typedef struct {
	nameTable *nt; /* table of names */
	char *fn; /* file name of scope */
	char *sn; /* name of scope */
} context;

/* functions */
extern context *contextNew(char *, char *); /* create a new context */
extern void contextFree(context *); /* free a context */

#endif /* _CONTEXT_H */