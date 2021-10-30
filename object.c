#include "object.h"
#include "error.h" /* error handling */
#include "file.h" /* basic file handling system */
#include "arrayobject.h" /* array */
#include "intobject.h" /* integers and chars */
#include "pointerobject.h" /* pointer */
#include "token.h" /* yes */

#include <stdlib.h> /* malloc/realloc/free, etc */
#include <string.h> /* strcpy, strlen, etc */
#include <stdio.h> /* sprintf, printf, etc */
#include <unistd.h> /* more io */

/* forward declarations */
static object *intcharobjectNew(int isch, int val);

/* list of all objects */
object **objects = NULL;
int n_of_objects; /* number of objects stored in list */
int cap_objects; /* capacity of object list */

/* create an object */
extern object *objectNew(unsigned char type, size_t size) {
	
	object *o = (object*)malloc(size); /* new object! :D */

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
	o->refcnt = 0; /* can be freed if it needs to */
	
	o->lineno = 0;
	o->colno = 0;
	o->fname = NULL;

	/* auto initalise list */
	if (objects == NULL) {
		objects = (object **)malloc(sizeof(object*) * 8);
		n_of_objects = 0;
		cap_objects = 8;

		#if defined(DEBUG) && DEBUG == 1
		printf("[DEBUG] Initalised object list.\n");
		#endif
	}

	/* if list is already full then resize it */
	if (n_of_objects >= cap_objects) {
		objects = (object **)realloc(objects, sizeof(object*) * cap_objects * 2);
		cap_objects *= 2;

		#if defined(DEBUG) && DEBUG == 1
		printf("[DEBUG] Resized object list to %d item(s) capacity.\n", cap_objects);
		#endif
	}

	/* add item to list */
	objects[n_of_objects++] = o;

	/* debug mode */
	#if defined(DEBUG) && DEBUG == 1
	printf("Successfully created an object (%p) of type %d.\n", o, type);
	#endif

	return o; /* last piece of the puzzle */
}

/* perform operation on object */
extern object *objectOperation(object *obj, object *other, unsigned int op_num) {

	/* must be same types */
	if (obj->type != other->type) {

		/* set an error */
		errorSet(ERROR_TYPE_RUNTIME, ERROR_CODE_ILLEGALOP,
				 "Illegal operation");
		errorSetPos(obj->lineno, obj->colno, obj->fname);

		/* return NULL */
		return NULL;
	}

	/* '+' */
	if (op_num == TOKEN_PLUS) {

		/* integer */
		if (obj->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val + O_INT(other)->val);
	}

	/* '-' */
	else if (op_num == TOKEN_MINUS) {

		/* integer */
		if (obj->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val - O_INT(other)->val);
	}

	/* '*' */
	else if (op_num == TOKEN_MUL) {

		/* integer */
		if (obj->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val * O_INT(other)->val);
	}

	/* '/' */
	else if (op_num == TOKEN_DIV) {

		/* integer */
		if (obj->type == OBJECT_INT)
			return intobjectNew(O_INT(obj)->val / O_INT(other)->val);
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

/* free an object */
extern void objectFree(object *obj) {

	/* free the object */
	free(obj);
}

extern void objectCollect() {
	/* unimplemented */
}

extern void objectFreeAll() {

	/* go through list and free all objects. */
	for (int i = 0; i < n_of_objects; i++) {
		objectFree(objects[i]);

		/* debug mode stuff */
		#if defined(DEBUG) && DEBUG == 1
		printf("[DEBUG] Freed %p.\n", objects[i]);
		#endif
	}

	/* free object list */
	free(objects);
}