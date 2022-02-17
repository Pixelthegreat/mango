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

/* object handling */
#ifndef _OBJECT_H
#define _OBJECT_H

#include <stdlib.h>

/* debug mode and other constants */
extern int DEBUG;
extern int objdout;
extern int INT_SIGNAL;

/* operation types */
#define OPERATION_PLUS 0
#define OPERATION_MINUS 1
#define OPERATION_MUL 2
#define OPERATION_DIV 3
#define OPERATION_MOD 4
#define OPERATION_EQ 5
#define OPERATION_NEQ 6
#define OPERATION_LT 7
#define OPERATION_GT 8
#define OPERATION_LTE 9
#define OPERATION_GTE 10

#include "names.h"
#include "context.h"
#include "obhead.h"

/* functions */
extern void objectIntHandler(int n); /* interrupt handler for SIGINT (aka Ctrl+C) */
extern void objectSegvHandler(int n); /* interrupt handler for segmentation faults */
extern object *objectNew(unsigned char type, unsigned int size); /* create a new object and increase reference count by 1 */
//extern object *objectNewInt(int value); /* create a new integer */
//extern object *objectNewString(char *value); /* create a new string */
//extern object *objectNewChar(char value); /* create a new char */
//extern object *objectNewPointer(object *vo); /* create a pointer value */
extern void objectFree(object *obj); /* free an object */
extern object *objectOperation(object *obj, object *other, unsigned int op_num); /* perform an operation (i.e., +, -, <, >, etc) on an object */
extern void objectCollect(); /* garbage collection routine */
extern void objectFreeAll(); /* free all objects */
//extern object *objectRepresent(object *obj); /* represent an object */
//extern void objectWrite(int fd, object *value); /* write a string value to a file descriptor */
//extern object *objectRead(int fd, object *buf); /* read text from file */
//extern void objectPrint(object *obj); /* wrapper for "write(FD_CONSOLE, represent(value));" */
extern object *objectCopy(object *o); /* copy an object */

/* builtin functions */
extern object *builtinWrite(object **ob_args, void *ctx);
extern object *builtinRead(object **ob_args, void *ctx);

#endif /* _OBJECT_H */