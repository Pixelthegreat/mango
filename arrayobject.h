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

#ifndef _ARRAYOBJECT_H
#define _ARRAYOBJECT_H

/* object system */
#ifndef NOOBJECT
#include "object.h"
#endif

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