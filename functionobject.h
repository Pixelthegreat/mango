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

/* function object */
#ifndef _FUNCTIONOBJECT_H
#define _FUNCTIONOBJECT_H

#include "object.h"
#include "vm.h"

/* for builtin functions */
typedef object *(*bfunc_handle_t)(object **, void *);

/* function object struct */
typedef struct {
	OB_HEAD
	char *func_name; /* name of function */
	unsigned char rt_type; /* return type of function */
	char **fa_names; /* function argument names */
	unsigned char *fa_types; /* types of function arguments */
	int n_of_args; /* number of arguments */
	unsigned char *fb_start; /* function body start (NULL to indicate function that hasn't been defined yet), also pointer to function if it is a builtin function */
	int fb_n; /* number of nodes in function body */
	int is_builtin; /* builtin functions are functions that run C code but can be called in Mango */
	vm *ov; /* original vm (to avoid segfaults and keep original settings/idata) */
} functionobject;

/* macros */
#define O_FUNC(o) ((functionobject *)(o))

/* functions */
extern object *functionobjectNew(char *func_name, unsigned char rt_type, char **fa_names, unsigned char *fa_types, int n_of_args);
extern object *functionobjectBuiltinNew(char *func_name, unsigned char rt_type, char **fa_names, unsigned char *fa_types, int n_of_args, bfunc_handle_t f);

#endif /* _FUNCTIONOBJECT_H */