#include "node.h" /* header */
#include <stdlib.h> /* malloc, realloc, free */
#include <stdio.h> /* printf */

/* create a node */
extern node *nodeNew(unsigned int type,
					 unsigned int lineno,
					 unsigned int colno,
					 char *fname) {

	/* malloc a new node */
	node *n = (node *)malloc(sizeof(node));

	/* ;( */
	if (n == NULL)
		return NULL;

	/* values */
	n->type = type;
	n->n_of_children = 0;
	n->n_of_tokens = 0;
	n->n_of_values = 0;
	n->cap_children = 8;
	n->cap_tokens = 8;
	n->cap_values = 8;
	n->lineno = lineno;
	n->colno = colno;
	n->fname = fname;

	/* lists */
	n->children = (node **)malloc(sizeof(node *) * 8);
	n->tokens = (token **)malloc(sizeof(token *) * 8);
	n->values = (int *)malloc(sizeof(int) * 8);

	/* return */
	return n;
}

/* add a child node */
extern void nodeAddChild(node *n, node *chd) {

	/* realloc list if we need to */
	if (n->n_of_children >= n->cap_children) {

		n->cap_children *= 2;
		n->children = (node **)realloc(n->children, sizeof(node *) * n->cap_children);
	}

	/* add child */
	n->children[n->n_of_children++] = chd;
}

/* add a token */
extern void nodeAddToken(node *n, token *tok) {

	/* realloc list if we need to */
	if (n->n_of_tokens >= n->cap_tokens) {

		n->cap_tokens *= 2;
		n->tokens = (token **)realloc(n->tokens, sizeof(token *) * n->cap_tokens);
	}

	/* add token */
	n->tokens[n->n_of_tokens++] = tok;
}

/* add a value */
extern void nodeAddValue(node *n, int val) {

	/* realloc list if we need to */
	if (n->n_of_values >= n->cap_values) {

		n->cap_values *= 2;
		n->values = (int *)realloc(n->values, sizeof(int) * n->cap_values);
	}

	/* add value */
	n->values[n->n_of_values++] = val;
}

/* print the tree of a node */
extern void nodePrintTree(node *n) {

	/* int */
	if (n->type == NODE_INT) {

		printf("%d", atoi(n->tokens[0]->t_value));
	}
	/* string */
	else if (n->type == NODE_STRING) {

		printf("\'%s\'", n->tokens[0]->t_value);
	}
	/* variable access */
	else if (n->type == NODE_VARACCESS) {

		printf("%s", n->tokens[0]->t_value);
	}
	/* variable assignment */
	else if (n->type == NODE_VARASSIGN) {

		printf("%s %s = ", n->tokens[0]->t_value,
						   n->tokens[1]->t_value);
		nodePrintTree(n->children[0]);
	}
	/* function definition */
	else if (n->type == NODE_FUNCDEF) {

		//
	}
	/* function declaration */
	else if (n->type == NODE_FUNCDEC) {

		//
	}
	/* function call */
	else if (n->type == NODE_CALL) {

		printf("%s(");

		for (int i = 0; i < n->n_of_children; i++) {

			nodePrintTree(n->children[i]);
			printf(", ");
		}

		printf(")");
	}
	/* binary operation */
	else if (n->type == NODE_BINOP) {

		printf("(");

		nodePrintTree(n->children[0]);
		printf(" %d ", n->tokens[0]->t_type);
		nodePrintTree(n->children[1]);

		printf(")");
	}
	/* unary operation */
	else if (n->type == NODE_UNOP) {

		printf("(%d ", n->tokens[0]->t_type);
		nodePrintTree(n->children[0]);
		printf(")");
	}
	/* if statement */
	else if (n->type == NODE_IFNODE) {

		printf("if (");
		nodePrintTree(n->children[0]);
		printf(") ");
		nodePrintTree(n->children[1]);
	}
	/* for loop */
	/* while loop */
	else if (n->type == NODE_WHILENODE) {

		printf("while (");
		nodePrintTree(n->children[0]);
		printf(") ");
		nodePrintTree(n->children[1]);
	}
	/* else node */
	else if (n->type == NODE_ELSENODE) {

		printf("else ");
		nodePrintTree(n->children[0]);
	}
	/* statements */
	else if (n->type == NODE_STATEMENTS) {

		printf("{\n");

		for (int i = 0; i < n->n_of_children; i++) {

			nodePrintTree(n->children[i]);
			/* newline */
			printf("\n");
		}

		printf("}\n");
	}
	/* extern */
	else if (n->type == NODE_EXTERN) {

		printf("extern ");
		nodePrintTree(n->children[0]);
		printf("\n");
	}
}

/* free a node and it's children */
extern void nodeFree(node *n) {

	/* free children */
	for (int i = 0; i < n->n_of_children; i++)
		nodeFree(n->children[i]);

	/* free lists */
	free(n->children);
	free(n->tokens);
	free(n->values);

	/* free node */
	free(n);
}