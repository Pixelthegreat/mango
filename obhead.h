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

/* header to fix the object head issues from names.c */
#ifndef _OBHEAD_H
#define _OBHEAD_H

#include <stdlib.h>

/* object types */
#define OBJECT_INT 0
#define OBJECT_CHR 1
#define OBJECT_FUNC 2
#define OBJECT_STRUCT 3

#define OBJECT_ARRAY (1 << 2)
#define OBJECT_POINTER (1 << 3)
#define OBJECT_TYPE (1 << 4)
#define OBJECT_DL (1 << 5)

/* these will make builtin function creation much less ugly */
#define FUNC_ARGNAME_LIST(n) ((char **)malloc(sizeof(char *) * n))
#define FUNC_ARGTYPE_LIST(n) ((unsigned char *)malloc(sizeof(unsigned char) * n))

/* object head */
#define OB_HEAD 	unsigned int refcnt; /* number of references */ \
	unsigned char type; /* type of object */ \
	unsigned int lineno; /* for errors */ \
	unsigned int colno; /* for errors */ \
	char *fname; /* for errors */ \
	unsigned int sz; /* for objectCopy */\
	char copied; /* copied from existing object's data (using objectCopy) */

/* object */
typedef struct {
	OB_HEAD
} object;

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

/* get a pointer type */
#define POINTER(t) (t | OBJECT_POINTER)

#define O_OBJ(o) ((object *)(o))
#define O_CHR(o) ((charobject *)(o))
#define O_INT(o) ((intobject *)(o))

/* get the size of a type */
#define O_TPSZ(o) (((o) & OBJECT_POINTER)? sizeof(void *): (((o) & 3) == OBJECT_INT? sizeof(int): (((o) & 3) == OBJECT_CHR? sizeof(char): 0)))

/* macros (inspired by python's stellar object
allocation system it has :D) */
#define INCREF(o) ((o)->refcnt++)
#define DECREF(o) ((o)->refcnt--)
#define XINCREF(o) if ((o) != NULL) ((o)->refcnt++)
#define XDECREF(o) if ((o) != NULL && (o)->refcnt > 0) ((o)->refcnt--)

#endif /* _OBHEAD_H */