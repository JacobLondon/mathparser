#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mparser.h"

static int run_tests(Mparser *m);

int main(int argc, char **argv)
{
	int rv;
	double answer;
	Mparser parser;

	if (argc < 2) {
		rv = run_tests(&parser);
		if (rv == 0) {
			printf("All tests have passed\n");
		}
		return rv;
	}

	if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		printf("Math Parser Test Suite\n\n");
		printf("USAGE: %s [-h, --help] View this help\n", argv[0]);
		printf("       %s              Run test suite\n", argv[0]);
		printf("       %s EXPRESSION   Run an expression\n\n", argv[0]);
		printf("EXPRESSION\n");
		printf("  Infix notation math consisting of number literals and operations.\n");
		printf("  Numbers can be integer, floating point, or hexadecimal.\n");
		printf("  Operations can be one of: + - * /.\n\n");
		return 0;
	}

	mparser_init(&parser, NULL);
	if (mparser_solve(&parser, argv[1], &answer) != 0) {
		(void)fprintf(stderr, "%s: Could not parse\n", argv[0]);
		return 1;
	}

	(void)printf("%lf\n", answer);

	return 0;
}

static int run_tests(Mparser *m)
{
#define XROW(EXPRESSION, EXPECTED) {.expression=(EXPRESSION), .expected=(EXPECTED), __LINE__, 0}
#define XROWF(EXPRESSION) {.expression=(EXPRESSION), .expected=0, __LINE__, 1}
	struct {
		char *expression;
		double expected;
		int line;
		int parsing_should_fail;
	} table[] = {
		XROW("1+1", 2),
		XROW("1 + 1", 2),
		XROW("10000 + 10000", 20000),
		XROWF(""),
		XROWF("("),
		XROW("1", 1),
		XROWF("1 +"),
		XROWF("1 -"),
		XROWF("1 *"),
		XROWF("1 /"),
		XROWF("p8cn23pr98j3p9 + 23ra"),
		XROWF("fae;oiwje;ofj;oifj;oij+_)(*&^@!%$!#@&*7_$(*"),
		XROW("(10 + 4 / 2) - 3", 9),
		XROW("(8*8) + (2 + (3 * 2))", 72),
		XROW("((((1))))", 1),
		//XROWF("((((1))))(((("),
		XROW(NULL, 0)
	};
#undef XROW

	int i;
	int rv;
	int total = 0;;
	double answer;
	assert(m != NULL);

	for (i = 0; table[i].expression != NULL; i++) {
		mparser_init(m, NULL);
		total += rv = mparser_solve(m, table[i].expression, &answer);

		/* parsing intentionally failed */
		if (rv != 0 && table[i].parsing_should_fail) {
			total -= 1;
			continue;
		}

		/* parsing has failed */
		if (rv != 0) {
			printf("%d: \"%s\" failed to parse\n",
				table[i].line, table[i].expression);
		}
		/* parsing has succeeded */
		else {
			if (answer != table[i].expected) {
				printf("%d: \"%s == %lf != %lf\"\n",
					table[i].line, table[i].expression, answer, table[i].expected);
			}
			/* else test passed */
		}
	}

	return total;
}

