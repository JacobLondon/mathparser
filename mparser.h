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
	MparserTokenType type;
	union {
		char *value;
		double f;
	};
} MparserToken;

/** lookup a numeric value to a symbol. Return 0 on success, >0 if failure */
typedef int (* Mlexer_lookup_func)(char *literal, double *out);

typedef struct MlexerStruct
{
	char *text;
	char *current;
	long index;
	long length;
	Mlexer_lookup_func lookup;
} Mlexer;

typedef struct MparserStruct
{
	Mlexer lexer;
	MparserToken current;
} Mparser;

/**
 * Initialize a math parser, return 0 on success >0 otherwise.
 * Nullable function which fills the numeric value of that symbol
 */
void mparser_init(Mparser *m, Mlexer_lookup_func lookup);
/** optional, clean up a math parser */
void mparser_cleanup(Mparser *m);

/**
 * Solve a string expression consisting of:
 * - atoms: 0x[0-9]|[1-9][0-9]*|[1-9]+\.[0-9]*|0\.[0-9]*|[str:32]
 * - parentheses
 * - operations: + - * /
 *
 * Write solution into answer
 */
int mparser_solve(Mparser *m, const char *expression, double *answer);
int mparser_solve_int(Mparser *m, const char *expression, int *answer);

#endif /* MPARSER_H */

