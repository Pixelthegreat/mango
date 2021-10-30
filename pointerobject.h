/* pointer object type */
#ifndef _POINTEROBJECT_H
#define _POINTEROBJECT_H

#include "object.h"

/* pointer object type */
typedef struct {
	OB_HEAD;
	void *val;
} pointerobject;

/* macros */
#define O_PTR(o) ((pointerobject *)(o))

/* functions */
extern object *pointerobjectNew(int tp, void *p); /* create a pointer object */

#endif /* _POINTEROBJECT_H */