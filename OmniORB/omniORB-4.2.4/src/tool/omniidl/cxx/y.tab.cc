/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 101 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:339  */


#include <stdlib.h>
#include <string.h>

#include <idlutil.h>
#include <idlerr.h>
#include <idlrepoId.h>
#include <idlscope.h>
#include <idltype.h>
#include <idlexpr.h>
#include <idlast.h>

#define YYDEBUG 1

// Globals from lexer
extern int         yylineno;
extern char*       currentFile;
extern IDL_Boolean mainFile;

void yyerror(const char *s) {
}
extern int yylex();

// Nasty hack for abstract valuetypes
ValueAbs* valueabs_hack = 0;

#ifdef __VMS
/*  Apparently, __ALLOCA is defined for some versions of the C (but not C++)
    compiler on VAX. */
#if defined(__ALPHA) || defined(__DECC) && __DECC_VER >= 60000000
#include <builtins.h>
#define alloca __ALLOCA
#else
#define alloca malloc
#endif
#endif


#line 106 "y.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
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
#line 141 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:355  */

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

#line 268 "y.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 285 "y.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  134
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1432

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  91
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  148
/* YYNRULES -- Number of rules.  */
#define YYNRULES  307
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  508

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   323

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    84,    79,     2,
      74,    75,    82,    80,    73,    81,    90,    83,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    72,    69,
      86,    76,    87,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    88,     2,    89,    78,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    70,    77,    71,    85,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   408,   408,   409,   416,   417,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   439,   443,   443,   450,   459,
     463,   464,   468,   472,   472,   479,   488,   494,   502,   503,
     504,   508,   512,   513,   520,   521,   522,   523,   524,   525,
     526,   533,   534,   538,   542,   549,   557,   565,   569,   572,
     575,   586,   587,   588,   589,   593,   596,   602,   610,   610,
     617,   617,   626,   639,   646,   651,   656,   659,   665,   668,
     671,   677,   684,   685,   689,   696,   707,   711,   712,   719,
     720,   721,   725,   733,   734,   738,   738,   744,   744,   755,
     761,   762,   766,   767,   774,   782,   788,   789,   790,   791,
     792,   793,   794,   795,   796,   799,   803,   807,   808,   812,
     813,   817,   818,   824,   825,   828,   834,   835,   836,   840,
     841,   844,   847,   853,   858,   862,   863,   864,   868,   871,
     872,   876,   879,   882,   885,   888,   891,   894,   897,   903,
     904,   914,   915,   925,   926,   930,   939,   940,   941,   942,
     943,   946,   950,   958,   959,   963,   964,   965,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   983,   984,   985,
     986,   990,   991,   992,   996,   997,  1004,  1005,  1009,  1015,
    1019,  1020,  1021,  1025,  1026,  1030,  1031,  1032,  1036,  1040,
    1044,  1048,  1049,  1050,  1054,  1058,  1062,  1066,  1070,  1074,
    1078,  1082,  1086,  1090,  1094,  1103,  1109,  1110,  1117,  1122,
    1130,  1138,  1147,  1153,  1154,  1155,  1156,  1157,  1164,  1168,
    1169,  1176,  1183,  1184,  1191,  1194,  1200,  1207,  1211,  1219,
    1225,  1226,  1233,  1239,  1242,  1248,  1249,  1255,  1256,  1262,
    1268,  1269,  1276,  1280,  1286,  1287,  1291,  1292,  1299,  1303,
    1312,  1318,  1319,  1326,  1326,  1332,  1342,  1348,  1349,  1353,
    1357,  1358,  1362,  1363,  1364,  1372,  1373,  1380,  1386,  1387,
    1388,  1392,  1393,  1397,  1401,  1404,  1411,  1412,  1416,  1420,
    1423,  1430,  1431,  1432,  1433,  1439,  1462,  1468,  1472,  1475,
    1481,  1482,  1483,  1484,  1488,  1489,  1492,  1494,  1498,  1501,
    1507,  1511,  1517,  1521,  1527,  1533,  1539,  1540
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "ABSTRACT", "ANY",
  "ATTRIBUTE", "BOOLEAN", "CASE", "CHAR", "CONST", "CONTEXT", "CUSTOM",
  "DEFAULT", "DOUBLE", "ENUM", "EXCEPTION", "FACTORY", "FALSE_", "FIXED",
  "FLOAT", "IN", "INOUT", "INTERFACE", "LOCAL", "LONG", "MODULE", "NATIVE",
  "OBJECT", "OCTET", "ONEWAY", "OUT", "PRIVATE", "PUBLIC", "RAISES",
  "READONLY", "SEQUENCE", "SHORT", "STRING", "STRUCT", "SUPPORTS",
  "SWITCH", "TRUE_", "TRUNCATABLE", "TYPEDEF", "UNION", "UNSIGNED",
  "VALUEBASE", "VALUETYPE", "VOID", "WCHAR", "WSTRING", "PRAGMA",
  "PRAGMA_PREFIX", "PRAGMA_ID", "PRAGMA_VERSION", "OMNI_PRAGMA",
  "END_PRAGMA", "UNKNOWN_PRAGMA_BODY", "INTEGER_LITERAL",
  "CHARACTER_LITERAL", "WIDE_CHARACTER_LITERAL", "FLOATING_PT_LITERAL",
  "STRING_LITERAL", "WIDE_STRING_LITERAL", "FIXED_PT_LITERAL",
  "SCOPE_DELIM", "LEFT_SHIFT", "RIGHT_SHIFT", "';'", "'{'", "'}'", "':'",
  "','", "'('", "')'", "'='", "'|'", "'^'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'~'", "'<'", "'>'", "'['", "']'", "'.'", "$accept",
  "start", "definition_plus", "definition", "module", "$@1",
  "module_header", "interface", "interface_dcl", "$@2", "forward_dcl",
  "interface_header", "abstract_local_opt", "interface_body",
  "export_star", "export", "interface_inheritance_spec_opt",
  "interface_inheritance_spec", "interface_inheritance_list",
  "interface_name", "scoped_name", "value", "value_forward_dcl",
  "value_box_dcl", "value_abs_dcl", "$@3", "$@4", "value_dcl",
  "value_header", "value_inheritance_spec", "value_value_inheritance_spec",
  "truncatable_opt", "value_inheritance_list", "value_name",
  "value_element_star", "value_element", "state_member", "member_access",
  "init_dcl", "$@5", "$@6", "init_dcl_header", "init_param_decls_opt",
  "init_param_decls", "init_param_decl", "const_dcl", "const_type",
  "const_exp", "or_expr", "xor_expr", "and_expr", "shift_expr", "add_expr",
  "mult_expr", "unary_expr", "unary_operator", "primary_expr", "literal",
  "string_literal_plus", "wide_string_literal_plus", "boolean_literal",
  "positive_int_const", "type_dcl", "type_declarator", "type_spec",
  "simple_type_spec", "base_type_spec", "template_type_spec",
  "constr_type_spec", "declarators", "declarator", "simple_declarator",
  "complex_declarator", "floating_pt_type", "integer_type", "signed_int",
  "signed_short_int", "signed_long_int", "signed_long_long_int",
  "unsigned_int", "unsigned_short_int", "unsigned_long_int",
  "unsigned_long_long_int", "char_type", "wide_char_type", "boolean_type",
  "octet_type", "any_type", "object_type", "struct_type", "struct_header",
  "member_list", "member", "union_type", "union_header",
  "switch_type_spec", "switch_body", "case_plus", "case",
  "case_label_plus", "case_label", "element_spec", "enum_type",
  "enum_header", "enumerator_list", "enumerator", "sequence_type",
  "string_type", "wide_string_type", "array_declarator",
  "fixed_array_size_plus", "fixed_array_size", "attr_dcl", "readonly_opt",
  "simple_declarator_list", "except_dcl", "except_header", "member_star",
  "op_dcl", "$@7", "op_header", "op_attribute_opt", "op_attribute",
  "op_type_spec", "parameter_dcls", "param_dcl_list", "param_dcl",
  "param_attribute", "raises_expr_opt", "raises_expr", "scoped_name_list",
  "context_expr_opt", "context_expr", "string_literal_list",
  "param_type_spec", "fixed_pt_type", "fixed_pt_const_type",
  "value_base_type", "constr_forward_decl", "pragma", "pragmas",
  "pragmas_opt", "pragma_prefix", "pragma_id", "pragma_version",
  "unknown_pragma", "omni_pragma", "unknown_pragma_body_plus", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,    59,
     123,   125,    58,    44,    40,    41,    61,   124,    94,    38,
      43,    45,    42,    47,    37,   126,    60,    62,    91,    93,
      46
};
# endif

#define YYPACT_NINF -343

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-343)))

#define YYTABLE_NINF -297

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1263,   -33,   120,  1231,   151,    76,   215,  -343,   219,   230,
     238,  1039,   241,   247,    96,    25,    44,    53,    96,   252,
    1320,  -343,   194,   101,   195,  -343,  -343,    36,   242,   199,
    -343,  -343,  -343,  -343,   196,   203,   204,  -343,   106,  -343,
     307,  -343,   127,   208,   183,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,   288,  -343,  -343,  -343,  -343,  -343,  -343,    51,
    -343,  -343,   206,    34,  -343,   211,   291,   232,   296,  -343,
    -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,  -343,  -343,  -343,   299,  -343,  -343,  -343,
    -343,   234,  -343,   218,  -343,   225,   302,   304,  -343,   232,
    -343,   312,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,   253,   281,  -343,   128,  -343,  -343,    17,   259,
      18,   272,     9,   197,  -343,  -343,  -343,   264,  -343,    86,
     268,  -343,   265,  -343,   339,  -343,  -343,  -343,  -343,  -343,
     284,  -343,   319,  -343,   294,  -343,  -343,   305,    -9,  -343,
    -343,    31,   341,  -343,    31,  -343,   369,   298,    -8,    31,
    1088,  -343,  -343,   289,   303,    86,  -343,  -343,  -343,    54,
     337,  -343,  -343,  -343,  -343,  -343,  -343,  -343,   102,  -343,
     295,  -343,   316,  -343,  1376,   317,   323,   600,    95,   542,
      86,   314,    86,    86,   320,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,  -343,    31,  -343,  -343,  -343,   232,  -343,
     321,   318,   325,   105,   114,   130,  -343,   145,  -343,  -343,
     336,   342,  -343,   329,  -343,   330,  -343,    31,  -343,   332,
     -27,    31,   289,  -343,    86,  -343,  -343,   335,    86,   232,
    -343,   372,    48,  -343,   359,  1376,   879,  -343,  -343,  -343,
    -343,  -343,  -343,   350,   351,   353,   417,   355,   356,    71,
    1137,  -343,  -343,   354,   424,  -343,  -343,  -343,  -343,  -343,
    -343,  1039,  -343,   357,   364,    86,   425,  -343,  -343,   362,
     361,    31,    31,    31,    31,    31,    31,    31,    31,    31,
      31,  -343,  -343,  -343,  -343,  -343,    31,    31,  -343,   340,
    -343,   312,    86,  -343,    54,   232,   365,  -343,   380,   936,
    -343,   368,  -343,  -343,  -343,  1186,  -343,  -343,  -343,   367,
    -343,   232,  -343,  -343,  -343,   439,  -343,    54,  -343,  -343,
    -343,   312,    29,  -343,   312,   774,    86,   221,  -343,   150,
      86,   827,   658,  -343,  -343,   318,   325,   105,   114,   114,
     130,   130,  -343,  -343,  -343,   358,   360,  -343,    86,    48,
     335,    48,  -343,  -343,  -343,   441,    77,  -343,  -343,   335,
     -25,   371,  1186,   373,   376,  -343,    -3,  -343,    86,  -343,
     426,   232,  -343,  -343,  -343,    86,  -343,  -343,    86,  -343,
    -343,  -343,  -343,   716,  -343,  -343,  -343,    86,  -343,  -343,
      86,   383,   378,   113,    86,  -343,  -343,   455,  -343,   440,
    -343,  -343,   385,   425,  -343,  -343,  -343,    86,  -343,  -343,
    -343,  -343,  -343,   154,    86,  1186,   428,   394,  -343,   428,
    -343,    86,    86,   441,    86,  -343,  -343,   461,   391,   456,
      86,  -343,   397,   398,  -343,    86,   115,  -343,    48,   395,
    -343,    86,  -343,  -343,    86,  -343,    86,   -13,   159,   407,
    -343,    69,  -343,  -343,    86,  -343,   137,   167,    31,   399,
     403,    69,    86,   990,  -343,    48,  -343,    86,  -343,   404,
      86,  -343,    86,  -343,   312,  -343,   406,   -13,   407,    86,
    -343,  -343,  -343,  -343,  -343,   137,  -343,  -343
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,    14,    29,     0,     0,     0,     0,    30,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     0,     0,     0,    20,    21,     0,     0,     0,
      54,    53,    52,    51,     0,     0,     0,   147,     0,   148,
       0,   149,     0,     0,     0,   151,    12,    13,   290,   291,
     292,   293,     0,    48,   199,   197,   181,   286,   180,   189,
     200,   188,   236,     0,   198,   238,     0,   104,     0,   100,
      96,   183,   185,   186,   187,   184,   191,   192,   193,    97,
      98,    99,   105,   101,   102,   103,     0,   229,   250,    19,
     150,   205,   201,     0,   202,     0,     0,     0,   287,   157,
     146,     0,   153,   155,   156,   154,   158,   159,   160,   161,
     162,   163,   164,   165,   171,   172,   173,   167,   168,   169,
     170,   166,   212,    55,   306,     0,   299,   139,     0,     0,
       0,     0,     0,     0,     1,     5,    10,    16,   294,   297,
       0,     9,    23,    32,     0,    11,    77,     7,     6,   204,
       0,   211,     0,   228,     0,     8,   249,     0,    56,   182,
     190,     0,   195,   194,     0,    49,     0,     0,    67,     0,
       0,   205,   212,   178,   152,   296,   176,   177,   179,     0,
      72,    64,    57,   304,   307,   298,   140,   301,     0,   303,
       0,   305,     0,   295,     0,     0,     0,     0,   296,     0,
     296,     0,   296,   296,     0,    60,   144,   143,   131,   134,
     135,   137,   141,   136,     0,   126,   125,   127,   128,   145,
     106,   107,   109,   111,   113,   116,   119,     0,   124,   129,
     132,   133,   138,     0,   196,     0,    50,     0,    65,     0,
       0,     0,   239,   240,   296,   174,    46,    70,   296,    47,
      73,    69,     0,   300,     0,     0,     0,    32,    22,    40,
     259,   245,    33,     0,     0,     0,     0,     0,     0,     0,
       0,   258,    39,    41,     0,    84,    83,    63,    79,    78,
      80,     0,    81,     0,     0,   296,     0,   251,    32,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   123,   142,   235,   237,    95,     0,     0,   234,     0,
     241,     0,   296,    44,     0,    76,    71,    74,     0,     0,
      15,     0,    35,    34,    37,     0,    36,    38,   255,     0,
     261,   284,   281,   282,   283,     0,   260,     0,    27,    42,
      89,     0,     0,   209,     0,     0,   296,     0,   232,     0,
     296,     0,     0,    32,   130,   108,   110,   112,   115,   114,
     117,   118,   120,   121,   122,     0,     0,   242,   296,     0,
      68,     0,   302,    17,    24,     0,     0,   253,   256,    43,
       0,     0,     0,     0,    91,    92,     0,   203,   296,   206,
     189,   217,   213,   214,   215,   296,   216,   227,   296,   230,
     248,   252,    59,     0,   285,   233,   175,   296,    75,   178,
     296,   243,     0,     0,   296,    82,    87,     0,    85,     0,
     208,   207,     0,     0,    61,    45,   246,   296,   264,   268,
     270,   269,   263,     0,   296,     0,   271,     0,    94,   271,
      93,   296,   296,     0,   296,   262,   265,     0,     0,   276,
     296,    88,     0,     0,   231,   296,     0,   267,     0,     0,
     254,   296,   272,    86,   296,   247,   296,   296,     0,     0,
     277,     0,   266,   274,   296,   273,   296,     0,     0,     0,
       0,   218,   296,     0,   222,     0,   279,   296,   278,     0,
     296,   210,   296,   219,     0,   223,     0,   296,     0,   296,
     225,   220,   226,   221,   275,   296,   224,   280
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -343,  -343,  -125,    -7,  -343,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,   223,  -250,   286,  -343,  -343,  -279,   119,
      -1,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -106,
    -343,  -343,  -343,   121,  -343,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,  -343,    72,  -178,  -343,  -197,  -343,   202,
     198,   201,   -38,   -35,   -83,  -343,   269,  -343,   -15,  -343,
    -343,  -141,  -177,  -343,    -5,   327,  -258,  -343,  -343,  -174,
    -304,  -342,  -343,   495,     5,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,     8,   496,    11,   498,  -343,  -343,    -6,
    -343,  -343,  -149,    -2,  -343,  -343,  -343,  -343,    21,  -343,
      20,  -343,   -10,  -343,  -343,    81,  -343,     0,    15,  -343,
    -343,   263,  -343,  -343,  -343,  -172,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,  -343,  -343,  -343,    52,  -343,    68,  -343,
    -343,  -343,  -343,  -343,  -296,  -343,  -343,  -343,  -343,     4,
    -343,    45,  -343,  -343,  -343,  -343,  -343,   492
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    19,    20,    21,    22,   192,    23,    24,    25,   195,
      26,    27,    28,   196,   197,   262,   338,   339,   247,   248,
     218,    29,    30,    31,    32,   204,   289,    33,    34,   181,
     251,   252,   316,   317,   199,   279,   280,   281,   282,   439,
     437,   283,   383,   384,   385,    35,    68,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,    36,   100,   344,   102,   103,   104,   105,   174,
     175,   176,   177,   106,   107,    71,    72,    73,    74,    75,
      76,    77,    78,   108,   109,   110,   111,   112,   113,    37,
      38,   345,   346,    39,    40,   395,   480,   481,   482,   483,
     484,   496,    41,    42,   349,   350,   117,   118,   119,   178,
     242,   243,   265,   266,   411,    43,    44,   351,   268,   414,
     269,   270,   271,   335,   377,   433,   434,   435,   449,   450,
     468,   460,   461,   477,   336,   120,    85,   121,    45,   138,
     139,   140,    47,    48,    49,    50,    51,   125
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     128,   116,    67,    83,    46,   114,   101,   368,    70,   115,
      99,    79,   332,   135,    81,   130,   132,   290,    84,   263,
     264,   263,   264,   235,    46,   267,   126,   267,   239,   375,
     381,   179,   179,   410,    53,   370,   -62,   142,   352,    14,
     305,    16,    17,    18,   415,   129,   307,    53,   244,   206,
     382,    53,   205,   166,   131,   246,    53,    53,   379,   162,
     308,   -58,   238,   180,   180,   159,   420,   332,   190,   256,
     244,   163,   328,   207,   185,   166,   160,   478,   412,    87,
     186,   127,   479,   150,   166,   152,   417,   154,   127,   157,
     208,   209,   210,   211,   127,   212,   213,    66,  -296,  -296,
     309,   455,   137,   403,   -90,   214,   143,   149,  -296,   365,
      66,   215,   216,   116,    66,   188,   217,   114,   182,    66,
      66,   115,    99,    14,   332,    16,    17,    18,   153,    14,
     319,    16,    17,    18,   429,   430,   429,   430,    14,   447,
      16,    17,    18,   193,   431,  -296,   431,    14,    53,    16,
      17,    18,  -296,    14,   124,    16,    17,    18,    14,   253,
      16,    17,    18,   206,   -26,   186,   366,   380,    52,    99,
     386,  -296,   294,   295,   263,   264,  -296,   332,   249,    14,
     267,    16,    17,    18,   156,   183,   184,   207,   432,    14,
     502,    16,    17,    18,   296,   297,   388,  -296,    46,    86,
     186,   272,   401,   272,   208,   209,   210,   211,   127,   212,
     213,    66,   298,   299,   300,   362,   363,   364,    88,   214,
     245,   397,    89,   398,    53,   263,   264,   444,    54,   445,
      55,   267,   474,    90,   475,    14,     5,    16,    17,    18,
     487,    91,   488,   273,   122,   284,   390,   286,   287,   135,
     123,   315,   134,  -296,   191,   184,   358,   359,    61,    46,
      46,   360,   361,   136,   141,   144,   146,    63,   145,   331,
     333,   116,   147,   148,   116,   114,   341,   155,   114,   115,
      99,   489,   115,    99,    53,   334,    92,    66,    54,   311,
      55,   158,   161,   313,   165,    56,     5,   164,   166,   167,
      93,    58,   168,  -288,   169,   171,    59,   172,   151,    94,
      60,   170,   135,   249,   329,   173,   187,    95,    61,    62,
      96,   179,  -289,    46,   331,   333,    97,    63,    98,   189,
     347,    64,    65,   -18,   -25,   116,   249,   396,   194,   114,
     334,   116,   198,   115,    99,   114,   391,    66,  -296,   115,
      99,   -66,   392,   180,   200,   393,   272,   369,   394,    14,
     201,    16,    17,    18,   202,   343,   234,    53,   249,    92,
     315,    54,   236,    55,   237,   203,   244,   241,    56,     5,
     250,   331,   333,    93,    58,   254,   255,   257,   285,    59,
     288,   389,    94,    60,   258,   399,   292,   334,   291,   186,
      95,    61,    62,    96,   293,   306,   302,   272,   312,    97,
      63,    98,   314,   406,    64,    65,   303,   304,   318,   322,
     323,   413,   324,   325,   326,   327,   337,   340,   348,   367,
      66,   342,   353,   421,   331,   333,   354,   372,   371,   374,
     422,   376,   378,   423,   409,   404,   416,   405,   418,   419,
     334,   160,   425,   428,   476,   426,   427,   467,   438,   436,
     441,   382,   448,   451,   457,   458,   463,   459,   464,   469,
     127,   490,   443,   116,   491,   503,   499,   114,   494,   446,
     321,   115,    99,   505,   497,   278,   453,   454,   407,   456,
     356,   440,   408,   355,   357,   462,   301,   240,    69,    80,
     465,    82,   492,   495,   442,   310,   470,   452,   466,   471,
     133,   472,   473,     0,     0,     0,     0,     0,     0,   485,
       0,   486,     0,     0,     0,     0,     0,   493,     0,     0,
       0,     0,   498,     0,     0,   500,     0,   501,     0,     0,
       0,     0,   504,   259,   506,  -257,     0,  -257,  -244,  -257,
     507,  -257,     3,     0,     0,     0,  -257,     5,     6,   274,
       0,     0,  -257,     0,     0,     0,     0,  -257,     0,     9,
    -257,  -257,   260,     0,   275,   276,     0,   261,     0,  -257,
    -257,    10,     0,     0,     0,     0,    11,    12,  -257,  -257,
       0,  -257,  -257,  -257,    14,     0,    16,    17,    18,     0,
       0,   259,     0,  -257,     0,  -257,  -244,  -257,  -257,  -257,
       3,     0,     0,   277,  -257,     5,     6,     0,     0,     0,
    -257,     0,     0,     0,     0,  -257,     0,     9,  -257,  -257,
     260,     0,     0,     0,     0,   261,     0,  -257,  -257,    10,
       0,     0,     0,     0,    11,    12,  -257,  -257,     0,  -257,
    -257,  -257,    14,     0,    16,    17,    18,     0,     0,   259,
       0,  -257,     0,  -257,  -244,  -257,  -257,  -257,     3,     0,
       0,   -31,  -257,     5,     6,     0,     0,     0,  -257,     0,
       0,     0,     0,  -257,     0,     9,  -257,  -257,   260,     0,
       0,     0,     0,   261,     0,  -257,  -257,    10,     0,     0,
       0,     0,    11,    12,  -257,  -257,     0,  -257,  -257,  -257,
      14,     0,    16,    17,    18,     0,     0,   259,     0,  -257,
       0,  -257,  -244,  -257,  -257,  -257,     3,     0,     0,   402,
    -257,     5,     6,     0,     0,     0,  -257,     0,     0,     0,
       0,  -257,     0,     9,  -257,  -257,   260,     0,     0,     0,
       0,   261,     0,  -257,  -257,    10,     0,     0,     0,     0,
      11,    12,  -257,  -257,     0,  -257,  -257,  -257,    14,     0,
      16,    17,    18,     0,     0,   343,     0,    53,     0,    92,
       0,    54,  -257,    55,     0,     0,     0,   424,    56,     5,
       0,     0,     0,    93,    58,     0,     0,     0,     0,    59,
       0,     0,    94,    60,     0,     0,     0,     0,     0,     0,
      95,    61,    62,    96,     0,     0,     0,     0,     0,    97,
      63,    98,     0,     0,    64,    65,     0,     0,   343,     0,
      53,     0,    92,     0,    54,     0,    55,     0,     0,     0,
      66,    56,     5,     0,     0,   387,    93,    58,     0,     0,
       0,     0,    59,     0,     0,    94,    60,     0,     0,     0,
       0,     0,     0,    95,    61,    62,    96,     0,     0,     0,
       0,     0,    97,    63,    98,     0,     0,    64,    65,     0,
       1,     0,     0,     2,     0,     0,     0,     0,     0,     3,
       0,     4,     0,    66,     5,     6,     0,     0,   400,     0,
       0,     0,   -28,     7,     0,     8,     9,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,    12,     0,     0,    13,     0,     0,
       0,    14,    15,    16,    17,    18,     0,     1,     0,     0,
       2,     0,     0,     0,     0,     0,     3,     0,     4,     0,
     320,     5,     6,     0,     0,     0,     0,     0,     0,   -28,
       7,     0,     8,     9,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
      11,    12,     0,     0,    13,     0,     0,     0,    14,    15,
      16,    17,    18,    53,     0,    92,     0,    54,   478,    55,
       0,     0,     0,   479,    56,     5,     0,   373,     0,    93,
      58,     0,     0,     0,     0,    59,     0,     0,    94,    60,
       0,     0,     0,     0,     0,     0,    95,    61,    62,    96,
       0,     0,     0,     0,     0,    97,    63,    98,     0,     0,
      64,    65,    53,     0,    92,     0,    54,     0,    55,     0,
       0,     0,     0,    56,     5,     0,    66,     0,    93,    58,
       0,     0,     0,     0,    59,     0,     0,    94,    60,     0,
       0,     0,     0,     0,     0,    95,    61,    62,    96,     0,
       0,     0,     0,     0,    97,    63,    98,     0,     0,    64,
      65,    53,     0,    92,     0,    54,     0,    55,     0,     0,
       0,     0,    56,     0,     0,    66,     0,    93,    58,     0,
       0,     0,     0,    59,     0,     0,    94,    60,     0,     0,
       0,     0,     0,     0,    95,    61,    62,     0,     0,     0,
       0,     0,     0,     0,    63,    98,     0,     0,    64,    65,
      53,     0,    92,     0,    54,     0,    55,     0,     0,     0,
       0,    56,     0,     0,    66,     0,     0,    58,     0,     0,
       0,     0,    59,     0,     0,    94,    60,     0,     0,     0,
       0,     0,     0,     0,    61,    62,     0,     0,     0,     0,
       0,     0,     0,    63,    98,     0,   330,    64,    65,    53,
       0,    92,     0,    54,     0,    55,     0,     0,     0,     0,
      56,     0,     0,    66,     0,     0,    58,     0,     0,     0,
       0,    59,     0,     0,    94,    60,     0,     0,     0,     0,
       0,     0,     0,    61,    62,     0,     0,     0,     0,     0,
       0,     0,    63,    98,    53,     0,    64,    65,    54,     0,
      55,     0,     0,     0,     0,    56,     0,     0,     0,     0,
      57,    58,    66,     0,     0,     0,    59,     0,     0,     0,
      60,     0,     0,    -2,     1,     0,     0,     2,    61,    62,
       0,     0,     0,     3,     0,     4,     0,    63,     5,     6,
       0,    64,    65,     0,     0,     0,   -28,     7,     0,     8,
       9,     0,     0,     0,     0,     0,     0,    66,     0,     0,
       0,     0,    10,     0,     0,     0,     0,    11,    12,     0,
       0,    13,     0,     0,     0,    14,    15,    16,    17,    18,
      -3,     1,     0,     0,     2,     0,     0,     0,     0,     0,
       3,     0,     4,     0,     0,     5,     6,     0,     0,     0,
       0,     0,     0,   -28,     7,     0,     8,     9,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,    11,    12,     0,     0,    13,     0,
       0,     0,    14,    15,    16,    17,    18,     1,     0,     0,
       2,     0,     0,     0,     0,     0,     3,     0,     4,     0,
       0,     5,     6,     0,     0,     0,     0,     0,     0,   -28,
       7,     0,     8,     9,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
      11,    12,     0,     0,    13,     0,     0,     0,    14,    15,
      16,    17,    18
};

static const yytype_int16 yycheck[] =
{
      15,    11,     3,     3,     0,    11,    11,   311,     3,    11,
      11,     3,   270,    20,     3,    16,    17,   214,     3,   197,
     197,   199,   199,   164,    20,   197,     1,   199,   169,   325,
       1,    40,    40,   375,     3,   314,    69,     1,   288,    52,
     237,    54,    55,    56,    69,     1,    73,     3,    73,    18,
      21,     3,   158,    66,     1,     1,     3,     3,   337,    25,
      87,    70,   168,    72,    72,    14,    69,   325,    59,   194,
      73,    37,     1,    42,    57,    66,    25,     8,     1,     3,
      63,    63,    13,    38,    66,    40,   382,    42,    63,    44,
      59,    60,    61,    62,    63,    64,    65,    66,    21,    22,
     241,   443,     1,   353,    75,    74,    70,     1,    31,   306,
      66,    80,    81,   123,    66,   130,    85,   123,   123,    66,
      66,   123,   123,    52,   382,    54,    55,    56,     1,    52,
     255,    54,    55,    56,    21,    22,    21,    22,    52,   435,
      54,    55,    56,   139,    31,    74,    31,    52,     3,    54,
      55,    56,    75,    52,    58,    54,    55,    56,    52,    57,
      54,    55,    56,    18,    69,    63,   307,   341,    48,   170,
     344,    70,    67,    68,   352,   352,    70,   435,   179,    52,
     352,    54,    55,    56,     1,    57,    58,    42,    75,    52,
     494,    54,    55,    56,    80,    81,   345,    70,   194,    48,
      63,   197,   351,   199,    59,    60,    61,    62,    63,    64,
      65,    66,    82,    83,    84,   298,   299,   300,     3,    74,
     175,    71,     3,    73,     3,   403,   403,    73,     7,    75,
       9,   403,    73,     3,    75,    52,    15,    54,    55,    56,
      73,     3,    75,   198,     3,   200,    25,   202,   203,   256,
       3,   252,     0,    70,    57,    58,   294,   295,    37,   255,
     256,   296,   297,    69,    69,    23,    70,    46,    69,   270,
     270,   281,    69,    69,   284,   281,   281,    69,   284,   281,
     281,   478,   284,   284,     3,   270,     5,    66,     7,   244,
       9,     3,    86,   248,     3,    14,    15,    86,    66,     3,
      19,    20,     3,    69,    86,     3,    25,     3,     1,    28,
      29,    86,   319,   314,   269,     3,    57,    36,    37,    38,
      39,    40,    69,   319,   325,   325,    45,    46,    47,    57,
     285,    50,    51,    69,    69,   345,   337,   347,    70,   345,
     325,   351,     3,   345,   345,   351,   347,    66,    41,   351,
     351,    70,   347,    72,    70,   347,   352,   312,   347,    52,
      41,    54,    55,    56,    70,     1,    25,     3,   369,     5,
     371,     7,     3,     9,    76,    70,    73,    88,    14,    15,
      43,   382,   382,    19,    20,    90,    70,    70,    74,    25,
      70,   346,    28,    29,    71,   350,    78,   382,    77,    63,
      36,    37,    38,    39,    79,    73,    64,   403,    73,    45,
      46,    47,    40,   368,    50,    51,    87,    87,    59,    69,
      69,   376,    69,     6,    69,    69,    72,     3,     3,    89,
      66,    74,    70,   388,   435,   435,    75,    57,    73,    71,
     395,    74,     3,   398,     3,    87,    75,    87,    75,    73,
     435,    25,   407,    75,   469,   410,    73,   458,     3,   414,
      75,    21,    34,    69,     3,    74,    69,    11,    70,    74,
      63,    72,   427,   483,    71,    69,    72,   483,   483,   434,
     257,   483,   483,   498,   485,   199,   441,   442,   369,   444,
     292,   419,   371,   291,   293,   450,   227,   170,     3,     3,
     455,     3,   481,   483,   423,   242,   461,   439,   456,   464,
      18,   466,   467,    -1,    -1,    -1,    -1,    -1,    -1,   474,
      -1,   476,    -1,    -1,    -1,    -1,    -1,   482,    -1,    -1,
      -1,    -1,   487,    -1,    -1,   490,    -1,   492,    -1,    -1,
      -1,    -1,   497,     1,   499,     3,    -1,     5,     6,     7,
     505,     9,    10,    -1,    -1,    -1,    14,    15,    16,    17,
      -1,    -1,    20,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    -1,    32,    33,    -1,    35,    -1,    37,
      38,    39,    -1,    -1,    -1,    -1,    44,    45,    46,    47,
      -1,    49,    50,    51,    52,    -1,    54,    55,    56,    -1,
      -1,     1,    -1,     3,    -1,     5,     6,     7,    66,     9,
      10,    -1,    -1,    71,    14,    15,    16,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    -1,    -1,    -1,    -1,    35,    -1,    37,    38,    39,
      -1,    -1,    -1,    -1,    44,    45,    46,    47,    -1,    49,
      50,    51,    52,    -1,    54,    55,    56,    -1,    -1,     1,
      -1,     3,    -1,     5,     6,     7,    66,     9,    10,    -1,
      -1,    71,    14,    15,    16,    -1,    -1,    -1,    20,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    -1,
      -1,    -1,    -1,    35,    -1,    37,    38,    39,    -1,    -1,
      -1,    -1,    44,    45,    46,    47,    -1,    49,    50,    51,
      52,    -1,    54,    55,    56,    -1,    -1,     1,    -1,     3,
      -1,     5,     6,     7,    66,     9,    10,    -1,    -1,    71,
      14,    15,    16,    -1,    -1,    -1,    20,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    -1,    -1,    -1,
      -1,    35,    -1,    37,    38,    39,    -1,    -1,    -1,    -1,
      44,    45,    46,    47,    -1,    49,    50,    51,    52,    -1,
      54,    55,    56,    -1,    -1,     1,    -1,     3,    -1,     5,
      -1,     7,    66,     9,    -1,    -1,    -1,    71,    14,    15,
      -1,    -1,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    37,    38,    39,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    50,    51,    -1,    -1,     1,    -1,
       3,    -1,     5,    -1,     7,    -1,     9,    -1,    -1,    -1,
      66,    14,    15,    -1,    -1,    71,    19,    20,    -1,    -1,
      -1,    -1,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    50,    51,    -1,
       1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,
      -1,    12,    -1,    66,    15,    16,    -1,    -1,    71,    -1,
      -1,    -1,    23,    24,    -1,    26,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    48,    -1,    -1,
      -1,    52,    53,    54,    55,    56,    -1,     1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    12,    -1,
      71,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,
      24,    -1,    26,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    48,    -1,    -1,    -1,    52,    53,
      54,    55,    56,     3,    -1,     5,    -1,     7,     8,     9,
      -1,    -1,    -1,    13,    14,    15,    -1,    71,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    -1,    -1,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    38,    39,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      50,    51,     3,    -1,     5,    -1,     7,    -1,     9,    -1,
      -1,    -1,    -1,    14,    15,    -1,    66,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    38,    39,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    50,
      51,     3,    -1,     5,    -1,     7,    -1,     9,    -1,    -1,
      -1,    -1,    14,    -1,    -1,    66,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    -1,    -1,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    50,    51,
       3,    -1,     5,    -1,     7,    -1,     9,    -1,    -1,    -1,
      -1,    14,    -1,    -1,    66,    -1,    -1,    20,    -1,    -1,
      -1,    -1,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    49,    50,    51,     3,
      -1,     5,    -1,     7,    -1,     9,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    66,    -1,    -1,    20,    -1,    -1,    -1,
      -1,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,     3,    -1,    50,    51,     7,    -1,
       9,    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,
      19,    20,    66,    -1,    -1,    -1,    25,    -1,    -1,    -1,
      29,    -1,    -1,     0,     1,    -1,    -1,     4,    37,    38,
      -1,    -1,    -1,    10,    -1,    12,    -1,    46,    15,    16,
      -1,    50,    51,    -1,    -1,    -1,    23,    24,    -1,    26,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    48,    -1,    -1,    -1,    52,    53,    54,    55,    56,
       0,     1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    12,    -1,    -1,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    23,    24,    -1,    26,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    48,    -1,
      -1,    -1,    52,    53,    54,    55,    56,     1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    12,    -1,
      -1,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,
      24,    -1,    26,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    48,    -1,    -1,    -1,    52,    53,
      54,    55,    56
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,    10,    12,    15,    16,    24,    26,    27,
      39,    44,    45,    48,    52,    53,    54,    55,    56,    92,
      93,    94,    95,    97,    98,    99,   101,   102,   103,   112,
     113,   114,   115,   118,   119,   136,   153,   180,   181,   184,
     185,   193,   194,   206,   207,   229,   230,   233,   234,   235,
     236,   237,    48,     3,     7,     9,    14,    19,    20,    25,
      29,    37,    38,    46,    50,    51,    66,   111,   137,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   198,   199,   227,    48,     3,     3,     3,
       3,     3,     5,    19,    28,    36,    39,    45,    47,   111,
     154,   155,   156,   157,   158,   159,   164,   165,   174,   175,
     176,   177,   178,   179,   180,   184,   193,   197,   198,   199,
     226,   228,     3,     3,    58,   238,     1,    63,   149,     1,
     111,     1,   111,   238,     0,    94,    69,     1,   230,   231,
     232,    69,     1,    70,    23,    69,    70,    69,    69,     1,
     232,     1,   232,     1,   232,    69,     1,   232,     3,    14,
      25,    86,    25,    37,    86,     3,    66,     3,     3,    86,
      86,     3,     3,     3,   160,   161,   162,   163,   200,    40,
      72,   120,   155,    57,    58,    57,    63,    57,   149,    57,
      59,    57,    96,   230,    70,   100,   104,   105,     3,   125,
      70,    41,    70,    70,   116,   120,    18,    42,    59,    60,
      61,    62,    64,    65,    74,    80,    81,    85,   111,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,    25,   152,     3,    76,   120,   152,
     156,    88,   201,   202,    73,   232,     1,   109,   110,   111,
      43,   121,   122,    57,    90,    70,    93,    70,    71,     1,
      30,    35,   106,   136,   153,   203,   204,   206,   209,   211,
     212,   213,   230,   232,    17,    32,    33,    71,   106,   126,
     127,   128,   129,   132,   232,    74,   232,   232,    70,   117,
     138,    77,    78,    79,    67,    68,    80,    81,    82,    83,
      84,   147,    64,    87,    87,   138,    73,    73,    87,   152,
     202,   232,    73,   232,    40,   111,   123,   124,    59,    93,
      71,   104,    69,    69,    69,     6,    69,    69,     1,   232,
      49,   111,   157,   198,   199,   214,   225,    72,   107,   108,
       3,   155,    74,     1,   155,   182,   183,   232,     3,   195,
     196,   208,   105,    70,    75,   140,   141,   142,   143,   143,
     144,   144,   145,   145,   145,   138,   152,    89,   161,   232,
     109,    73,    57,    71,    71,   225,    74,   215,     3,   109,
     160,     1,    21,   133,   134,   135,   160,    71,   183,   232,
      25,   111,   165,   174,   176,   186,   193,    71,    73,   232,
      71,   183,    71,   105,    87,    87,   232,   110,   124,     3,
     162,   205,     1,   232,   210,    69,    75,   225,    75,    73,
      69,   232,   232,   232,    71,   232,   232,    73,    75,    21,
      22,    31,    75,   216,   217,   218,   232,   131,     3,   130,
     135,    75,   196,   232,    73,    75,   232,   225,    34,   219,
     220,    69,   219,   232,   232,   162,   232,     3,    74,    11,
     222,   223,   232,    69,    70,   232,   217,   111,   221,    74,
     232,   232,   232,   232,    73,    75,   149,   224,     8,    13,
     187,   188,   189,   190,   191,   232,   232,    73,    75,   138,
      72,    71,   189,   232,   155,   191,   192,   111,   232,    72,
     232,   232,   161,    69,   232,   149,   232,   232
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    91,    92,    92,    93,    93,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    95,    96,    95,    95,    97,
      98,    98,    99,   100,    99,    99,   101,   102,   103,   103,
     103,   104,   105,   105,   106,   106,   106,   106,   106,   106,
     106,   107,   107,   108,   109,   109,   109,   110,   111,   111,
     111,   112,   112,   112,   112,   113,   113,   114,   116,   115,
     117,   115,   115,   118,   119,   119,   119,   119,   120,   120,
     120,   121,   122,   122,   123,   123,   124,   125,   125,   126,
     126,   126,   127,   128,   128,   130,   129,   131,   129,   132,
     133,   133,   134,   134,   135,   136,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   138,   139,   139,   140,
     140,   141,   141,   142,   142,   142,   143,   143,   143,   144,
     144,   144,   144,   145,   145,   146,   146,   146,   147,   147,
     147,   148,   148,   148,   148,   148,   148,   148,   148,   149,
     149,   150,   150,   151,   151,   152,   153,   153,   153,   153,
     153,   153,   154,   155,   155,   156,   156,   156,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   158,   158,   158,
     158,   159,   159,   159,   160,   160,   161,   161,   162,   163,
     164,   164,   164,   165,   165,   166,   166,   166,   167,   168,
     169,   170,   170,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   180,   181,   182,   182,   183,   183,
     184,   184,   185,   186,   186,   186,   186,   186,   187,   188,
     188,   189,   190,   190,   191,   191,   192,   193,   193,   194,
     195,   195,   196,   197,   197,   198,   198,   199,   199,   200,
     201,   201,   202,   203,   204,   204,   205,   205,   206,   206,
     207,   208,   208,   210,   209,   209,   211,   212,   212,   213,
     214,   214,   215,   215,   215,   216,   216,   217,   218,   218,
     218,   219,   219,   220,   221,   221,   222,   222,   223,   224,
     224,   225,   225,   225,   225,   226,   227,   228,   229,   229,
     230,   230,   230,   230,   231,   231,   232,   232,   233,   233,
     234,   234,   235,   235,   236,   237,   238,   238
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     2,     2,     2,     2,
       2,     2,     1,     1,     1,     5,     0,     6,     2,     2,
       1,     1,     4,     0,     6,     2,     3,     5,     0,     1,
       1,     1,     0,     2,     2,     2,     2,     2,     2,     1,
       1,     0,     1,     2,     2,     5,     1,     1,     1,     2,
       3,     1,     1,     1,     1,     2,     3,     3,     0,     7,
       0,     8,     1,     4,     3,     4,     2,     3,     4,     2,
       2,     2,     0,     1,     1,     3,     1,     0,     2,     1,
       1,     1,     4,     1,     1,     0,     7,     0,     6,     2,
       0,     1,     1,     3,     3,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     1,     3,     1,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     2,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     2,     1,     1,     1,     2,     1,     1,     1,
       2,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     5,     1,     1,     1,     1,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     1,     1,     2,     2,     3,     1,     1,     1,
       1,     1,     1,     6,     2,     2,     2,     3,     3,     1,
      13,     2,     2,     1,     1,     1,     1,     1,     1,     2,
       3,     3,     1,     2,     4,     3,     2,     6,     2,     2,
       2,     5,     1,     6,     4,     4,     1,     4,     1,     2,
       1,     2,     3,     4,     0,     1,     2,     5,     6,     2,
       2,     0,     2,     0,     7,     2,     3,     0,     1,     1,
       1,     1,     4,     3,     3,     2,     5,     3,     1,     1,
       1,     0,     2,     4,     2,     5,     0,     2,     4,     2,
       5,     1,     1,     1,     1,     6,     1,     1,     2,     2,
       1,     1,     1,     1,     1,     2,     0,     1,     3,     2,
       4,     3,     6,     3,     3,     3,     1,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 408 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = 0; }
#line 1971 "y.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 409 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.decl_val) = (yyvsp[0].decl_val);
      AST::tree()->setDeclarations((yyvsp[0].decl_val));
    }
#line 1980 "y.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 416 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].decl_val); }
#line 1986 "y.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 417 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-1].decl_val)) { (yyvsp[-1].decl_val)->append((yyvsp[0].decl_val)); (yyval.decl_val) = (yyvsp[-1].decl_val); }
      else (yyval.decl_val) = (yyvsp[0].decl_val);
    }
#line 1995 "y.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 424 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].decl_val); }
#line 2001 "y.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 425 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].const_val); }
#line 2007 "y.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 426 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].exception_val); }
#line 2013 "y.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 427 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].decl_val); }
#line 2019 "y.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 428 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].module_val); }
#line 2025 "y.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 429 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].value_base_val); }
#line 2031 "y.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 430 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = 0; }
#line 2037 "y.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 431 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = 0; }
#line 2043 "y.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 432 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno, "Syntax error in definition");
      (yyval.decl_val) = 0;
    }
#line 2052 "y.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 439 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-4].module_val)->finishConstruction((yyvsp[-1].decl_val));
      (yyval.module_val) = (yyvsp[-4].module_val);
    }
#line 2061 "y.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 443 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in module definition");
    }
#line 2070 "y.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 446 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-5].module_val)->finishConstruction((yyvsp[-1].decl_val));
      (yyval.module_val) = (yyvsp[-5].module_val);
    }
#line 2079 "y.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 450 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in module definition (no body found)");
      (yyvsp[-1].module_val)->finishConstruction(0);
      (yyval.module_val) = (yyvsp[-1].module_val);
    }
#line 2090 "y.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 459 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.module_val) = new Module(currentFile, yylineno, mainFile, (yyvsp[0].id_val)); }
#line 2096 "y.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 463 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].interface_val); }
#line 2102 "y.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 464 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].forward_val); }
#line 2108 "y.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 468 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-3].interface_val)->finishConstruction((yyvsp[-1].decl_val));
      (yyval.interface_val) = (yyvsp[-3].interface_val);
    }
#line 2117 "y.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 472 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in interface definition");
    }
#line 2126 "y.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 475 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-5].interface_val)->finishConstruction((yyvsp[-1].decl_val));
      (yyval.interface_val) = (yyvsp[-5].interface_val);
    }
#line 2135 "y.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 479 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in interface definition (no body found)");
      (yyvsp[-1].interface_val)->finishConstruction(0);
      (yyval.interface_val) = (yyvsp[-1].interface_val);
    }
#line 2146 "y.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 488 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.forward_val) = new Forward(currentFile, yylineno, mainFile, (yyvsp[0].id_val), (yyvsp[-2].int_val)==1, (yyvsp[-2].int_val)==2);
    }
#line 2154 "y.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 495 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.interface_val) = new Interface(currentFile, yylineno, mainFile,
			 (yyvsp[-2].id_val), (yyvsp[-4].int_val)==1, (yyvsp[-4].int_val)==2, (yyvsp[0].inheritspec_val));
    }
#line 2163 "y.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 502 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.int_val) = 0; }
#line 2169 "y.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 503 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.int_val) = 1; }
#line 2175 "y.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 504 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.int_val) = 2; }
#line 2181 "y.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 508 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].decl_val); }
#line 2187 "y.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 512 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = 0; }
#line 2193 "y.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 513 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-1].decl_val)) { (yyvsp[-1].decl_val)->append((yyvsp[0].decl_val)); (yyval.decl_val) = (yyvsp[-1].decl_val); }
      else (yyval.decl_val) = (yyvsp[0].decl_val);
    }
#line 2202 "y.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 520 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].decl_val); }
#line 2208 "y.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 521 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].const_val); }
#line 2214 "y.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 522 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].exception_val); }
#line 2220 "y.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 523 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].attribute_val); }
#line 2226 "y.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 524 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[-1].operation_val); }
#line 2232 "y.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 525 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = 0; }
#line 2238 "y.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 526 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno, "Syntax error in interface body");
      (yyval.decl_val) = 0;
    }
#line 2247 "y.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 533 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.inheritspec_val) = 0; }
#line 2253 "y.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 534 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.inheritspec_val) = (yyvsp[0].inheritspec_val); }
#line 2259 "y.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 538 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.inheritspec_val) = (yyvsp[0].inheritspec_val); }
#line 2265 "y.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 542 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.inheritspec_val) = new InheritSpec((yyvsp[-1].scopedname_val), currentFile, yylineno);
      if (!(yyval.inheritspec_val)->interface()) {
	delete (yyval.inheritspec_val);
	(yyval.inheritspec_val) = 0;
      }
    }
#line 2277 "y.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 549 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-4].inheritspec_val)) {
	(yyvsp[-4].inheritspec_val)->append(new InheritSpec((yyvsp[-1].scopedname_val), currentFile, yylineno),
		   currentFile, yylineno);
	(yyval.inheritspec_val) = (yyvsp[-4].inheritspec_val);
      }
      else (yyval.inheritspec_val) = new InheritSpec((yyvsp[-1].scopedname_val), currentFile, yylineno);
    }
#line 2290 "y.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 557 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in inheritance list");
      (yyval.inheritspec_val) = 0;
    }
#line 2300 "y.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 565 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.scopedname_val) = (yyvsp[0].scopedname_val); }
#line 2306 "y.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 569 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.scopedname_val) = new ScopedName((yyvsp[0].id_val), 0);
    }
#line 2314 "y.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 572 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.scopedname_val) = new ScopedName((yyvsp[0].id_val), 1);
    }
#line 2322 "y.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 575 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-2].scopedname_val)->append((yyvsp[0].id_val));
      (yyval.scopedname_val)=(yyvsp[-2].scopedname_val);
    }
#line 2331 "y.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 586 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.value_base_val) = (yyvsp[0].value_val); }
#line 2337 "y.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 587 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.value_base_val) = (yyvsp[0].value_abs_val); }
#line 2343 "y.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 588 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.value_base_val) = (yyvsp[0].value_box_val); }
#line 2349 "y.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 589 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.value_base_val) = (yyvsp[0].value_forward_val); }
#line 2355 "y.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 593 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.value_forward_val) = new ValueForward(currentFile, yylineno, mainFile, 0, (yyvsp[0].id_val));
    }
#line 2363 "y.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 596 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.value_forward_val) = new ValueForward(currentFile, yylineno, mainFile, 1, (yyvsp[0].id_val));
    }
#line 2371 "y.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 602 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.value_box_val) = new ValueBox(currentFile, yylineno, mainFile,
			(yyvsp[-1].id_val), (yyvsp[0].type_spec_val)->type(), (yyvsp[0].type_spec_val)->constr());
      delete (yyvsp[0].type_spec_val);
    }
#line 2381 "y.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 610 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      valueabs_hack = new ValueAbs(currentFile, yylineno, mainFile, (yyvsp[0].id_val), 0, 0);
    }
#line 2389 "y.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 612 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      valueabs_hack->finishConstruction((yyvsp[-1].decl_val));
      (yyval.value_abs_val) = valueabs_hack;
      valueabs_hack = 0;
    }
#line 2399 "y.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 617 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      valueabs_hack = new ValueAbs(currentFile, yylineno, mainFile, (yyvsp[-1].id_val),
				   (yyvsp[0].valueinheritsupportspec_val)->inherits(), (yyvsp[0].valueinheritsupportspec_val)->supports());
      delete (yyvsp[0].valueinheritsupportspec_val);
    }
#line 2409 "y.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 621 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      valueabs_hack->finishConstruction((yyvsp[-1].decl_val));
      (yyval.value_abs_val) = valueabs_hack;
      valueabs_hack = 0;
    }
#line 2419 "y.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 626 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in abstract valuetype");
      if (valueabs_hack) {
	valueabs_hack->finishConstruction(0);
	(yyval.value_abs_val) = valueabs_hack;
	valueabs_hack = 0;
      }
      else (yyval.value_abs_val) = 0;
    }
#line 2434 "y.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 639 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-3].value_val)->finishConstruction((yyvsp[-1].decl_val));
      (yyval.value_val) = (yyvsp[-3].value_val);
    }
#line 2443 "y.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 646 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.value_val) = new Value(currentFile, yylineno, mainFile, 0, (yyvsp[-1].id_val),
		     (yyvsp[0].valueinheritsupportspec_val)->inherits(), (yyvsp[0].valueinheritsupportspec_val)->supports());
      delete (yyvsp[0].valueinheritsupportspec_val);
    }
#line 2453 "y.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 651 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.value_val) = new Value(currentFile, yylineno, mainFile, 1, (yyvsp[-1].id_val),
		     (yyvsp[0].valueinheritsupportspec_val)->inherits(), (yyvsp[0].valueinheritsupportspec_val)->supports());
      delete (yyvsp[0].valueinheritsupportspec_val);
    }
#line 2463 "y.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 656 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.value_val) = new Value(currentFile, yylineno, mainFile, 0, (yyvsp[0].id_val), 0, 0);
    }
#line 2471 "y.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 659 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.value_val) = new Value(currentFile, yylineno, mainFile, 1, (yyvsp[0].id_val), 0, 0);
    }
#line 2479 "y.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 665 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.valueinheritsupportspec_val) = new ValueInheritSupportSpec((yyvsp[-2].valueinheritspec_val), (yyvsp[0].inheritspec_val));
    }
#line 2487 "y.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 668 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.valueinheritsupportspec_val) = new ValueInheritSupportSpec((yyvsp[0].valueinheritspec_val), 0);
    }
#line 2495 "y.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 671 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.valueinheritsupportspec_val) = new ValueInheritSupportSpec(0, (yyvsp[0].inheritspec_val));
    }
#line 2503 "y.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 677 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-1].boolean_val)) (yyvsp[0].valueinheritspec_val)->setTruncatable();
      (yyval.valueinheritspec_val) = (yyvsp[0].valueinheritspec_val);
    }
#line 2512 "y.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 684 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.boolean_val) = 0; }
#line 2518 "y.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 685 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.boolean_val) = 1; }
#line 2524 "y.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 689 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.valueinheritspec_val) = new ValueInheritSpec((yyvsp[0].scopedname_val), currentFile, yylineno);
      if (!(yyval.valueinheritspec_val)->value()) {
	delete (yyval.valueinheritspec_val);
	(yyval.valueinheritspec_val) = 0;
      }
    }
#line 2536 "y.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 696 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-2].valueinheritspec_val)) {
	(yyvsp[-2].valueinheritspec_val)->append(new ValueInheritSpec((yyvsp[0].scopedname_val), currentFile, yylineno),
		   currentFile, yylineno);
	(yyval.valueinheritspec_val) = (yyvsp[-2].valueinheritspec_val);
      }
      else (yyval.valueinheritspec_val) = new ValueInheritSpec((yyvsp[0].scopedname_val), currentFile, yylineno);
    }
#line 2549 "y.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 707 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.scopedname_val) = (yyvsp[0].scopedname_val); }
#line 2555 "y.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 711 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = 0; }
#line 2561 "y.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 712 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-1].decl_val)) { (yyvsp[-1].decl_val)->append((yyvsp[0].decl_val)); (yyval.decl_val) = (yyvsp[-1].decl_val); }
      else (yyval.decl_val) = (yyvsp[0].decl_val);
    }
#line 2570 "y.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 719 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].decl_val); }
#line 2576 "y.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 720 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].statemember_val); }
#line 2582 "y.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 721 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].factory_val); }
#line 2588 "y.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 725 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.statemember_val) = new StateMember(currentFile, yylineno, mainFile,
			   (yyvsp[-3].ulong_val), (yyvsp[-2].type_spec_val)->type(), (yyvsp[-2].type_spec_val)->constr(), (yyvsp[-1].declarator_val));
      delete (yyvsp[-2].type_spec_val);
    }
#line 2598 "y.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 733 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.ulong_val) = 0; }
#line 2604 "y.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 734 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.ulong_val) = 1; }
#line 2610 "y.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 738 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-3].factory_val)->closeParens();
    }
#line 2618 "y.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 740 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-6].factory_val)->finishConstruction((yyvsp[-4].parameter_val), (yyvsp[-1].raisesspec_val));
      (yyval.factory_val) = (yyvsp[-6].factory_val);
    }
#line 2627 "y.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 744 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-3].factory_val)->closeParens();
    }
#line 2635 "y.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 746 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in factory parameters");
      (yyvsp[-5].factory_val)->finishConstruction(0, 0);
      (yyval.factory_val) = (yyvsp[-5].factory_val);
    }
#line 2646 "y.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 755 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.factory_val) = new Factory(currentFile, yylineno, mainFile, (yyvsp[0].id_val));
    }
#line 2654 "y.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 761 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.parameter_val) = 0; }
#line 2660 "y.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 762 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.parameter_val) = (yyvsp[0].parameter_val); }
#line 2666 "y.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 766 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.parameter_val) = (yyvsp[0].parameter_val); }
#line 2672 "y.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 767 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-2].parameter_val)) { (yyvsp[-2].parameter_val)->append((yyvsp[0].parameter_val)); (yyval.parameter_val) = (yyvsp[-2].parameter_val); }
      else (yyval.parameter_val) = (yyvsp[0].parameter_val);
    }
#line 2681 "y.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 774 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.parameter_val) = new Parameter(currentFile, yylineno, mainFile, 0, (yyvsp[-1].type_val), (yyvsp[0].id_val));
    }
#line 2689 "y.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 782 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.const_val) = new Const(currentFile, yylineno, mainFile, (yyvsp[-3].type_val), (yyvsp[-2].id_val), (yyvsp[0].expr_val));
    }
#line 2697 "y.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 788 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 2703 "y.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 789 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 2709 "y.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 790 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 2715 "y.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 791 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 2721 "y.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 792 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 2727 "y.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 793 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 2733 "y.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 794 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 2739 "y.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 795 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 2745 "y.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 796 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.type_val) = IdlType::scopedNameToType(currentFile, yylineno, (yyvsp[0].scopedname_val));
    }
#line 2753 "y.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 799 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 2759 "y.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 803 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = (yyvsp[0].expr_val); }
#line 2765 "y.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 807 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = (yyvsp[0].expr_val); }
#line 2771 "y.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 808 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = new OrExpr(currentFile, yylineno, (yyvsp[-2].expr_val), (yyvsp[0].expr_val)); }
#line 2777 "y.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 812 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = (yyvsp[0].expr_val); }
#line 2783 "y.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 813 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = new XorExpr(currentFile, yylineno, (yyvsp[-2].expr_val), (yyvsp[0].expr_val)); }
#line 2789 "y.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 817 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = (yyvsp[0].expr_val); }
#line 2795 "y.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 818 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new AndExpr(currentFile, yylineno, (yyvsp[-2].expr_val), (yyvsp[0].expr_val));
    }
#line 2803 "y.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 824 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = (yyvsp[0].expr_val); }
#line 2809 "y.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 825 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
    (yyval.expr_val) = new RShiftExpr(currentFile, yylineno, (yyvsp[-2].expr_val), (yyvsp[0].expr_val));
  }
#line 2817 "y.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 828 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
    (yyval.expr_val) = new LShiftExpr(currentFile, yylineno, (yyvsp[-2].expr_val), (yyvsp[0].expr_val));
  }
#line 2825 "y.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 834 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = (yyvsp[0].expr_val); }
#line 2831 "y.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 835 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = new AddExpr(currentFile, yylineno, (yyvsp[-2].expr_val), (yyvsp[0].expr_val)); }
#line 2837 "y.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 836 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = new SubExpr(currentFile, yylineno, (yyvsp[-2].expr_val), (yyvsp[0].expr_val)); }
#line 2843 "y.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 840 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = (yyvsp[0].expr_val); }
#line 2849 "y.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 841 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new MultExpr(currentFile, yylineno, (yyvsp[-2].expr_val), (yyvsp[0].expr_val));
    }
#line 2857 "y.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 844 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new DivExpr(currentFile, yylineno, (yyvsp[-2].expr_val), (yyvsp[0].expr_val));
    }
#line 2865 "y.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 847 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new ModExpr(currentFile, yylineno, (yyvsp[-2].expr_val), (yyvsp[0].expr_val));
    }
#line 2873 "y.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 853 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-1].char_val) == '-') (yyval.expr_val) = new MinusExpr(currentFile, yylineno, (yyvsp[0].expr_val));
      if ((yyvsp[-1].char_val) == '+') (yyval.expr_val) = new PlusExpr(currentFile, yylineno, (yyvsp[0].expr_val));
      if ((yyvsp[-1].char_val) == '~') (yyval.expr_val) = new InvertExpr(currentFile, yylineno, (yyvsp[0].expr_val));
    }
#line 2883 "y.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 858 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = (yyvsp[0].expr_val); }
#line 2889 "y.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 862 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.char_val) = '-'; }
#line 2895 "y.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 863 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.char_val) = '+'; }
#line 2901 "y.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 864 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.char_val) = '~'; }
#line 2907 "y.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 868 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = IdlExpr::scopedNameToExpr(currentFile, yylineno, (yyvsp[0].scopedname_val));
    }
#line 2915 "y.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 871 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = (yyvsp[0].expr_val); }
#line 2921 "y.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 872 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.expr_val) = (yyvsp[-1].expr_val); }
#line 2927 "y.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 876 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new IntegerExpr(currentFile, yylineno, (yyvsp[0].int_literal_val));
    }
#line 2935 "y.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 879 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new StringExpr(currentFile, yylineno, (yyvsp[0].string_val));
    }
#line 2943 "y.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 882 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new WStringExpr(currentFile, yylineno, (yyvsp[0].wstring_val));
    }
#line 2951 "y.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 885 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new CharExpr(currentFile, yylineno, (yyvsp[0].char_val));
    }
#line 2959 "y.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 888 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new WCharExpr(currentFile, yylineno, (yyvsp[0].wchar_val));
    }
#line 2967 "y.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 891 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new FixedExpr(currentFile, yylineno, (yyvsp[0].fixed_val));
    }
#line 2975 "y.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 894 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new FloatExpr(currentFile, yylineno, (yyvsp[0].float_literal_val));
    }
#line 2983 "y.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 897 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.expr_val) = new BooleanExpr(currentFile, yylineno, (yyvsp[0].boolean_val));
    }
#line 2991 "y.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 903 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.string_val) = (yyvsp[0].string_val); }
#line 2997 "y.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 904 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.string_val) = new char [strlen((yyvsp[-1].string_val)) + strlen((yyvsp[0].string_val)) + 1];
      strcpy((yyval.string_val), (yyvsp[-1].string_val));
      strcat((yyval.string_val), (yyvsp[0].string_val));
      delete [] (yyvsp[-1].string_val);
      delete [] (yyvsp[0].string_val);
    }
#line 3009 "y.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 914 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.wstring_val) = (yyvsp[0].wstring_val); }
#line 3015 "y.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 915 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.wstring_val) = new IDL_WChar [idl_wstrlen((yyvsp[-1].wstring_val)) + idl_wstrlen((yyvsp[0].wstring_val)) + 1];
      idl_wstrcpy((yyval.wstring_val), (yyvsp[-1].wstring_val));
      idl_wstrcat((yyval.wstring_val), (yyvsp[0].wstring_val));
      delete [] (yyvsp[-1].wstring_val);
      delete [] (yyvsp[0].wstring_val);
    }
#line 3027 "y.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 925 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.boolean_val) = 1; }
#line 3033 "y.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 926 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.boolean_val) = 0; }
#line 3039 "y.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 930 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlLongVal v = (yyvsp[0].expr_val)->evalAsLongV();
      if (v.negative || v.u == 0)
	IdlError(currentFile, yylineno, "Size must be at least 1");
      (yyval.ulong_val) = v.u;
    }
#line 3050 "y.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 939 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].typedef_val); }
#line 3056 "y.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 940 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].struct_val); }
#line 3062 "y.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 941 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].union_val); }
#line 3068 "y.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 942 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].enum_val); }
#line 3074 "y.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 943 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.decl_val) = new Native(currentFile, yylineno, mainFile, (yyvsp[0].id_val));
    }
#line 3082 "y.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 946 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.decl_val) = (yyvsp[0].decl_val); }
#line 3088 "y.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 950 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.typedef_val) = new Typedef(currentFile, yylineno, mainFile,
		       (yyvsp[-1].type_spec_val)->type(), (yyvsp[-1].type_spec_val)->constr(), (yyvsp[0].declarator_val));
      delete (yyvsp[-1].type_spec_val);
    }
#line 3098 "y.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 958 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_spec_val) = new TypeSpec((yyvsp[0].type_val), 0); }
#line 3104 "y.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 959 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_spec_val) = new TypeSpec((yyvsp[0].type_val), 1); }
#line 3110 "y.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 963 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3116 "y.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 964 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3122 "y.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 965 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.type_val) = IdlType::scopedNameToType(currentFile, yylineno, (yyvsp[0].scopedname_val));
    }
#line 3130 "y.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 971 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3136 "y.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 972 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3142 "y.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 973 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3148 "y.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 974 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3154 "y.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 975 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3160 "y.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 976 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3166 "y.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 977 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3172 "y.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 978 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3178 "y.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 979 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3184 "y.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 983 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3190 "y.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 984 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3196 "y.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 985 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3202 "y.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 986 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3208 "y.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 990 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].struct_val)->thisType(); }
#line 3214 "y.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 991 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].union_val)->thisType(); }
#line 3220 "y.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 992 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].enum_val)->thisType(); }
#line 3226 "y.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 996 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.declarator_val) = (yyvsp[-1].declarator_val); }
#line 3232 "y.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 997 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-4].declarator_val)) { (yyvsp[-4].declarator_val)->append((yyvsp[-1].declarator_val)); (yyval.declarator_val) = (yyvsp[-4].declarator_val); }
      else (yyval.declarator_val) = (yyvsp[-1].declarator_val);
    }
#line 3241 "y.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1004 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.declarator_val) = (yyvsp[0].declarator_val); }
#line 3247 "y.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1005 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.declarator_val) = (yyvsp[0].declarator_val); }
#line 3253 "y.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1009 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.declarator_val) = new Declarator(currentFile, yylineno, mainFile, (yyvsp[0].id_val), 0);
    }
#line 3261 "y.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1015 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.declarator_val) = (yyvsp[0].declarator_val); }
#line 3267 "y.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1019 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::floatType; }
#line 3273 "y.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1020 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::doubleType; }
#line 3279 "y.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1021 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::longdoubleType; }
#line 3285 "y.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1025 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3291 "y.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1026 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3297 "y.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1030 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3303 "y.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1031 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3309 "y.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1032 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3315 "y.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1036 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::shortType; }
#line 3321 "y.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 1040 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::longType; }
#line 3327 "y.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 1044 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::longlongType; }
#line 3333 "y.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 1048 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3339 "y.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 1049 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3345 "y.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 1050 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3351 "y.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1054 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::ushortType; }
#line 3357 "y.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1058 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::ulongType; }
#line 3363 "y.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 1062 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::ulonglongType; }
#line 3369 "y.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 1066 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::charType; }
#line 3375 "y.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 1070 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::wcharType; }
#line 3381 "y.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 1074 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::booleanType; }
#line 3387 "y.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 1078 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::octetType; }
#line 3393 "y.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 1082 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::anyType; }
#line 3399 "y.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 1086 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = DeclaredType::corbaObjectType; }
#line 3405 "y.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 1090 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-5].struct_val)->finishConstruction((yyvsp[-1].member_val));
      (yyval.struct_val) = (yyvsp[-5].struct_val);
    }
#line 3414 "y.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 1094 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in struct definition");
      (yyvsp[-1].struct_val)->finishConstruction(0);
      (yyval.struct_val) = (yyvsp[-1].struct_val);
    }
#line 3425 "y.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 1103 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.struct_val) = new Struct(currentFile, yylineno, mainFile, (yyvsp[0].id_val));
    }
#line 3433 "y.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 1109 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.member_val) = (yyvsp[-1].member_val); }
#line 3439 "y.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 1110 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-2].member_val)) { (yyvsp[-2].member_val)->append((yyvsp[-1].member_val)); (yyval.member_val) = (yyvsp[-2].member_val); }
      else (yyval.member_val) = (yyvsp[-1].member_val);
    }
#line 3448 "y.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 1117 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.member_val) = new Member(currentFile, yylineno, mainFile,
		      (yyvsp[-2].type_spec_val)->type(), (yyvsp[-2].type_spec_val)->constr(), (yyvsp[-1].declarator_val));
      delete (yyvsp[-2].type_spec_val);
    }
#line 3458 "y.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 1122 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in member declaration");
      (yyval.member_val) = 0;
    }
#line 3468 "y.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 1132 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {

      (yyvsp[-12].union_val)->finishConstruction((yyvsp[-7].type_spec_val)->type(), (yyvsp[-7].type_spec_val)->constr(), (yyvsp[-1].union_case_val));
      delete (yyvsp[-7].type_spec_val);
      (yyval.union_val) = (yyvsp[-12].union_val);
    }
#line 3479 "y.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 1138 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in union declaration");
      (yyvsp[-1].union_val)->finishConstruction(0, 0, 0);
      (yyval.union_val) = (yyvsp[-1].union_val);
    }
#line 3490 "y.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 1147 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.union_val) = new Union(currentFile, yylineno, mainFile, (yyvsp[0].id_val));
    }
#line 3498 "y.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 1153 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_spec_val) = new TypeSpec((yyvsp[0].type_val), 0); }
#line 3504 "y.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 1154 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_spec_val) = new TypeSpec((yyvsp[0].type_val), 0); }
#line 3510 "y.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 1155 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_spec_val) = new TypeSpec((yyvsp[0].type_val), 0); }
#line 3516 "y.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 1156 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_spec_val) = new TypeSpec((yyvsp[0].enum_val)->thisType(), 1); }
#line 3522 "y.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 1157 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.type_spec_val) = new TypeSpec(IdlType::scopedNameToType(currentFile, yylineno, (yyvsp[0].scopedname_val)),
			0);
    }
#line 3531 "y.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 1164 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.union_case_val) = (yyvsp[0].union_case_val); }
#line 3537 "y.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 1168 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.union_case_val) = (yyvsp[-1].union_case_val); }
#line 3543 "y.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 1169 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-2].union_case_val)->append((yyvsp[-1].union_case_val));
      (yyval.union_case_val) = (yyvsp[-2].union_case_val);
    }
#line 3552 "y.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 1176 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-1].union_case_val)->finishConstruction((yyvsp[-2].case_label_val));
      (yyval.union_case_val) = (yyvsp[-1].union_case_val);
    }
#line 3561 "y.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 1183 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.case_label_val) = (yyvsp[0].case_label_val); }
#line 3567 "y.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 1184 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-1].case_label_val)->append((yyvsp[0].case_label_val));
      (yyval.case_label_val) = (yyvsp[-1].case_label_val);
    }
#line 3576 "y.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 1191 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.case_label_val) = new CaseLabel(currentFile, yylineno, mainFile, (yyvsp[-2].expr_val));
    }
#line 3584 "y.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 1194 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.case_label_val) = new CaseLabel(currentFile, yylineno, mainFile, 0);
    }
#line 3592 "y.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 1200 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.union_case_val) = new UnionCase(currentFile, yylineno, mainFile,
			 (yyvsp[-1].type_spec_val)->type(), (yyvsp[-1].type_spec_val)->constr(), (yyvsp[0].declarator_val));
    }
#line 3601 "y.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 1207 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-5].enum_val)->finishConstruction((yyvsp[-1].enumerator_val));
      (yyval.enum_val) = (yyvsp[-5].enum_val);
    }
#line 3610 "y.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 1211 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno, "Syntax error in enum definition");
      (yyvsp[-1].enum_val)->finishConstruction(0);
      (yyval.enum_val) = (yyvsp[-1].enum_val);
    }
#line 3620 "y.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 1219 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.enum_val) = new Enum(currentFile, yylineno, mainFile, (yyvsp[0].id_val));
    }
#line 3628 "y.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 1225 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.enumerator_val) = (yyvsp[-1].enumerator_val); }
#line 3634 "y.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 1226 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-4].enumerator_val)->append((yyvsp[-1].enumerator_val));
      (yyval.enumerator_val) = (yyvsp[-4].enumerator_val);
    }
#line 3643 "y.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 1233 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.enumerator_val) = new Enumerator(currentFile, yylineno, mainFile, (yyvsp[0].id_val));
    }
#line 3651 "y.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 1239 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.type_val) = new SequenceType((yyvsp[-3].type_val), (yyvsp[-1].ulong_val));
    }
#line 3659 "y.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 1242 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.type_val) = new SequenceType((yyvsp[-1].type_val), 0);
    }
#line 3667 "y.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 1248 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = new StringType((yyvsp[-1].ulong_val)); }
#line 3673 "y.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 1249 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.type_val) = StringType::unboundedStringType;
    }
#line 3681 "y.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 1255 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = new WStringType((yyvsp[-1].ulong_val)); }
#line 3687 "y.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 1256 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.type_val) = WStringType::unboundedWStringType;
    }
#line 3695 "y.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 1262 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.declarator_val) = new Declarator(currentFile, yylineno, mainFile, (yyvsp[-1].id_val), (yyvsp[0].array_size_val));
    }
#line 3703 "y.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 1268 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.array_size_val) = (yyvsp[0].array_size_val); }
#line 3709 "y.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 1269 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-1].array_size_val)->append((yyvsp[0].array_size_val));
      (yyval.array_size_val) = (yyvsp[-1].array_size_val);
    }
#line 3718 "y.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 1276 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.array_size_val) = new ArraySize((yyvsp[-1].ulong_val)); }
#line 3724 "y.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 1280 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.attribute_val) = new Attribute(currentFile, yylineno, mainFile, (yyvsp[-3].boolean_val), (yyvsp[-1].type_val), (yyvsp[0].declarator_val));
    }
#line 3732 "y.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 1286 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.boolean_val) = 0; }
#line 3738 "y.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1287 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.boolean_val) = 1; }
#line 3744 "y.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 1291 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.declarator_val) = (yyvsp[-1].declarator_val); }
#line 3750 "y.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1292 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-4].declarator_val)) { (yyvsp[-4].declarator_val)->append((yyvsp[-1].declarator_val)); (yyval.declarator_val) = (yyvsp[-4].declarator_val); }
      else (yyval.declarator_val) = (yyvsp[-1].declarator_val);
    }
#line 3759 "y.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 1299 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-5].exception_val)->finishConstruction((yyvsp[-1].member_val));
      (yyval.exception_val) = (yyvsp[-5].exception_val);
    }
#line 3768 "y.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 1303 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in exception definition");
      (yyvsp[-1].exception_val)->finishConstruction(0);
      (yyval.exception_val) = (yyvsp[-1].exception_val);
    }
#line 3779 "y.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 1312 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.exception_val) = new Exception(currentFile, yylineno, mainFile, (yyvsp[0].id_val));
    }
#line 3787 "y.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 1318 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.member_val) = 0; }
#line 3793 "y.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 1319 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-1].member_val)) { (yyvsp[-1].member_val)->append((yyvsp[0].member_val));	(yyval.member_val) = (yyvsp[-1].member_val); }
      else (yyval.member_val) = (yyvsp[0].member_val);
    }
#line 3802 "y.tab.c" /* yacc.c:1646  */
    break;

  case 253:
#line 1326 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-2].operation_val)->closeParens();
    }
#line 3810 "y.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 1328 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-6].operation_val)->finishConstruction((yyvsp[-4].parameter_val), (yyvsp[-1].raisesspec_val), (yyvsp[0].contextspec_val));
      (yyval.operation_val) = (yyvsp[-6].operation_val);
    }
#line 3819 "y.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 1332 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in operation declaration");
      (yyvsp[-1].operation_val)->closeParens();
      (yyvsp[-1].operation_val)->finishConstruction(0, 0, 0);
      (yyval.operation_val) = (yyvsp[-1].operation_val);
    }
#line 3831 "y.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 1342 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.operation_val) = new Operation(currentFile, yylineno, mainFile, (yyvsp[-2].boolean_val), (yyvsp[-1].type_val), (yyvsp[0].id_val));
    }
#line 3839 "y.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 1348 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.boolean_val) = 0; }
#line 3845 "y.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 1349 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.boolean_val) = (yyvsp[0].boolean_val); }
#line 3851 "y.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 1353 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.boolean_val) = 1; }
#line 3857 "y.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 1357 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 3863 "y.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 1358 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = BaseType::voidType; }
#line 3869 "y.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 1362 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.parameter_val) = (yyvsp[-1].parameter_val); }
#line 3875 "y.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 1363 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.parameter_val) = 0; }
#line 3881 "y.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 1364 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in operation parameters");
      (yyval.parameter_val) = 0;
    }
#line 3891 "y.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 1372 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.parameter_val) = (yyvsp[-1].parameter_val); }
#line 3897 "y.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 1373 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[-4].parameter_val)) { (yyvsp[-4].parameter_val)->append((yyvsp[-1].parameter_val)); (yyval.parameter_val) = (yyvsp[-4].parameter_val); }
      else (yyval.parameter_val) = (yyvsp[-1].parameter_val);
    }
#line 3906 "y.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 1380 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.parameter_val) = new Parameter(currentFile, yylineno, mainFile, (yyvsp[-2].int_val), (yyvsp[-1].type_val), (yyvsp[0].id_val));
    }
#line 3914 "y.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 1386 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.int_val) = 0; }
#line 3920 "y.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 1387 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.int_val) = 1; }
#line 3926 "y.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 1388 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.int_val) = 2; }
#line 3932 "y.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 1392 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.raisesspec_val) = 0; }
#line 3938 "y.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 1393 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.raisesspec_val) = (yyvsp[-1].raisesspec_val); }
#line 3944 "y.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 1397 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.raisesspec_val) = (yyvsp[-1].raisesspec_val); }
#line 3950 "y.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 1401 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.raisesspec_val) = new RaisesSpec((yyvsp[-1].scopedname_val), currentFile, yylineno);
    }
#line 3958 "y.tab.c" /* yacc.c:1646  */
    break;

  case 275:
#line 1404 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-4].raisesspec_val)->append(new RaisesSpec((yyvsp[-1].scopedname_val), currentFile, yylineno));
      (yyval.raisesspec_val) = (yyvsp[-4].raisesspec_val);
    }
#line 3967 "y.tab.c" /* yacc.c:1646  */
    break;

  case 276:
#line 1411 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.contextspec_val) = 0; }
#line 3973 "y.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 1412 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.contextspec_val) = (yyvsp[-1].contextspec_val); }
#line 3979 "y.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 1416 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.contextspec_val) = (yyvsp[-1].contextspec_val); }
#line 3985 "y.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 1420 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.contextspec_val) = new ContextSpec((yyvsp[-1].string_val), currentFile, yylineno);
    }
#line 3993 "y.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 1423 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyvsp[-4].contextspec_val)->append(new ContextSpec((yyvsp[-1].string_val), currentFile, yylineno));
      (yyval.contextspec_val) = (yyvsp[-4].contextspec_val);
    }
#line 4002 "y.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 1430 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 4008 "y.tab.c" /* yacc.c:1646  */
    break;

  case 282:
#line 1431 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 4014 "y.tab.c" /* yacc.c:1646  */
    break;

  case 283:
#line 1432 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = (yyvsp[0].type_val); }
#line 4020 "y.tab.c" /* yacc.c:1646  */
    break;

  case 284:
#line 1433 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.type_val) = IdlType::scopedNameToType(currentFile, yylineno, (yyvsp[0].scopedname_val));
    }
#line 4028 "y.tab.c" /* yacc.c:1646  */
    break;

  case 285:
#line 1439 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlLongVal scalev = (yyvsp[-1].expr_val)->evalAsLongV();

      if (scalev.negative) {
	IdlError(currentFile, yylineno,
		 "Fixed point scale must be >= 0");
      }
      IDL_ULong scale = scalev.u;

      if ((yyvsp[-3].ulong_val) > 31) {
	IdlError(currentFile, yylineno,
		 "Fixed point values may not have more than 31 digits");
      }
      if (scale > (yyvsp[-3].ulong_val)) {
	IdlError(currentFile, yylineno,
		 "Fixed point scale factor is greater than "
		 "the number of digits");
      }
      (yyval.type_val) = new FixedType((yyvsp[-3].ulong_val), scale);
    }
#line 4053 "y.tab.c" /* yacc.c:1646  */
    break;

  case 286:
#line 1462 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.type_val) = new FixedType(0, 0);
    }
#line 4061 "y.tab.c" /* yacc.c:1646  */
    break;

  case 287:
#line 1468 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.type_val) = new DeclaredType(IdlType::tk_value, 0, 0); }
#line 4067 "y.tab.c" /* yacc.c:1646  */
    break;

  case 288:
#line 1472 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.decl_val) = new StructForward(currentFile, yylineno, mainFile, (yyvsp[0].id_val));
    }
#line 4075 "y.tab.c" /* yacc.c:1646  */
    break;

  case 289:
#line 1475 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.decl_val) = new UnionForward(currentFile, yylineno, mainFile, (yyvsp[0].id_val));
    }
#line 4083 "y.tab.c" /* yacc.c:1646  */
    break;

  case 298:
#line 1498 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      Prefix::setPrefix(idl_strdup((yyvsp[-1].string_val)));
    }
#line 4091 "y.tab.c" /* yacc.c:1646  */
    break;

  case 299:
#line 1501 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno, "Malformed #pragma prefix");
    }
#line 4099 "y.tab.c" /* yacc.c:1646  */
    break;

  case 300:
#line 1507 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      Decl* d = Decl::scopedNameToDecl(currentFile, yylineno, (yyvsp[-2].scopedname_val));
      if (d) DeclRepoId::setRepoId(d, (yyvsp[-1].string_val), currentFile, yylineno);
    }
#line 4108 "y.tab.c" /* yacc.c:1646  */
    break;

  case 301:
#line 1511 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno, "Malformed #pragma id");
    }
#line 4116 "y.tab.c" /* yacc.c:1646  */
    break;

  case 302:
#line 1517 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      Decl* d = Decl::scopedNameToDecl(currentFile, yylineno, (yyvsp[-4].scopedname_val));
      if (d) DeclRepoId::setVersion(d, (yyvsp[-3].int_literal_val), (yyvsp[-1].int_literal_val), currentFile, yylineno);
    }
#line 4125 "y.tab.c" /* yacc.c:1646  */
    break;

  case 303:
#line 1521 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      IdlSyntaxError(currentFile, yylineno, "Malformed #pragma version");
    }
#line 4133 "y.tab.c" /* yacc.c:1646  */
    break;

  case 304:
#line 1527 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      Pragma::add((yyvsp[-1].string_val), currentFile, yylineno-1);
    }
#line 4141 "y.tab.c" /* yacc.c:1646  */
    break;

  case 305:
#line 1533 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      Pragma::add((yyvsp[-1].string_val), currentFile, yylineno);
    }
#line 4149 "y.tab.c" /* yacc.c:1646  */
    break;

  case 306:
#line 1539 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    { (yyval.string_val) = (yyvsp[0].string_val); }
#line 4155 "y.tab.c" /* yacc.c:1646  */
    break;

  case 307:
#line 1540 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1646  */
    {
      (yyval.string_val) = new char [strlen((yyvsp[-1].string_val)) + strlen((yyvsp[0].string_val)) + 1];
      strcpy((yyval.string_val), (yyvsp[-1].string_val));
      strcat((yyval.string_val), (yyvsp[0].string_val));
      delete [] (yyvsp[-1].string_val);
      delete [] (yyvsp[0].string_val);
    }
#line 4167 "y.tab.c" /* yacc.c:1646  */
    break;


#line 4171 "y.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1549 "../../../../../src/tool/omniidl/cxx/idl.yy" /* yacc.c:1906  */

