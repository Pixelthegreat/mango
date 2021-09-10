#include "parser.h" /* header */
#include "error.h" /* errors */
#include <stdlib.h> /* malloc, realloc, free */
#include <stdio.h> /* printf */
#include <string.h> /* strcmp */

/*
 * parser flags
 *
 * as a reminder, parser flags will help solve issues such as 'x * y' becoming 'x *y;'
 * as well as other things (hopefully not though)
 */
unsigned int pflag_include_vardec = 1;

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
	return parserBinOp(p, parserFuncDef, types, 3);
}

/* parse a factor */
extern node *parserFactor(parser *p) {

	/* current token */
	token *tok = p->current_token;

	/* +, - */
	int tps_1[6] = {TOKEN_PLUS,TOKEN_MINUS,TOKEN_MUL,TOKEN_AMP,TOKEN_INC,TOKEN_DEC};

	if (is_int_in(tps_1, tok->t_type, 6)) {

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
		unsigned int _pflag_include_vardec = pflag_include_vardec;
		pflag_include_vardec = 0;
		node *expr = parserExpr(p);
		pflag_include_vardec = _pflag_include_vardec;

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
	/* identifier */
	else if (tok->t_type == TOKEN_IDENT) {

		/* advance */
		parserAdvance(p);

		/* new node */
		node *n = nodeNew(NODE_VARACCESS,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL)
			return NULL;

		/* add token */
		nodeAddToken(n, tok);

		/* new variable */
		if ((p->current_token->t_type == TOKEN_IDENT ||
			 p->current_token->t_type == TOKEN_MUL) && pflag_include_vardec) {

			int is_pointer = 0; /* is pointer */

			/* pointer */
			if (p->current_token->t_type == TOKEN_MUL) {

				/* advance */
				parserAdvance(p);

				is_pointer = 1;
			}

			/* keep token and advance */
			token *t = p->current_token;
			parserAdvance(p);

			nodeAddToken(n, t); /* add tok */

			int is_array = 0; /* is array */

			/* pointer/pointer array */
			if (p->current_token->t_type == TOKEN_MUL) {

				/* advance */
				parserAdvance(p);

				is_pointer = 1;
			}

			/* array */
			if (p->current_token->t_type == TOKEN_OPENBRACKET) {

				/* advance */
				parserAdvance(p);

				/* get array size */
				node *arr_sz = parserExpr(p);

				/* error */
				if (arr_sz == NULL || errorIsSet()) {

					/* free and exit */
					nodeFree(n);
					return NULL;
				}

				/* add node */
				nodeAddChild(n, arr_sz);

				/* expects ']' */
				if (p->current_token->t_type != TOKEN_CLOSEBRACKET) {

					/* set error */
					errorSet(ERROR_TYPE_SYNTAX,
							 ERROR_CODE_EXPECTEDTOKEN,
							 "Expected ']'");
					errorSetPos(p->current_token->lineno,
								p->current_token->colno,
								p->current_token->fname);

					/* free node */
					nodeFree(n);

					return NULL; /* exit */
				}

				/* advance */
				parserAdvance(p);

				/* add values */
				is_array = 1;
			}

			/* add value */
			nodeAddValue(n, is_array);
			nodeAddValue(n, is_pointer);

			/* no '=' means undefined value */
			if (p->current_token->t_type != TOKEN_EQ) {

				/* create error */
				if (p->current_token->t_type != TOKEN_EOL) {

					/* set error */
					errorSet(ERROR_TYPE_SYNTAX,
							 ERROR_CODE_EXPECTEDTOKEN,
							 "Expected ';'");
					errorSetPos(p->current_token->lineno,
								p->current_token->colno,
								p->current_token->fname);

					/* free node */
					nodeFree(n);

					return NULL; /* exit */
				}

				/* change type */
				n->type = NODE_VARUN;

				/* return node */
				return n;
			}

			/* change type and advance */
			n->type = NODE_VARNEW;
			parserAdvance(p);

			/* get expression for value */
			unsigned int _pflag_include_vardec = pflag_include_vardec;
			pflag_include_vardec = 0;
			node *expr = parserExpr(p);
			pflag_include_vardec = _pflag_include_vardec;

			/* error */
			if (expr == NULL || errorIsSet()) {

				/* free and return */
				nodeFree(n);
				return NULL;
			}

			/* add node */
			nodeAddChild(n, expr);

			/* return node */
			return n;
		}

		/* '->' */
		while (p->current_token->t_type == TOKEN_ARROW) {

			/* advance */
			parserAdvance(p);

			/* expected identifier */
			if (p->current_token->t_type != TOKEN_IDENT) {

				nodeFree(n); /* free node */

				/* set error and return */
				errorSet(ERROR_TYPE_SYNTAX,
						 ERROR_CODE_EXPECTEDTOKEN,
						 "Expected identifier");
				errorSetPos(p->current_token->lineno,
							p->current_token->colno,
							p->current_token->fname);

				return NULL;
			}

			/* add token */
			nodeAddToken(n, p->current_token);

			parserAdvance(p); /* advance */
		}

		/* getitem, setitem */
		if (p->current_token->t_type == TOKEN_OPENBRACKET) {

			/* advance */
			parserAdvance(p);

			/* get expr */
			n->type = NODE_GETITEM;

			node *expr = parserExpr(p);

			/* error */
			if (expr == NULL || errorIsSet()) {

				/* free and exit */
				nodeFree(n);
				return NULL;
			}

			/* add node */
			nodeAddChild(n, expr);

			/* expects ']' */
			if (p->current_token->t_type != TOKEN_CLOSEBRACKET) {

				/* set error */
				errorSet(ERROR_TYPE_SYNTAX,
						 ERROR_CODE_EXPECTEDTOKEN,
						 "Expected ']'");
				errorSetPos(p->current_token->lineno,
							p->current_token->colno,
							p->current_token->fname);

				/* free node and exit */
				nodeFree(n);
				return NULL;
			}

			/* advance */
			parserAdvance(p);

			/* '=' setitem */
			if (p->current_token->t_type == TOKEN_EQ) {

				/* advance */
				parserAdvance(p);

				/* get expression */
				n->type = NODE_SETITEM;

				unsigned int _pflag_include_vardec = pflag_include_vardec;
				pflag_include_vardec = 0;
				node *val = parserExpr(p);
				pflag_include_vardec = _pflag_include_vardec;

				/* error */
				if (val == NULL || errorIsSet()) {

					/* free node */
					nodeFree(n);
					return NULL; /* exit */
				}

				/* add node */
				nodeAddChild(n, val);

				/* return node */
				return n;
			}

			/* return node */
			return n;
		}

		/* variable assignment */
		if (p->current_token->t_type == TOKEN_EQ) {

			parserAdvance(p); /* advance */
			
			/* change type and get expression */
			n->type = NODE_VARASSIGN;

			unsigned int _pflag_include_vardec = pflag_include_vardec;
			pflag_include_vardec = 0;
			node *expr = parserExpr(p);
			pflag_include_vardec = _pflag_include_vardec;

			/* error */
			if (expr == NULL || errorIsSet()) {

				/* free and return */
				nodeFree(n);
				return NULL;
			}

			/* add node */
			nodeAddChild(n, expr);

			/* return node */
			return n;
		}

		/* increment operator */
		else if (p->current_token->t_type == TOKEN_INC) {

			/* advance */
			parserAdvance(p);

			n->type = NODE_INC;

			/* return node */
			return n;
		}

		/* decrement operator */
		else if (p->current_token->t_type == TOKEN_DEC) {

			/* advance */
			parserAdvance(p);

			n->type = NODE_DEC;

			/* return node */
			return n;
		}

		/* return node */
		return n;
	}
	/* keyword */
	else if (p->current_token->t_type == TOKEN_KEYWORD) {

		/* return keyword */
		return parserKeyword(p);
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

/* keyword */
extern node *parserKeyword(parser *p) {

	/* keep token */
	token *tok = p->current_token;
	parserAdvance(p);

	/* struct */
	if (tokenMatches(tok, TOKEN_KEYWORD, "struct")) {

		/* get name of struct */
		if (p->current_token->t_type != TOKEN_IDENT) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected struct name");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		token *struct_name = p->current_token;
		parserAdvance(p); /* advance */

		/* '->' */
		if (p->current_token->t_type != TOKEN_ARROW) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '->'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		parserAdvance(p); /* advance */

		/* create new node */
		node *n = nodeNew(NODE_STRUCT,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL)
			return NULL;

		/* add token */
		nodeAddToken(n, struct_name);

		/* expects '[' */
		if (p->current_token->t_type != TOKEN_OPENBRACKET) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '['");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* get next expr */
		node *next = parserExpr(p);

		/* error */
		if (next == NULL || errorIsSet()) {

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* add node */
		nodeAddChild(n, next);

		/* ';' */
		while (p->current_token->t_type == TOKEN_EOL) {

			/* advance past EOL */
			while (p->current_token->t_type == TOKEN_EOL)
				parserAdvance(p);

			/* ']' */
			if (p->current_token->t_type == TOKEN_CLOSEBRACKET) {

				/* break */
				break;
			}

			/* get next node */
			next = parserExpr(p);

			/* error */
			if (next == NULL || errorIsSet()) {

				/* free and return */
				nodeFree(n);
				return NULL;
			}

			/* add node */
			nodeAddChild(n, next);
		}

		/* expects ']' */
		if (p->current_token->t_type != TOKEN_CLOSEBRACKET) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ']'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		parserAdvance(p); /* advance */

		return n; /* return node */
	}

	/* unsigned */
	else if (tokenMatches(tok, TOKEN_KEYWORD, "unsigned")) {

		/* get expr */
		node *expr = parserExpr(p);

		/* error */
		if (expr == NULL || errorIsSet())
			return NULL;

		/* new node */
		node *n = nodeNew(NODE_UNSIGNED,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		nodeAddChild(n, expr);

		/* return node */
		return n;
	}

	/* const */
	else if (tokenMatches(tok, TOKEN_KEYWORD, "const")) {

		/* get expr */
		node *expr = parserExpr(p);

		/* error */
		if (expr == NULL || errorIsSet())
			return NULL;

		/* new node */
		node *n = nodeNew(NODE_CONST,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		nodeAddChild(n, expr);

		/* return node */
		return n;
	}

	/* typedef */
	else if (tokenMatches(tok, TOKEN_KEYWORD, "typedef")) {

		/* get token */
		if (p->current_token->t_type != TOKEN_IDENT) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected type name");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		/* advance */
		token *type_name = p->current_token;
		parserAdvance(p);

		/* pointer */
		int is_pointer = 0;

		if (p->current_token->t_type == TOKEN_MUL) {

			/* advance */
			parserAdvance(p);

			is_pointer = 1;
		}

		/* get new type name */
		if (p->current_token->t_type != TOKEN_IDENT) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected type name");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		/* advance */
		token *typedef_name = p->current_token;
		parserAdvance(p);

		/* create new node */
		node *n = nodeNew(NODE_TYPEDEF,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL)
			return NULL;

		/* add tokens */
		nodeAddToken(n, type_name);
		nodeAddToken(n, typedef_name);
		nodeAddValue(n, is_pointer);

		/* return node */
		return n;
	}

	/* include */
	else if (tokenMatches(tok, TOKEN_KEYWORD, "include")) {

		/* get token */
		if (p->current_token->t_type != TOKEN_STRING) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected string");

			/* set error position */
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		token *filename = p->current_token;
		parserAdvance(p); /* advance */

		/* create node */
		node *n = nodeNew(NODE_INCLUDE,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL)
			return NULL;

		/* add token and return node */
		nodeAddToken(n, filename);

		return n;
	}

	/* return */
	else if (tokenMatches(tok, TOKEN_KEYWORD, "return")) {

		/* get node */
		unsigned int _pflag_include_vardec = pflag_include_vardec;
		pflag_include_vardec = 0; /* variable declarations inside of return statement aren't possible */
		node *expr = parserExpr(p);
		pflag_include_vardec = _pflag_include_vardec;

		/* error */
		if (expr == NULL || errorIsSet()) {

			return NULL;
		}

		/* create node */
		node *n = nodeNew(NODE_RETURN,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL) {

			nodeFree(expr);
			return NULL;
		}

		/* add node and return */
		nodeAddChild(n, expr);

		return n;
	}

	/* if */
	else if (tokenMatches(tok, TOKEN_KEYWORD, "if")) {

		/* expects '(' */
		if (p->current_token->t_type != TOKEN_LPAREN) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '('");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* new node */
		node *n = nodeNew(NODE_IFNODE,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL)
			return NULL;

		/* get statement */
		node *expr = parserExpr(p);

		/* error */
		if (expr == NULL || errorIsSet()) {

			nodeFree(n); /* free node */
			return NULL; /* exit */
		}

		/* add child */
		nodeAddChild(n, expr);

		/* expects ')' */
		if (p->current_token->t_type != TOKEN_RPAREN) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ')'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return and free */
			nodeFree(n);
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* expects '->' */
		if (p->current_token->t_type != TOKEN_ARROW) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '->'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* expects '[' */
		if (p->current_token->t_type != TOKEN_OPENBRACKET) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '['");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* get next expr */
		node *next = parserExpr(p);

		/* error */
		if (next == NULL || errorIsSet()) {

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* add node */
		nodeAddChild(n, next);

		/* ';' */
		while (p->current_token->t_type == TOKEN_EOL) {

			/* advance past EOL */
			while (p->current_token->t_type == TOKEN_EOL)
				parserAdvance(p);

			/* ']' */
			if (p->current_token->t_type == TOKEN_CLOSEBRACKET) {

				/* break */
				break;
			}

			/* get next node */
			next = parserExpr(p);

			/* error */
			if (next == NULL || errorIsSet()) {

				/* free and return */
				nodeFree(n);
				return NULL;
			}

			/* add node */
			nodeAddChild(n, next);
		}

		/* expects ']' */
		if (p->current_token->t_type != TOKEN_CLOSEBRACKET) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ']'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		parserAdvance(p); /* advance */

		/* return node */
		return n;
	}

	/* while */
	else if (tokenMatches(tok, TOKEN_KEYWORD, "while")) {

		/* expects '(' */
		if (p->current_token->t_type != TOKEN_LPAREN) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '('");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* new node */
		node *n = nodeNew(NODE_WHILENODE,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL)
			return NULL;

		/* get statement */
		node *expr = parserExpr(p);

		/* error */
		if (expr == NULL || errorIsSet()) {

			nodeFree(n); /* free node */
			return NULL; /* exit */
		}

		/* add child */
		nodeAddChild(n, expr);

		/* expects ')' */
		if (p->current_token->t_type != TOKEN_RPAREN) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ')'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return and free */
			nodeFree(n);
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* expects '->' */
		if (p->current_token->t_type != TOKEN_ARROW) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '->'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* expects '[' */
		if (p->current_token->t_type != TOKEN_OPENBRACKET) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '['");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* get next expr */
		node *next = parserExpr(p);

		/* error */
		if (next == NULL || errorIsSet()) {

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* add node */
		nodeAddChild(n, next);

		/* ';' */
		while (p->current_token->t_type == TOKEN_EOL) {

			/* advance past EOL */
			while (p->current_token->t_type == TOKEN_EOL)
				parserAdvance(p);

			/* ']' */
			if (p->current_token->t_type == TOKEN_CLOSEBRACKET) {

				/* break */
				break;
			}

			/* get next node */
			next = parserExpr(p);

			/* error */
			if (next == NULL || errorIsSet()) {

				/* free and return */
				nodeFree(n);
				return NULL;
			}

			/* add node */
			nodeAddChild(n, next);
		}

		/* expects ']' */
		if (p->current_token->t_type != TOKEN_CLOSEBRACKET) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ']'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		parserAdvance(p); /* advance */

		/* return node */
		return n;
	}

	/* for */
	else if (tokenMatches(tok, TOKEN_KEYWORD, "for")) {

		/* expects '(' */
		if (p->current_token->t_type != TOKEN_LPAREN) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '('");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		parserAdvance(p); /* advance */

		/* new node */
		node *n = nodeNew(NODE_FORNODE,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL)
			return NULL;

		/* expressions */
		for (int i = 0; i < 3; i++) {

			/* get expr */
			node *expr = parserExpr(p);

			/* error */
			if (expr == NULL || errorIsSet()) {

				/* free node */
				nodeFree(n);

				return NULL; /* exit */
			}

			/* add node */
			nodeAddChild(n, expr);

			/* advance past end-of-lines */
			while (p->current_token->t_type == TOKEN_EOL)
				parserAdvance(p);
		}

		/* expects ')' */
		if (p->current_token->t_type != TOKEN_RPAREN) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ')'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return and free */
			nodeFree(n);
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* expects '->' */
		if (p->current_token->t_type != TOKEN_ARROW) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '->'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* expects '[' */
		if (p->current_token->t_type != TOKEN_OPENBRACKET) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '['");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* get next expr */
		node *next = parserExpr(p);

		/* error */
		if (next == NULL || errorIsSet()) {

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		/* add node */
		nodeAddChild(n, next);

		/* ';' */
		while (p->current_token->t_type == TOKEN_EOL) {

			/* advance past EOL */
			while (p->current_token->t_type == TOKEN_EOL)
				parserAdvance(p);

			/* ']' */
			if (p->current_token->t_type == TOKEN_CLOSEBRACKET) {

				/* break */
				break;
			}

			/* get next node */
			next = parserExpr(p);

			/* error */
			if (next == NULL || errorIsSet()) {

				/* free and return */
				nodeFree(n);
				return NULL;
			}

			/* add node */
			nodeAddChild(n, next);
		}

		/* expects ']' */
		if (p->current_token->t_type != TOKEN_CLOSEBRACKET) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ']'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free and return */
			nodeFree(n);
			return NULL;
		}

		parserAdvance(p); /* advance */

		/* return node */
		return n;
	}

	/* fun */
	else if (tokenMatches(tok, TOKEN_KEYWORD, "fun")) {

		/* get function type */
		if (p->current_token->t_type != TOKEN_IDENT) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected function type");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		token *func_type = p->current_token;

		parserAdvance(p); /* advance */

		/* pointer */
		int is_pointer = 0;

		if (p->current_token->t_type == TOKEN_MUL) {

			/* advance */
			parserAdvance(p);

			is_pointer = 1;
		}

		/* get function name */
		if (p->current_token->t_type != TOKEN_IDENT) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected function name");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		token *func_name = p->current_token;

		parserAdvance(p); /* advance */

		/* expects '(' */
		if (p->current_token->t_type != TOKEN_LPAREN) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '('");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* create new node */
		node *n = nodeNew(NODE_FUNCDEC,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL)
			return NULL;

		/* add tokens */
		nodeAddToken(n, func_type);
		nodeAddToken(n, func_name);
		nodeAddValue(n, is_pointer);

		/* arguments */
		if (p->current_token->t_type != TOKEN_RPAREN) {

			/* add argument type */
			if (p->current_token->t_type != TOKEN_IDENT) {

				/* set error */
				errorSet(ERROR_TYPE_SYNTAX,
						 ERROR_CODE_EXPECTEDTOKEN,
						 "Expected argument type");
				errorSetPos(p->current_token->lineno,
							p->current_token->colno,
							p->current_token->fname);

				nodeFree(n); /* free node */
				return NULL; /* exit */
			}

			/* add token */
			nodeAddToken(n, p->current_token);
			parserAdvance(p);

			/* pointer */
			is_pointer = 0;

			if (p->current_token->t_type == TOKEN_MUL) {

				/* advance */
				parserAdvance(p);

				is_pointer = 1;
			}

			nodeAddValue(n, is_pointer);

			/* add argument name */
			if (p->current_token->t_type != TOKEN_IDENT) {

				/* set error */
				errorSet(ERROR_TYPE_SYNTAX,
						 ERROR_CODE_EXPECTEDTOKEN,
						 "Expected argument name");
				errorSetPos(p->current_token->lineno,
							p->current_token->colno,
							p->current_token->fname);

				nodeFree(n); /* free node */
				return NULL; /* exit */
			}

			/* add token */
			nodeAddToken(n, p->current_token);
			parserAdvance(p);

			/* more arguments */
			while (p->current_token->t_type == TOKEN_COMMA) {

				parserAdvance(p); /* advance */

				/* add argument type */
				if (p->current_token->t_type != TOKEN_IDENT) {

					/* set error */
					errorSet(ERROR_TYPE_SYNTAX,
							 ERROR_CODE_EXPECTEDTOKEN,
							 "Expected argument type");
					errorSetPos(p->current_token->lineno,
								p->current_token->colno,
								p->current_token->fname);

					nodeFree(n); /* free node */
					return NULL; /* exit */
				}

				/* add token */
				nodeAddToken(n, p->current_token);
				parserAdvance(p);

				/* pointer */
				is_pointer = 0;

				if (p->current_token->t_type == TOKEN_MUL) {

					/* advance */
					parserAdvance(p);

					is_pointer = 1;
				}

				nodeAddValue(n, is_pointer);

				/* add argument name */
				if (p->current_token->t_type != TOKEN_IDENT) {

					/* set error */
					errorSet(ERROR_TYPE_SYNTAX,
							 ERROR_CODE_EXPECTEDTOKEN,
							 "Expected argument name");
					errorSetPos(p->current_token->lineno,
								p->current_token->colno,
								p->current_token->fname);

					nodeFree(n); /* free node */
					return NULL; /* exit */
				}

				/* add token */
				nodeAddToken(n, p->current_token);
				parserAdvance(p);
			}
		}

		if (p->current_token->t_type != TOKEN_RPAREN) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ')'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free node */
			nodeFree(n);
			return NULL; /* exit */
		}

		/* advance */
		parserAdvance(p);

		/* return node */
		return n;
	}

	/* extern */
	else if (tokenMatches(tok, TOKEN_KEYWORD, "extern")) {

		/* get expr */
		node *expr = parserExpr(p);

		/* error */
		if (expr == NULL || errorIsSet())
			return NULL;

		/* create new node */
		node *n = nodeNew(NODE_EXTERN,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL) {

			/* free and exit */
			nodeFree(expr);
			return NULL;
		}

		/* add child */
		nodeAddChild(n, expr);

		/* return node */
		return n;
	}

	/* unimpl */
	errorSet(ERROR_TYPE_INTERNAL,
			 ERROR_CODE_UNIMPLEMENTED,
			 "Unimplemented");
	errorSetPos(tok->lineno,
				tok->colno,
				tok->fname);

	/* return */
	return NULL;
}

/* function call */
extern node *parserCall(parser *p) {

	/* first token */
	token *tok = p->current_token;

	/* get factor node */
	node *factor = parserFactor(p);

	/* error */
	if (factor == NULL || errorIsSet()) {

		return NULL; /* exit */
	}

	/* function call */
	if (p->current_token->t_type == TOKEN_LPAREN) {

		/* advance */
		parserAdvance(p);

		/* create a new node */
		node *n = nodeNew(NODE_CALL,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL) {

			nodeFree(factor); /* free node */
			return NULL; /* exit */
		}

		/* add first node */
		nodeAddChild(n, factor);

		/* ')' means end of function call */
		if (p->current_token->t_type == TOKEN_RPAREN) {

			/* advance */
			parserAdvance(p);

			/* return node */
			return n;
		}

		node *next = parserExpr(p); /* next node in list */
		
		/* error */
		if (next == NULL || errorIsSet()) {

			/* free nodes and exit */
			nodeFree(n);
			return NULL;
		}

		/* add node */
		nodeAddChild(n, next);

		/* ',' means more! */
		while (p->current_token->t_type == TOKEN_COMMA) {

			/* advance */
			parserAdvance(p);

			/* get next node */
			node *next = parserExpr(p);

			/* error */
			if (next == NULL || errorIsSet()) {

				nodeFree(n);
				return NULL;
			}

			/* add node */
			nodeAddChild(n, next);
		}

		/* expects ')' */
		if (p->current_token->t_type != TOKEN_RPAREN) {

			/* free node */
			nodeFree(n);

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ')'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* return */
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* return node */
		return n;
	}

	/* return factor */
	return factor;
}

/* function definition */
extern node *parserFuncDef(parser *p) {

	token *tok = p->current_token; /* current token */

	/* get call node */
	node *call = parserCall(p);

	/* error */
	if (call == NULL || errorIsSet())
		return NULL;

	/* '->' function definition */
	if (p->current_token->t_type == TOKEN_ARROW) {

		/* advance */
		parserAdvance(p);

		/* expects '[' */
		if (p->current_token->t_type != TOKEN_OPENBRACKET) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected '['");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free node */
			nodeFree(call);

			/* return */
			return NULL;
		}

		/* advance */
		parserAdvance(p);

		/* create a new node */
		node *n = nodeNew(NODE_FUNCDEF,
						  tok->lineno,
						  tok->colno,
						  tok->fname);

		if (n == NULL) {

			nodeFree(call);
			return NULL;
		}

		/* add call node */
		nodeAddChild(n, call);
		
		/* get first expression */
		node *next = parserExpr(p);

		/* error */
		if (next == NULL || errorIsSet()) {

			/* free nodes */
			nodeFree(n);
			return NULL; /* exit */
		}

		/* add node */
		nodeAddChild(n, next);

		/* expecting ';' */
		while (p->current_token->t_type == TOKEN_EOL) {

			/* advance past eols */
			while (p->current_token->t_type == TOKEN_EOL)
				parserAdvance(p);

			/* ']' exit */
			if (p->current_token->t_type == TOKEN_CLOSEBRACKET)
				break; /* exit */

			/* get next node */
			next = parserExpr(p);

			/* error */
			if (next == NULL || errorIsSet()) {

				nodeFree(n);
				return NULL;
			}

			/* add child */
			nodeAddChild(n, next);
		}

		/* expects ']' */
		if (p->current_token->t_type != TOKEN_CLOSEBRACKET) {

			/* set error */
			errorSet(ERROR_TYPE_SYNTAX,
					 ERROR_CODE_EXPECTEDTOKEN,
					 "Expected ']'");
			errorSetPos(p->current_token->lineno,
						p->current_token->colno,
						p->current_token->fname);

			/* free node */
			nodeFree(n);

			return NULL; /* exit */
		}

		parserAdvance(p); /* advance */

		return n; /* return node */
	}

	return call; /* no other option */
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