#ifndef _ARRAYOBJECT_H
#define _ARRAYOBJECT_H

/* object system */
#include "object.h"

/* convert to array object */
#define O_ARRAY(o) ((arrayobject *)(o))

/* array object */
typedef struct {
	OB_HEAD
	int n_len; /* length of array (aka number of items it can store) */
	int n_sz; /* size of each array unit */
	void *n_start; /* start of array */
} arrayobject;

/* functions */
extern object *arrayobjectNew(int, unsigned char); /* create a new array of length n and type t */
extern void arrayobjectFree(object *); /* free an array */

#endif /* _ARRAYOBJECT_H */