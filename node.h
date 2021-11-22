/*
 *
 * Copyright 2021, Elliot Kohlmyer
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

/* node.h -- parser node system */
#ifndef _NODE_H
#define _NODE_H

#include "token.h"

/* node types */
#define NODE_INT			0 /* integer value */
#define NODE_STRING			1 /* string value */
#define NODE_VARACCESS		2 /* variable access */
#define NODE_VARASSIGN		3 /* variable assignment */
#define NODE_FUNCDEF		4 /* function definition */
#define NODE_FUNCDEC		5 /* function declaration */
#define NODE_CALL			6 /* function call */
#define NODE_BINOP			7 /* binary operation */
#define NODE_UNOP			8 /* unary operation */
#define NODE_IFNODE			9 /* if statement */
#define NODE_FORNODE		10 /* for loop */
#define NODE_WHILENODE		11 /* while loop */
#define NODE_ELSENODE		12 /* else block */
#define NODE_STATEMENTS		13 /* block of statements */
#define NODE_EXTERN			14 /* extern */
#define NODE_GETITEM		16 /* get an item in array */
#define NODE_SETITEM		17 /* set an item in array */
#define NODE_VARNEW			18 /* new variable with value specified */
#define NODE_VARUN			19 /* variable with no value specified */
#define NODE_INC			20 /* increment operator (++) */
#define NODE_DEC			21 /* decrement operator (--) */
#define NODE_RETURN			22 /* return a value from function */
#define NODE_INCLUDE		23 /* include a file into program */
#define NODE_TYPEDEF		24 /* typedef a type based on another type */
#define NODE_CONST			25 /* constant */
#define NODE_UNSIGNED		26 /* unsigned */
#define NODE_STRUCT			27 /* struct */

/* node struct */
typedef struct _node {
	unsigned int type; /* node type */
	struct _node **children; /* child nodes */
	token **tokens; /* tokens */
	int *values; /* extra values */
	unsigned int n_of_children; /* number of children */
	unsigned int n_of_tokens; /* number of tokens */
	unsigned int n_of_values; /* number of values */
	unsigned int cap_children; /* capacity of children */
	unsigned int cap_tokens; /* capacity of tokens */
	unsigned int cap_values; /* capacity of values */
	/* error stuff */
	unsigned int lineno;
	unsigned int colno;
	char *fname;
} node;

/* functions */
extern node *nodeNew(unsigned int type, unsigned int lineno, unsigned int colno, char *fname); /* create a new node */
extern void nodeAddChild(node *n, node *chd); /* add a child node */
extern void nodeAddToken(node *n, token *tok); /* add a token */
extern void nodeAddValue(node *n, int val); /* add an extra value */
extern void nodePrintTree(node *n); /* print the tree of a node */
extern void nodeFree(node *n); /* free a node and it's children */

/* macros */
#define NODETOKEN(n, i) (n->tokens[i])
#define NODECHILD(n, i) (n->children[i])
#define NODEVALUE(n, i) (n->values[i])

#endif /* _NODE_H */