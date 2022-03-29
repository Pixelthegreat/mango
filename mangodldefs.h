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

/* function definitions for mangodl */
#ifndef _MANGODLDEFS_H
#define _MANGODLDEFS_H

#include "obhead.h"
#include "nametable.h"

/* function types */
typedef void (*mangodl_errorset_t)(unsigned int, unsigned int, const char *);
typedef void (*mangodl_errorsetpos_t)(unsigned int, unsigned int, char *);
typedef unsigned int (*mangodl_errorisset_t)();
typedef object * (*mangodl_objectnew_t)(unsigned char, unsigned int);
typedef object * (*mangodl_intobjectnew_t)(int);
typedef object * (*mangodl_charobjectnew_t)(char);
typedef object * (*mangodl_arrayobjectnew_t)(int, unsigned char);
typedef object * (*mangodl_pointerobjectnew_t)(int, void *);
typedef void (*mangodl_namesset_t)(nameTable *, char *, object *);
typedef object * (*mangodl_namesget_t)(nameTable *, char *);
typedef object * (*mangodl_functionbuiltinnew_t)(char *, unsigned char, char **, unsigned char *, int, object * (*)(object **, void *));
typedef int (*mangodl_init_t)(nameTable *);

#endif /* _MANGODLDEFS_H */