#ifndef _RUN_H
#define _RUN_H

#include <stdlib.h> /* stdlib */

/* mango context */
typedef struct {
	char **fnames; /* file names */
	int flen; /* number of file names */
	int bc_mode; /* mode for bytecode */
} mango_ctx;

/* functions */
extern mango_ctx *mango_ctx_new(char **, int, int); /* create a new mango context */
extern void mango_ctx_free(mango_ctx *); /* free a mango context */
extern int mango_ctx_run(mango_ctx *); /* run from context */

extern int run(char *, int); /* run a single file */

/* run directly instead of creating context */
static inline int run_all(char **fnames, int flen, int bc_mode) {

	/* create context */
	mango_ctx *mc = mango_ctx_new(fnames, flen, bc_mode);

	/* failed to allocate */
	if (mc == NULL)
		return -1;

	/* run each file */
	int res = mango_ctx_run(mc);

	/* free context and return result */
	mango_ctx_free(mc);
	return res;
}

#endif /* _RUN_H */