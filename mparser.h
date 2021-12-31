#ifndef MPARSER_H
#define MPARSER_H

typedef enum MparserTokenTypeEnum
{
	TT_NUMBER,
	TT_PLUS,
	TT_MINUS,
	TT_ASTRISK,
	TT_FSLASH,
	TT_DOT,
	TT_LPAREN,
	TT_RPAREN,
	TT_EOF,
} MparserTokenType;

typedef struct MparserTokenStruct
{
	char *value;
	MparserTokenType type;
} MparserToken;

typedef struct MlexerStruct
{
	char *text;
	char *current;
	long index;
	long length;
} Mlexer;

typedef struct MparserStruct
{
	Mlexer lexer;
} Mparser;

/** initialize a math parser, return 0 on success >0 otherwise */
int mparser_init(Mparser *m);
/** clean up a math parser */
void mparser_cleanup(Mparser *m);

/**
 * Solve a string expression consisting of:
 * - numbers: 0x[0-9]|[1-9][0-9]*|[1-9]+\.[0-9]*|0\.[0-9]*
 * - parentheses
 * - operations: + - * /
 *
 * Write solution into answer
 */
int mparser_solve(Mparser *m, const char *expression, double *answer);
int mparser_solve_int(Mparser *m, const char *expression, int *answer);

#endif /* MPARSER_H */

