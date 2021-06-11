/* object handling */
#ifndef _OBJECT_H
#define _OBJECT_H

/* debug mode and other constants */
#define DEBUG 0

/* operation types */
#define OPERATION_PLUS 0
#define OPERATION_MINUS 1
#define OPERATION_MUL 2
#define OPERATION_DIV 3
#define OPERATION_MOD 4
#define OPERATION_EQ 5
#define OPERATION_NEQ 6
#define OPERATION_LT 7
#define OPERATION_GT 8
#define OPERATION_LTE 9
#define OPERATION_GTE 10

/* object types */
#define OBJECT_INT 0
#define OBJECT_STR 1
#define OBJECT_CHR 2

/* file descriptors */
#define FD_CONSOLE 0

/* object */
typedef struct {
	unsigned int refcnt; /* number of references */
	unsigned char type; /* type of object */
	void *value; /* value pointer */
	unsigned int lineno; /* for errors */
	unsigned int colno; /* for errors */
	char *fname; /* for errors */
} object;

/* functions */
extern object *objectNew(unsigned char type); /* create a new object and increase reference count by 1 */
extern object *objectNewInt(int value); /* create a new integer */
extern object *objectNewString(char *value); /* create a new string */
extern object *objectNewChar(char value); /* create a new char */
extern void objectFree(object *obj); /* free an object */
extern object *objectOperation(object *obj, object *other, unsigned int op_num); /* perform an operation (i.e., +, -, <, >, etc) on an object */
extern void objectCollect(); /* garbage collection routine */
extern void objectFreeAll(); /* free all objects */
extern object *objectRepresent(object *obj); /* represent an object */
extern void objectWrite(int fd, object *value); /* write a string value to a file descriptor */
extern void objectPrint(object *obj); /* wrapper for "write(FD_CONSOLE, represent(value));" */

/* macros (inspired by python's stellar object
allocation system it has :D) */
#define INCREF(o) (o->refcnt++)
#define DECREF(o) (o->refcnt--)
#define XINCREF(o) if (o != NULL) (o->refcnt++)
#define XDECREF(o) if (o != NULL) (o->refcnt--)

#endif /* _OBJECT_H */