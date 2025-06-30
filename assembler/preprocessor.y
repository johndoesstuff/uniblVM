%code requires{
#include "../unibl_asm.h"
}
%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "../../inc/unibl.h"
extern int yylex();
extern FILE *yyin;
void yyerror(const char *s) {
	fprintf(stderr, "Parser error: %s\n", s);
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
%token COLON COMMA NEWLINE PLUS MINUS MACRO MACROEND

%type <u64> term expression
%type <oplist> operands

%%

program:
	items
;

items:
	item
	| items item
;

item:
	macro
	| line
;

macro:
	MACRO IDENT params NEWLINE macro_body MACROEND
;

params:
	IDENT
	| params COMMA IDENT
;

macro_body:
	macro_line
	| macro_body macro_line
;

macro_line:
	IDENT COLON			{ add_label_to_macro($1); char* st; asprintf(&st, "%%%s", $1); $$ = st; free($1); }
	| IDENT macro_operands NEWLINE
	| IDENT NEWLINE
	| NEWLINE

macro_operands:
	macro_term				{ $$ = $1; }
	| macro_operands COMMA macro_term	{ char* st; asprintf(&st, "%s, %s", $1, $3); $$ = st; free($1); free($3); }

macro_term:
	PARAM			{ $$ = $1; }
	| NUM			{ char* st; asprintf(&st, "%" PRIu64, $1); $$ = st; }
	| IDENT			{
					if (check_label_in_macro($1)) {
						char* st;
						asprintf(&st, "%%%s", $1);
						$$ = st;
						free($1);
					} else { $$ = $1; }
				}
	| macro_expression	{ $$ = $1; }
;

macro_expression:
	macro_term PLUS NUM		{ char* st; asprintf(&st, "%s + %" PRIu64, $1, $3); $$ = st; free($1); }
	| macro_term PLUS PARAM		{ char* st; asprintf(&st, "%s + %s", $1, $3); $$ = st; free($1); free($3); }
	| macro_term MINUS NUM		{ char* st; asprintf(&st, "%s - %" PRIu64, $1, $3); $$ = st; free($1); }
	| macro_term MINUS PARAM	{ char* st; asprintf(&st, "%s - %s", $1, $3); $$ = st; free($1); free($3); }
;

line:
	IDENT COLON
	| IDENT operands NEWLINE
	| IDENT NEWLINE
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
	term PLUS NUM
	| term MINUS NUM

%%

