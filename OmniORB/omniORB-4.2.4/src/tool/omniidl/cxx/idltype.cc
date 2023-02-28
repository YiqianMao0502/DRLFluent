// -*- c++ -*-
//                          Package   : omniidl
// idltype.cc               Created on: 1999/10/21
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
//   Type objects

#include <idltype.h>
#include <idlast.h>
#include <idlerr.h>

const char*
IdlType::
kindAsString() const
{
  switch(kind_) {
  case tk_null:               return "null";
  case tk_void:               return "void";
  case tk_short:              return "short";
  case tk_long:               return "long";
  case tk_ushort:             return "unsigned short";
  case tk_ulong:              return "unsigned long";
  case tk_float:              return "float";
  case tk_double:             return "double";
  case tk_boolean:            return "boolean";
  case tk_char:               return "char";
  case tk_octet:              return "octet";
  case tk_any:                return "any";
  case tk_TypeCode:           return "CORBA::TypeCode";
  case tk_Principal:          return "CORBA::Principal";
  case tk_objref:             return "interface";
  case tk_struct:             return "struct";
  case tk_union:              return "union";
  case tk_enum:               return "enum";
  case tk_string:             return "string";
  case tk_sequence:           return "sequence";
  case tk_array:              return "array";
  case tk_alias:              return "typedef";
  case tk_except:             return "exception";
  case tk_longlong:           return "long long";
  case tk_ulonglong:          return "unsigned long long";
  case tk_longdouble:         return "long double";
  case tk_wchar:              return "wchar";
  case tk_wstring:            return "wstring";
  case tk_fixed:              return "fixed";
  case tk_value:              return "value";
  case tk_value_box:          return "value box";
  case tk_native:             return "native";
  case tk_abstract_interface: return "abstract interface";
  case tk_local_interface:    return "local interface";
  case ot_structforward:      return "forward struct";
  case ot_unionforward:       return "forward union";
  }
  assert(0);
  return ""; // To keep MSVC happy
}

IdlType*
IdlType::
unalias()
{
  IdlType* t = this;
  while (t && t->kind() == tk_alias) {
    if (((Declarator*)((DeclaredType*)t)->decl())->sizes()) break;
    t = ((Declarator*)((DeclaredType*)t)->decl())->alias()->aliasType();
  }
  return t;
}


IdlType*
IdlType::
scopedNameToType(const char* file, int line, const ScopedName* sn)
{
  const Scope::Entry* se = Scope::current()->findForUse(sn, file, line);

  if (se) {
    if (se->kind() == Scope::Entry::E_DECL) {
      IdlType *t = se->idltype();
      if (t) return t;
    }
    char* ssn = sn->toString();
    IdlError(file, line, "'%s' is not a type", ssn);
    IdlErrorCont(se->file(), se->line(), "('%s' declared here)", ssn);
    delete [] ssn;
  }
  return 0;
}

// Static type object pointers
IDL_Boolean    IdlType::initialised_             = 0;
BaseType*      BaseType::nullType                = 0;
BaseType*      BaseType::voidType                = 0;
BaseType*      BaseType::shortType               = 0;
BaseType*      BaseType::longType                = 0;
BaseType*      BaseType::ushortType              = 0;
BaseType*      BaseType::ulongType               = 0;
BaseType*      BaseType::floatType               = 0;
BaseType*      BaseType::doubleType              = 0;
BaseType*      BaseType::booleanType             = 0;
BaseType*      BaseType::charType                = 0;
BaseType*      BaseType::octetType               = 0;
BaseType*      BaseType::anyType                 = 0;
BaseType*      BaseType::TypeCodeType            = 0;
BaseType*      BaseType::PrincipalType           = 0;
BaseType*      BaseType::longlongType            = 0;
BaseType*      BaseType::ulonglongType           = 0;
BaseType*      BaseType::longdoubleType          = 0;
BaseType*      BaseType::wcharType               = 0;
StringType*    StringType::unboundedStringType   = 0;
WStringType*   WStringType::unboundedWStringType = 0;
DeclaredType*  DeclaredType::corbaObjectType     = 0;


void
IdlType::
init()
{
  if (!initialised_) {
    BaseType::nullType                = new BaseType(IdlType::tk_null);
    BaseType::voidType                = new BaseType(IdlType::tk_void);
    BaseType::shortType               = new BaseType(IdlType::tk_short);
    BaseType::longType                = new BaseType(IdlType::tk_long);
    BaseType::ushortType              = new BaseType(IdlType::tk_ushort);
    BaseType::ulongType               = new BaseType(IdlType::tk_ulong);
    BaseType::floatType               = new BaseType(IdlType::tk_float);
    BaseType::doubleType              = new BaseType(IdlType::tk_double);
    BaseType::booleanType             = new BaseType(IdlType::tk_boolean);
    BaseType::charType                = new BaseType(IdlType::tk_char);
    BaseType::octetType               = new BaseType(IdlType::tk_octet);
    BaseType::anyType                 = new BaseType(IdlType::tk_any);
    BaseType::TypeCodeType            = new BaseType(IdlType::tk_TypeCode);
    BaseType::PrincipalType           = new BaseType(IdlType::tk_Principal);
    BaseType::longlongType            = new BaseType(IdlType::tk_longlong);
    BaseType::ulonglongType           = new BaseType(IdlType::tk_ulonglong);
    BaseType::longdoubleType          = new BaseType(IdlType::tk_longdouble);
    BaseType::wcharType               = new BaseType(IdlType::tk_wchar);
    StringType::unboundedStringType   = new StringType(0);
    WStringType::unboundedWStringType = new WStringType(0);
    DeclaredType::corbaObjectType     = new DeclaredType(IdlType::tk_objref,
							 0, 0);
    initialised_ = 1;
  }
}
