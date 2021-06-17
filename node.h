/* node.h -- parser node system */
#ifndef _NODE_H
#define _NODE_H

#include "token.h"

/* node types */
#define NODE_INT			0
#define NODE_STRING			1
#define NODE_VARACCESS		2
#define NODE_VARASSIGN		3
#define NODE_FUNCDEF		4
#define NODE_FUNCDEC		5
#define NODE_CALL			6
#define NODE_BINOP			7
#define NODE_UNOP			8
#define NODE_IFNODE			9
#define NODE_FORNODE		10
#define NODE_WHILENODE		11
#define NODE_ELSENODE		12
#define NODE_STATEMENTS		13
#define NODE_EXTERN			14

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