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

/* eof byte */
#define BYTECODE_EOF	0x00

/* bytecode struct */
typedef struct {
	unsigned char *bytes; /* actual bytes */
	char **idata; /* for independant data mode */
	unsigned int is_idat; /* is in independant data mode */
	unsigned int i_len; /* idata length */
	unsigned int i_cap; /* idata cap */
	unsigned int len; /* number of bytes stored */
	unsigned int cap; /* capacity of byte array */
	unsigned int mode; /* mode of bytecode */
	node *n; /* default node */
	node *cn; /* current node being processed */
	file *f; /* not used in EX mode */
	char *curr_fname; /* current filename */
	char *prev_fname; /* previous filename */
} bytecode;

/* bytecode functions */
extern unsigned int bytecodeGetc(unsigned char c);

extern bytecode *bytecodeNew(node *n, unsigned int mode); /* create new bytecode object */

extern void bytecodeAdd(bytecode *bc, unsigned char b); /* add byte to byte array */
extern void bytecodeAddIdat(bytecode *bc, char *s); /* idata (independant data) mode only */
extern void bytecodeComp(bytecode *bc); /* compile node */
extern void bytecodeWrite(bytecode *bc, node *n); /* write node */
extern void bytecodeWriteHeader(bytecode *bc); /* write bytecode file header */
extern void bytecodeWriteErrInf(bytecode *bc, unsigned int lineno, unsigned int colno); /* write error info */
extern void bytecodeWriteFileInf(bytecode *bc, char *fname); /* write a file name to change current file attribute */
extern void bytecodeWriteInt(bytecode *bc, int i); /* write integer */
extern void bytecodeWriteIntNS(bytecode *bc, int i); /* write integer without signature byte in beginning */
extern void bytecodeWriteStr(bytecode *bc, char *s); /* write string */
extern void bytecodeWriteStrI(bytecode *bc, char *s); /* write string and ignore idata mode */
extern void bytecodeWriteIdt(bytecode *bc, char *s); /* same as bytecodeWriteStr, uses different signature byte */
extern void bytecodeWriteCall(bytecode *bc, node *n); /* write a function call */
extern void bytecodeWriteVarAcc(bytecode *bc, node *n); /* write a variable access node */
extern void bytecodeWriteInc(bytecode *bc, node *n); /* increment operator (++) */
extern void bytecodeWriteDec(bytecode *bc, node *n); /* decrement operator (--) */
extern void bytecodeWriteBinOp(bytecode *bc, node *n); /* binary operation */
extern void bytecodeWriteUnOp(bytecode *bc, node *n); /* unary operation */
extern void bytecodeWriteGetItem(bytecode *bc, node *n); /* getitem (arr[idx]) */
extern void bytecodeWriteSetItem(bytecode *bc, node *n); /* setitem (arr[idx] = (?);) */
extern void bytecodeWriteInclude(bytecode *bc, node *n); /* include a file */

extern void bytecodePrintf(bytecode *bc); /* print bytecode data in hexdump style */
extern void bytecodeFree(bytecode *bc); /* free a bytecode object */

#endif /* _BYTECODE_H */