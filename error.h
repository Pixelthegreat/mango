/* error.h - global error handling. */
#ifndef _ERROR_H
#define _ERROR_H

/* error types */
#define ERROR_TYPE_SYNTAX 0
#define ERROR_TYPE_RUNTIME 1
#define ERROR_TYPE_BYTECODE 2
#define ERROR_TYPE_INTERNAL 3

/* error codes */
/* interal errors start at 0 */
#define ERROR_CODE_UNIMPLEMENTED 0
/* lexer errors start at 100 */
#define ERROR_CODE_UNKNOWNCHAR 100
#define ERROR_CODE_EXPECTEDCHAR 101
/* parser errors start at 250 */
#define ERROR_CODE_EXPECTEDTOKEN 250
/* bytecode errors start at 300 */
#define ERROR_CODE_BYTECODEUNIMPL 300
/* interpreter errors start at 400 */
#define ERROR_CODE_ILLEGALOP 400 /* illegal operation */
#define ERROR_CODE_MEMORY 401 /* memory allocation failure */
#define ERROR_CODE_INVALIDTYPE 402 /* invalid type matching */
#define ERROR_CODE_UNKNOWNFD 403 /* unknown/invalid file descriptor */
#define ERROR_CODE_UNDEFINEDNAME 404 /* cannot get name from list of names */

/* functions */
extern void errorSet(unsigned int err_type, unsigned int err_code, const char *err_msg); /* set the error */
extern void errorSetPos(unsigned int lineno, unsigned int colno, char *fname); /* set the error pos */
extern unsigned int errorIsSet(); /* check if error is set */
extern void errorPrint(); /* print the error */
extern void errorClear(); /* clear the error */

#endif /* _ERROR_H */