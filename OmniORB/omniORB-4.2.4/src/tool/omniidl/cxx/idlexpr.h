// -*- c++ -*-
//                          Package   : omniidl
// idlexpr.h                Created on: 1999/10/18
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see http://www.gnu.org/licenses/
//
// Description:
//   
//   Expression tree and evaluator

#ifndef _idlexpr_h_
#define _idlexpr_h_

#include <idlutil.h>
#include <idlscope.h>
#include <idlfixed.h>

class Enumerator;
class Enum;
class Const;


struct IdlLongVal {
  IdlLongVal(IDL_ULong a) : negative(0), u(a) {}
  IdlLongVal(IDL_Long  a) : negative(0), s(a) { if (a<0) negative=1; }

  IDL_Boolean negative;
  union {
    IDL_ULong u;
    IDL_Long  s;
  };
};

#ifdef HAS_LongLong
struct IdlLongLongVal {
  IdlLongLongVal(IDL_ULongLong a) : negative(0), u(a) {}
  IdlLongLongVal(IDL_LongLong  a) : negative(0), s(a) { if (a<0) negative=1; }

  IDL_Boolean negative;
  union {
    IDL_ULongLong u;
    IDL_LongLong  s;
  };
};
#endif

class IdlExpr {
public:
  IdlExpr(const char* file, int line) : file_(idl_strdup(file)), line_(line) {}
  virtual ~IdlExpr() { delete [] file_; }

  //
  // Virtual functions overridded by derived expression types
  //

  virtual IdlLongVal       evalAsLongV();
#ifdef HAS_LongLong
  virtual IdlLongLongVal   evalAsLongLongV();
#endif
  virtual IDL_Float        evalAsFloat();
  virtual IDL_Double       evalAsDouble();
  virtual IDL_Boolean      evalAsBoolean();
  virtual IDL_Char         evalAsChar();
  virtual const char*      evalAsString();
  virtual Enumerator*      evalAsEnumerator(const Enum* target);
#ifdef HAS_LongDouble
  virtual IDL_LongDouble   evalAsLongDouble();
#endif
  virtual IDL_WChar        evalAsWChar();
  virtual const IDL_WChar* evalAsWString();
  virtual IDL_Fixed*       evalAsFixed();

  //
  // Functions to convert an integer represented as a signed/unsigned
  // union to an IDL integer type
  //

  IDL_Short     evalAsShort();
  IDL_Long      evalAsLong();
  IDL_UShort    evalAsUShort();
  IDL_ULong 	evalAsULong();
  IDL_Octet 	evalAsOctet();
#ifdef HAS_LongLong
  IDL_LongLong  evalAsLongLong();
  IDL_ULongLong evalAsULongLong();
#endif

  inline const char* file() { return file_; }
  inline int         line() { return line_; }

  virtual const char* errText() = 0;

  static IdlExpr* scopedNameToExpr(const char* file, int line, ScopedName* sn);

private:
  char* file_;
  int   line_;
};


// Dummy expression class used as a place-holder after an error
class DummyExpr : public IdlExpr {
public:
  DummyExpr(const char* file, int line) : IdlExpr(file, line) {}
  virtual ~DummyExpr() {}

  IdlLongVal       evalAsLongV()     { return IdlLongVal (IDL_ULong(1)); }
#ifdef HAS_LongLong
  IdlLongLongVal   evalAsLongLongV() { return IdlLongLongVal(IDL_ULongLong(1));}
#endif

  IDL_Float        evalAsFloat()                        { return 1.0; }
  IDL_Double       evalAsDouble()                       { return 1.0; }
  IDL_Boolean      evalAsBoolean()                      { return 0; }
  IDL_Char         evalAsChar()                         { return '!'; }
  const char*      evalAsString()                       { return "!"; }
  Enumerator*      evalAsEnumerator(const Enum* target) { return 0; }
#ifdef HAS_LongDouble
  IDL_LongDouble   evalAsLongDouble()                   { return 1.0; }
#endif
  IDL_WChar        evalAsWChar()                        { return '!'; }
  const IDL_WChar* evalAsWString();
  IDL_Fixed*       evalAsFixed()    { return new IDL_Fixed("1"); }

  const char*      errText() { return "dummy"; }
};


// Literals

class IntegerExpr : public IdlExpr {
public:
  IntegerExpr(const char* file, int line, IdlIntLiteral v)
    : IdlExpr(file, line), value_(v) { }
  ~IntegerExpr() {}

  IdlLongVal       evalAsLongV();
#ifdef HAS_LongLong
  IdlLongLongVal   evalAsLongLongV();
#endif
  const char*      errText() { return "integer literal"; }
private:
  IdlIntLiteral    value_;
};

class StringExpr : public IdlExpr {
public:
  StringExpr(const char* file, int line, const char* v)
    : IdlExpr(file, line), value_(idl_strdup(v)) { }
  ~StringExpr() { delete [] value_; }

  const char*      evalAsString();
  const char*      errText() { return "string literal"; }
private:
  char* value_;
};

class WStringExpr : public IdlExpr {
public:
  WStringExpr(const char* file, int line, const IDL_WChar* v)
    : IdlExpr(file, line), value_(idl_wstrdup(v)) {}
  ~WStringExpr() { delete [] value_; }

  const IDL_WChar* evalAsWString();
  const char*      errText() { return "wide string literal"; }
private:
  IDL_WChar*       value_;
};

class CharExpr : public IdlExpr {
public:
  CharExpr(const char* file, int line, IDL_Char v)
    : IdlExpr(file, line), value_(v) { }
  ~CharExpr() {}

  IDL_Char         evalAsChar();
  const char*      errText() { return "character literal"; }
private:
  IDL_Char         value_;
};

class WCharExpr : public IdlExpr {
public:
  WCharExpr(const char* file, int line, IDL_WChar v)
    : IdlExpr(file, line), value_(v) {}
  ~WCharExpr() {}

  IDL_WChar        evalAsWChar();
  const char*      errText() { return "wide character literal"; }
private:
  IDL_WChar        value_;
};

class FixedExpr : public IdlExpr {
public:
  FixedExpr(const char* file, int line, IDL_Fixed* v)
    : IdlExpr(file, line), value_(v) {}
  ~FixedExpr() {}

  IDL_Fixed*       evalAsFixed();
  const char*      errText() { return "fixed point literal"; }
private:
  IDL_Fixed*       value_;
};

class FloatExpr : public IdlExpr {
public:
  FloatExpr(const char* file, int line, IdlFloatLiteral v)
    : IdlExpr(file, line), value_(v) { }
  ~FloatExpr() {}

  IDL_Float        evalAsFloat();
  IDL_Double       evalAsDouble();
#ifdef HAS_LongDouble
  IDL_LongDouble   evalAsLongDouble();
#endif
  const char*      errText() { return "floating point literal"; }
private:
  IdlFloatLiteral value_;
};

class BooleanExpr : public IdlExpr {
public:
  BooleanExpr(const char* file, int line, IDL_Boolean v)
    : IdlExpr(file, line), value_(v) { }
  ~BooleanExpr() {}

  IDL_Boolean      evalAsBoolean();
  const char*      errText() { return "boolean literal"; }
private:
  IDL_Boolean      value_;
};

// Enumerator referred to by scoped name
class EnumExpr : public IdlExpr {
public:
  EnumExpr(const char* file, int line, Enumerator* e, ScopedName* sn)
    : IdlExpr(file, line), value_(e), scopedName_(sn) {}
  ~EnumExpr() {}

  Enumerator*      evalAsEnumerator(const Enum* target);
  const char*      errText() { return "enumerator"; }
private:
  Enumerator* 	   value_;
  ScopedName* 	   scopedName_;
};

// Constant referred to by scoped name
class ConstExpr : public IdlExpr {
public:
  ConstExpr(const char* file, int line, Const* c, ScopedName* sn)
    : IdlExpr(file, line), c_(c), scopedName_(sn) {}
  ~ConstExpr() {}

  IdlLongVal       evalAsLongV();
#ifdef HAS_LongLong
  IdlLongLongVal   evalAsLongLongV();
#endif
  IDL_Float        evalAsFloat();
  IDL_Double       evalAsDouble();
  IDL_Boolean      evalAsBoolean();
  IDL_Char         evalAsChar();
  IDL_Octet        evalAsOctet();
  const char*      evalAsString();
  Enumerator*      evalAsEnumerator(const Enum* target);
#ifdef HAS_LongDouble
  IDL_LongDouble   evalAsLongDouble();
#endif
  IDL_WChar        evalAsWChar();
  const IDL_WChar* evalAsWString();
  IDL_Fixed*       evalAsFixed();

  const char* errText() { return "constant"; }
private:
  Const*      c_;
  ScopedName* scopedName_;
};



// Expressions

#ifdef HAS_LongLong
#define EXPR_INT_CONVERSION_FUNCTIONS \
  IdlLongVal       evalAsLongV(); \
  IdlLongLongVal   evalAsLongLongV();
#else
#define EXPR_INT_CONVERSION_FUNCTIONS \
  IdlLongVal       evalAsLongV();
#endif

#ifdef HAS_LongDouble
#define EXPR_FLOAT_CONVERSION_FUNCTIONS \
  IDL_Float        evalAsFloat();    \
  IDL_Double       evalAsDouble();   \
  IDL_LongDouble   evalAsLongDouble();
#else
#define EXPR_FLOAT_CONVERSION_FUNCTIONS \
  IDL_Float        evalAsFloat();    \
  IDL_Double       evalAsDouble();
#endif

#define EXPR_FIXED_CONVERSION_FUNCTIONS \
  IDL_Fixed*       evalAsFixed();

#define EXPR_CONVERSION_FUNCTIONS    \
  EXPR_INT_CONVERSION_FUNCTIONS      \
  EXPR_FLOAT_CONVERSION_FUNCTIONS    \
  EXPR_FIXED_CONVERSION_FUNCTIONS


#define EXPR_INT_BINARY_CLASS(cls, str) \
class cls : public IdlExpr { \
public: \
  cls(const char* file, int line, IdlExpr* a, IdlExpr* b) \
    : IdlExpr(file, line), a_(a), b_(b) { } \
  ~cls() { delete a_; delete b_; } \
  EXPR_INT_CONVERSION_FUNCTIONS \
  const char* errText() { return "result of " str " operation"; } \
private: \
  IdlExpr* a_; \
  IdlExpr* b_; \
};

#define EXPR_BINARY_CLASS(cls, str) \
class cls : public IdlExpr { \
public: \
  cls(const char* file, int line, IdlExpr* a, IdlExpr* b) \
    : IdlExpr(file, line), a_(a), b_(b) { } \
  ~cls() { delete a_; delete b_; } \
  EXPR_CONVERSION_FUNCTIONS \
  const char* errText() { return "result of " str " operation"; } \
private: \
  IdlExpr* a_; \
  IdlExpr* b_; \
};

EXPR_INT_BINARY_CLASS(OrExpr,     "or")
EXPR_INT_BINARY_CLASS(XorExpr,    "exclusive or")
EXPR_INT_BINARY_CLASS(AndExpr,    "and")
EXPR_INT_BINARY_CLASS(RShiftExpr, "right shift")
EXPR_INT_BINARY_CLASS(LShiftExpr, "left shift")
EXPR_INT_BINARY_CLASS(ModExpr,    "remainder")

EXPR_BINARY_CLASS(AddExpr,  "add")
EXPR_BINARY_CLASS(SubExpr,  "subtract")
EXPR_BINARY_CLASS(MultExpr, "multiply")
EXPR_BINARY_CLASS(DivExpr,  "divide")


class InvertExpr : public IdlExpr {
public:
  InvertExpr(const char* file, int line, IdlExpr* e)
    : IdlExpr(file, line), e_(e) { }
  ~InvertExpr() { delete e_; }
  EXPR_INT_CONVERSION_FUNCTIONS
  const char* errText() { return "result of unary invert operator"; }
private:
  IdlExpr* e_;
};

class MinusExpr : public IdlExpr {
public:
  MinusExpr(const char* file, int line, IdlExpr* e)
    : IdlExpr(file, line), e_(e) { }
  ~MinusExpr() { delete e_; }
  EXPR_CONVERSION_FUNCTIONS
  const char* errText() { return "result of unary negate operator"; }
private:
  IdlExpr* e_;
};

class PlusExpr : public IdlExpr {
public:
  PlusExpr(const char* file, int line, IdlExpr* e)
    : IdlExpr(file, line), e_(e) { }
  ~PlusExpr() { delete e_; }
  EXPR_CONVERSION_FUNCTIONS
  const char* errText() { return "result of unary plus operator"; }
private:
  IdlExpr* e_;
};


#endif // _idlexpr_h_
