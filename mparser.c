#include <assert.h>
#include <ctypes.h>
#include <stdlib.h>
#include <string.h>
#include "mparser.h"

static void print_token(Token *t);
static void lexer_advance(Mlexer *lexer);
static void lexer_skipwhitespace(Mlexer *lexer);
static long lexer_charsleft(Mlexer *lexer);
static double lexer_tonumber(Mlexer *lexer);

int mparser_init(Mparser *m)
{
	assert(m != NULL);
	(void)memset(m, 0, sizeof(Mparser));
}

void mparser_cleanup(Mparser *m)
{
	assert(m != NULL);
}

int mparser_solve(Mparser *m, const char *expression, double *answer)
{
	assert(m != NULL);
	assert(expression != NULL);
	assert(answer != NULL);

	m->current = m->text = expression;
	m->index = 0;
	m->length = (long)strlen(expression);
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
	long count;
	assert(lexer != NULL);

	return lexer->length - 1 - lexer->index;
}

static int lexer_tonumber(Mlexer *lexer, double *result)
{
	double scan;
	assert(lexer != NULL);
	assert(result != NULL);

	/* hexadecimal */
	if (
		(lexer_charsleft(lexer) > 2) &&
		(lexer->current[0] == '0') &&
		(lexer->current[1] == 'x' || lexer->current[1] == 'X'))
	{
		// TODO this is no good, need to get length to advance with atoi or something
		if (sscanf(&lexer->current[2], "%X", &scan) == 1) {
			*result = scan;
			return 0;
		}
		else {
			return 1;
		}
	}
}

