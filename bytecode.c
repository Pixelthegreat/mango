/* header */
#include "bytecode.h"
#include "error.h" /* errors */
#include <stdlib.h> /* malloc, realloc, free */

/* get a character */
extern unsigned int bytecodeGetc(unsigned char c) {

	if (c >= 32 && c <= 126)
		return (unsigned int)c;

	return (unsigned int)'.'; /* invalid */
}

/* create new bytecode */
extern bytecode *bytecodeNew(node *n, unsigned int mode) {

	/* malloc bytecode */
	bytecode *bc = (bytecode *)malloc(sizeof(bytecode));

	if (bc == NULL)
		return NULL;

	/* values */
	bc->bytes = (unsigned char *)malloc(8);
	bc->len = 0;
	bc->cap = 8;
	bc->mode = mode;
	bc->n = n;
	bc->f = NULL;

	/* return */
	return bc;
}

/* add a byte to the code */
extern void bytecodeAdd(bytecode *bc, unsigned char b) {

	/* resize list if needed */
	if (bc->len >= bc->cap) {
	
		bc->cap *= 2;
		bc->bytes = (unsigned char *)realloc(bc->bytes, bc->cap);
	}

	/* add byte */
	bc->bytes[bc->len++] = b;
}

/* compile node */
extern void bytecodeComp(bytecode *bc) {

	/* write header */
	bytecodeWriteHeader(bc);

	/* compile */
	bytecodeWrite(bc, bc->n);
}

/* write node */
extern void bytecodeWrite(bytecode *bc, node *n) {

	/* integer */
	if (n->type == NODE_INT) {

		/* write integer */
		bytecodeWriteInt(bc, atoi(n->tokens[0]->t_value));
	}

	/* string */
	else if (n->type == NODE_STRING) {

		/* write string */
		bytecodeWriteStr(bc, n->tokens[0]->t_value);
	}

	/* statements */
	else if (n->type == NODE_STATEMENTS) {

		/* loop and write */
		for (unsigned int i = 0; i < n->n_of_children; i++) {

			bytecodeWrite(bc, n->children[i]);
		}
	}

	/* access a variable's value */
	else if (n->type == NODE_VARACCESS) {

		/* write node */
		bytecodeWriteVarAcc(bc, n);
	}

	/* function call */
	else if (n->type == NODE_CALL) {

		/* write node */
		bytecodeWriteCall(bc, n);
	}

	else {
		/* unimplemented */
		errorSet(ERROR_TYPE_BYTECODE,
				 ERROR_CODE_BYTECODEUNIMPL,
				 "Unimplemented");
		errorSetPos(n->lineno,
					n->colno,
					n->fname);
	}

	return;
}

/* write bytecode header */
extern void bytecodeWriteHeader(bytecode *bc) {

	bytecodeAdd(bc, 0x0B);
	bytecodeAdd(bc, 'm');

	/* library */
	if (bc->mode == BYTECODE_CLIB) {

		/* write header */
		bytecodeAdd(bc, 'l');
		bytecodeAdd(bc, 0x0F);
	}

	/* executable or other */
	else {

		/* write header */
		bytecodeAdd(bc, 'c');
		bytecodeAdd(bc, 0x0E);
	}
}

/* write an integer */
extern void bytecodeWriteInt(bytecode *bc, int i) {

	/* write integer signature */
	bytecodeAdd(bc, 0x9B);

	/* write integer bytes */
	bytecodeAdd(bc, (i >> 24) & 0xFF);
	bytecodeAdd(bc, (i >> 16) & 0xFF);
	bytecodeAdd(bc, (i >> 8) & 0xFF);
	bytecodeAdd(bc, i & 0xFF);
}

/* write a string */
extern void bytecodeWriteStr(bytecode *bc, char *s) {

	/* add signature */
	bytecodeAdd(bc, 0x9E);

	/* loop through string */
	char c;
	while (c = *s++) {

		bytecodeAdd(bc, (unsigned char)c); /* add char */
	}

	/* add null term char */
	bytecodeAdd(bc, 0x00);
}

/* write an identifier */
extern void bytecodeWriteIdt(bytecode *bc, char *s) {

	/* add signature */
	bytecodeAdd(bc, 0x9F);

	/* loop through string */
	char c;
	while (c = *s++) {

		bytecodeAdd(bc, (unsigned char)c); /* add char */
	}

	/* add null term char */
	bytecodeAdd(bc, 0x00);
}

/* write a function call */
extern void bytecodeWriteCall(bytecode *bc, node *n) {

	/* write child */
	bytecodeWrite(bc, n->children[0]);

	/* error */
	if (errorIsSet())
		return;

	/* write number of arguments */
	bytecodeWriteInt(bc, n->n_of_children-1);

	/* write arguments */
	for (unsigned int i = 1; i < n->n_of_children; i++) {

		bytecodeWrite(bc, n->children[i]);

		/* error */
		if (errorIsSet())
			return;
	}
}

/* write a variable access node */
extern void bytecodeWriteVarAcc(bytecode *bc, node *n) {

	/* write signature byte */
	bytecodeAdd(bc, 0xD6);

	/* write number of tokens */
	bytecodeWriteInt(bc, n->n_of_tokens);

	/* write each name */
	for (unsigned int i = 0; i < n->n_of_tokens; i++) {

		bytecodeWriteIdt(bc, n->tokens[i]->t_value);
	}
}

/* print bytecode data hexdump style */
extern void bytecodePrintf(bytecode *bc) {

	/* format strings */
	char *fmt = "%08x  %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x  |%s|\n";
	
	/* byte buffer */
	unsigned char buf[16];
	char str[17]; /* ASCII representation */
	str[16] = '\0'; /* null term */
	unsigned int n_of_bytes = 0; /* number of bytes currently in buffer */
	unsigned int pos = 0; /* pos */
	unsigned int at_eof = 0; /* eof */

	while (!at_eof) {

		/* if we have reached end */
		at_eof = (pos == bc->len);

		/* time to print */
		if (n_of_bytes >= 16 || pos == (bc->len)) {

			/* fill in rest of buf */
			for (int i = n_of_bytes; i < 16; i++)
				buf[i] = 0x00;

			/* fill in 'str' */
			for (int i = 0; i < 16; i++)
				str[i] = (char)bytecodeGetc(buf[i]);

			/* print buffer */
			printf(fmt, pos - n_of_bytes,
						buf[0],
						buf[1],
						buf[2],
						buf[3],
						buf[4],
						buf[5],
						buf[6],
						buf[7],
						buf[8],
						buf[9],
						buf[10],
						buf[11],
						buf[12],
						buf[13],
						buf[14],
						buf[15],
						str);

			/* refill buffer with 0s */
			for (int i = 0; i < 16; i++)
				buf[i] = 0;
		
			n_of_bytes = 0;
		}

		/* add char */
		if (pos != bc->len) buf[n_of_bytes++] = bc->bytes[pos++];
	}
}

/* free a bytecode object */
extern void bytecodeFree(bytecode *bc) {

	/* free */
	free(bc->bytes);
	free(bc);
}