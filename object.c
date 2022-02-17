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

#include "object.h"
#include "error.h" /* error handling */
#include "arrayobject.h" /* arrays */
#include "intobject.h" /* integers and chars */
#include "pointerobject.h" /* pointers */
#include "structobject.h" /* structs */
#include "functionobject.h" /* functions */
#include "token.h" /* yes */
#include "context.h"
#include "typedef.h"

#include <stdlib.h> /* malloc/realloc/free, etc */
#include <string.h> /* strcpy, strlen, etc */
#include <stdio.h> /* sprintf, printf, etc */
#include <unistd.h> /* more io */

/* list of all objects */
object **objects = NULL;
int n_of_objects; /* number of objects stored in list */
int cap_objects; /* capacity of object list */
int DEBUG = 0; /* debugging */
int INT_SIGNAL = 0; /* if we were interrupted */
int objdout = 0; /* for disabling non-debug output */
int SEGV_SIGNAL = 0; /* for checking if objectSegvHandler was already called */
FILE *debug_file = NULL; /* file that debug info gets sent to */

/* for builtinRead and builtinWrite */
static int open_fds[8] = {0,1,2};
static int open_fdsl = 3;

/* typedef types */
static typeobject **types = NULL;
static int types_len = 0;
static int types_cap = 8;

/* misc */
static int struct_ids = 0;

/* interrupt handler for SIGINT */
extern void objectIntHandler(int n) {

	INT_SIGNAL = 1;
}

/* interrupt handler for SIGSEGV */
extern void objectSegvHandler(int n) {

	/* if we have already tried to exit */
	if (SEGV_SIGNAL) {
		
		fprintf(stderr, "second segv caught! exiting...\n");
		abort();
		return;
	}

	fprintf(stderr, "segmentation fault! exiting...\n");
	SEGV_SIGNAL = 1;
	exit(0); /* exit program */
}

/* create an object */
extern object *objectNew(unsigned char type, unsigned int size) {
	
	object *o = (object *)malloc(size); /* new object! :D */

	/* we have failed */
	if (o == NULL) {
		/* set error */
		if (!errorIsSet()) {
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_MEMORY,
					 "Memory allocation error");
		}
		return NULL; /* :( */
	}

	/* set our values */
	o->type = type;
	o->refcnt = 0; /* can be freed if it needs to (starts out at 1 so that garbage collection won't immediately take care of it) */
	
	o->lineno = 0;
	o->colno = 0;
	o->fname = NULL;

	o->sz = size;

	o->copied = 0;

	/* auto initalise list */
	if (objects == NULL) {
		objects = (object **)malloc(sizeof(object*) * 8);
		n_of_objects = 0;
		cap_objects = 8;
		memset(objects, 0, sizeof(object *) * 8);

		if (DEBUG) fprintf(debug_file, "[DEBUG] Initalised object list.\n");
	}

	unsigned int lnums;

	/* find index for object */
	for (lnums = 0; lnums < n_of_objects; lnums++) {

		/* null */
		if (objects[lnums] == NULL)
			break;
	}

	/* if list is already full then resize it */
	if (n_of_objects >= cap_objects) {
		objects = (object **)realloc(objects, sizeof(object*) * cap_objects * 2);
		cap_objects *= 2;

		if (DEBUG) fprintf(debug_file, "[DEBUG] Resized object list to %d item(s) capacity.\n", cap_objects);
	}

	/* increase n_of_objects if necessary */
	if (lnums == n_of_objects) n_of_objects++;

	/* add item to list */
	objects[lnums] = o;

	/* debug mode */
	if (DEBUG) fprintf(debug_file, "[DEBUG] Successfully created an object (%p) of type %d.\n", o, type);

	return o; /* last piece of the puzzle */
}

/* copy an object */
extern object *objectCopy(object *o) {

	object *o2 = objectNew(o->type, o->sz);
	memcpy(o2, o, o->sz);
	
	o2->refcnt = 0;
	o2->copied = 1;

	/* return object */
	return o2;
}

/* create a new type */
extern object *typeNew(char *tp_name, unsigned char tp_type) {

	/* create object */
	object *tp = objectNew(tp_type | OBJECT_TYPE, sizeof(typeobject));

	/* TODO: implement rest of function */
	O_TYPE(tp)->tp_name = tp_name;
	O_TYPE(tp)->tp_type = tp_type;

	/* return type */
	return tp;
}

/* create and register a new type */
extern void typeRegister(char *tp_name, unsigned char tp_type) {

	/* create object */
	object *tp = typeNew(tp_name, tp_type);

	/* create list */
	if (types == NULL) {

		types_len = 0;
		types_cap = 8;
		types = (typeobject **)malloc(sizeof(typeobject *) * types_cap);
	}

	/* realloc list */
	if (types_len >= types_cap) {

		types_cap *= 2;
		types = (typeobject **)realloc(types, sizeof(typeobject *) * types_cap);
	}

	/* add item */
	types[types_len++] = O_TYPE(tp);
}

/* create and register struct type */
extern void typeRegisterStruct(char *tp_name, object *st) {

	/* create object */
	object *tp = typeNew(tp_name, OBJECT_STRUCT);
	O_TYPE(tp)->st = st;

	/* create list */
	if (types == NULL) {

		types_len = 0;
		types_cap = 8;
		types = (typeobject **)malloc(sizeof(typeobject *) * types_cap);
	}

	/* realloc list */
	if (types_len >= types_cap) {

		types_cap *= 2;
		types = (typeobject **)realloc(types, sizeof(typeobject *) * types_cap);
	}

	/* add item */
	types[types_len++] = O_TYPE(tp);
}

/* get a new type */
extern unsigned char typeGet(char *tp_name) {

	/* loop through types */
	for (int i = 0; i < types_len; i++) {

		if (!strcmp(types[i]->tp_name, tp_name))
			return types[i]->tp_type;
	}

	/* no type */
	return 0xFF;
}

/* get a new type */
extern object *typeobjectGet(char *tp_name) {

	/* loop through types */
	for (int i = 0; i < types_len; i++) {

		if (!strcmp(types[i]->tp_name, tp_name))
			return O_OBJ(types[i]);
	}

	/* no type */
	return NULL;
}

/* perform operation on object */
extern object *objectOperation(object *obj, object *other, unsigned int op_num) {

	if (DEBUG) fprintf(debug_file, "[DEBUG] operation: (type:%d op:%d type:%d)\n", obj->type, op_num, other->type);

	/* '+' */
	if (op_num == TOKEN_PLUS) {

		/* integer */
		if (obj->type == OBJECT_INT && other->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val + O_INT(other)->val);

		/* char + int */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_INT))
			return intobjectNew(O_CHR(obj)->val + O_INT(other)->val);

		/* char + char */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_CHR))
			return intobjectNew(O_CHR(obj)->val + O_CHR(other)->val);
	}

	/* '-' */
	else if (op_num == TOKEN_MINUS) {

		/* integer */
		if (obj->type == OBJECT_INT && other->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val - O_INT(other)->val);

		/* char - int */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_INT))
			return intobjectNew(O_CHR(obj)->val - O_INT(other)->val);

		/* char - char */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_CHR))
			return intobjectNew(O_CHR(obj)->val - O_CHR(other)->val);
	}

	/* '*' */
	else if (op_num == TOKEN_MUL) {

		/* integer */
		if (obj->type == OBJECT_INT && other->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val * O_INT(other)->val);

		/* char * int */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_INT))
			return intobjectNew(O_CHR(obj)->val * O_INT(other)->val);

		/* char * char */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_CHR))
			return intobjectNew(O_CHR(obj)->val * O_CHR(other)->val);
	}

	/* '/' */
	else if (op_num == TOKEN_DIV) {

		/* integer */
		if (obj->type == OBJECT_INT && other->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val / O_INT(other)->val);

		/* char / int */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_INT))
			return intobjectNew(O_CHR(obj)->val / O_INT(other)->val);

		/* char / char */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_CHR))
			return intobjectNew(O_CHR(obj)->val / O_CHR(other)->val);
	}

	/* '%' */
	else if (op_num == TOKEN_MOD) {

		/* integer */
		if (obj->type == OBJECT_INT && other->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val % O_INT(other)->val);

		/* char % int */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_INT))
			return intobjectNew(O_CHR(obj)->val % O_INT(other)->val);

		/* char % char */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_CHR))
			return intobjectNew(O_CHR(obj)->val % O_CHR(other)->val);
	}

	/* '==' */
	else if (op_num == TOKEN_EE) {

		/* integer */
		if (obj->type == OBJECT_INT && other->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val == O_INT(other)->val);

		/* char == int */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_INT))
			return intobjectNew(O_CHR(obj)->val == O_INT(other)->val);

		/* char == char */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_CHR))
			return intobjectNew(O_CHR(obj)->val == O_CHR(other)->val);
	}

	/* '!=' */
	else if (op_num == TOKEN_NE) {

		/* integer */
		if (obj->type == OBJECT_INT && other->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val != O_INT(other)->val);

		/* char != int */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_INT))
			return intobjectNew(O_CHR(obj)->val != O_INT(other)->val);

		/* char != char */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_CHR))
			return intobjectNew(O_CHR(obj)->val != O_CHR(other)->val);
	}

	/* '<' */
	else if (op_num == TOKEN_LT) {

		/* integer */
		if (obj->type == OBJECT_INT && other->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val < O_INT(other)->val);

		/* char < int */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_INT))
			return intobjectNew(O_CHR(obj)->val < O_INT(other)->val);

		/* char < char */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_CHR))
			return intobjectNew(O_CHR(obj)->val < O_CHR(other)->val);
	}

	/* '>' */
	else if (op_num == TOKEN_GT) {

		/* integer */
		if (obj->type == OBJECT_INT && other->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val > O_INT(other)->val);

		/* char > int */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_INT))
			return intobjectNew(O_CHR(obj)->val > O_INT(other)->val);

		/* char > char */
		else if ((obj->type == OBJECT_CHR) && (other->type == OBJECT_CHR))
			return intobjectNew(O_CHR(obj)->val > O_CHR(other)->val);
	}

	/* create error */
	errorSet(ERROR_TYPE_RUNTIME, ERROR_CODE_ILLEGALOP,
			 "Illegal operation");
	errorSetPos(obj->lineno, obj->colno, obj->fname);

	/* exit */
	return NULL;
}

/* create an array object */
extern object *arrayobjectNew(int n, unsigned char t) {

	/* malloc new array object */
	arrayobject *a = (arrayobject *)objectNew(t | OBJECT_ARRAY, sizeof(arrayobject) + (n * sizeof(O_TPSZ(t))));

	/* :( */
	if (a == NULL)
		return NULL;

	/* assign values */
	a->lineno = 0;
	a->colno = 0;
	a->fname = NULL;
	a->refcnt = 0;
	a->n_len = n;
	a->n_sz = 0;
	a->n_start = ((void *)a + sizeof(arrayobject));

	/* clear buffer */
	memset(a->n_start, 0, (n * sizeof(O_TPSZ(t))));

	/* return object */
	return O_OBJ(a);
}

/* create an integer */
extern object *intobjectNew(int val) {

	/* return a thing */
	return intcharobjectNew(0, val);
}

/* create a char */
extern object *charobjectNew(char val) {

	/* return another thing */
	return intcharobjectNew(1, val);
}

/* create an integer or character */
extern object *intcharobjectNew(int isch, int val) {

	/* allocate object */
	size_t sz = isch? sizeof(intobject): sizeof(charobject);
	object *o = objectNew(isch? OBJECT_CHR: OBJECT_INT, sz);

	/* failed to allocate */
	if (o == NULL)
		return NULL;

	/* set value */
	if (isch) O_CHR(o)->val = (char)val;
	else O_INT(o)->val = val;

	/* return object */
	return o;
}

/* create a pointer */
extern object *pointerobjectNew(int tp, void *p) {

	/* create object */
	object *o = objectNew((unsigned char)(tp | OBJECT_POINTER), sizeof(pointerobject));

	/* failed to allocate */
	if (o == NULL)
		return NULL;

	/* set value */
	O_PTR(o)->val = p;

	/* return object */
	return o;
}

/* create a function */
extern object *functionobjectNew(char *func_name, unsigned char rt_type, char **fa_names, unsigned char *fa_types, int n_of_args) {

	/* create object */
	object *o = objectNew(OBJECT_FUNC, sizeof(functionobject));

	/* failed to allocate */
	if (o == NULL)
		return NULL;

	/* set value */
	O_FUNC(o)->func_name = func_name;
	O_FUNC(o)->rt_type = rt_type;
	O_FUNC(o)->fa_names = fa_names;
	O_FUNC(o)->fa_types = fa_types;
	O_FUNC(o)->fb_start = NULL;
	O_FUNC(o)->n_of_args = n_of_args;
	O_FUNC(o)->fb_n = 0;
	O_FUNC(o)->is_builtin = 0;

	/* return object */
	return o;
}

/* create a new struct */
extern object *structobjectNew(context *ctx, char *struct_name) {

	/* create new object */
	object *o = objectNew(OBJECT_STRUCT, sizeof(structobject));

	/* failed to allocate */
	if (o == NULL)
		return NULL;

	/* set values */
	O_STRUCT(o)->struct_name = struct_name;
	O_STRUCT(o)->ctx = ctx;
	O_STRUCT(o)->nt = ctx->nt;
	O_STRUCT(o)->parent = O_STRUCT(o);
	O_STRUCT(o)->is_templ = 1;
	O_STRUCT(o)->id = struct_ids++;

	/* return object */
	return o;
}

/* create a new instance of a struct */
extern object *structobjectInstance(object *s) {

	/* create new object */
	object *s2 = objectNew(OBJECT_STRUCT, sizeof(structobject));

	/* failed to allocate */
	if (s2 == NULL)
		return NULL;

	/* set values */
	O_STRUCT(s2)->struct_name = O_STRUCT(s)->struct_name;
	O_STRUCT(s2)->nt = namesCopy(O_STRUCT(s)->ctx->nt);
	O_STRUCT(s2)->parent = O_STRUCT(s);
	O_STRUCT(s2)->is_templ = 0;

	/* allocate context */
	O_STRUCT(s2)->ctx = (context *)malloc(sizeof(context));
	O_STRUCT(s2)->ctx->fn = O_STRUCT(s)->ctx->fn;
	O_STRUCT(s2)->ctx->sn = O_STRUCT(s)->ctx->sn;
	O_STRUCT(s2)->ctx->nt = O_STRUCT(s2)->nt;

	/* return struct */
	return s2;
}

/* free an object */
extern void objectFree(object *obj) {

	/* function */
	if (obj->type == OBJECT_FUNC) {

		/* free lists of argument names and types */
		free(O_FUNC(obj)->fa_names);
		free(O_FUNC(obj)->fa_types);

		if (DEBUG) fprintf(debug_file, "[DEBUG] Freed fa_names %p, fa_types %p.\n", O_FUNC(obj)->fa_names, O_FUNC(obj)->fa_types);
	}

	/* struct */
	if (obj->type == OBJECT_STRUCT && O_STRUCT(obj)->is_templ) {

		/* free context */
		contextFree(O_STRUCT(obj)->ctx);

		if (DEBUG) fprintf(debug_file, "[DEBUG] Freed ctx %p.\n", O_STRUCT(obj)->ctx);
	}

	/* non-template struct */
	if (obj->type == OBJECT_STRUCT && !(O_STRUCT(obj)->is_templ)) {

		/* free name table */
		contextFree(O_STRUCT(obj)->ctx);

		if (DEBUG) fprintf(debug_file, "[DEBUG] Freed nt %p.\n", O_STRUCT(obj)->nt);
	}

	/* free the object */
	free(obj);
}

extern void objectCollect() {
	
	/* task: go through list, free any objects with low reference counts */
	for (int i = 0; i < n_of_objects; i++) {

		if ((objects[i] != NULL) && (objects[i]->refcnt < 1)) {

			/* debug info */
			if (DEBUG) fprintf(debug_file, "[DEBUG] Freeing %p with type %d...\n", objects[i], objects[i]->type);

			objectFree(objects[i]);

			/* debug */
			if (DEBUG) fprintf(debug_file, "[DEBUG] Garbage freed %p.\n", objects[i]);
			objects[i] = NULL;
		}
	}
}

extern void objectFreeAll() {

	if (DEBUG) fprintf(debug_file, "[DEBUG] Preparing to free objects...\n");

	/* go through list and free all objects. */
	for (int i = 0; i < n_of_objects; i++) {
		if (objects[i] != NULL) {

			/* debug info */
			if (DEBUG) fprintf(debug_file, "[DEBUG] Freeing %p with type %d...\n", objects[i], objects[i]->type);

			objectFree(objects[i]);

			/* debug mode stuff */
			if (DEBUG) fprintf(debug_file, "[DEBUG] Freed %p.\n", objects[i]);
		}
	}

	/* free object list */
	free(objects);

	/* free type list */
	if (types != NULL) free(types);
}

/* builtin function : write */
extern object *builtinWrite(object **ob_args, void *ctx) {

	/* leave if there is no buffer */
	if (O_PTR(ob_args[1])->val == NULL)
		return intobjectNew(0);

	/* get context */
	context *fctx = (context *)ctx;

	/* get arguments */
	int fd = O_INT(ob_args[0])->val;
	char *buf = (char *)(O_ARRAY(O_PTR(ob_args[1])->val)->n_start);
	int n = O_INT(ob_args[2])->val;

	int n_of_chars = 0;

	int i; /* fd position */

	/* validate file descriptor */
	for (i = 0; i < open_fdsl; i++) {

		if (open_fds[i] == fd)
			break;
	}

	/* hasn't been opened by program */
	if (i == open_fdsl) {

		errorSet(ERROR_TYPE_RUNTIME,
				 ERROR_CODE_UNKNOWNFD,
				 "File descriptor not opened by Mango");
		errorSetPos(ob_args[1]->lineno, ob_args[1]->colno, ob_args[1]->fname);
		return NULL;
	}

	/* write info */
	if (!objdout) n_of_chars = write(fd, buf, n);

	/* return from function */
	return intobjectNew(n_of_chars);
}

/* builtin function : read */
extern object *builtinRead(object **ob_args, void *ctx) {

	/* get context */
	context *fctx = (context *)ctx;

	/* get arguments */
	int fd = O_INT(ob_args[0])->val;
	char *buf = (char *)(O_ARRAY(O_PTR(ob_args[1])->val)->n_start);
	int n = O_INT(ob_args[2])->val;

	int n_of_chars = 0;

	int i; /* fd position */

	/* validate file descriptor */
	for (i = 0; i < open_fdsl; i++) {

		if (open_fds[i] == fd)
			break;
	}

	/* hasn't been opened by program */
	if (i == open_fdsl) {

		errorSet(ERROR_TYPE_RUNTIME,
				 ERROR_CODE_UNKNOWNFD,
				 "File descriptor not opened by Mango");
		errorSetPos(ob_args[1]->lineno, ob_args[1]->colno, ob_args[1]->fname);
		return NULL;
	}

	/* read info */
	if (!objdout) n_of_chars = read(fd, buf, n);

	/* return from function */
	return intobjectNew(n_of_chars);
}