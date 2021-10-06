/* header */
#include "bytecode.h"
#include "error.h" /* errors */
#include "run.h" /* runlp */
#include <stdlib.h> /* malloc, realloc, free */
#include <string.h> /* strcmp */

/* variables */
char *old_fname_1 = NULL;

/* get a character */
extern unsigned int bytecodeGetc(unsigned char c) {

	if ((c >= 32 && c <= 126))
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
	bc->idata = (char **)malloc(8);
	bc->i_len = 0;
	bc->i_cap = 8;
	bc->is_idat = 0;

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

/* add idata info */
extern void bytecodeAddIdat(bytecode *bc, char *s) {

	/* resize */
	if (bc->i_len >= bc->i_cap) {

		bc->i_cap *= 2;
		bc->idata = (char **)realloc(bc->idata, bc->i_cap);
	}

	/* malloc data */
	char *s2 = (char *)malloc(strlen(s)+2);
	strcpy(s2, s);

	/* add data */
	bc->idata[bc->i_len++] = s2;
}

/* compile node */
extern void bytecodeComp(bytecode *bc) {

	/* set current and previous filenames */
	bc->curr_fname = bc->n->fname;
	bc->prev_fname = bc->n->fname;

	/* write header */
	bytecodeWriteHeader(bc);

	/* write filename */
	bytecodeWriteFileInf(bc, bc->curr_fname);

	/* compile */
	bytecodeWrite(bc, bc->n);

	/* idata byte */
	if (bc->is_idat) {

		bytecodeAdd(bc, 0xFD);

		/* idata table */
		unsigned int idat_table = bc->len; /* position of idata table */
		unsigned int idat_table_len = (bc->i_len) * 6; /* each entry takes 6 bytes */
		unsigned int idat_pos = idat_table + idat_table_len; /* position in idata table */

		for (unsigned int i = 0; i < bc->i_len; i++) {

			/* add bytes */
			bytecodeAdd(bc, (unsigned int)i);
			bytecodeWriteInt(bc, idat_pos);

			/* add to idat_pos */
			idat_pos += strlen(bc->idata[i]) + 2;
		}

		/* write idata */
		for (unsigned int i = 0; i < bc->i_len; i++) {

			bytecodeWriteStrI(bc, bc->idata[i]);
		}
	}

	/* add EOF byte */
	bytecodeAdd(bc, BYTECODE_EOF);
}

/* write node */
extern void bytecodeWrite(bytecode *bc, node *n) {

	bc->cn = n; /* current node */

	/* current filename */
	bc->curr_fname = n->fname;

	if (!(!strcmp(bc->curr_fname, bc->prev_fname))) {

		/* write file info */
		bytecodeWriteFileInf(bc, bc->curr_fname);
	}

	/* integer */
	if (n->type == NODE_INT) {

		/* write integer */
		bytecodeWriteInt(bc, atoi(n->tokens[0]->t_value));
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* string */
	else if (n->type == NODE_STRING) {

		/* write string */
		bytecodeWriteStr(bc, n->tokens[0]->t_value);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
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
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* function call */
	else if (n->type == NODE_CALL) {

		/* write node */
		bytecodeWriteCall(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* increment operator */
	else if (n->type == NODE_INC) {

		/* write node */
		bytecodeWriteInc(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* decrement operator */
	else if (n->type == NODE_DEC) {

		/* write node */
		bytecodeWriteDec(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* binary operation */
	else if (n->type == NODE_BINOP) {

		/* write node */
		bytecodeWriteBinOp(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* unary operation */
	else if (n->type == NODE_UNOP) {

		/* write node */
		bytecodeWriteUnOp(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* getitem */
	else if (n->type == NODE_GETITEM) {

		/* write node */
		bytecodeWriteGetItem(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* setitem */
	else if (n->type == NODE_SETITEM) {

		/* write node */
		bytecodeWriteSetItem(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* undefined variable */
	else if (n->type == NODE_VARUN) {

		/* write node */
		bytecodeWriteVarUndefined(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* new variable */
	else if (n->type == NODE_VARNEW) {

		/* write node */
		bytecodeWriteVarNew(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* if statement */
	else if (n->type == NODE_IFNODE) {

		/* write node */
		bytecodeWriteIfStatement(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* while loop */
	else if (n->type == NODE_WHILENODE) {

		/* write node */
		bytecodeWriteWhile(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* for loop */
	else if (n->type == NODE_FORNODE) {

		/* write node */
		bytecodeWriteFor(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* function declaration */
	else if (n->type == NODE_FUNCDEC) {

		/* write node */
		bytecodeWriteFuncDec(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* function definition */
	else if (n->type == NODE_FUNCDEF) {
		
		/* write node */
		bytecodeWriteFuncDef(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* extern */
	else if (n->type == NODE_EXTERN) {

		/* write node */
		bytecodeWriteExtern(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* typedef */
	else if (n->type == NODE_TYPEDEF) {

		/* write node */
		bytecodeWriteTypeDef(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* struct */
	else if (n->type == NODE_STRUCT) {

		/* write node */
		bytecodeWriteStruct(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* variable assignment */
	else if (n->type == NODE_VARASSIGN) {

		/* write node */
		bytecodeWriteVarAsg(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* else node */
	else if (n->type == NODE_ELSENODE) {

		/* write node */
		bytecodeWriteElse(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
	}

	/* include a file */
	else if (n->type == NODE_INCLUDE) {

		char *fn = bc->curr_fname;

		/* write file info */
		bytecodeWriteFileInf(bc, n->tokens[0]->t_value);
		bc->prev_fname = n->tokens[0]->t_value;
		bc->curr_fname = n->tokens[0]->t_value;

		/* write node */
		bytecodeWriteInclude(bc, n, n->tokens[0]->t_value);

		/* write file info */
		bytecodeWriteFileInf(bc, fn);
		bc->prev_fname = fn;
		bc->curr_fname = fn;
	}

	/* const, unsigned, return */
	else if (n->type == NODE_CONST || n->type == NODE_UNSIGNED || n->type == NODE_RETURN) {

		/* write node */
		bytecodeWriteCUR(bc, n);
		bytecodeWriteErrInf(bc, n->lineno, n->colno);
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

	/* previous filename */
	bc->prev_fname = bc->curr_fname;

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

	/* idata inf */
	bytecodeAdd(bc, 0x00);
	bytecodeAdd(bc, 0x00);
	bytecodeAdd(bc, 0x00);
	bytecodeAdd(bc, (unsigned char)bc->is_idat);
}

/* write error info */
extern void bytecodeWriteErrInf(bytecode *bc, unsigned int lineno, unsigned int colno) {

	/* write signature byte and line, column */
	bytecodeAdd(bc, 0xFE);

	bytecodeWriteIntNS(bc, lineno);
	bytecodeWriteIntNS(bc, colno);
}

/* write filename info */
extern void bytecodeWriteFileInf(bytecode *bc, char *fname) {

	/* write byte and string
	 * FILE "filename"
	 */

	bytecodeAdd(bc, 0xFF);
	bytecodeWriteIdt(bc, fname);
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

/* write an integer without signature byte */
extern void bytecodeWriteIntNS(bytecode *bc, int i) {

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

	/* idata */
	if (bc->is_idat == 1) {

		/* get idx */
		int idx;
		for (idx = 0; idx < bc->i_len; idx++) {

			/* break */
			if (!strcmp(bc->idata[idx], s))
				break;
		}

		/* need to add */
		if (idx == bc->i_len)
			bytecodeAddIdat(bc, s);

		/* add byte */
		bytecodeAdd(bc, (unsigned char)idx);

		return;
	}

	/* loop through string */
	char c;
	while (c = *s++) {

		bytecodeAdd(bc, (unsigned char)c); /* add char */
	}

	/* add null term char */
	bytecodeAdd(bc, 0x00);
}

/* write a string and ignore idata mode */
extern void bytecodeWriteStrI(bytecode *bc, char *s) {

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

	/* write sig byte */
	bytecodeAdd(bc, 0xD5);

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

/* increment */
extern void bytecodeWriteInc(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xD3);

	/* write number of tokens */
	bytecodeWriteInt(bc, n->n_of_tokens);

	/* write each name */
	for (unsigned int i = 0; i < n->n_of_tokens; i++) {

		bytecodeWriteIdt(bc, n->tokens[i]->t_value);
	}
}

/* decrement */
extern void bytecodeWriteDec(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xD4);

	/* write number of tokens */
	bytecodeWriteInt(bc, n->n_of_tokens);

	/* write each name */
	for (unsigned int i = 0; i < n->n_of_tokens; i++) {

		bytecodeWriteIdt(bc, n->tokens[i]->t_value);
	}
}

/* write binary operation */
extern void bytecodeWriteBinOp(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0x9D);

	/* write operation type */
	bytecodeAdd(bc, n->tokens[0]->t_type);

	/* write left and right nodes */
	bytecodeWrite(bc, n->children[0]);
	bytecodeWrite(bc, n->children[1]);
}

/* write unary operation */
extern void bytecodeWriteUnOp(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0x9C);

	/* write operation type */
	bytecodeAdd(bc, n->tokens[0]->t_type);

	/* write node */
	bytecodeWrite(bc, n->children[0]);
}

/* getitem ([]) */
extern void bytecodeWriteGetItem(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0x9A);

	/* write number of tokens */
	bytecodeWriteInt(bc, n->n_of_tokens);

	/* write each name */
	for (unsigned int i = 0; i < n->n_of_tokens; i++) {

		bytecodeWriteIdt(bc, n->tokens[i]->t_value);
	}

	/* write index node */
	bytecodeWrite(bc, n->children[0]);
}

/* setitem ([] = ?) */
extern void bytecodeWriteSetItem(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xDD);

	/* write number of tokens */
	bytecodeWriteInt(bc, n->n_of_tokens);

	/* write each name */
	for (unsigned int i = 0; i < n->n_of_tokens; i++) {

		bytecodeWriteIdt(bc, n->tokens[i]->t_value);
	}

	/* write index node */
	bytecodeWrite(bc, n->children[0]);

	/* write value node */
	bytecodeWrite(bc, n->children[1]);
}

/* write a new variable */
extern void bytecodeWriteVarUndefined(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xD7);

	/* has no value */
	bytecodeAdd(bc, 0x00);

	/* write type and name */
	bytecodeWriteIdt(bc, n->tokens[0]->t_value);
	bytecodeWriteIdt(bc, n->tokens[1]->t_value);
	
	/* values */
	bytecodeAdd(bc, (unsigned char)n->values[0]);
	bytecodeAdd(bc, (unsigned char)n->values[1]);

	/* array */
	if (n->values[0])
		bytecodeWrite(bc, n->children[0]);
}

/* write an undefined variable */
extern void bytecodeWriteVarNew(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xD7);

	/* has value */
	bytecodeAdd(bc, 0x01);

	/* write type and name */
	bytecodeWriteIdt(bc, n->tokens[0]->t_value);
	bytecodeWriteIdt(bc, n->tokens[1]->t_value);
	
	/* values */
	bytecodeAdd(bc, (unsigned char)n->values[0]);
	bytecodeAdd(bc, (unsigned char)n->values[1]);

	/* array */
	if (n->values[0])
		bytecodeWrite(bc, n->children[0]);

	/* value */
	bytecodeWrite(bc, n->children[n->values[0]]);
}

/* write an if statement */
extern void bytecodeWriteIfStatement(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xD8);

	/* write condition node */
	bytecodeWrite(bc, n->children[0]);

	/* write number of body nodes */
	bytecodeWriteInt(bc, n->n_of_children-1);

	/* write body nodes */
	for (unsigned int i = 1; i < n->n_of_children; i++)
		bytecodeWrite(bc, n->children[i]);
}

/* write a while loop */
extern void bytecodeWriteWhile(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xD9);

	/* write condition node */
	bytecodeWrite(bc, n->children[0]);

	/* write number of body nodes */
	bytecodeWriteInt(bc, n->n_of_children-1);

	/* write body nodes */
	for (unsigned int i = 1; i < n->n_of_children; i++)
		bytecodeWrite(bc, n->children[i]);
}

/* write a for loop */
extern void bytecodeWriteFor(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xDA);

	/* write condition nodes */
	for (int i = 0; i < 3; i++)
		bytecodeWrite(bc, n->children[i]);

	/* write number of body nodes */
	bytecodeWriteInt(bc, n->n_of_children-3);

	/* write body nodes */
	for (int j = 3; j < n->n_of_children; j++)
		bytecodeWrite(bc, n->children[j]);
}

/* write function declaration */
extern void bytecodeWriteFuncDec(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xDB);

	/* write function return type and function name */
	bytecodeWriteIdt(bc, n->tokens[0]->t_value);
	bytecodeWriteIdt(bc, n->tokens[1]->t_value);

	/* return type is pointer */
	bytecodeAdd(bc, (unsigned char)n->values[0]);

	/* write number of arguments */
	bytecodeWriteInt(bc, (n->n_of_tokens - 2) / 2);

	/* write arguments */
	for (unsigned int i = 2; i < n->n_of_tokens; i += 2) {

		/* write argument type and name */
		bytecodeWriteIdt(bc, n->tokens[i]->t_value);
		bytecodeWriteIdt(bc, n->tokens[i+1]->t_value);

		/* is it a pointer */
		bytecodeAdd(bc, (unsigned char)n->values[((i - 2) / 2) + 1]);
	}
}

/* write a function definition */
extern void bytecodeWriteFuncDef(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xDC);

	/* write child */
	bytecodeWrite(bc, n->children[0]);

	/* write number of body nodes */
	bytecodeWriteInt(bc, n->n_of_children-1);

	/* write body nodes */
	for (unsigned int i = 1; i < n->n_of_children; i++)
		bytecodeWrite(bc, n->children[i]);
}

/* write an external reference */
extern void bytecodeWriteExtern(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xC1);

	/* write child */
	bytecodeWrite(bc, n->children[0]);
}

/* write const, unsigned, or return */
extern void bytecodeWriteCUR(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, (n->type == NODE_CONST? 0xDE: (n->type == NODE_UNSIGNED? 0xDF: 0xC0)));

	/* write child node */
	bytecodeWrite(bc, n->children[0]);
}

/* write struct info */
extern void bytecodeWriteStruct(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xC2);

	/* write name of struct */
	bytecodeWriteIdt(bc, n->tokens[0]->t_value);

	/* write number of nodes */
	bytecodeWriteInt(bc, n->n_of_children);

	/* struct value nodes */
	for (unsigned int i = 0; i < n->n_of_children; i++) {

		bytecodeWrite(bc, n->children[i]);
	}
}

/* write typedef */
extern void bytecodeWriteTypeDef(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xC3);

	/* write values and tokens */
	bytecodeAdd(bc, (unsigned char)n->values[0]);
	bytecodeWriteIdt(bc, n->tokens[0]->t_value);
	bytecodeWriteIdt(bc, n->tokens[1]->t_value);
}

/* write else */
extern void bytecodeWriteElse(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xC4);

	/* write child node */
	bytecodeWrite(bc, n->children[0]);
}

/* write variable assignment */
extern void bytecodeWriteVarAsg(bytecode *bc, node *n) {

	/* write sigbyte */
	bytecodeAdd(bc, 0xD2);

	/* number of names */
	bytecodeWriteInt(bc, n->n_of_tokens);

	/* write variable name tokens */
	for (unsigned int i = 0; i < n->n_of_tokens; i++) {

		bytecodeWriteIdt(bc, n->tokens[i]->t_value);
	}

	/* write value */
	bytecodeWrite(bc, n->children[0]);
}

/* write include file */
extern void bytecodeWriteInclude(bytecode *bc, node *n, char *fname) {

	/* run code */
	mango_ctx ctx = runlp(fname, n->fname, n->lineno, n->colno);

	/* error */
	if (ctx.e) {

		/* raise error to stop system */
		errorSet(ERROR_TYPE_BREAK, 0, NULL);
		return;
	}

	/* write node */
	bytecodeWrite(bc, ctx.parse->pn);

	/* free l and p */
	node *pn = ctx.parse->pn;
	parserFree(ctx.parse);
	lexerFree(ctx.lex);
	nodeFree(pn);
}

/* finish with bytecode */
extern void bytecodeFinish(bytecode *bc) {

	/* all we really have to do in this situation is check if we are in a mode which requires us to write to a file, which in this case, we will */
	if (bc->mode == BYTECODE_CLIB || bc->mode == BYTECODE_CMP) {

		/* get final file extension */
		char *ext = (bc->mode == BYTECODE_CLIB)? "ml": "mc";

		/* get position of last dot */
		int dotcnt = 0; /* number of dots counted */
		int curdotpos = 0; /* current dot position */

		char c; /* current character */

		for (int i = 0; i < strlen(bc->curr_fname); i++) {

			/* get dot */
			if ((c = bc->curr_fname[i]) == '.') {

				dotcnt++;
				curdotpos = i;
			}
		}

		/* no dots found */
		if (!dotcnt) curdotpos = strlen(bc->curr_fname);
		else curdotpos++; /* this will need to be the position AFTER the dot */

		/* create filename buffer */
		char *mb = (char *)malloc(strlen(bc->curr_fname) + 4);
		strcpy(mb, bc->curr_fname);
		strcpy(mb + curdotpos, ext);

		printf("compiling to '%s'...\n", mb);

		/* open file */
		FILE *fp = fopen(mb, "wb");

		/* write to file */
		fwrite(bc->bytes, 1, bc->len, fp);

		/* close file */
		fclose(fp);

		/* free buffer and print highly important message */
		free(mb);

		printf("done.\n");
	}
}

/* print bytecode data hexdump style */
extern void bytecodePrintf(bytecode *bc) {

	/* format strings */
	char *fmt = "%08x  %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x  |%s|\n";
	
	/* byte buffer */
	unsigned char buf[16];
	char str[17]; /* ASCII representation */
	str[16] = 0; /* null term char */
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

			/* end */
			if (pos == (bc->len)) {

				printf("         ");

				for (int i = 0; i < n_of_bytes; i++)
					printf(" ^^");

				printf("\n");
			}

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

	/* free idata */
	for (unsigned int i = 0; i < bc->i_len; i++)
		free(bc->idata[i]);

	free(bc->idata);
	free(bc);
}