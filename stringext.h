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

/* an extension to string.h */
#ifndef _STRINGEXT_H
#define _STRINGEXT_H

#include <stdio.h> /* FILE */

/*
 * input functions
 *
 * note to reader: the "d" prefix stands for dynamic,
 * implying that these are supposed to act as more
 * versatile versions of existing string functions
 * in <string.h>. The reason for the name is that unlike
 * the original gets function, which reads a buffer of
 * an unknown size (which is quite literally a dangerous
 * function), dgets uses a dynamic buffer which reallocates
 * itself during the reading of input.
 */
extern char *dfgets(FILE *f); /* get string from file */
extern char *dgets(); /* get stdin string */
extern char *dstrcat(char *a, char *b); /* concatenate two strings and return new buffer as result */
extern char *fngext(char *fn); /* get extension for filename */

#endif /* _STRINGEXT_H */