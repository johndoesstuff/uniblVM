%define parse.error verbose
%locations

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

uint64_t PC = ENTRY_POINT;
void asm_error(const char *s) {
	extern int yylineno;
	fprintf(stderr, "Syntax Error: %s at line%d\n", s, yylineno);
}
%}

%union {
	uint64_t u64;
	char *str;
	OperandList *oplist;
}

%token <str> IDENT FSTRING
%token <u64> NUM
%token COLON COMMA NEWLINE PLUS MINUS DIRECTIVE DEF_DIRECTIVE DUMP_DIRECTIVE

%type <u64> term expression directive_term directive_expression
%type <oplist> operands directive_operands

%left PLUS MINUS

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
	| DEF_DIRECTIVE IDENT directive_term NEWLINE	{ add_label($2, &$3); }
	| DUMP_DIRECTIVE FSTRING
	| IDENT operands NEWLINE		{ add_i($1, $2, &PC); }
	| DIRECTIVE IDENT directive_operands NEWLINE	{ directive_i($2, $3, &PC); }
	| IDENT NEWLINE				{ add_i($1, NULL, &PC); }
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
	| directive_expression		{ $$ = $1; }

expression:
	term PLUS term		{ $$ = $1 + $3; }
	| term MINUS term	{ $$ = $1 - $3; }

directive_expression:
	directive_term PLUS directive_term	{ $$ = $1 + $3; }
	| directive_term MINUS directive_term	{ $$ = $1 - $3; }

%%

