%{
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%option noyywrap
%option prefix="asm_"

DIGIT		[0-9]
HEX		0[xX][0-9a-fA-F]+
ID		[a-zA-Z_][a-zA-Z0-9_]*
CHAR		\'(\\.|[^\\'])\'
STRING		\"([^\\\"]|\\.)*\"
FSTRING		\<([^\\\>]|\\.)*\>

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

char* strip_angles(char *str) {
	size_t len = strlen(str);
	if (len < 2) return str;
	str[len - 1] = '\0';
	return str + 1;
}

%}

%%
[ \t\r]+	;
";".*		;
{HEX}		{ asm_lval.u64 = strtoull(asm_text, NULL, 0); return NUM; }
{DIGIT}+	{ asm_lval.u64 = strtoull(asm_text, NULL, 10); return NUM; }
{CHAR}		{ asm_lval.u64 = asm_text[1]; return NUM; }
{ID}		{ asm_lval.str = strdup(asm_text); return IDENT; }
{STRING}	{ asm_lval.u64 = encode_le(asm_text); return NUM; }
{FSTRING}	{ asm_lval.str = strip_angles(asm_text); return FSTRING; }

":"		return COLON;
","		return COMMA;
"+"		return PLUS;
"-"		return MINUS;
"\n"		{ yylineno++; return NEWLINE; }
"$DEF"		return DEF_DIRECTIVE;
"$DUMP"		return DUMP_DIRECTIVE;
"$"		return DIRECTIVE;

.		{ return *asm_text; }
%%

