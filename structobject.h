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

/* struct object */
#ifndef _STRUCTOBJECT_H
#define _STRUCTOBJECT_H

#include "object.h"

/* structobject */
typedef struct _structobject {
	OB_HEAD
	struct _structobject *parent; /* pointer to parent struct */
	nameTable *nt; /* table of names */
	context *ctx; /* context */
	char *struct_name; /* name of struct */
	int is_templ; /* template = how the structure will look, otherwise = actual instance of struct */
} structobject;

/* macros */
#define O_STRUCT(o) ((structobject *)(o))

/* functions */
extern object *structobjectNew(context *ctx, char *struct_name);
extern object *structobjectInstance(object *s);

#endif /* _STRUCTOBJECT_H */