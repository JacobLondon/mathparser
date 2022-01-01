#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "mparser.h"

//#define MYDEBUG(...) printf(__VA_ARGS__)
#define MYDEBUG(...)

/* lexer */
static void print_token(MparserToken *t);
static void lexer_advance(Mlexer *lexer);
static void lexer_skipwhitespace(Mlexer *lexer);
static long lexer_charsleft(Mlexer *lexer);
static int lexer_tonumber(Mlexer *lexer, double *result);
static void lexer_advanceto(Mlexer *lexer, char *p);
static int lexer_gettoken(Mlexer *lexer, MparserToken *token);
/* parser */
static int parser_eat(Mparser *m, MparserTokenType expected);
static int parser_factor(Mparser *m, double *result);
static int parser_term(Mparser *m, double *result);
static int parser_expression(Mparser *m, double *result);

void mparser_init(Mparser *m, Mlexer_lookup_func lookup)
{
	assert(m != NULL);
	(void)memset(m, 0, sizeof(Mparser));
	m->lexer.lookup = lookup;
}

void mparser_cleanup(Mparser *m)
{
	assert(m != NULL);
	(void)memset(m, 0, sizeof(Mparser));
}

int mparser_solve(Mparser *m, const char *expression, double *answer)
{
	double out;
	assert(m != NULL);
	assert(expression != NULL);
	assert(answer != NULL);

	m->lexer.current = m->lexer.text = (char *)expression;
	m->lexer.index = 0;
	m->lexer.length = (long)strlen(expression);

	if (lexer_gettoken(&m->lexer, &m->current) != 0) {
		MYDEBUG("mparser_solve: gettoken\n");
		return 1;
	}

	if (parser_expression(m, &out) != 0) {
		MYDEBUG("mparser_solve: expression\n");
		return 1;
	}

	*answer = out;
	return 0;
}

int mparser_solve_int(Mparser *m, const char *expression, int *answer)
{
	double out;
	int rv;
	assert(m != NULL);
	assert(expression != NULL);
	assert(answer != NULL);
	rv = mparser_solve(m, expression, &out);
	if (rv == 0) {
		*answer = (int)(out);
	}
	return rv;
}

static void print_token(MparserToken *t)
{
	assert(t != NULL);
	switch (t->type) {
	case TT_NUMBER:  printf("(NUMBER, %s)", t->value); break;
	case TT_PLUS:    printf("(PLUS, +)"); break;
	case TT_MINUS:   printf("(MINUS, -)"); break;
	case TT_ASTRISK: printf("(ASTRISK, *)"); break;
	case TT_FSLASH:  printf("(FSLASH, /)"); break;
	case TT_DOT:     printf("(DOT, .)"); break;
	case TT_LPAREN:  printf("(LPAREN, ()"); break;
	case TT_RPAREN:  printf("(RPAREN, ))"); break;
	case TT_EOF:     printf("(EOF, \\0)"); break;
	}
}

static void lexer_advance(Mlexer *lexer)
{
	assert(lexer != NULL);
	lexer->index += 1;
	if (lexer->index >= lexer->length) {
		lexer->current = NULL;
	}
	else {
		lexer->current = &lexer->text[lexer->index];
	}
}

static void lexer_skipwhitespace(Mlexer *lexer)
{
	assert(lexer != NULL);
	while (lexer->current != NULL && isspace(*lexer->current)) {
		lexer_advance(lexer);
	}
}

static long lexer_charsleft(Mlexer *lexer)
{
	assert(lexer != NULL);
	return lexer->length - 1 - lexer->index;
}

static int lexer_tonumber(Mlexer *lexer, double *result)
{
	union {
		long l;
		double f;
		char s[32];
	} parsed;
	char *endptr;
	assert(lexer != NULL);
	assert(result != NULL);

	/* hexadecimal */
	if (
		(lexer_charsleft(lexer) > 2) &&
		(lexer->current[0] == '0') &&
		(lexer->current[1] == 'x' || lexer->current[1] == 'X'))
	{
		errno = 0;
		parsed.l = strtol(lexer->current, &endptr, 16);
		if (errno == 0) {
			*result = (double)parsed.l;
			lexer_advanceto(lexer, endptr);
			return 0;
		}
		else {
			MYDEBUG("tonumber: strtol\n");
			return 1;
		}
	}

	/* int/float */
	errno = 0;
	parsed.f = strtod(lexer->current, &endptr);
	if (errno == 0) {
		*result = parsed.f;
		lexer_advanceto(lexer, endptr);
		return 0;
	}

	/* string and lookup */
	if (lexer->lookup && sscanf(lexer->current, "%31s", parsed.s) == 1) {
		parsed.s[31] = '\0';
		endptr = &lexer->current[strlen(parsed.s)];
		lexer_advanceto(lexer, endptr);
		return lexer->lookup(parsed.s, result);
	}

	MYDEBUG("tonumber: none\n");
	return 1;
}

static void lexer_advanceto(Mlexer *lexer, char *p)
{
	assert(lexer != NULL);
	assert(p != NULL);

	/* endptr always goes one char too far so go 1 less */
	while (lexer->current && lexer->current != p - 1) {
		lexer_advance(lexer);
	}
}

static int lexer_gettoken(Mlexer *lexer, MparserToken *token)
{
	double number;
	MparserToken newtoken = (MparserToken){.type=TT_EOF, .value=NULL};
	assert(lexer != NULL);
	assert(token != NULL);

	while (lexer->current != NULL) {
		if (isspace(*lexer->current)) {
			lexer_skipwhitespace(lexer);
			continue;
		}

		if (isalnum(*lexer->current)) {
			if (lexer_tonumber(lexer, &number) != 0) {
				MYDEBUG("gettoken: tonumber\n");
				return 1;
			}
			newtoken = (MparserToken){.type=TT_NUMBER, .f=number};
			goto out;
		}

		switch (*lexer->current) {
		case '*': newtoken = (MparserToken){.type=TT_ASTRISK, .value="*"}; goto out;
		case '/': newtoken = (MparserToken){.type=TT_FSLASH,  .value="/"}; goto out;
		case '+': newtoken = (MparserToken){.type=TT_PLUS,    .value="+"}; goto out;
		case '-': newtoken = (MparserToken){.type=TT_MINUS,   .value="-"}; goto out;
		case '(': newtoken = (MparserToken){.type=TT_LPAREN,  .value="("}; goto out;
		case ')': newtoken = (MparserToken){.type=TT_RPAREN,  .value=")"}; goto out;
		default:
			break;
		}

		MYDEBUG("gettoken: none\n");
		return 1;
	}

out:
	lexer_advance(lexer);
	*token = newtoken;
	return 0;
}

static int parser_eat(Mparser *m, MparserTokenType expected)
{
	assert(m != NULL);
	if (m->current.type == expected) {
		if (lexer_gettoken(&m->lexer, &m->current) == 0) {
			return 0;
		}
		MYDEBUG("eat: no token\n");
		return 1;
	}
	MYDEBUG("eat: none\n");
	return 1;
}

static int parser_factor(Mparser *m, double *result)
{
	MparserToken token;
	assert(m != NULL);
	assert(result != NULL);

	token = m->current;
	if (token.type == TT_NUMBER) {
		if (parser_eat(m, TT_NUMBER) != 0) {
			MYDEBUG("factor: eat number\n");
			return 1;
		}
		*result = token.f;
		return 0;
	}
	else if (token.type == TT_LPAREN) {
		if (parser_eat(m, TT_LPAREN) != 0) {
			MYDEBUG("factor: eat lparen\n");
			return 1;
		}
		if (parser_expression(m, result) != 0) {
			MYDEBUG("factor: expression\n");
			return 1;
		}
		if (parser_eat(m, TT_RPAREN) != 0) {
			MYDEBUG("factor: eat rparen\n");
			return 1;
		}
		return 0;
	}

	MYDEBUG("factor: none\n");
	return 1;
}

static int parser_term(Mparser *m, double *result)
{
	double number;
	double initial;
	MparserToken token;
	assert(m != NULL);
	assert(result != NULL);

	if (parser_factor(m, &initial) != 0) {
		MYDEBUG("term: eat factor\n");
		return 1;
	}

	token = m->current;
	if (token.type == TT_ASTRISK) {
		if (parser_eat(m, TT_ASTRISK) != 0) {
		MYDEBUG("term: eat astrisk\n");
			return 1;
		}
		if (parser_factor(m, &number) != 0) {
			MYDEBUG("term: factor1\n");
			return 1;
		}
		*result = initial * number;
		return 0;
	}
	else if (token.type == TT_FSLASH) {
		if (parser_eat(m, TT_FSLASH) != 0) {
			MYDEBUG("term: eat fslash\n");
			return 1;
		}
		if (parser_factor(m, &number) != 0) {
			MYDEBUG("term: eat factor2\n");
			return 1;
		}
		*result = initial / number;
		return 0;
	}

	*result = initial;
	return 0;
}

static int parser_expression(Mparser *m, double *result)
{
	double initial;
	double number;
	MparserToken token;
	assert(m != NULL);
	assert(result != NULL);

	if (parser_term(m, &initial) != 0) {
		MYDEBUG("expression: eat term\n");
		return 1;
	}

	token = m->current;
	if (token.type == TT_PLUS) {
		if (parser_eat(m, TT_PLUS) != 0) {
		MYDEBUG("expression: eat plus\n");
			return 1;
		}
		if (parser_term(m, &number) != 0) {
			MYDEBUG("expression: term1\n");
			return 1;
		}
		*result = initial + number;
		return 0;
	}
	else if (token.type == TT_MINUS) {
		if (parser_eat(m, TT_MINUS) != 0) {
			MYDEBUG("expression: eat minus\n");
			return 1;
		}
		if (parser_term(m, &number) != 0) {
			MYDEBUG("expression: term2\n");
			return 1;
		}
		*result = initial - number;
		return 0;
	}

	*result = initial;
	return 0;
}

