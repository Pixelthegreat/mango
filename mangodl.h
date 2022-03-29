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

#ifndef _MANGODL_H
#define _MANGODL_H

#include "obhead.h" /* for objects */
#include "names.h"
#include "mangodldefs.h"

/* dl object */
typedef struct {
	OB_HEAD
	void *dl; /* pointer to dynamic loader stuff */
	int op; /* is open */
	char *n; /* name */
	nameTable *nt; /* nametable that mangodl can use */
	/* functions */
	mangodl_errorset_t *errorset;
	mangodl_errorisset_t *errorisset;
	mangodl_errorsetpos_t *errorsetpos;
	mangodl_intobjectnew_t *intobjectnew;
	mangodl_charobjectnew_t *charobjectnew;
	mangodl_arrayobjectnew_t *arrayobjectnew;
	mangodl_pointerobjectnew_t *pointerobjectnew;
	mangodl_namesset_t *namesset;
	mangodl_namesget_t *namesget;
	mangodl_functionbuiltinnew_t *functionbuiltinnew;
} mangodlobject;

/* cast to a dl */
#define O_DL(o) ((mangodlobject *)(o))

/* functions */
extern mangodlobject *mangodlobjectNew(void); /* create a new mangodlobject */
extern int mangodlOpen(object *s, char *name); /* load a dynamic library */
extern mangodlobject *mangodlGet(object *s); /* get a mangodlobject from a name */
extern object *mangodlSym(mangodlobject *d, object *s); /* query for a function or other data */
extern int mangodlClose(mangodlobject *d); /* close a mangodlobject */
extern void mangodlCloseAll(); /* close all mango dls if they are open */

#endif /* _MANGODL_H */