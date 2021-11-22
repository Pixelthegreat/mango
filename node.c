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
	/* include */
	else if (n->type == NODE_INCLUDE) {

		printf("include \'%s\'", n->tokens[0]->t_value);
	}
	/* for node */
	else if (n->type == NODE_FORNODE) {

		printf("for (");
		nodePrintTree(n->children[0]);
		printf("; ");
		nodePrintTree(n->children[1]);
		printf("; ");
		nodePrintTree(n->children[2]);
		printf(") -> [\n");

		for (unsigned int i = 3; i < n->n_of_children; i++) {

			nodePrintTree(n->children[i]);
			printf(";\n");
		}
		printf("]");
	}
	/* variable access */
	else if (n->type == NODE_VARACCESS) {

		/* ( */
		printf("(");

		/* loop through tokens */
		for (unsigned int i = 0; i < n->n_of_tokens; i++) {

			/* token */
			printf("%s", n->tokens[i]->t_value);

			/* print '->' */
			if (i < n->n_of_tokens - 1)
				printf("->");
		}

		/* ) */
		printf(")");
	}
	/* variable assignment */
	else if (n->type == NODE_VARNEW) {

		/* pointer */
		int is_p = n->values[1];

		/* not array */
		if (n->values[0] == 0) {

			if (!is_p) printf("%s %s = ", n->tokens[0]->t_value, n->tokens[1]->t_value);
			else printf("%s *%s = ", n->tokens[0]->t_value, n->tokens[1]->t_value);
			nodePrintTree(n->children[0]);
		}
		/* array */
		else {

			if (!is_p) printf("%s %s[", n->tokens[0]->t_value, n->tokens[1]->t_value);
			else printf("%s *%s[", n->tokens[0]->t_value, n->tokens[1]->t_value);
			nodePrintTree(n->children[0]);
			printf("] = ");
			nodePrintTree(n->children[1]);
		}
	}
	/* also */
	else if (n->type == NODE_VARUN) {

		/* is pointer */
		int is_p = n->values[1];

		/* not array */
		if (n->values[0] == 0) {

			if (!is_p) printf("%s %s", n->tokens[0]->t_value, n->tokens[1]->t_value);
			else printf("%s *%s", n->tokens[0]->t_value, n->tokens[1]->t_value);
		}
		/* array */
		else {

			if (!is_p) printf("%s %s[", n->tokens[0]->t_value, n->tokens[1]->t_value);
			else printf("%s *%s[", n->tokens[0]->t_value, n->tokens[1]->t_value);
			nodePrintTree(n->children[0]);
			printf("]");
		}
	}
	/* function definition */
	else if (n->type == NODE_FUNCDEF) {

		/* print original */
		printf("(");
		nodePrintTree(n->children[0]);
		printf(" -> [\n");

		for (unsigned int i = 1; i < n->n_of_children; i++) {

			/* print node */
			nodePrintTree(n->children[i]);
			printf(";\n");
		}

		/* print end */
		printf("])");
	}
	/* function declaration */
	else if (n->type == NODE_FUNCDEC) {

		/* is pointer */
		int is_p = n->values[0];
		unsigned int idx = 1;

		/* print stuff */
		if (!is_p) printf("fun %s %s(", n->tokens[0]->t_value, n->tokens[1]->t_value);
		else printf("fun %s *%s(", n->tokens[0]->t_value, n->tokens[1]->t_value);

		for (unsigned int i = 2; i < n->n_of_tokens; i++) {

			/* is pointer */
			is_p = n->values[idx++];

			/* print token */
			if (!is_p) printf("%s %s", n->tokens[i]->t_value, n->tokens[i+1]->t_value);
			else printf("%s *%s", n->tokens[i]->t_value, n->tokens[i+1]->t_value);

			/* print ', ' if not at end of list */
			if (i < n->n_of_tokens-2)
				printf(", ");

			i++;
		}

		/* ) */
		printf(")");
	}
	/* function call */
	else if (n->type == NODE_CALL) {

		nodePrintTree(n->children[0]);

		printf("(");

		for (int i = 1; i < n->n_of_children; i++) {

			nodePrintTree(n->children[i]);
			if (i < n->n_of_children-1) printf(", ");
		}

		printf(")");
	}
	/* binary operation */
	else if (n->type == NODE_BINOP) {

		printf("(");

		nodePrintTree(n->children[0]);
		printf(" %s ", n->tokens[0]->t_value);
		nodePrintTree(n->children[1]);

		printf(")");
	}
	/* unary operation */
	else if (n->type == NODE_UNOP) {

		printf("(%s", n->tokens[0]->t_value);
		nodePrintTree(n->children[0]);
		printf(")");
	}
	/* if statment */
	else if (n->type == NODE_IFNODE) {

		printf("if (");
		nodePrintTree(n->children[0]);
		printf(") -> [\n");

		for (unsigned int i = 1; i < n->n_of_children; i++) {

			nodePrintTree(n->children[i]);
			printf(";\n");
		}
		printf("]");
	}
	/* struct */
	else if (n->type == NODE_STRUCT) {

		printf("struct %s -> [\n", n->tokens[0]->t_value);

		for (unsigned int i = 0; i < n->n_of_children; i++) {

			nodePrintTree(n->children[i]);
			printf(";\n");
		}
		printf("]");
	}
	/* for loop */
	/* while loop */
	else if (n->type == NODE_WHILENODE) {

		printf("while (");
		nodePrintTree(n->children[0]);
		printf(") -> [\n");

		for (unsigned int i = 1; i < n->n_of_children; i++) {

			nodePrintTree(n->children[i]);
			printf(";\n");
		}
		printf("]");
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
			printf(";\n");
		}

		printf("}\n");
	}
	/* extern */
	else if (n->type == NODE_EXTERN) {

		printf("extern ");
		nodePrintTree(n->children[0]);
	}
	/* return */
	else if (n->type == NODE_RETURN) {

		printf("return ");
		nodePrintTree(n->children[0]);
	}
	/* unsigned */
	else if (n->type == NODE_UNSIGNED) {
		
		printf("unsigned ");
		nodePrintTree(n->children[0]);
	}
	/* const */
	else if (n->type == NODE_CONST) {
		
		printf("const ");
		nodePrintTree(n->children[0]);
	}
	/* variable assigment */
	else if (n->type == NODE_VARASSIGN) {

		/* ( */
		printf("(");

		/* loop through tokens */
		for (unsigned int i = 0; i < n->n_of_tokens; i++) {

			/* token */
			printf("%s", n->tokens[i]->t_value);

			/* print '->' */
			if (i < n->n_of_tokens - 1)
				printf("->");
		}

		/* value */
		printf(" = ");

		nodePrintTree(n->children[0]);

		/* ) */
		printf(")");
	}
	/* getitem */
	else if (n->type == NODE_GETITEM) {

		/* nodes */
		printf("(");

		for (unsigned int i = 0; i < n->n_of_tokens; i++) {

			/* token */
			printf("%s", n->tokens[i]->t_value);

			/* print '->' */
			if (i < n->n_of_tokens - 1)
				printf("->");
		}

		printf("[");
		nodePrintTree(n->children[0]);
		printf("])");
	}
	/* setitem */
	else if (n->type == NODE_SETITEM) {

		/* nodes */
		printf("(");

		for (unsigned int i = 0; i < n->n_of_tokens; i++) {

			/* token */
			printf("%s", n->tokens[i]->t_value);

			/* print '->' */
			if (i < n->n_of_tokens - 1)
				printf("->");
		}

		printf("[");
		nodePrintTree(n->children[0]);
		printf("] = ");

		/* value */
		nodePrintTree(n->children[1]);
		printf(")");
	}
	else if (n->type == NODE_INC) {

		/* nodes */
		printf("(");

		for (unsigned int i = 0; i < n->n_of_tokens; i++) {

			/* token */
			printf("%s", n->tokens[i]->t_value);

			/* print '->' */
			if (i < n->n_of_tokens - 1)
				printf("->");
		}

		printf("++)");
	}
	else if (n->type == NODE_DEC) {

		/* nodes */
		printf("(");

		for (unsigned int i = 0; i < n->n_of_tokens; i++) {

			/* token */
			printf("%s", n->tokens[i]->t_value);

			/* print '->' */
			if (i < n->n_of_tokens - 1)
				printf("->");
		}

		printf("--)");
	}
	/* typedef */
	else if (n->type == NODE_TYPEDEF) {

		int is_p = n->values[0]; /* is a pointer */

		if (!is_p) printf("typedef %s %s", n->tokens[0]->t_value, n->tokens[1]->t_value);
		else printf("typedef %s *%s", n->tokens[0]->t_value, n->tokens[1]->t_value);
	}
}

/* free a node and it's children */
extern void nodeFree(node *n) {

	/* free children */
	for (int i = 0; i < n->n_of_children; i++)
		if (n->children[i] != NULL) nodeFree(n->children[i]);

	/* free lists */
	free(n->children);
	free(n->tokens);
	free(n->values);

	/* free node */
	free(n);
}