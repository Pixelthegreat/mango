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

#ifndef _NAMETABLE_H
#define _NAMETABLE_H

#include "obhead.h"

/* name table for storing names */
typedef struct _nameTable {
	struct _nameTable *parent; /* for accessing "outside" values */
	char **names; /* list of names */
	object **values; /* list of values */
	unsigned int n_of_names; /* number of names */
	unsigned int cap_names; /* capacity of lists */
	int id; /* identity number for better tracking of what is what */
} nameTable;

#endif /* _NAMETABLE_H */