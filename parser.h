/* parser.h -- node parsing system */
#ifndef _PARSER_H
#define _PARSER_H

#include "node.h" /* node system */

/* parser */
typedef struct {
	node *pn; /* parsed node */
	token **tokens; /* list of tokens to parse */
	token *current_token; /* current token being parsed */
	unsigned int tok_index; /* token index */
	unsigned int n_of_toks; /* number of tokens in list */
} parser;

/* typedefs */
typedef node *(*pfunc)(parser *); /* parser function type */

/* functions */
extern parser *parserNew(token **tokens, unsigned int n_of_toks); /* create a new parser */
extern void parserParse(parser *p); /* parse a node */
extern node *parserExpr(parser *p); /* parse an expression */
extern node *parserCompExpr(parser *p); /* comparison expression */
extern node *parserArithExpr(parser *p); /* arithmatic expression */
extern node *parserTerm(parser *p); /* parse a term */
extern node *parserFactor(parser *p); /* parse a factor */
extern node *parserCall(parser *p); /* parse a function call which goes straight to factor otherwise */
extern node *parserVarDec(parser *p); /* parse a variable declaration */
extern node *parserVarAsn(parser *p); /* parse a variable assignment */
extern node *parserFuncDef(parser *p); /* parse a function definition (depends on how user put it) */
extern node *parserCall(parser *p); /* parse a function call */
extern node *parserString(parser *p); /* parse a string */
extern node *parserInt(parser *p); /* parse an int */
extern node *parserKeyword(parser *p); /* parse a regular old keyword */
extern node *parserBinOp(parser *p, pfunc func, int types[], unsigned int n_of_types); /* binary operation */
extern node *parserStatements(parser *p); /* get multiple statements */
extern void parserFree(parser *p); /* free a parser */
extern void parserAdvance(parser *p); /* advance a parser */
extern int is_int_in(int values[], int val, unsigned int len);

#endif /* _PARSER_H */