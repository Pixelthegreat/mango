#include "parser.h" /* header */
#include "error.h" /* errors */
#include <stdlib.h> /* malloc, realloc, free */
#include <stdio.h> /* printf */

/* create a new parser */
parser *parserNew(token **tokens, unsigned int n_of_toks) {

	/* malloc new parser */
	parser *p = (parser *)malloc(sizeof(parser));

	/* :( */
	if (p == NULL)
		return NULL;

	/* token array */
	p->tokens = tokens;
	p->n_of_toks = n_of_toks;

	/* other values */
	p->current_token = NULL;
	p->pn = NULL;
	p->tok_index = 0;

	/* return parser */
	return p;
}

/* parse a node */
extern void parserParse(parser *p) {

	/* set first token up */
	p->tok_index = -1;
	parserAdvance(p);

	/* get statements */
	p->pn = parserStatements(p);
}

/* parse an expression */
extern node *parserExpr(parser *p) {

	return parserCompExpr(p); /* comparison */
}

/* parser comparison */
extern node *parserCompExpr(parser *p) {

	/* node */
	int types[6] = {TOKEN_EE,TOKEN_NE,TOKEN_LT,TOKEN_GT,TOKEN_LTE,TOKEN_GTE};
	return parserBinOp(p, parserArithExpr, types, 6);
}

/* arithmatic expression */
extern node *parserArithExpr(parser *p) {

	/* node */
	int types[2] = {TOKEN_PLUS,TOKEN_MINUS};
	return parserBinOp(p, parserTerm, types, 2);
}

/* parse a term */
extern node *parserTerm(parser *p) {

	int types[3] = {TOKEN_MUL,TOKEN_DIV,TOKEN_MOD};
	return parserBinOp(p, parserFactor, types, 3);
}

/* parse a factor */
extern node *parserFactor(parser *p) {

	/* current token */
	token *tok = p->current_token;

	/* +, - */
	int tps_1[2] = {TOKEN_PLUS,TOKEN_MINUS};

	if (is_int_in(tps_1, tok->t_type, 2)) {

		/* advance */
		parserAdvance(p);

		/* get factor */
		node *factor = parserFactor(p);

		if (factor == NULL || errorIsSet())
			return NULL;

		/* create new unary operation node */
		node *n = nodeNew(NODE_UNOP, tok->lineno, tok->colno, tok->fname);

		if (n == NULL) {

			nodeFree(factor); /* free original */
			return NULL;
		}

		/* add children and tokens */
		nodeAddChild(n, factor);
		nodeAddToken(n, tok);

		/* return node */
		return n;
	}

	/* integer */
	else if (tok->t_type == TOKEN_INT) {

		/* advance */
		parserAdvance(p);

		/* new node */
		node *n = nodeNew(NODE_INT, tok->lineno, tok->colno, tok->fname);

		/* add token and set error info */
		nodeAddToken(n, tok);

		/* return node */
		return n;
	}
	/* parenthesis expression */
	else if (tok->t_type == TOKEN_LPAREN) {

		/* advance */
		parserAdvance(p);

		/* get expression */
		node *expr = parserExpr(p);

		if (expr == NULL || errorIsSet())
			return NULL;

		/* expects ')' */
		if (p->current_token->t_type != TOKEN_RPAREN) {

			/* create error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ')'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free expression */
			nodeFree(expr);

			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* return expression */
		return expr;
	}
	/* string */
	else if (tok->t_type == TOKEN_STRING) {

		/* advance */
		parserAdvance(p);

		/* new node */
		node *n = nodeNew(NODE_STRING,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL)
			return NULL;

		/* add token */
		nodeAddToken(n, tok);

		/* return */
		return n;
	}
	/* error */
	else {

		errorSet(ERROR_TYPE_SYNTAX,
				 ERROR_CODE_EXPECTEDTOKEN,
				 "Invalid token");
		errorSetPos(tok->lineno,
					tok->colno,
					tok->fname);
		return NULL;
	}
}

/* binary operation */
extern node *parserBinOp(parser *p, pfunc func, int types[], unsigned int n_of_types) {

	/* get first node */
	node *left = func(p);

	/* new node stuff */
	node *n = NULL;
	node *right = NULL;

	/* error or something */
	if (left == NULL || errorIsSet())
		return NULL;

	/* operation token */
	token *op_token = NULL;

	/* while the token is in the list */
	while (is_int_in(types, p->current_token->t_type, n_of_types)) {

		/* get operation token */
		op_token = p->current_token;

		parserAdvance(p); /* advance */

		/* get right */
		right = func(p);

		/* error */
		if (right == NULL || errorIsSet()) {

			/* free left and right */
			if (n != NULL) nodeFree(n);
			if (right != NULL) nodeFree(right);

			/* return */
			return NULL;
		}

		/* allocate new */
		n = nodeNew(NODE_BINOP,
					left->lineno,
					left->colno,
					left->fname);

		/* add stuff */
		nodeAddChild(n, left);
		nodeAddChild(n, right);

		/* add op token */
		nodeAddToken(n, op_token);

		/* set left to new */
		left = n;
	}

	/* return node */
	return left;
}

/* get multiple statements */
extern node *parserStatements(parser *p) {

	/* new node */
	node *n = nodeNew(NODE_STATEMENTS,
					  p->current_token->lineno,
					  p->current_token->colno,
					  p->current_token->fname);

	/* ;( */
	if (n == NULL)
		return NULL;

	/* empty file */
	if (p->current_token->t_type == TOKEN_EOF)
		return n;

	/* get next statement */
	node *next = parserExpr(p);

	/* failed to retrieve node or error */
	if (next == NULL || errorIsSet()) {

		nodeFree(n); /* free nodes */
		return NULL;
	}

	/* add node */
	nodeAddChild(n, next);

	/* loop until token is not of type EOL */
	while (p->current_token->t_type == TOKEN_EOL) {

		/* advance */
		parserAdvance(p);

		/* EOF */
		if (p->current_token->t_type == TOKEN_EOF)
			break;

		/* advance past more EOLs */
		while (p->current_token->t_type == TOKEN_EOL)
			parserAdvance(p);

		/* get next statement */
		next = parserExpr(p);

		/* error */
		if (next == NULL || errorIsSet()) {

			nodeFree(n);
			return NULL;
		}

		/* add node */
		nodeAddChild(n, next);
	}

	/* return node */
	return n;
}

/* free parser */
extern void parserFree(parser *p) {

	/* free parser */
	free(p);
}

/* advance */
extern void parserAdvance(parser *p) {

	/* advance position */
	p->tok_index++;

	/* end */
	if (p->tok_index >= p->n_of_toks)
		p->current_token = NULL;
	/* otherwise */
	else
		p->current_token = p->tokens[p->tok_index];
}

/* check if int is in list */
extern int is_int_in(int values[], int val, unsigned int len) {

	for (int i = 0; i < len; i++)
		if (values[i] == val)
			return 1; /* value */

	return 0;
}