%{
#include "parser.tab.h"
#include <string.h>
%}

%x COMMENT

DIGIT     [0-9]
HEX       0[xX][0-9a-fA-F]+
DEC       {DIGIT}+
ID        [a-zA-Z_][a-zA-Z0-9_]*
CHAR      \'(\\.|[^\\'])\'

%%
[ \t]+                     ; // Skip whitespace
";".*                     ; // Skip comments

{HEX}                     { yylval.u64 = strtoull(yytext, NULL, 0); return NUM; }
{DEC}                     { yylval.u64 = strtoull(yytext, NULL, 0); return NUM; }
{CHAR}                    { yylval.u64 = yytext[1]; return NUM; }

{ID}                      { yylval.str = strdup(yytext); return IDENT; }

":"                       { return COLON; }
","                       { return COMMA; }
"\n"                      { return NEWLINE; }

.                         { return *yytext; }

%%
