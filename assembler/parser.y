%code requires{
#include "../unibl_asm.h"
}
%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "../../inc/unibl.h"
extern int yylex();
extern FILE *yyin;
void yyerror(const char *s) {
	fprintf(stderr, "Parser error %s\n", s);
}

uint64_t PC = ENTRY_POINT;
%}

%union {
	uint64_t u64;
	char *str;
	OperandList *oplist;
}

%token <str> IDENT
%token <u64> NUM
%token COLON COMMA NEWLINE PLUS MINUS DIRECTIVE

%type <u64> term expression directive_term
%type <oplist> operands directive_operands

%%

program:
	lines
;

lines:
	line
	| lines line
;

line:
	IDENT COLON 				{ add_label($1, &PC); }
	| IDENT operands NEWLINE		{ add_i($1, $2, &PC); }
	| DIRECTIVE IDENT directive_operands NEWLINE	{ directive_i($2, $3, &PC); }
	| IDENT NEWLINE				{ add_si($1, &PC); }
	| DIRECTIVE IDENT NEWLINE		{ directive_si($2, &PC); }
	| NEWLINE
;

operands:
	term			{ $$ = make_operand_list($1); }
	| operands COMMA term	{ $$ = append_operand($1, $3); }

directive_operands:
	directive_term					{ $$ = make_operand_list($1); }
	| directive_operands COMMA directive_term	{ $$ = append_operand($1, $3); }

term:
	NUM 			{ $$ = $1; }
	| IDENT			{ $$ = get_label($1, 0); }
	| expression		{ $$ = $1; }

directive_term:
	NUM 			{ $$ = $1; }
	| IDENT			{ $$ = get_label($1, 1); }
	| expression		{ $$ = $1; }

expression:
	term PLUS NUM		{ $$ = $1 + $3; }
	| term MINUS NUM	{ $$ = $1 - $3; }

%%

