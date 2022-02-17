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

/* bytecode virtual machine */
#ifndef _VM_H
#define _VM_H

/* includes */
#include "context.h"
#include "bytecode.h"
#include "object.h"

/* macros */
#define IS_IDAT(fl) (fl & 0x01)

/* vm struct */
typedef struct {
	context *ctx; /* context info */
	void *bc; /* for running code */
	unsigned int bc_len; /* length of bc */
	int fromf; /* if it was from a file, then we can free it's bytecode struct */
	unsigned int nofbytes; /* number of bytes that an instruction took */
	unsigned int lowbi; /* lowest byte index: the index of a byte when VM calls vmHandle directly */
	unsigned char bcflags; /* flag values for bytecode */
	char *idata[256]; /* 256 entries for idata */
	unsigned char nidat; /* number of idata table entries added */
} vm;

/* functions */
extern vm *vmNew(bytecode *bc); /* from existing bytecode */
extern vm *vmNewFromFile(char *f); /* file */
extern void vmExec(vm *v); /* execute code in a vm */
extern void vmLoadBuiltins(); /* initialise builtin functions for VM */
extern void vmLoadIdataTable(vm *v); /* load a vm's idata table if necessary */
extern object *vmHandle(vm *v, unsigned int i); /* return an object from an instruction */
extern void vmGetErrorInfo(vm *v, unsigned int *lineno, unsigned int *colno); /* get error information if there is any */
extern void vmFree(vm *v); /* free a vm */
extern void vmFreeAll(); /* free all created vms */

/* macros */
#define vmGetInt(v, p) (((u8*)(v)->bc)[(p)+0]<<24)|(((u8*)(v)->bc)[(p)+1]<<16)|(((u8*)(v)->bc)[(p)+2]<<8)|(((u8*)(v)->bc)[(p)+3]<<0)

#endif /* _VM_H */