/* integer objects */
#ifndef _INTOBJECT_H
#define _INTOBJECT_H

#include "object.h"

/* intobject for ints */
typedef struct {
	OB_HEAD
	int val; /* value for integer */
} intobject;

/* charobject for chars */
typedef struct {
	OB_HEAD
	char val; /* value for char */
} charobject;

/* macros */
#define O_CHR(o) ((charobject *)(o))
#define O_INT(o) ((intobject *)(o))

/* functions */
extern object *intobjectNew(int val);
extern object *charobjectNew(char val);

#endif /* _INTOBJECT_H */