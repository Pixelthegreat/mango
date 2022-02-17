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

/* object types */
#define OBJECT_INT 0
#define OBJECT_CHR 1
#define OBJECT_FUNC 2
#define OBJECT_STRUCT 3

#define OBJECT_ARRAY (1 << 2)
#define OBJECT_POINTER (1 << 3)
#define OBJECT_TYPE (1 << 4)

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

/* get a pointer type */
#define POINTER(t) (t | OBJECT_POINTER)

#define O_OBJ(o) ((object *)(o))

/* get the size of a type */
#define O_TPSZ(o) (((o) & OBJECT_POINTER)? sizeof(void *): (((o) & 3) == OBJECT_INT? sizeof(int): (((o) & 3) == OBJECT_CHR? sizeof(char): 0)))

/* macros (inspired by python's stellar object
allocation system it has :D) */
#define INCREF(o) ((o)->refcnt++)
#define DECREF(o) ((o)->refcnt--)
#define XINCREF(o) if ((o) != NULL) ((o)->refcnt++)
#define XDECREF(o) if ((o) != NULL && (o)->refcnt > 0) ((o)->refcnt--)

#endif /* _OBHEAD_H */