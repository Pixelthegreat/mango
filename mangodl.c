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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <dlfcn.h>
#include "mangodl.h"
#include "object.h"
#include "error.h"
#include "pointerobject.h"
#include "arrayobject.h"
#include "intobject.h"
#include "names.h"
#include "context.h"
#include "functionobject.h"

static mangodlobject **mangodls = NULL; /* dynamic libraries */
static int mangodls_len = 0;
static int mangodls_cap = 0;

extern context *vmdctx; /* primary vm context */

/* create a new dl object */
extern mangodlobject *mangodlobjectNew(void) {

	/* create object */
	mangodlobject *d = (mangodlobject *)objectNew(OBJECT_DL, sizeof(mangodlobject));

	/* set value(s) */
	d->dl = NULL;
	d->op = 0;
	d->n = NULL;
	d->nt = NULL;

	/* create list if not created */
	if (mangodls == NULL) {

		mangodls_cap = 8;
		mangodls = (mangodlobject **)malloc(sizeof(mangodlobject *) * mangodls_cap);
	}

	/* resize list */
	if (mangodls_len >= mangodls_cap) {

		mangodls_cap *= 2;
		mangodls = (mangodlobject **)realloc(mangodls, sizeof(mangodlobject *) * mangodls_cap);
	}

	/* add item */
	mangodls[mangodls_len++] = d;

	/* return object */
	return d;
}

/* open a dynamic library */
extern int mangodlOpen(object *s, char *name) {

	/* try to check if file exists */
	struct stat st;
	if (stat(name, &st) < 0) {

		/* set error */
		errorSet(ERROR_TYPE_RUNTIME,
				 ERROR_CODE_NOFILE,
				 strerror(errno));
		errorSetPos(s->lineno, s->colno, s->fname);
		return -1;
	}

	/* create dl object */
	mangodlobject *d = mangodlobjectNew();
	d->n = name;
	d->op = 1;

	XINCREF(d);

	/* try to open dynamic lib */
	d->dl = dlopen(name, RTLD_NOW);

	/* unable to open */
	if (d->dl == NULL) {

		errorSet(ERROR_TYPE_RUNTIME,
				 ERROR_CODE_NOFILE,
				 dlerror());
		errorSetPos(s->lineno, s->colno, s->fname);
		d->op = 0;
		return -1;
	}

	/* try to find init symbols (errorSet, errorSetPos, errorIsSet, etc ...) */
	char *names[] = {"errorSet", "errorSetPos", "errorIsSet", "intobjectNew", "charobjectNew", "arrayobjectNew", "pointerobjectNew", "namesSet", "namesGet", "functionBuiltinNew"};

	for (int i = 0; i < sizeof(names) / sizeof(names[0]); i++) {

		/* locate symbol */
		void *sym;
		if ((sym = dlsym(d->dl, names[i])) == NULL) {

			/* set error */
			errorSet(ERROR_TYPE_RUNTIME,
					 ERROR_CODE_NOFILE,
					 "Failed to locate symbol");
			errorSetPos(s->lineno, s->colno, s->fname);

			/* close lib */
			dlclose(d->dl);
			d->op = 0;
			return -1;
		}

		/* errorSet */
		if (i == 0) {
			d->errorset = sym;
			*d->errorset = errorSet;
		}
		/* errorSetPos */
		else if (i == 1) {
			d->errorsetpos = sym;
			*d->errorsetpos = errorSetPos;
		}
		/* errorIsSet */
		else if (i == 2) {
			d->errorisset = sym;
			*d->errorisset = errorIsSet;
		}
		/* intobjectNew */
		else if (i == 3) {
			d->intobjectnew = sym;
			*d->intobjectnew = intobjectNew;
		}
		/* charobjectNew */
		else if (i == 4) {
			d->charobjectnew = sym;
			*d->charobjectnew = charobjectNew;
		}
		/* arrayobjectNew */
		else if (i == 5) {
			d->arrayobjectnew = sym;
			*d->arrayobjectnew = arrayobjectNew;
		}
		/* pointerobjectNew */
		else if (i == 6) {
			d->pointerobjectnew = sym;
			*d->pointerobjectnew = pointerobjectNew;
		}
		/* namesSet */
		else if (i == 7) {
			d->namesset = sym;
			*d->namesset = namesSet;
		}
		/* namesGet */
		else if (i == 8) {
			d->namesget = sym;
			*d->namesget = namesGet;
		}
		/* functionBuiltinNew */
		else if (i == 9) {
			d->functionbuiltinnew = sym;
			*d->functionbuiltinnew = functionobjectBuiltinNew;
		}
	}

	/* try to find init function */
	mangodl_init_t initfn = dlsym(d->dl, "mangodl_init");

	/* unable to find init function */
	if (initfn == NULL) {

		errorSet(ERROR_TYPE_RUNTIME,
				 ERROR_CODE_NOFILE,
				 "Failed to find 'mangodl_init' function");
		errorSetPos(s->lineno, s->colno, s->fname);

		/* close lib */
		dlclose(d->dl);
		d->op = 0;
		return -1;
	}

	/* create name table */
	d->nt = namesNew();

	/* run function */
	if (initfn(d->nt) < 0) return -1;

	/* return index */
	return mangodls_len - 1;
}

/* close dl object */
extern int mangodlClose(mangodlobject *d) {

	/* already closed */
	if (!d->op)
		return -1;

	/* otherwise */
	d->op = 0;
	dlclose(d->dl);

	return 0;
}

/* close all open dls */
extern void mangodlCloseAll() {

	/* none open */
	if (mangodls == NULL)
		return;

	for (int i = 0; i < mangodls_len; i++)
		if (mangodls[i]->op) mangodlClose(mangodls[i]);

	/* free list */
	free(mangodls);
}

/* get a mangodlobject */
extern mangodlobject *mangodlGet(object *s) {

	/* check type */
	if (s->type != OBJECT_INT) {

		errorSet(ERROR_TYPE_RUNTIME,
				 ERROR_CODE_INVALIDTYPE,
				 "Index must be integer");
		errorSetPos(s->lineno, s->colno, s->fname);
		return NULL;
	}

	/* check value */
	else if (O_INT(s)->val < 0 || O_INT(s)->val >= mangodls_len) {

		errorSet(ERROR_TYPE_RUNTIME,
				 ERROR_CODE_INVALIDVALUE,
				 "Invalid index");
		errorSetPos(s->lineno, s->colno, s->fname);
		return NULL;
	}

	/* return */
	return mangodls[O_INT(s)->val];
}

/* try and get a symbol from a dl object */
extern object *mangodlSym(mangodlobject *d, object *s) {

	/* check type */
	if (s->type != (OBJECT_CHR | OBJECT_POINTER) || O_OBJ(O_PTR(s)->val)->type != (OBJECT_CHR | OBJECT_ARRAY)) {

		errorSet(ERROR_TYPE_RUNTIME,
				 ERROR_CODE_INVALIDTYPE,
				 "Symbol name must be a string");
		errorSetPos(s->lineno, s->colno, s->fname);
		return NULL;
	}

	/* get name */
	object *o = namesGetN(d->nt, O_ARRAY(O_PTR(s)->val)->n_start, 0);

	/* no object found */
	if (o == NULL) {

		errorSet(ERROR_TYPE_RUNTIME,
				 ERROR_CODE_UNDEFINEDNAME,
				 "Failed to locate symbol");
		errorSetPos(s->lineno, s->colno, s->fname);
		return NULL;
	}

	/* return object */
	return o;
}