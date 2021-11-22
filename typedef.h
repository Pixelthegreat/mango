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

/* type definition system */
#ifndef _TYPEDEF_H
#define _TYPEDEF_H

#ifndef NOOBJECT
#include "object.h"
#endif

/* type struct */
typedef struct {
	OB_HEAD
	char *tp_name; /* name of type */
	unsigned char tp_type; /* type of type */
} typeobject;

/* functions */
extern typeobject *typeNew(char *tp_name, unsigned char tp_type); /* creates a new type */
extern void typeRegister(char *tp_name, unsigned char tp_type); /* creates a new type and adds it to a list */

/* macros */
#define O_TYPE(o) ((typeobject *)(o))

#endif /* _TYPEDEF_H */