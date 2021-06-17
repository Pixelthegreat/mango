/* file.h -- * ellaborate * file management system
it should be noted that in no way am I trying to
make something like how actual files work in code,
just my own somewhat similar system.
*/
#ifndef _FILE_H
#define _FILE_H

#include <stdlib.h> /* malloc, realloc, free, etc */
#include <stdio.h> /* FILE */

/* modes */
#define FILE_MODE_READ			0
#define FILE_MODE_WRITE			1
#define FILE_MODE_READWRITE		2

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
	FILE *f; /* file i guess */
} file;

/* functions */
extern file *fileNew(char *fname, unsigned int mode); /* open a new file */
extern void fileRead(file *f); /* read the file contents (enough to fit in buffer) */
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

#endif /* _FILE_H */