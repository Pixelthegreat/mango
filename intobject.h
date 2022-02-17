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
extern object *intcharobjectNew(int isch, int val);

#endif /* _INTOBJECT_H */