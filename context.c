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

/* context system */
#include "context.h"
#include <stdlib.h>
#include <stdio.h>

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
	ctx->tp = CONTEXT_MAIN;
	ctx->rt = NULL;

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