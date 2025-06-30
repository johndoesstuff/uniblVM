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
	int len = strlen(str);
	int out_i = 0;

	for (int in_i = 1; str[in_i] != '"' && str[in_i] != '\0' && out_i < 8; in_i++) {
		char c = str[in_i];

		if (c == '\\') {
			char next = str[++in_i];
			switch (next) {
				case 'n': c = '\n'; break;
				case 't': c = '\t'; break;
				case 'r': c = '\r'; break;
				case '\\': c = '\\'; break;
				case '\'': c = '\''; break;
				case '\"': c = '\"'; break;
				case '0': c = '\0'; break;
				default: c = next; break;
			}
		}

		result |= ((uint64_t)c << (8 * out_i));
		out_i++;
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
"%"{ID}		{ yylval.str = strdup(yytext); return PARAM; }
{STRING}	{ yylval.u64 = encode_le(yytext); return NUM; }

":"		return COLON;
","		return COMMA;
"+"		return PLUS;
"-"		return MINUS;
".MACRO"	return MACRO;
".ENMAC"	return ENDMACRO;
"\n"		return NEWLINE;

.		{ return *yytext; }
%%

