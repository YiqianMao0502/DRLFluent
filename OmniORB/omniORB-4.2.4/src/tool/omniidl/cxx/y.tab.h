/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IDENTIFIER = 258,
    ABSTRACT = 259,
    ANY = 260,
    ATTRIBUTE = 261,
    BOOLEAN = 262,
    CASE = 263,
    CHAR = 264,
    CONST = 265,
    CONTEXT = 266,
    CUSTOM = 267,
    DEFAULT = 268,
    DOUBLE = 269,
    ENUM = 270,
    EXCEPTION = 271,
    FACTORY = 272,
    FALSE_ = 273,
    FIXED = 274,
    FLOAT = 275,
    IN = 276,
    INOUT = 277,
    INTERFACE = 278,
    LOCAL = 279,
    LONG = 280,
    MODULE = 281,
    NATIVE = 282,
    OBJECT = 283,
    OCTET = 284,
    ONEWAY = 285,
    OUT = 286,
    PRIVATE = 287,
    PUBLIC = 288,
    RAISES = 289,
    READONLY = 290,
    SEQUENCE = 291,
    SHORT = 292,
    STRING = 293,
    STRUCT = 294,
    SUPPORTS = 295,
    SWITCH = 296,
    TRUE_ = 297,
    TRUNCATABLE = 298,
    TYPEDEF = 299,
    UNION = 300,
    UNSIGNED = 301,
    VALUEBASE = 302,
    VALUETYPE = 303,
    VOID = 304,
    WCHAR = 305,
    WSTRING = 306,
    PRAGMA = 307,
    PRAGMA_PREFIX = 308,
    PRAGMA_ID = 309,
    PRAGMA_VERSION = 310,
    OMNI_PRAGMA = 311,
    END_PRAGMA = 312,
    UNKNOWN_PRAGMA_BODY = 313,
    INTEGER_LITERAL = 314,
    CHARACTER_LITERAL = 315,
    WIDE_CHARACTER_LITERAL = 316,
    FLOATING_PT_LITERAL = 317,
    STRING_LITERAL = 318,
    WIDE_STRING_LITERAL = 319,
    FIXED_PT_LITERAL = 320,
    SCOPE_DELIM = 321,
    LEFT_SHIFT = 322,
    RIGHT_SHIFT = 323
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 141 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1909  */

  char*                    id_val;
  int                      int_val;
  IDL_ULong                ulong_val;
  IdlIntLiteral            int_literal_val;
#ifndef __VMS
  IdlFloatLiteral          float_literal_val;
#else
  double                   float_literal_val;
#endif
  char                     char_val;
  char*                    string_val;
  IDL_WChar                wchar_val;
  IDL_WChar*               wstring_val;
  IDL_Boolean              boolean_val;
  IDL_Fixed*               fixed_val;
  IdlType*                 type_val;
  TypeSpec*                type_spec_val;
  IdlExpr*                 expr_val;
  ScopedName*              scopedname_val;
  Decl*                    decl_val;
  Module*                  module_val;
  Interface*               interface_val;
  InheritSpec*             inheritspec_val;
  Forward*                 forward_val;
  Const*                   const_val;
  Typedef*                 typedef_val;
  Struct*                  struct_val;
  Exception*               exception_val;
  Member*                  member_val;
  Declarator*              declarator_val;
  Union*                   union_val;
  UnionCase*               union_case_val;
  CaseLabel*               case_label_val;
  ValueBase*               value_base_val;
  Value*                   value_val;
  ValueForward*            value_forward_val;
  ValueBox*                value_box_val;
  ValueAbs*                value_abs_val;
  ValueInheritSpec*        valueinheritspec_val;
  ValueInheritSupportSpec* valueinheritsupportspec_val;
  StateMember*             statemember_val;
  Factory*                 factory_val;
  Enumerator*              enumerator_val;
  Enum*                    enum_val;
  ArraySize*               array_size_val;
  Attribute*               attribute_val;
  Operation*               operation_val;
  Parameter*               parameter_val;
  RaisesSpec*              raisesspec_val;
  ContextSpec*             contextspec_val;

#line 176 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
