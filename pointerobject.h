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

/* pointer object type */
#ifndef _POINTEROBJECT_H
#define _POINTEROBJECT_H

#ifndef NOOBJECT
#include "object.h"
#endif

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