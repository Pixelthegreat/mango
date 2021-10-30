/* virtual machine bytecode interpreter */
#include "vm.h"
#include "object.h"
#include "arrayobject.h"
#include "intobject.h"
#include "pointerobject.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef unsigned char u8;

/* for debugging */
#if defined DEBUG && DEBUG == 1
#define VM_DEBUG
#endif

/* create a new vm struct from existing bytecode structure */
extern vm *vmNew(bytecode *bc) {

	/* malloc new vm struct */
	vm *v = (vm *)malloc(sizeof(vm));

	/* failed to allocate */
	if (v == NULL)
		return NULL;

	/* create a context */
	v->ctx = contextNew("fname", "main");
	
	if (bc != NULL) {

		/* create a buffer */
		v->bc = (void *)malloc(bc->len);
		v->bc_len = bc->len;
		v->fromf = 1;

		/* copy data from memory */
		memcpy(v->bc, bc->bytes, bc->len);
	}
	else v->fromf = 0;

	/* return vm */
	return v;
}

/* create a vm from a file */
extern vm *vmNewFromFile(char *f) {

	/* check if file exists */
	struct stat st;
	if (stat(f, &st) < 0) {

		/* print error */
		fprintf(stderr, "Could not open file '%s'!\n", f);
		return NULL;
	}

	/* create vm */
	vm *v = vmNew(NULL);

	/* create buffer */
	v->bc = (void *)malloc(st.st_size);
	v->bc_len = st.st_size;
	v->ctx->fn = f;

	/* open file for binary reading */
	FILE *fp = fopen(f, "rb");

	fread(v->bc, 1, st.st_size, fp);

	fclose(fp);

	/* check the file header and make sure it is a mangobc file */
	if (!(!strncmp(v->bc, "\x0bmc\x0e", 4) || !strncmp(v->bc, "\x0bml\x0f", 4)) || st.st_size < 8) {

		/* print error */
		fprintf(stderr, "File '%s' is not a bytecode file!\n", f);

		/* free vm and exit */
		free(v->ctx);
		free(v->bc);
		free(v);
		return NULL;
	}

	/* return vm pointer */
	return v;
}

/* return an object from handler */
extern object *vmHandle(vm *v, unsigned int i) {

	object *o = NULL;

	/* check options */

	/* create a string object */
	if (((u8 *)v->bc)[i] == 0x9E) {

		if (!IS_IDAT(v->bcflags)) {
		
			/* get length of string */
			int len = strlen(&((char *)v->bc)[i + 1]) + 1;
		
			/* create an array object */
			o = arrayobjectNew(len, OBJECT_CHR);
		
			/* copy string value */
			strcpy(O_ARRAY(o)->n_start, &((char *)v->bc)[i + 1]);
		
			/* advance number of bytes */
			v->nofbytes += (len + 1);
		} else {

			/* get length of string */
			char *stocp = v->idata[((u8 *)v->bc)[i+1]] + 1;
			int len = strlen(stocp);

			/* create an array object */
			object *a = arrayobjectNew(len, OBJECT_CHR);

			/* copy string value */
			strcpy(O_ARRAY(a)->n_start, stocp);

			/* advance number of bytes */
			v->nofbytes += 2;

			/* create pointer */
			o = pointerobjectNew(OBJECT_CHR, (void *)a);
		}

		/* debug info */
		#ifdef VM_DEBUG
		printf("[vm] created string with value '%s'\n", O_ARRAY(o)->n_start);
		#endif
	}

	/* create an int object */
	if (((u8 *)v->bc)[i] == 0x9B) {

		/* get int value */
		int val = (((u8*)v->bc)[i+1]<<24) | (((u8*)v->bc)[i+2]<<16) | (((u8*)v->bc)[i+3]<<8) | (((u8*)v->bc)[i+4]);

		/* create object */
		o = intobjectNew(val);

		/* debug info */
		#ifdef VM_DEBUG
		printf("[vm] created integer object with value %d\n", O_INT(o)->val);
		#endif

		/* advance number of bytes */
		v->nofbytes += 5;
	}

	/* binary operation */
	if (((u8 *)v->bc)[i] == 0x9D) {

		/* get operation */
		u8 op = ((u8 *)v->bc)[i + 1];
		v->nofbytes += 2;

		/* get first object */
		object *a = vmHandle(v, v->lowbi + v->nofbytes);

		/* error */
		if (a == NULL || errorIsSet())
			return NULL;

		/* get second object */
		object *b = vmHandle(v, v->lowbi + v->nofbytes);

		/* error */
		if (b == NULL || errorIsSet())
			return NULL;

		/* operate on object */
		object *c = objectOperation(a, b, op);

		/* error */
		if (c == NULL || errorIsSet())
			return NULL;

		/* set object */
		o = c;

		/* debug */
		#ifdef VM_DEBUG
		printf("[vm] interpreted binary operation with operation %d\n", op);
		if (c->type == OBJECT_INT) printf("[vm] binary operation value is %d\n", O_INT(c)->val);
		#endif
	}

	/* create a variable */
	if (((u8 *)v->bc)[i] == 0xD1) {

		/* increase byte number */
		v->nofbytes += 3;

		/* get variable type name */
		char *tp_name = &(((char *)v->bc)[i + 3]);
		v->nofbytes += strlen(tp_name) + 2;
		u8 ob_type = 0; /* object type value */

		/* get object name */
		char *ob_name = &(((char *)v->bc)[i + strlen(tp_name) + 5]);
		v->nofbytes += strlen(ob_name) + 1;

		/* array and pointer */
		ob_type |= (((u8 *)v->bc)[v->lowbi + v->nofbytes]? OBJECT_ARRAY: 0);
		ob_type |= (((u8 *)v->bc)[v->lowbi + v->nofbytes + 1]? OBJECT_POINTER: 0);
		v->nofbytes += 2;

		/* get array object */
		if (ob_type & OBJECT_ARRAY) {

			object *asz = vmHandle(v, v->lowbi + v->nofbytes);

			/* error */
			if (asz == NULL || errorIsSet())
				return NULL;
		}

		/* get value object */
		object *val = vmHandle(v, v->lowbi + v->nofbytes);

		/* error */
		if (val == NULL || errorIsSet())
			return NULL;

		/* TODO: add check to typedef'd names; get object type from name */
		if (!strcmp(tp_name, "int")) ob_type = OBJECT_INT | ob_type;
		else if (!strcmp(tp_name, "chr")) ob_type = OBJECT_CHR | ob_type;
		else {

			/* create error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Illegal operation");
			errorSetPos(val->lineno, val->colno, v->ctx->fn);
			return NULL;
		}

		/* mismatched types */
		if ((ob_type & 0x3) != (val->type & 0x3)) {

			/* create error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Mismatched types");
			errorSetPos(val->lineno, val->colno, v->ctx->fn);
			return NULL;
		}

		/* set the name value */
		namesSet(v->ctx->nt, ob_name, val);

		/* set object */
		o = val;

		/* debug info */
		#ifdef VM_DEBUG
		printf("[vm] set object '%s' with type %02x", ob_name, o->type);
		if ((ob_type & 0x3) == OBJECT_INT) printf(" and with value %d", O_INT(o)->val);
		else if ((o->type == (OBJECT_CHR | OBJECT_ARRAY)) || (o->type == (OBJECT_CHR | OBJECT_POINTER))) printf(" and with value '%s'", O_ARRAY(o)->n_start);
		printf(".\n");
		#endif
	}

	if (o == NULL) {
	
		/* vm debug */
		#ifdef VM_DEBUG
		printf("[vm] not built to handle '%02x'\n", ((u8 *)v->bc)[i]);
		#endif
	
		/* no other option */
		return NULL;
	}

	/* get error info */
	if (((u8 *)v->bc)[v->lowbi + v->nofbytes] == 0xFE) {

		/* get line and column numbers */
		int pos = v->lowbi + v->nofbytes + 1;
		unsigned int lineno = (((u8 *)v->bc)[pos] << 24) | (((u8 *)v->bc)[pos + 1] << 16) | (((u8 *)v->bc)[pos + 2] << 8) | (((u8 *)v->bc)[pos + 3]);
		unsigned int colno = (((u8 *)v->bc)[pos + 4] << 24) | (((u8 *)v->bc)[pos + 5] << 16) | (((u8 *)v->bc)[pos + 6] << 8) | (((u8 *)v->bc)[pos + 7]);

		/* set object values */
		o->lineno = lineno;
		o->colno = colno;

		/* debug info */
		#ifdef VM_DEBUG
		printf("[vm] line and column numbers: %d, %d\n", o->lineno, o->colno);
		#endif

		/* advance v->nofbytes */
		v->nofbytes += 9;
	}

	return o; /* object */
}

/* load idata table */
extern void vmLoadIdataTable(vm *v) {

	v->nidat = 0;

	/* search for idata table */
	unsigned int i;
	for (i = 8; i < v->bc_len; i++) {

		/* search for 0xfd */
		if (((u8 *)v->bc)[i] == 0xFD) {

			i++; /* advance i to beginning of table */

			/* loop until we find '0x9b' is not a second character in a 6 byte sequence */
			while ((i < (v->bc_len - 1)) && (((u8 *)v->bc)[i + 1] == 0x9B)) {

				/* get position in idata */
				unsigned int loc = (((u8*)v->bc)[i+2]<<24)|(((u8*)v->bc)[i+3]<<16)|(((u8*)v->bc)[i+4]<<8)|(((u8*)v->bc)[i+5]);
			
				/* set entry */
				v->idata[v->nidat++] = &(((char *)v->bc)[loc]);

				/* advance i */
				i += 6;
			}

			break;
		}
	}

	/* debug */
	#ifdef VM_DEBUG
	printf("[vm] loaded idata table successfully.\n");
	#endif
}

/* execute bytecode */
extern void vmExec(vm *v) {

	object *co = NULL; /* current object */

	/* get flags */
	v->bcflags = ((u8 *)v->bc)[7];

	/* idata mode */
	if (IS_IDAT(v->bcflags)) {

		/* idata */
		vmLoadIdataTable(v);
	}

	/* go through bytecode */
	unsigned int i = 8;
	while (i < v->bc_len && ((u8 *)v->bc)[i] != 0x00) {

		/* set lowbi */
		v->lowbi = i;

		/* change file name */
		if (((u8 *)v->bc)[i] == 0xFF) {

			/* set filename */
			v->ctx->fn = &((char *)v->bc)[i + 2];

			/* debug info */
			#ifdef VM_DEBUG
			printf("[vm] changed filename of scope '%s' to '%s'\n", v->ctx->sn, v->ctx->fn);
			#endif

			i += strlen(v->ctx->fn) + 3;
		}

		/* exit because we've reached the end with the idata table */
		else if (((u8 *)v->bc)[i] == 0xFD) {

			break;
		}

		/* otherwise */
		else {

			v->nofbytes = 0; /* reset the number of bytes recorded */
			co = vmHandle(v, i); /* handle object */
			i += v->nofbytes; /* advance number of bytes */

			/* error */
			if (co == NULL || errorIsSet())
				return;
		}
	}
}

/* free a vm */
extern void vmFree(vm *v) {

	free(v->bc);
	free(v->ctx);
	free(v);
}