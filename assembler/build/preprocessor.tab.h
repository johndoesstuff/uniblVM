/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_PP_ASSEMBLER_BUILD_PREPROCESSOR_TAB_H_INCLUDED
# define YY_PP_ASSEMBLER_BUILD_PREPROCESSOR_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int pp_debug;
#endif
/* "%code requires" blocks.  */
#line 4 "assembler/preprocessor.y"

#include "../unibl_preprocessor.h"

#line 53 "assembler/build/preprocessor.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IDENT = 258,                   /* IDENT  */
    PARAM = 259,                   /* PARAM  */
    NUM = 260,                     /* NUM  */
    FSTRING = 261,                 /* FSTRING  */
    COLON = 262,                   /* COLON  */
    COMMA = 263,                   /* COMMA  */
    NEWLINE = 264,                 /* NEWLINE  */
    PLUS = 265,                    /* PLUS  */
    MINUS = 266,                   /* MINUS  */
    MACRO = 267,                   /* MACRO  */
    ENDMACRO = 268,                /* ENDMACRO  */
    DIRECTIVE = 269,               /* DIRECTIVE  */
    DEF_DIRECTIVE = 270,           /* DEF_DIRECTIVE  */
    DUMP_DIRECTIVE = 271           /* DUMP_DIRECTIVE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 22 "assembler/preprocessor.y"

	uint64_t u64;
	char *str;
	MacroBody *mac;
	MacroParams *par;
	ArgumentList* arg;

#line 94 "assembler/build/preprocessor.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE pp_lval;
extern YYLTYPE pp_lloc;

int pp_parse (void);


#endif /* !YY_PP_ASSEMBLER_BUILD_PREPROCESSOR_TAB_H_INCLUDED  */
