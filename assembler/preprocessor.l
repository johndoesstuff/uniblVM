%{
#include "preprocessor.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%option noyywrap
%option prefix="pp_"

DIGIT		[0-9]
HEX		0[xX][0-9a-fA-F]+
ID		[a-zA-Z_][a-zA-Z0-9_]*
CHAR		\'(\\.|[^\\'])\'
STRING		\"([^\\\"]|\\.)*\"

%{
uint64_t encode_le(const char *str);
%}

%%
[ \t\r]+	;
";".*		;
{HEX}		{ pp_lval.str = strdup(pp_text); return NUM; }
{DIGIT}+	{ pp_lval.str = strdup(pp_text); return NUM; }
{CHAR}		{ pp_lval.str = strdup(pp_text); return NUM; }
{ID}		{ pp_lval.str = strdup(pp_text); return IDENT; }
"%"{ID}		{ pp_lval.str = strdup(pp_text); return PARAM; }
{STRING}	{ pp_lval.str = strdup(pp_text); return NUM; }

":"		return COLON;
","		return COMMA;
"+"		return PLUS;
"-"		return MINUS;
".MACRO"	return MACRO;
".ENMAC"	return ENDMACRO;
"\n"		return NEWLINE;

.		{ return *pp_text; }
%%

