#include "object.h"
#include "error.h" /* error handling */
#include "file.h" /* basic file handling system */

#include <stdlib.h> /* malloc/realloc/free, etc */
#include <string.h> /* strcpy, strlen, etc */
#include <stdio.h> /* sprintf, printf, etc */

/* list of all objects */
object **objects = NULL;
int n_of_objects; /* number of objects stored in list */
int cap_objects; /* capacity of object list */

/* create an object */
extern object *objectNew(unsigned char type) {
	
	object *o = (object*)malloc(sizeof(object)); /* new object! :D */

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
	o->value = (unsigned long)0; /* no value YET */
	
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

/* create an integer */
extern object *objectNewInt(int value) {

	/* get object */
	object *o = objectNew(OBJECT_INT);

	/* failed :( */
	if (o == NULL)
		return NULL;

	/* pun the value */
	o->value = (unsigned long)*(unsigned int *)&value;
	return o;
}

/* create string */
extern object *objectNewString(char *value) {

	/* get object */
	object *o = objectNew((OBJECT_CHR | OBJECT_ARRAY));

	/* failed :( */
	if (o == NULL)
		return NULL;

	/* malloc new string and copy contents */
	unsigned int value_len = strlen(value);
	char *dyn = (char *)malloc(value_len + 2);
	strcpy(dyn, value);

	/* store value and return */
	o->value = (unsigned long)dyn;
	return objectNewPointer(o);
}

/* create a char */
extern object *objectNewChar(char value) {

	/* get object */
	object *o = objectNew(OBJECT_CHR);

	/* failed :( */
	if (o == NULL)
		return NULL;

	/* pun the value */
	o->value = (unsigned long)*(unsigned char *)&value;
	return o;
}

/* create a pointer object */
extern object *objectNewPointer(object *vo) {

	/* get object */
	object *o = objectNew(POINTER(vo->type));

	/* failed :( */
	if (o == NULL)
		return NULL;

	/* pun the value */
	o->value = (unsigned long)vo;
	return o;
}

/* free an object */
extern void objectFree(object *obj) {

	/* free string if necessary */
	if (obj->type == OBJECT_STR)
		free((char *)(((object *)obj->value)->value));

	/* free the object */
	free(obj);
}

/* perform an operation on an object and return a new object */
extern object *objectOperation(object *obj, object *other, unsigned int op_num) {

	/* compare types */
	if (obj->type != other->type) {
		if (!errorIsSet()) {
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_INVALIDTYPE,
					 "Mismatched types");
			errorSetPos(obj->lineno, obj->colno, obj->fname);
		}

		return NULL;
	}

	/* '+' */
	if (op_num == OPERATION_PLUS) {
		/* integer addition */
		if (obj->type == OBJECT_INT)
			return objectNewInt((*(int*)obj->value) + (*(int*)other->value));
	}

	/* '-' */
	if (op_num == OPERATION_MINUS) {
		/* integer substraction */
		if (obj->type == OBJECT_INT)
			return objectNewInt((*(int*)obj->value) + (*(int*)other->value));
	}

	/* set error */
	if (!errorIsSet()) {
		errorSet(ERROR_TYPE_RUNTIME,
								ERROR_CODE_ILLEGALOP,
								"Illegal Operation");
		errorSetPos(obj->lineno, obj->colno, obj->fname);
	}

	/* not fully implemented :( */
	return NULL;
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

extern object *objectRepresent(object *obj) {

	/* string */
	if (obj->type == OBJECT_STR)
		return obj;

	/* int */
	else if (obj->type == OBJECT_INT) {
		char buf[100]; /* string buffer */
		sprintf(buf, "%d", *(int*)obj->value); /* result */
		return objectNewString(buf);
	}

	/* char */
	else if (obj->type == OBJECT_CHR) {
		char buf[2]; /* string buffer */
		buf[0] = *(char*)obj->value;
		buf[1] = '\0';
		return objectNewString(buf);
	}

	/* otherwise */
	return NULL;
}

extern void objectWrite(int fd, object *value) {

	/* can only write string value */
	if (value->type != OBJECT_STR) {

		/* set error */
		if (!errorIsSet()) {
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_ILLEGALOP,
					 "Mismatched types");
			errorSetPos(value->lineno, value->colno, value->fname);
		}

		return;
	}

	/* write to descriptor */
	fputs((char *)(((object *)value->value)->value), FILEGET(fd));

	/* debug */
	#if defined(DEBUG) && DEBUG == 1
	printf("[DEBUG] Successfully wrote to descriptor %d.\n", fd);
	#endif
}

extern void objectPrint(object *obj) {
	objectWrite(FD_CONSOLE, objectRepresent(obj));
}

/* for future: remove this warning after it is no longer needed */
#warning the function 'objectRead' is currently being rewritten and as such shall not be used in any code.

/* read text from file */
extern object *objectRead(int fd) {

	/* return fileReadFD */
	//return objectNewString(fileReadLine(fd));

	return NULL;
}
