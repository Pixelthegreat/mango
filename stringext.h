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

#endif /* _STRINGEXT_H */