/* context system */
#include "context.h"
#include <stdlib.h>

/* create a context */
extern context *contextNew(char *fn, char *sn) {

	/* create new context */
	context *ctx = (context *)malloc(sizeof(context));

	/* :( */
	if (ctx == NULL)
		return NULL;

	/* set values */
	ctx->fn = fn;
	ctx->sn = sn;

	/* create a nameTable */
	nameTable *nt = namesNew();

	/* unable to be created */
	if (nt == NULL) {

		free(ctx);
		return NULL;
	}

	/* return context */
	ctx->nt = nt;

	return ctx;
}

/* free context */
extern void contextFree(context *ctx) {

	/* free name table */
	namesFree(ctx->nt);

	/* free context */
	free(ctx);
}