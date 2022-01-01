#include <stdio.h>
#include "mparser.h"

int main(int argc, char **argv)
{
	double answer;
	Mparser parser;
	mparser_init(&parser, NULL);

	if (argc < 2) {
		(void)fprintf(stderr, "Expected an expression\n");
		return 1;
	}

	if (mparser_solve(&parser, argv[1], &answer) != 0) {
		(void)fprintf(stderr, "Could not parse\n");
		return 1;
	}

	(void)printf("%lf\n", answer);

	return 0;
}

