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

#ifndef _FILE_H
#define _FILE_H

#include <stdlib.h> /* malloc, realloc, free, etc */
#include <stdio.h> /* FILE */

/* modes */
#define FILE_MODE_READ			0
#define FILE_MODE_WRITE			1
#define FILE_MODE_READWRITE		2
/* modifiers */
#define FILE_BINARY				(1 << 2)
#define FILE_TEXT				(0)

/* file struct */
typedef struct {
	char *fname; /* file name */
	unsigned int fd; /* file descriptor */
	char *buffer; /* content buffer */
	unsigned int mode; /* file mode */
	unsigned int is_open; /* check file state */
	unsigned int f_pos; /* file position */
	unsigned int buffer_cap; /* capacity of buffer */
	unsigned int at_eof; /* determine if we are at the end of the file */
	char first_char; /* first character in file */
	unsigned int is_extern; /* determine if the file was from an external source (i.e., stdin, stdout, ...) */
	FILE *f; /* file i guess */
} file;

/* functions */
extern file *fileNew(char *fname, unsigned int mode); /* open a new file */
extern file *fileFromFD(int fd); /* get a file from a descriptor */
extern void fileRead(file *f); /* read the file contents (enough to fit in buffer) */
extern char *fileReadLine(int fd); /* read from a descriptor */
extern char *fileReadAll(file *f); /* read all the contents of a file */
extern void fileReadChar(file *f, unsigned int idx); /* read a character */
extern void fileWrite(file *f, char *text); /* write to a file */
extern void fileSeek(file *f, unsigned int pos); /* reset file position */
extern void fileReset(file *f); /* reset file buffer and position */
extern void fileFlush(file *f); /* write the contents of buffer and reset buffer */
extern file *fileGet(unsigned int fd); /* get a file from a file descriptor */
extern void fileClose(file *f); /* close a file */
extern void fileFree(file *f); /* free a file from memory */
extern void fileFreeAll(); /* free all files and file list */

extern char *fileGetMode(file *f); /* get mode string for file */

/* get a file from a descriptor */
#ifdef __MINGW32__ /* mingw defines an IO buffer */
#define FILEGET(n) (&_iob[n])
#else
#define FILEGET(n) (fdopen(n, "r+"))
#endif

#endif /* _FILE_H */