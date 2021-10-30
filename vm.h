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
extern void vmLoadIdataTable(vm *v); /* load a vm's idata table if necessary */
extern object *vmHandle(vm *v, unsigned int i); /* return an object from an instruction */
extern void vmFree(vm *v); /* free a vm */

#endif /* _VM_H */