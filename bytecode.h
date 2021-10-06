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
extern unsigned int bytecodeGetc(unsigned char);

extern bytecode *bytecodeNew(node *, unsigned int); /* create new bytecode object */

extern void bytecodeAdd(bytecode *, unsigned char); /* add byte to byte array */
extern void bytecodeAddIdat(bytecode *, char *); /* idata (independant data) mode only */
extern void bytecodeComp(bytecode *); /* compile node */
extern void bytecodeWrite(bytecode *, node *); /* write node */
extern void bytecodeWriteHeader(bytecode *); /* write bytecode file header */
extern void bytecodeWriteErrInf(bytecode *, unsigned int, unsigned int); /* write error info */
extern void bytecodeWriteFileInf(bytecode *, char *); /* write a file name to change current file attribute */
extern void bytecodeWriteInt(bytecode *, int); /* write integer */
extern void bytecodeWriteIntNS(bytecode *, int); /* write integer without signature byte in beginning */
extern void bytecodeWriteStr(bytecode *, char *); /* write string */
extern void bytecodeWriteStrI(bytecode *, char *); /* write string and ignore idata mode */
extern void bytecodeWriteIdt(bytecode *, char *); /* same as bytecodeWriteStr, uses different signature byte */
extern void bytecodeWriteCall(bytecode *, node *); /* write a function call */
extern void bytecodeWriteVarAcc(bytecode *, node *); /* write a variable access node */
extern void bytecodeWriteVarAsg(bytecode *, node *); /* write a variable assign node */
extern void bytecodeWriteInc(bytecode *, node *); /* increment operator (++) */
extern void bytecodeWriteDec(bytecode *, node *); /* decrement operator (--) */
extern void bytecodeWriteBinOp(bytecode *, node *); /* binary operation */
extern void bytecodeWriteUnOp(bytecode *, node *); /* unary operation */
extern void bytecodeWriteGetItem(bytecode *, node *); /* getitem (arr[idx]) */
extern void bytecodeWriteSetItem(bytecode *, node *); /* setitem (arr[idx] = (?);) */
extern void bytecodeWriteVarUndefined(bytecode *, node *); /* undefined variable */
extern void bytecodeWriteVarNew(bytecode *, node *); /* new variable */
extern void bytecodeWriteIfStatement(bytecode *, node *); /* if statement */
extern void bytecodeWriteWhile(bytecode *, node *); /* while loop */
extern void bytecodeWriteFor(bytecode *, node *); /* for loop */
extern void bytecodeWriteFuncDec(bytecode *, node *); /* function declaration */
extern void bytecodeWriteFuncDef(bytecode *, node *); /* function definition */
extern void bytecodeWriteExtern(bytecode *, node *); /* external ref */
extern void bytecodeWriteCUR(bytecode *, node *); /* const, unsigned, or return */
extern void bytecodeWriteStruct(bytecode *, node *); /* struct */
extern void bytecodeWriteTypeDef(bytecode *, node *); /* type definition */
extern void bytecodeWriteElse(bytecode *, node *); /* else */
extern void bytecodeWriteInclude(bytecode *, node *, char *); /* include a file */
extern void bytecodeFinish(bytecode *); /* finish bytecode */

extern void bytecodePrintf(bytecode *); /* print bytecode data in hexdump style */
extern void bytecodeFree(bytecode *); /* free a bytecode object */

#endif /* _BYTECODE_H */