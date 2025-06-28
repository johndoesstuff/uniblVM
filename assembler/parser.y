%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

extern int yylex();
extern FILE *yyin;
void yyerror(const char *s) {
	fprintf(stderr, "Parser error: %s\n", s);
}
%}

%union {
	uint64_t u64;
	char *str;
}

%token <str> IDENT
%token <u64> NUM
%token COLON COMMA NEWLINE PLUS MINUS

%%

program:
	lines
;

lines:
	line
	| lines line
;

line:
	IDENT COLON
	| IDENT operands NEWLINE
	| NEWLINE
;

operands:
	term
	| operands COMMA term

term:
	NUM
	| IDENT
	| expression

expression:
	IDENT PLUS NUM
	| IDENT MINUS NUM
	| NUM PLUS NUM

%%

