%{
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%option noyywrap

DIGIT		[0-9]
HEX		0[xX][0-9a-fA-F]+
ID		[a-zA-Z_][a-zA-Z0-9_]*
CHAR		\'(\\.|[^\\'])\'
STRING		\"([^\\\"]|\\.)*\"

%{
uint64_t encode_le(const char *str) {
	uint64_t result = 0;
	for (int i = 7; i >= 0; i--) {
		result = (result << 8) | (uint8_t)str[i];
	}
	return result;
}
%}

%%
[ \t\r]+	;
";".*		;
{HEX}		{ yylval.u64 = strtoull(yytext, NULL, 0); return NUM; }
{DIGIT}+	{ yylval.u64 = strtoull(yytext, NULL, 10); return NUM; }
{CHAR}		{ yylval.u64 = yytext[1]; return NUM; }
{ID}		{ yylval.str = strdup(yytext); return IDENT; }
{STRING}	{ yylval.u64 = encode_le(yytext); return NUM; }

":"		return COLON;
","		return COMMA;
"+"		return PLUS;
"-"		return MINUS;
"\n"		return NEWLINE;

.		{ return *yytext; }
%%

