/* bytecode.h -- bytecode compiler */
#ifndef _BYTECODE_H
#define _BYTECODE_H

#include "node.h" /* node system */
#include "file.h" /* file system */

/* bytecode types */
#define BYTECODE_EX		0 /* execute immediately */
#define BYTECODE_CMP	1 /* compile to bytecode executable */
#define BYTECODE_CLIB	2 /* compile to bytecode library */
#define BYTECODE_LIB	3 /* bytecode library */
#define BYTECODE_BC		4 /* run from file */

/* bytecode struct */
typedef struct {
	unsigned char *bytes; /* actual bytes */
	unsigned int len; /* number of bytes stored */
	unsigned int cap; /* capacity of byte array */
	unsigned int mode; /* mode of bytecode */
	node *n; /* default node */
	file *f; /* not used in EX mode */
} bytecode;

/* bytecode functions */
extern unsigned int bytecodeGetc(unsigned char c);

extern bytecode *bytecodeNew(node *n, unsigned int mode); /* create new bytecode object */
extern void bytecodeAdd(bytecode *bc, unsigned char b); /* add byte to byte array */
extern void bytecodeComp(bytecode *bc); /* compile node */
extern void bytecodeWrite(bytecode *bc, node *n); /* write node */
extern void bytecodeWriteHeader(bytecode *bc); /* write bytecode file header */
extern void bytecodeWriteInt(bytecode *bc, int i); /* write integer */
extern void bytecodeWriteStr(bytecode *bc, char *s); /* write string */
extern void bytecodeWriteIdt(bytecode *bc, char *s); /* same as bytecodeWriteStr, uses different signature byte */
extern void bytecodeWriteCall(bytecode *bc, node *n); /* write a function call */
extern void bytecodeWriteVarAcc(bytecode *bc, node *n); /* write a variable access node */
extern void bytecodePrintf(bytecode *bc); /* print bytecode data in hexdump style */
extern void bytecodeFree(bytecode *bc); /* free a bytecode object */

#endif /* _BYTECODE_H */