/*
 *
 * Copyright 2021, Elliot Kohlmyer
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

/* virtual machine bytecode interpreter */
#include "object.h"
#include "vm.h"
#include "arrayobject.h"
#include "intobject.h"
#include "pointerobject.h"
#include "functionobject.h"
#include "structobject.h"
#include "typedef.h"
#include "error.h"


#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef unsigned char u8;
context *vmdctx = NULL; /* default context for vms to use */

/* for debugging */
int VM_DEBUG = -1;

/* vm list */
static vm **vm_list = NULL;
static int vm_list_len = 0;
static int vm_list_cap = 0;
static int gbc_iter = 0; /* garbage collection iteration */
static int gbc_len = 100; /* number of iterations of vmHandle before objectCollect is called */

/* debug trace list for determining where a problem is coming from */
object *debug_trace[8];
int dbt_n = 0;

/* create a new vm struct from existing bytecode structure */
extern vm *vmNew(bytecode *bc) {

	/* to debug */
	if (VM_DEBUG < 0)
		VM_DEBUG = DEBUG;

	/* malloc new vm struct */
	vm *v = (vm *)malloc(sizeof(vm));

	/* failed to allocate */
	if (v == NULL)
		return NULL;

	/* empty list */
	if (vm_list == NULL) {

		vm_list = (vm **)malloc(sizeof(vm *) * 8);
		vm_list_cap = 8;
		vm_list_len = 0;
	}

	/* reallocate list */
	else if (vm_list_len >= vm_list_cap) {

		vm_list_cap *= 2;
		vm_list = (vm **)realloc(vm_list, sizeof(vm *) * vm_list_cap);
	}

	/* add item to list */
	vm_list[vm_list_len++] = v;

	/* create a context */
	if (vmdctx == NULL)
		vmdctx = contextNew("fname", "main");

	/* set context */
	v->ctx = vmdctx;
	
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
	v->bcflags = 0;

	/* open file for binary reading */
	FILE *fp = fopen(f, "rb");

	fread(v->bc, 1, st.st_size, fp);

	fclose(fp);

	/* check the file header and make sure it is a mangobc file */
	if (!(!memcmp(v->bc, "\x0bmc\x0e", 4) || !memcmp(v->bc, "\x0bml\x0f", 4)) || st.st_size < 8) {

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

	gbc_iter++; /* advance garbage collection iterator */

	object *o = NULL;

	if (VM_DEBUG) printf("[vm] type %d\n", (((u8 *)v->bc)[i]));

	/* check options */

	/* create a string object */
	if (((u8 *)v->bc)[i] == 0x9E) {

		if (!IS_IDAT(v->bcflags)) {
		
			/* get length of string */
			char *stocp = &((char *)v->bc)[i + 1];
			int len = strlen(stocp) + 1;
		
			/* create an array object */
			o = arrayobjectNew(len, OBJECT_CHR);
		
			/* copy string value */
			strcpy(O_ARRAY(o)->n_start, stocp);
		
			/* advance number of bytes */
			v->nofbytes += (len + 1);

			/* create pointer */
			o = pointerobjectNew(OBJECT_CHR, (void *)o);
		
		} else {

			/* get length of string */
			char *stocp = v->idata[((u8 *)v->bc)[i+1]] + 1;
			int len = strlen(stocp) + 1;

			/* create an array object */
			object *a = arrayobjectNew(len, OBJECT_CHR);

			/* copy string value */
			strcpy(O_ARRAY(a)->n_start, stocp);

			/* advance number of bytes */
			v->nofbytes += 2;

			/* create pointer */
			o = pointerobjectNew(OBJECT_CHR, (void *)a);
		}

		o->fname = v->ctx->fn;

		/* debug info */
		if (VM_DEBUG) printf("[vm] created string with value '%s'\n", O_ARRAY(O_PTR(o)->val)->n_start);
	}

	/* create an int object */
	if (((u8 *)v->bc)[i] == 0x9B) {

		/* get int value */
		int val = (((u8*)v->bc)[i+1]<<24) | (((u8*)v->bc)[i+2]<<16) | (((u8*)v->bc)[i+3]<<8) | (((u8*)v->bc)[i+4]);

		/* create object */
		o = intobjectNew(val);
		o->fname = v->ctx->fn;

		/* debug info */
		if (VM_DEBUG) printf("[vm] created integer object with value %d\n", O_INT(o)->val);

		/* advance number of bytes */
		v->nofbytes += 5;
	}

	/* unary operation */
	if (((u8 *)v->bc)[i] == 0x9C) {

		/* get operation */
		u8 op = ((u8 *)v->bc)[i + 1];
		v->nofbytes += 2;

		/* get object */
		object *a = vmHandle(v, v->lowbi + v->nofbytes);

		/* get error info */
		int lineno, colno;
		vmGetErrorInfo(v, &lineno, &colno);

		/* error */
		if (a == NULL || errorIsSet())
			return NULL;

		/* negate */
		if (op == TOKEN_MINUS) {

			/* multiply */
			o = objectOperation(a, intobjectNew(-1), TOKEN_MUL);

			/* error */
			if (o == NULL || errorIsSet())
				return NULL;
		}

		/* increment */
		else if (op == TOKEN_INC) {

			/* add one */
			O_INT(a)->val++;
			o = a;
		}

		/* decrement */
		else if (op == TOKEN_DEC) {

			/* sub one */
			O_INT(a)->val--;
			o = a;
		}

		/* '&' */
		else if (op == TOKEN_AMP) {

			/* get address */
			o = pointerobjectNew(a->type, (void *)a);
		}

		/* '*' */
		else if (op == TOKEN_MUL) {

			/* not a pointer */
			if (!(a->type & OBJECT_POINTER) || (O_PTR(a)->val == NULL)) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Attempt to dereference a non-pointer");
				errorSetPos(lineno, colno, v->ctx->fn);
				return NULL;
			}

			/* get dereffed value */
			o = O_OBJ(O_PTR(a)->val);
		}

		else o = a;

		/* debug info */
		if (VM_DEBUG) {
			
			printf("[vm] interpreted unary operation with op code %d\n", op);
			if (o->type == OBJECT_INT) printf("[vm] unary operation value is %d\n", O_INT(o)->val);
		}
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
		if (VM_DEBUG) {
			
			printf("[vm] interpreted binary operation with operation %d\n", op);
			if (c->type == OBJECT_INT) printf("[vm] binary operation value is %d\n", O_INT(c)->val);
		}
	}

	/* assign to an existing variable */
	if ((((u8 *)v->bc)[i] == 0xD2) || (((u8 *)v->bc)[i] == 0xDD)) {

		v->nofbytes += 2;

		/* get number of items */
		unsigned int n = vmGetInt(v, v->lowbi + v->nofbytes);
		v->nofbytes += 4;

		nameTable *ntc = v->ctx->nt; /* current table of names */
		char *cn; /* current name we are looking at */
		int exists = 1; /* to determine an undefined name */

		/* loop through names */
		for (int j = 0; j < (n-1); j++) {

			/* get name and advance forward */
			cn = &(((char *)v->bc)[v->lowbi + (++v->nofbytes)]);
			v->nofbytes += strlen(cn) + 1;

			/* get object associated with name */
			if (ntc != NULL) {

				/* name */
				object *st = namesGet(ntc, cn);

				/* non-existant */
				if (st == NULL) {

					ntc = NULL;
					exists = 0;
					continue;
				}

				/* value is not a struct */
				if ((st->type & 0x3) != OBJECT_STRUCT) {

					ntc = NULL;
					continue;
				}

				/* struct pointer */
				else if (st->type & OBJECT_POINTER)
					st = O_OBJ(O_PTR(st)->val);

				ntc = O_STRUCT(st)->nt;
			}
		}

		/* get last name */
		cn = &(((char *)v->bc)[v->lowbi + (++v->nofbytes)]);
		v->nofbytes += strlen(cn) + 1;

		/* array index */
		object *arr_idx;
		if ((((u8 *)v->bc)[i]) == 0xDD) {

			arr_idx = vmHandle(v, v->lowbi + v->nofbytes);

			/* error */
			if (arr_idx == NULL || errorIsSet())
				return NULL;
		}

		/* get value object */
		object *val = vmHandle(v, v->lowbi + v->nofbytes);

		/* error */
		if (val == NULL || errorIsSet())
			return NULL;

		/* get error information */
		int lineno, colno;
		vmGetErrorInfo(v, &lineno, &colno);

		//printf("%08x\n", v->lowbi + v->nofbytes);
		//return NULL;

		/* undefined name */
		if (!exists) {

			/* set error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_UNDEFINEDNAME,
					 "Undefined name");
			errorSetPos(lineno, colno, v->ctx->fn);
			return NULL;
		}

		/* not a struct */
		if (ntc == NULL) {

			/* set error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Illegal operation");
			errorSetPos(lineno, colno, v->ctx->fn);
			return NULL;
		}

		object *curobj;

		/* not a value */
		if ((curobj = namesGet(ntc, cn)) == NULL) {

			/* set error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_UNDEFINEDNAME,
					 "Undefined name");
			errorSetPos(lineno, colno, v->ctx->fn);
			return NULL;
		}

		/* check types */
		if ((curobj->type != val->type) && ((((u8 *)v->bc)[i]) != 0xDD)) {

			/* set error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Mismatched types");
			errorSetPos(lineno, colno, v->ctx->fn);
			return NULL;
		}

		/* if array */
		if ((((u8 *)v->bc)[i]) == 0xDD) {

			/* dereference if it is a pointer */
			object *a = curobj;
			if (a->type & OBJECT_POINTER) a = O_OBJ(O_PTR(a)->val);

			/* check if it is an array */
			if (!(a->type & OBJECT_ARRAY)) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Illegal operation");
				errorSetPos(lineno, colno, v->ctx->fn);
				return NULL;
			}

			/* check type of array index */
			if (arr_idx->type != OBJECT_INT) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Illegal operation");
				errorSetPos(lineno, colno, v->ctx->fn);
				return NULL;
			}

			/* check length of array */
			if (O_INT(arr_idx)->val >= O_ARRAY(a)->n_len) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Illegal operation");
				errorSetPos(lineno, colno, v->ctx->fn);
				return NULL;
			}

			/* check types */
			if (((a->type & 0x3) != val->type) || ((a->type & OBJECT_POINTER) != (val->type & OBJECT_POINTER))) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Illegal operation");
				errorSetPos(lineno, colno, v->ctx->fn);
				return NULL;
			}

			/* set value */
			if (a->type & OBJECT_POINTER) ((void **)O_ARRAY(a)->n_start)[O_INT(arr_idx)->val] = O_PTR(val)->val;
			else if ((a->type & 0x3) == OBJECT_INT) ((int *)O_ARRAY(a)->n_start)[O_INT(arr_idx)->val] = O_INT(val)->val;
			else if ((a->type & 0x3) == OBJECT_CHR) ((char *)O_ARRAY(a)->n_start)[O_INT(arr_idx)->val] = O_CHR(val)->val;
		}
		else {

			/* set value */
			namesSet(ntc, cn, val);
		}

		o = val;

		/* debug info */
		if (VM_DEBUG) {

			printf("[vm] set object '%s' with type %d in nameTable %p.\n", cn, val->type, ntc);
		}
	}

	/* get a variable */
	if (((((u8 *)v->bc)[i]) == 0xD6) || ((((u8 *)v->bc)[i]) == 0xD3) || ((((u8 *)v->bc)[i]) == 0xD4) || ((((u8 *)v->bc)[i]) == 0x9A)) {

		u8 _v = (((u8 *)v->bc)[i]);

		/* increase byte number */
		v->nofbytes += 2;

		/* get number of items */
		unsigned int n = vmGetInt(v, v->lowbi + v->nofbytes);
		v->nofbytes += 5;

		/* first name */
		char *first = &(((char *)v->bc)[v->lowbi + v->nofbytes]);
		v->nofbytes += strlen(first) + 1;

		/* get object */
		object *f = namesGet(v->ctx->nt, first);

		/* if it a struct pointer */
		if ((n > 1) && (f != NULL) && (f->type == OBJECT_STRUCT | OBJECT_POINTER))
			f = O_OBJ(O_PTR(f)->val);

		int j = 1;

		nameTable *ntc = v->ctx->nt;
		context *nctx = v->ctx;

		/* more names */
		if (n > 1) {

			/* loop through names */
			for (int i = 1; i < n; i++) {

				first = &(((char *)v->bc)[v->lowbi + (++v->nofbytes)]);
				v->nofbytes += strlen(first) + 1;

				/* non-existant */
				if (f == NULL)
					continue;

				/* not a struct */
				if ((f == NULL) || ((f->type & 0x3) != OBJECT_STRUCT)) {

					/* do nothing */
				} else {

					j++; /* advance */

					/* struct pointer */
					if (f->type & OBJECT_POINTER)
						f = O_OBJ(O_PTR(f)->val);
					
					/* get next object */
					ntc = O_STRUCT(f)->nt;
					nctx = O_STRUCT(f)->ctx;
					f = namesGet(ntc, first);

					/* skip if NULL */
					if (f == NULL)
						continue;

					/* if f is a pointer to a struct */
					if (f->type == (OBJECT_STRUCT | OBJECT_POINTER))
						f = O_OBJ(O_PTR(f)->val);
				}
			}
		}

		/* get getitem values */
		object *arr_idx;

		if (_v == 0x9A) {

			arr_idx = vmHandle(v, v->lowbi + v->nofbytes);

			/* error */
			if (arr_idx == NULL || errorIsSet())
				return NULL;
		}

		/* error info */
		int lineno, colno;
		vmGetErrorInfo(v, &lineno, &colno);

		/* the object was not found */
		if (f == NULL) {

			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_UNDEFINEDNAME,
					 "Undefined name");
			errorSetPos(lineno, colno, v->ctx->fn);
			return NULL;
		}

		/* attempt to get something from a non-struct (illegal operation) */
		if (j < n) {

			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Illegal operation");
			errorSetPos(lineno, colno, v->ctx->fn);
			return NULL;
		}

		/* increment */
		if (_v == 0xD3)
			o = intobjectNew(O_INT(f)->val++);

		/* decrement */
		else if (_v == 0xD4)
			o = intobjectNew(O_INT(f)->val--);

		/* getitem */
		else if (_v == 0x9A) {

			object *a = f;

			/* dereference object if it is a pointer */
			if (a->type & OBJECT_POINTER) {

				a = O_OBJ(O_PTR(a)->val);
			}

			/* check if it is an array */
			if (!(a->type & OBJECT_ARRAY)) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Illegal operation");
				errorSetPos(lineno, colno, v->ctx->fn);
				return NULL;
			}

			/* check the array index */
			if (arr_idx->type != OBJECT_INT) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Illegal operation");
				errorSetPos(lineno, colno, v->ctx->fn);
				return NULL;
			}

			/* "get" the item */
			if ((a->type & 0x3) == OBJECT_INT) o = intobjectNew(((int *)O_ARRAY(a)->n_start)[O_INT(arr_idx)->val]);
			else if ((a->type & 0x3) == OBJECT_CHR) o = charobjectNew(((char *)O_ARRAY(a)->n_start)[O_INT(arr_idx)->val]);
		}

		/* otherwise, we have gotten the object */
		else o = f;

		/* debug info */
		if (VM_DEBUG) {

			if (_v == 0x9A) printf("[vm] retreived object '%s[%d]' with type %d.\n", first, O_INT(arr_idx)->val, o->type);
			else printf("[vm] retreived object '%s' with type %d from nameTable %p and context '%s'.\n", first, o->type, ntc, nctx->sn);
		}
	}

	/* create an undefined variable */
	if (((u8 *)v->bc)[i] == 0xD7) {

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

		/* get error info */
		unsigned int f_ln;
		unsigned int f_col;
		vmGetErrorInfo(v, &f_ln, &f_col);

		/* get object type from name */
		if (!strcmp(tp_name, "int")) ob_type = OBJECT_INT | ob_type;
		else if (!strcmp(tp_name, "chr")) {

			ob_type = OBJECT_CHR | ob_type;
		}
		else {

			/* get type from list */
			unsigned char tt = typeGet(tp_name);

			/* failed to get type */
			if (tt == 0xFF) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Unknown type");
				errorSetPos(f_ln, f_col, v->ctx->fn);
				return NULL;
			}

			/* set type */
			ob_type = tt | ob_type;
		}

		/* get array object */
		if (ob_type & OBJECT_ARRAY) {

			object *asz = vmHandle(v, v->lowbi + v->nofbytes);

			/* error */
			if (asz == NULL || errorIsSet())
				return NULL;

			int nsz_arr;

			/* array size should be an integer */
			if (asz->type != OBJECT_INT) {

				/* set error and exit */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Array size must be an integer");
				errorSetPos(asz->lineno, asz->colno, v->ctx->fn);
				return NULL;
			}

			/* get array size */
			nsz_arr = O_INT(asz)->val;

			/* create an array object */
			object *a = arrayobjectNew(nsz_arr, ob_type & 0x3);

			o = pointerobjectNew(OBJECT_CHR, (void *)a);
		}

		/* pointer */
		else if (ob_type & OBJECT_POINTER) {

			/* create pointer object */
			o = pointerobjectNew(ob_type & 3, NULL);
		}

		/* struct */
		else if (ob_type == OBJECT_STRUCT) {

			/* get struct type object */
			object *tp = typeobjectGet(tp_name);

			/* copy struct */
			o = structobjectInstance(O_TYPE(tp)->st);
		}

		/* otherwise */
		else {

			/* create an int or char */
			o = intcharobjectNew(((ob_type & 3) != OBJECT_INT)? 1: 0, 0);
		}

		/* set name */
		namesSet(v->ctx->nt, ob_name, o);
		o->lineno = f_ln;
		o->colno  = f_col;

		/* debug info */
		if (VM_DEBUG) {
			
			printf("[vm] created undefined variable with type '%s", tp_name);
			if (ob_type & OBJECT_POINTER) printf(" *");
			if (ob_type & OBJECT_ARRAY) printf("[]");
			printf("' and with name '%s'\n", ob_name);
		}
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

		/* get object type from name */
		if (!strcmp(tp_name, "int")) ob_type = OBJECT_INT | ob_type;
		else if (!strcmp(tp_name, "chr")) ob_type = OBJECT_CHR | ob_type;
		else {

			/* get type from list */
			unsigned char tt = typeGet(tp_name);

			/* failed to get type */
			if (tt == 0xFF) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Unknown type");
				errorSetPos(val->lineno, val->colno, val->fname);
				return NULL;
			}

			/* set type */
			ob_type = tt | ob_type;
		}

		/* mismatched types */
		if ((ob_type & 0x3) != (val->type & 0x3)) {

			/* create error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Mismatched types");
			errorSetPos(val->lineno, val->colno, val->fname);
			return NULL;
		}

		/* set the name value */
		namesSet(v->ctx->nt, ob_name, val);

		/* set object */
		o = val;

		/* debug info */
		if (VM_DEBUG) {
			
			printf("[vm] set object '%s' with type %02x", ob_name, o->type);
			if ((ob_type & 0x3) == OBJECT_INT) printf(" and with value %d", O_INT(o)->val);
			//else if ((o->type == (OBJECT_CHR | OBJECT_ARRAY)) || (o->type == (OBJECT_CHR | OBJECT_POINTER))) printf(" and with value '%s'", O_ARRAY(o)->n_start);
			printf(".\n");
		}
	}

	/* function call */
	if (((u8 *)v->bc)[i] == 0xD5) {

		/* get function object */
		object *fnc = vmHandle(v, v->lowbi + (++v->nofbytes));

		/* error */
		if (fnc == NULL || errorIsSet())
			return NULL;

		/* get the number of args */
		v->nofbytes++;
		int n_of_args = vmGetInt(v, v->lowbi + v->nofbytes);
		v->nofbytes += 4;

		/* make list of objects */
		object **ob_args = (object **)malloc(sizeof(object *) * n_of_args);
		int err = 0;
		object *errobj = NULL;
		
		/* get the objects */
		for (int i = 0; i < n_of_args; i++) {

			/* get the argument */
			object *a = vmHandle(v, v->lowbi + v->nofbytes);

			/* error */
			if (a == NULL || errorIsSet()) {

				free(ob_args);
				return NULL;
			}

			/* if previously was an error */
			if (err)
				continue;

			/* match the type */
			if (a->type != O_FUNC(fnc)->fa_types[i]) {

				err = 1;
				errobj = a;
			}

			/* add the object */
			ob_args[i] = a;
		}

		/* get error info */
		int lineno, colno;
		vmGetErrorInfo(v, &lineno, &colno);

		/* check the argument count */
		if (n_of_args != O_FUNC(fnc)->n_of_args) {

			/* set error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Invalid number of arguments passed to function");
			errorSetPos(lineno, colno, v->ctx->fn);
			free(ob_args);
			return NULL;
		}

		/* mismatched types */
		if (err) {
		
			/* set an error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Mismatched types");
			errorSetPos(errobj->lineno, errobj->colno, errobj->fname);
			free(ob_args);
			return NULL;
		}

		/* check the type */
		if (fnc->type != OBJECT_FUNC) {

			/* set error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Illegal operation");
			errorSetPos(lineno, colno, v->ctx->fn);
			free(ob_args);
			return NULL;
		}

		/* check if the reference is undefined */
		if (O_FUNC(fnc)->fb_start == NULL) {

			/* set error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_UNDEFINEDNAME,
					 "Undefined reference");
			errorSetPos(lineno, colno, v->ctx->fn);
			free(ob_args);
			return NULL;
		}

		/* create a context for the function */
		context *fctx = contextNew(fnc->fname, O_FUNC(fnc)->func_name);
		fctx->nt->parent = v->ctx->nt;
		fctx->tp = CONTEXT_FUNC;

		/* if it is a builtin function */
		if (O_FUNC(fnc)->is_builtin) {

			/* get underlying function */
			bfunc_handle_t bf = (bfunc_handle_t)(O_FUNC(fnc)->fb_start);

			/* call function */
			object *a = bf(ob_args, fctx);

			/* error */
			if (a == NULL || errorIsSet()) {

				/* free values and exit */
				free(ob_args);
				contextFree(fctx);
				return NULL;
			}

			/* set object */
			o = a;
		}
		else {

			/* backup variables from vm */
			context *ctx_old = O_FUNC(fnc)->ov->ctx;
			int nofbytes_old = O_FUNC(fnc)->ov->nofbytes;
			int lowbi_old = O_FUNC(fnc)->ov->lowbi;
			void *bc_old = O_FUNC(fnc)->ov->bc;
	
			/* set new values */
			O_FUNC(fnc)->ov->ctx = fctx;
			O_FUNC(fnc)->ov->nofbytes = 0;
			O_FUNC(fnc)->ov->lowbi = 0;
			O_FUNC(fnc)->ov->bc = O_FUNC(fnc)->fb_start;
	
			/* set values for arguments */
			for (int i = 0; i < n_of_args; i++)
				namesSet(fctx->nt, O_FUNC(fnc)->fa_names[i], ob_args[i]);

			object *rt = NULL; /* return value */
	
			/* execute bytecode */
			for (int i = 0; i < O_FUNC(fnc)->fb_n; i++) {
	
				/* get object */
				O_FUNC(fnc)->ov->lowbi += O_FUNC(fnc)->ov->nofbytes;
				O_FUNC(fnc)->ov->nofbytes = 0;
				object *res = vmHandle(O_FUNC(fnc)->ov, O_FUNC(fnc)->ov->lowbi);

				/* return value (no error if there is one) */
				if (fctx->rt != NULL) {

					rt = fctx->rt;

					/* check types */
					if (rt->type != O_FUNC(fnc)->rt_type) {

						/* set error */
						errorSet(ERROR_TYPE_RUNTIME,
								 ERROR_CODE_ILLEGALOP,
								 "Mismatched types");
						errorSetPos(rt->lineno, rt->colno, rt->fname);
						return NULL;
					}

					res = rt;
					fctx->rt = NULL;
					break;
				}
	
				/* error */
				if (res == NULL || errorIsSet()) {
	
					contextFree(fctx); /* free the new context */
	
					/* restore values */
					O_FUNC(fnc)->ov->ctx = ctx_old;
					O_FUNC(fnc)->ov->nofbytes = nofbytes_old;
					O_FUNC(fnc)->ov->lowbi = lowbi_old;
					O_FUNC(fnc)->ov->bc = bc_old;
	
					free(ob_args);
	
					/* exit */
					return NULL;
				}
			}
	
			/* restore original values for vm and free new context */
			O_FUNC(fnc)->ov->ctx = ctx_old;
			O_FUNC(fnc)->ov->nofbytes = nofbytes_old;
			O_FUNC(fnc)->ov->lowbi = lowbi_old;
			O_FUNC(fnc)->ov->bc = bc_old;

			/* set return value */
			if (rt != NULL) o = rt;
			else o = intobjectNew(0);
		}

		contextFree(fctx);

		free(ob_args); /* free argument list because we don't need it anymore */

		/* debug info */
		if (VM_DEBUG) printf("[vm] called function '%s'.\n", O_FUNC(fnc)->func_name);
	}

	/* function definition */
	if (((u8 *)v->bc)[i] == 0xDC) {

		/* get declaration */
		object *dec = vmHandle(v, v->lowbi + (++v->nofbytes));

		/* error */
		if (dec == NULL || errorIsSet())
			return NULL;

		/* check type */
		if (dec->type != OBJECT_FUNC) {

			/* set error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Illegal operation");
			errorSetPos(dec->lineno, dec->colno, dec->fname);
			return NULL;
		}

		/* get number of nodes */
		v->nofbytes++;
		int ncnt = vmGetInt(v, v->lowbi + v->nofbytes);
		v->nofbytes += 4;

		/* size of function body */
		v->nofbytes++;
		int nsz = vmGetInt(v, v->lowbi + v->nofbytes);
		v->nofbytes += 4;

		/* get pointer to body and advance past it */
		void *fb_start = (void *)&(((u8 *)v->bc)[v->lowbi + v->nofbytes]);

		v->nofbytes += nsz;

		/* set values */
		O_FUNC(dec)->fb_start = fb_start;
		O_FUNC(dec)->fb_n = ncnt;
		O_FUNC(dec)->ov = v;

		o = dec;

		/* debug info */
		if (DEBUG) printf("[vm] set function body for '%s'.\n", O_FUNC(o)->func_name);
	}

	/* function declaration */
	if (((u8 *)v->bc)[i] == 0xDB) {

		/* function type */
		v->nofbytes += 2;
		char *tp_name = &(((char *)v->bc)[v->lowbi + v->nofbytes]);
		v->nofbytes += strlen(tp_name) + 1;

		/* name of function */
		char *fn_name = &(((char *)v->bc)[v->lowbi + (++v->nofbytes)]);
		v->nofbytes += strlen(fn_name) + 1;

		/* pointer type */
		int is_p = (((u8 *)v->bc)[v->lowbi + (v->nofbytes++)]);

		/* number of arguments */
		v->nofbytes++;
		int n_of_args = vmGetInt(v, v->lowbi + v->nofbytes);
		v->nofbytes += 4;

		int err = 0; /* error code (0 = none, 1 = unknown type name) */

		/* list of argument names and argument types */
		char **arg_names = (char **)malloc(sizeof(char *) * n_of_args);
		u8 *arg_types = (u8 *)malloc(n_of_args);

		/* loop and get argument types and names */
		for (int i = 0; i < n_of_args; i++) {

			/* arg type name */
			char *at = &(((char *)v->bc)[v->lowbi + (++v->nofbytes)]);
			v->nofbytes += strlen(at) + 1;

			/* arg name */
			char *an = &(((char *)v->bc)[v->lowbi + (++v->nofbytes)]);
			v->nofbytes += strlen(an) + 1;

			/* is a pointer */
			int ap = (((u8 *)v->bc)[v->lowbi + (v->nofbytes++)]);

			/* no error */
			if (!err) {

				/* add argument name */
				arg_names[i] = an;

				/* get argument type from string */
				u8 att = ap? OBJECT_POINTER: 0;

				if (!strcmp(at, "int")) att = OBJECT_INT | att;
				else if (!strcmp(at, "chr")) att = OBJECT_CHR | att;
				else {

					/* get type from list */
					unsigned char tt = typeGet(at);

					/* failed to get type */
					if (tt == 0xFF) {

						err = 1;
					} else att = tt | att;
				}

				/* add argument type */
				arg_types[i] = att;
			}
		}

		/* get error info */
		int lineno, colno;
		vmGetErrorInfo(v, &lineno, &colno);

		/* get return type from string */
		u8 rt_type = is_p? OBJECT_POINTER: 0;

		if (!strcmp(tp_name, "int")) rt_type = OBJECT_INT | rt_type;
		else if (!strcmp(tp_name, "chr")) rt_type = OBJECT_CHR | rt_type;
		else {

			/* get type from list */
			unsigned char tt = typeGet(tp_name);

			/* failed to get type */
			if (tt == 0xFF) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Unknown type");
				errorSetPos(lineno, colno, v->ctx->fn);
				return NULL;
			}

			/* set type */
			rt_type = tt | rt_type;
		}

		/* error from previously */
		if (err) {

			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Illegal operation");
			errorSetPos(lineno, colno, v->ctx->fn);
			return NULL;
		}

		/* create a function object */
		o = functionobjectNew(fn_name, rt_type, arg_names, arg_types, n_of_args);

		/* set the function name */
		namesSet(v->ctx->nt, fn_name, o);

		/* debug info */
		if (DEBUG) printf("[vm] created function reference '%s'.\n", fn_name);
	}

	/* extern */
	if (((u8 *)v->bc)[i] == 0xC1) {

		v->nofbytes++;

		/* set up scope */
		nameTable *nt_old = v->ctx->nt;
		v->ctx->nt = vmdctx->nt;

		/* get object */
		object *a = vmHandle(v, v->lowbi + v->nofbytes);

		/* restore context */
		v->ctx->nt = nt_old;

		/* error */
		if (a == NULL || errorIsSet())
			return NULL;

		/* set object */
		o = a;
	}

	/* return value */
	if (((u8 *)v->bc)[i] == 0xC0) {

		/* get value */
		object *rt = vmHandle(v, v->lowbi + (++v->nofbytes));

		/* error */
		if (rt == NULL || errorIsSet())
			return NULL;

		/* get error info */
		int lineno, colno;
		vmGetErrorInfo(v, &lineno, &colno);

		/* if we are not in a function */
		if (v->ctx->tp != CONTEXT_FUNC) {

			/* set error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Illegal operation");
			errorSetPos(lineno, colno, v->ctx->fn);
			return NULL;
		}

		/* set the return object */
		v->ctx->rt = rt;
		o = NULL; /* set to NULL so that no other code will be executed */
	}

	/* if statement */
	if ((((u8 *)v->bc)[i]) == 0xD8) {

		/* advance and get condition */
		object *cond = vmHandle(v, v->lowbi + (++v->nofbytes));

		/* error */
		if (cond == NULL || errorIsSet())
			return NULL;

		/* get number of nodes */
		v->nofbytes++;
		int nch = vmGetInt(v, v->lowbi + v->nofbytes);
		v->nofbytes += 4;

		/* get length of if node body */
		v->nofbytes++;
		int nofbytes = (vmGetInt(v, v->lowbi + v->nofbytes)) + v->nofbytes + 4;
		v->nofbytes += 4;

		/* if condition is true */
		if (!((cond->type == OBJECT_INT) && (O_INT(cond)->val == 0))) {

			/* execute nodes */
			for (int i = 0; i < nch; i++) {

				/* error */
				if (vmHandle(v, v->lowbi + v->nofbytes) == NULL || errorIsSet())
					return NULL;
			}
		}

		/* set number of bytes and return value */
		v->nofbytes = nofbytes;

		o = intobjectNew(0);
	}

	/* for loop */
	if ((((u8 *)v->bc)[i]) == 0xDA) {

		/* advance */
		v->nofbytes++;

		/* get start node */
		object *start = vmHandle(v, v->lowbi + v->nofbytes);

		/* error */
		if (start == NULL || errorIsSet())
			return NULL;

		/* store values */
		int nofbytes_old = (v->nofbytes);
		int nofbytes;

		/* loop */
		while (1) {

			/* get condition */
			object *cond = vmHandle(v, v->lowbi + v->nofbytes);

			/* error */
			if (cond == NULL || errorIsSet())
				return NULL;

			/* number of nodes */
			v->nofbytes++;
			int nch = vmGetInt(v, v->lowbi + v->nofbytes);
			v->nofbytes += 4;

			/* number of bytes */
			v->nofbytes++;
			nofbytes = (vmGetInt(v, v->lowbi + v->nofbytes)) + 4 + v->nofbytes;
			v->nofbytes += 4;

			/* condition isn't true */
			if ((cond->type == OBJECT_INT) && (O_INT(cond)->val == 0))
				break;

			object *rt;

			/* loop through nodes */
			for (int i = 0; i < nch; i++) {

				/* error */
				if ((rt = vmHandle(v, v->lowbi + v->nofbytes)) == NULL || errorIsSet())
					return NULL;
			}

			/* increment */
			object *inc = vmHandle(v, v->lowbi + v->nofbytes);

			/* error */
			if (inc == NULL || errorIsSet())
				return NULL;

			/* keyboard interrupt */
			if (INT_SIGNAL) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_KBINT,
						 "Keyboard interrupt");
				errorSetPos(rt->lineno, rt->colno, rt->fname);

				INT_SIGNAL = 0;
				return NULL;
			}

			/* reset value */
			v->nofbytes = nofbytes_old;

			/* garbage collect */
			objectCollect();
		}

		/* skip past body */
		v->nofbytes = nofbytes;

		/* set return value */
		o = intobjectNew(0);
	}

	/* while loop */
	if ((((u8 *)v->bc)[i]) == 0xD9) {

		int nofbytes_old = (++v->nofbytes);
		int nofbytes;

		/* loop */
		while (1) {

			/* get condition */
			object *cond = vmHandle(v, v->lowbi + v->nofbytes);

			/* couldn't get condition value */
			if (cond == NULL || errorIsSet())
				return NULL;

			/* get number of children */
			v->nofbytes++;
			int nch = vmGetInt(v, v->lowbi + v->nofbytes);
			v->nofbytes += 4;

			/* get size of while node body */
			v->nofbytes++;
			nofbytes = v->nofbytes + 4 + (vmGetInt(v, v->lowbi + v->nofbytes));
			v->nofbytes += 4;

			/* leave if condition says so */
			if ((cond->type == OBJECT_INT) && (O_INT(cond)->val == 0))
				break;

			object *rt;

			/* loop through body nodes */
			for (int i = 0; i < nch; i++) {

				/* error */
				if ((rt = vmHandle(v, v->lowbi + v->nofbytes)) == NULL || errorIsSet()) {

					return NULL;
				}
			}

			/* interrupt */
			if (INT_SIGNAL) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_KBINT,
						 "Keyboard interrupt");
				errorSetPos(rt->lineno, rt->colno, rt->fname);

				INT_SIGNAL = 0;
				return NULL;
			}

			/* reset pos */
			v->nofbytes = nofbytes_old;

			/* garbage collect */
			objectCollect();
		}

		/* skip past */
		v->nofbytes = nofbytes;

		/* set object */
		o = intobjectNew(0);
	}

	/* struct */
	if ((((u8 *)v->bc)[i]) == 0xC2) {

		/* get struct name */
		v->nofbytes += 2;
		char *struct_name = &(((char *)v->bc)[v->lowbi + v->nofbytes]);
		v->nofbytes += strlen(struct_name) + 1;

		/* get number of nodes */
		v->nofbytes++;
		int nch = vmGetInt(v, v->lowbi + v->nofbytes);
		v->nofbytes += 4;

		/* create struct */
		context *myctx = contextNew(v->ctx->fn, struct_name);
		myctx->tp = CONTEXT_STRUCT;

		/* backup ctx */
		context *octx = v->ctx;
		v->ctx = myctx;

		/* execute bytecode */
		for (int i = 0; i < nch; i++) {

			/* get object */
			object *c = vmHandle(v, v->lowbi + v->nofbytes);

			/* error */
			if (c == NULL || errorIsSet()) {

				/* return */
				v->ctx = octx;
				return NULL;
			}
		}

		object *st = structobjectNew(myctx, struct_name);

		/* set old ctx */
		v->ctx = octx;

		/* set struct */
		namesSet(v->ctx->nt, struct_name, st);
		typeRegisterStruct(struct_name, st);

		o = st;
	}

	/* typedef */
	if ((((u8 *)v->bc)[i]) == 0xC3) {

		/* get pointer value */
		int is_p = (((u8 *)v->bc)[++v->nofbytes]);
		v->nofbytes += 2;

		/* get name of old type */
		char *otp = &(((char *)v->bc)[v->lowbi + v->nofbytes]);
		v->nofbytes += strlen(otp) + 2;

		/* get name of new type */
		char *ntp = &(((char *)v->bc)[v->lowbi + v->nofbytes]);
		v->nofbytes += strlen(ntp) + 1;

		/* get error info */
		int lineno, colno;
		vmGetErrorInfo(v, &lineno, &colno);

		/* get type type */
		unsigned char tp_type = is_p? OBJECT_POINTER: 0;
		if (!strcmp(otp, "int")) tp_type = OBJECT_INT | tp_type;
		else if (!strcmp(otp, "chr")) tp_type = OBJECT_CHR | tp_type;
		else {

			/* get type from list */
			unsigned char tt = typeGet(otp);

			/* failed to get type */
			if (tt == 0xFF) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_ILLEGALOP,
						 "Unknown type");
				errorSetPos(lineno, colno, v->ctx->fn);
				return NULL;
			}

			/* set type */
			tp_type = tt | tp_type;
		}

		/* set object and type */
		typeRegister(ntp, tp_type);
		o = intobjectNew(0);

		/* debug */
		if (VM_DEBUG) printf("[vm] created type '%s' based off of '%s'\n", ntp, otp);
	}

	if (VM_DEBUG) printf("[vm] vm scope is '%s' in file '%s'\n", v->ctx->sn, v->ctx->fn);

	if (o == NULL) {
	
		/* vm debug */
		if (VM_DEBUG) {

			/* not built to handle */
			printf("[vm] not built to handle '%02x' (%08x)\n[vm] trace:\n", ((u8 *)v->bc)[i], i);
		
			/* trace */
			for (int i = 0; i < dbt_n; i++)
				printf("\t- pointer: %p, type: %d\n", debug_trace[i], debug_trace[i]->type);
		}
	
		/* no other option */
		return NULL;
	}

	/* add object to trace */
	if (dbt_n < 8) {

		debug_trace[dbt_n++] = o;
	}

	else {

		memcpy(debug_trace, &debug_trace[1], 7 * sizeof(object *));
		debug_trace[7] = o;
	}

	/* get error info */
	vmGetErrorInfo(v, &o->lineno, &o->colno);
	if (o->fname == NULL) o->fname = v->ctx->fn;

	if (VM_DEBUG) printf("[vm] finished interpreting object.\n");

	return o; /* object */
}

/* load idata table */
extern void vmLoadIdataTable(vm *v) {

	/* more debug info */
	if (VM_DEBUG) printf("[vm] idata table: %p\n", v->idata);

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
				char *idt = &(((char *)v->bc)[loc]);
				v->idata[v->nidat++] = idt;
				if (VM_DEBUG) printf("[vm] idata: '%s'\n", idt);

				/* advance i */
				i += 6;
			}

			break;
		}
	}

	/* debug */
	if (VM_DEBUG) printf("[vm] loaded idata table successfully.\n");
}

int _vmloadeddata = 0;

/* load builtin functions */
extern void vmLoadBuiltins() {

	/* write */
	char **write_arg_names = (char **)malloc(sizeof(char *) * 3);
	write_arg_names[0] = "fd";
	write_arg_names[1] = "buf";
	write_arg_names[2] = "n";
	u8 *write_arg_types = (u8 *)malloc(sizeof(u8) * 3);
	write_arg_types[0] = OBJECT_INT;
	write_arg_types[1] = OBJECT_CHR | OBJECT_POINTER;
	write_arg_types[2] = OBJECT_INT;
	object *bfWrite = functionobjectNew("write", OBJECT_INT, write_arg_names, write_arg_types, 3);
	O_FUNC(bfWrite)->is_builtin = 1;
	O_FUNC(bfWrite)->fb_start = (unsigned char *)builtinWrite;
	namesSet(vmdctx->nt, "write", bfWrite);

	/* read */
	char **read_arg_names = (char **)malloc(sizeof(char *) * 3);
	read_arg_names[0] = "fd";
	read_arg_names[1] = "buf";
	read_arg_names[2] = "n";
	u8 *read_arg_types = (u8 *)malloc(sizeof(u8) * 3);
	read_arg_types[0] = OBJECT_INT;
	read_arg_types[1] = OBJECT_CHR | OBJECT_POINTER;
	read_arg_types[2] = OBJECT_INT;
	object *bfRead = functionobjectNew("read", OBJECT_INT, read_arg_names, read_arg_types, 3);
	O_FUNC(bfRead)->is_builtin = 1;
	O_FUNC(bfRead)->fb_start = (unsigned char *)builtinRead;
	namesSet(vmdctx->nt, "read", bfRead);

	/* debug info */
	if (VM_DEBUG) printf("[vm] initialised builtin functions.\n");
}

/* execute bytecode */
extern void vmExec(vm *v) {

	/* create variables for true (1), false (0), and null (0) */
	if (!_vmloadeddata) {
	
		namesSet(vmdctx->nt, "true", intobjectNew(1));
		namesSet(vmdctx->nt, "false", intobjectNew(0));
		namesSet(vmdctx->nt, "null", intobjectNew(0));
		namesSet(vmdctx->nt, "nullchar", charobjectNew(0));
	
		/* load builtin functions */
		vmLoadBuiltins();

		_vmloadeddata = 1; /* loaded data for vmdctx */
	}

	/* debug info */
	if (VM_DEBUG) {

		printf("[vm] initialised variables true (0), false (0), and null (0).\n");
	}

	object *co = NULL; /* current object */

	/* get flags */
	v->bcflags = ((u8 *)v->bc)[7];

	if (VM_DEBUG) printf("[vm] flags for '%p': %02x\n", v, v->bcflags);

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
			if (VM_DEBUG) printf("[vm] changed filename of scope '%s' to '%s'\n", v->ctx->sn, v->ctx->fn);

			i += strlen(v->ctx->fn) + 3;
		}

		/* exit because we've reached the end with the idata table */
		else if (((u8 *)v->bc)[i] == 0xFD) {

			break;
		}

		/* library reference */
		else if (((u8 *)v->bc)[i] == 0xE0) {

			/* interrupt */
			if (INT_SIGNAL) {

				/* set error */
				errorSet(ERROR_TYPE_RUNTIME,
						 ERROR_CODE_KBINT,
						 "Keyboard interrupt");
				errorSetPos(0, 0, v->ctx->fn);

				INT_SIGNAL = 0;
				return;
			}

			char *fn = vmdctx->fn;

			/* get name of library */
			char *lib = &(((char *)v->bc)[i + 1]);
			i += strlen(lib) + 2;

			/* create vm from file */
			vm *sv = vmNewFromFile(lib);

			if (sv == NULL) {

				char *nm = (char *)malloc(strlen(lib));
				strncpy(nm, lib, strlen(lib) - 3);

				/* print and error and exit */
				fprintf(stderr, "Failed to load mango library '%s'.\n", nm);
				free(nm);
				return;
			}

			/* execute code */
			vmExec(sv);
			vmdctx->fn = fn;

			/* error */
			if (errorIsSet()) {

				/* print the error and exit */
				errorPrint();
				return;
			}
		}

		/* otherwise */
		else {

			v->nofbytes = 0; /* reset the number of bytes recorded */
			co = vmHandle(v, i); /* handle object */
			i += v->nofbytes; /* advance number of bytes */

			/* error */
			if (co == NULL || errorIsSet())
				return;

			/* debug */
			if (VM_DEBUG) printf("[vm] ----\n");
		}

		objectCollect();
	}
}

/* get error info */
extern void vmGetErrorInfo(vm *v, unsigned int *lineno, unsigned int *colno) {

	/* get error info */
	if (((u8 *)v->bc)[v->lowbi + v->nofbytes] == 0xFE) {

		/* get line and column numbers */
		int pos = v->lowbi + v->nofbytes + 1;
		unsigned int l = (((u8 *)v->bc)[pos] << 24) | (((u8 *)v->bc)[pos + 1] << 16) | (((u8 *)v->bc)[pos + 2] << 8) | (((u8 *)v->bc)[pos + 3]);
		unsigned int c = (((u8 *)v->bc)[pos + 4] << 24) | (((u8 *)v->bc)[pos + 5] << 16) | (((u8 *)v->bc)[pos + 6] << 8) | (((u8 *)v->bc)[pos + 7]);

		/* set values */
		*lineno = l;
		*colno = c;

		/* debug info */
		if (VM_DEBUG) printf("[vm] line and column numbers: %d, %d\n", l, c);

		/* advance v->nofbytes */
		v->nofbytes += 9;
	}
}

/* free a vm */
extern void vmFree(vm *v) {

	free(v->bc);
	free(v);
}

/* free all vms */
extern void vmFreeAll() {

	if (vm_list != NULL) {

		for (int i = 0; i < vm_list_len; i++) {

			if (vm_list[i] != NULL) vmFree(vm_list[i]);

			/* debug info */
			if (VM_DEBUG) printf("[vm] Freed vm at %p.\n", vm_list[i]);
		}

		/* free the context */
		if (vmdctx == NULL)
			free(vmdctx);

		/* free the list */
		free(vm_list);
	}
}