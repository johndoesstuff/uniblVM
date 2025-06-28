#include <stdio.h>
#include <stdlib.h>

extern FILE *yyin;
int yyparse(void);

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <input.uasm>\n", argv[0]);
		return 1;
	}

	yyin = fopen(argv[1], "r");
	if (!yyin) {
		perror("Failed to open input file");
		return 1;
	}

	if (yyparse() == 0) {
		printf("Parsing completed successfully.\n");
	} else {
		printf("Parsing failed.\n");
	}

	fclose(yyin);
	return 0;
}
