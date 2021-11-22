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

#ifndef _ARGPARSE_H
#define _ARGPARSE_H

/* flag types */
#define FLAG_COMP_LIB (unsigned int)(0)
#define FLAG_COMP_BIN (unsigned int)(1)
#define FLAG_IDATA (unsigned int)(2)

/* functions */
extern int argparse_set_flag(unsigned int); /* set a flag */
extern int argparse_get_flag(unsigned int); /* get flag state */
extern int argparse_flag2bcm(); /* get bytecode mode from different flags */
extern int argparse(int, char **); /* parse arguments from argc and argv */
extern int argparse_one(char *); /* parse one argument */
extern int argparse_two(char *, char *); /* parse two arguments */
extern char **argparse_filenames(); /* receive pointer to filename list */
extern int argparse_fileslen(); /* get number of names in filename list */
extern void argparse_free(); /* free file name list */

#endif /* _ARGPARSE_H */