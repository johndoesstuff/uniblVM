%locations
%define parse.error verbose

%code requires{
#include "../unibl_preprocessor.h"
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
	extern int pp_lineno;
	fprintf(stderr, "Parser error at line %d: %s\n", pp_lineno, s);
}
%}

%union {
	uint64_t u64;
	char *str;
	MacroBody *mac;
	MacroParams *par;
	ArgumentList* arg;
}

%token <str> IDENT PARAM NUM FSTRING
%token COLON COMMA NEWLINE PLUS MINUS MACRO ENDMACRO DIRECTIVE DEF_DIRECTIVE DUMP_DIRECTIVE

%type <str> macro_expression macro_term macro_operands macro_line
%type <str> term line expression
%type <arg> operands
%type <par> params
%type <mac> macro_body

%left PLUS MINUS

%%

program:
	{ init_program(program_str); } items
;

items:
	item
	| items item
;

item:
	macro
	| line		{ if ($1[0] != '\0') append_line(program_str, $1); }
;

macro:
	MACRO IDENT { start_macro($2); } params NEWLINE macro_body  ENDMACRO	{ define_macro($2, $4, $6); exit_macro(); free($2); }
;

params:
	IDENT				{ $$ = make_macro_params($1); free($1);  }
	| params COMMA IDENT		{ $$ = append_macro_params($1, $3); free($3); }
	|				{ $$ = make_macro_params(" "); }
;

macro_body:
	macro_line			{ $$ = make_macro_body($1); free($1); }
	| macro_body macro_line		{ $$ = append_macro_body($1, $2); free($2); }
;

macro_line:
	IDENT COLON			{ add_label_to_macro($1); char* st; asprintf(&st, "%%%s:", $1); $$ = st; free($1); }
	| DEF_DIRECTIVE IDENT macro_term NEWLINE	{ char* st; asprintf(&st, "$DEF %s %s", $2, $3); $$ = st; free($2); free($3); }
	| DUMP_DIRECTIVE FSTRING NEWLINE	{ char* st; asprintf(&st, "$DUMP %s", $2); $$ = st; free($2); }
	| IDENT macro_operands NEWLINE	{ char* st; asprintf(&st, "%s %s", $1, $2); $$ = st; free($1); free($2); }
	| DIRECTIVE IDENT macro_operands NEWLINE	{ char* st; asprintf(&st, "$%s %s", $2, $3); $$ = st; free($2); free($3); }
	| IDENT NEWLINE			{ char* st; asprintf(&st, "%s", $1); $$ = st; free($1); }
	| DIRECTIVE IDENT NEWLINE			{ char* st; asprintf(&st, "$%s", $2); $$ = st; free($2); }
	| NEWLINE			{ $$ = strdup(""); }

macro_operands:
	macro_term				{ $$ = $1; }
	| macro_operands COMMA macro_term	{ char* st; asprintf(&st, "%s, %s", $1, $3); $$ = st; free($1); free($3); }

macro_term:
	PARAM			{ $$ = $1; }
	| NUM			{ $$ = $1; }
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
	macro_term PLUS macro_term		{ char* st; asprintf(&st, "%s + %s", $1, $3); $$ = st; free($1); free($3); }
	| macro_term MINUS macro_term		{ char* st; asprintf(&st, "%s - %s", $1, $3); $$ = st; free($1); free($3); }
;

line:
	IDENT COLON			{ char* st; asprintf(&st, "%s:", $1); $$ = st; free($1); }
	| DEF_DIRECTIVE IDENT term NEWLINE	{ char* st; asprintf(&st, "$DEF %s %s", $2, $3); $$ = st; free($2); free($3); }
	| DUMP_DIRECTIVE FSTRING NEWLINE	{ char* st; asprintf(&st, "$DUMP %s", $2); $$ = st; free($2); }
	| IDENT operands NEWLINE	{ $$ = check_macro_expansion($1, $2); }
	| DIRECTIVE IDENT operands NEWLINE	{ char* st; asprintf(&st, "$%s", $2); $$ = check_macro_expansion(st, $3); free($2); }
	| IDENT NEWLINE			{ $$ = check_macro_expansion($1, NULL); }
	| DIRECTIVE IDENT NEWLINE	{ char* st; asprintf(&st, "$%s", $2); $$ = check_macro_expansion(st, NULL); free($2); }
	| NEWLINE			{ $$ = strdup(""); }
;

operands:
	term			{ $$ = make_argument_list($1); }
	| operands COMMA term	{ $$ = append_argument_list($1, $3); }

term:
	NUM		{ $$ = $1; }
	| IDENT		{ $$ = $1; }
	| expression	{ $$ = $1; }

expression:
	term PLUS term		{ char* st; asprintf(&st, "%s + %s", $1, $3); $$ = st; free($1); free($3); }
	| term MINUS term	{ char* st; asprintf(&st, "%s - %s", $1, $3); $$ = st; free($1); free($3); }

%%

