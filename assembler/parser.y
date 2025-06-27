%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
void yyerror(const char *s) { fprintf(stderr, "Error: %s\n", s); }

typedef struct {
    char* name;
    uint64_t address;
} Symbol;

%}

%union {
    uint64_t u64;
    char* str;
}

%token <u64> NUM
%token <str> IDENT
%token COLON COMMA NEWLINE

%%

program:
    lines
;

lines:
    line
    | lines line
;

line:
    IDENT COLON instruction NEWLINE
    | instruction NEWLINE
    | NEWLINE
;

instruction:
    IDENT
    | IDENT operands
;

operands:
    NUM
    | IDENT
    | NUM COMMA NUM
    | IDENT COMMA NUM
;

%%

int main() {
    return yyparse();
}
