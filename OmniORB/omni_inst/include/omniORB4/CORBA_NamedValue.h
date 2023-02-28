// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_NamedValue.h         Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library. If not, see http://www.gnu.org/licenses/
//
//
// Description:
//    CORBA::NamedValue, NVList
//

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////// NamedValue /////////////////////////////
//////////////////////////////////////////////////////////////////////

enum _Flags {
  ARG_IN              = 0x1,
  ARG_OUT             = 0x2,
  ARG_INOUT           = 0x3, //?  Defined in CORBA 2.5. Odd.
  CTX_RESTRICT_SCOPE  = 0xF, //?  Defined in CORBA 2.5. Odd.
  OUT_LIST_MEMORY     = 0x10,
  IN_COPY_VALUE       = 0x20
};

typedef ULong Flags;

class NamedValue;
typedef NamedValue* NamedValue_ptr;
typedef NamedValue_ptr NamedValueRef;
typedef _CORBA_PseudoObj_Var<NamedValue> NamedValue_var;
typedef _CORBA_PseudoObj_Out<NamedValue,NamedValue_var> NamedValue_out;

class NamedValue {
public:
  virtual ~NamedValue();

  virtual const char* name() const = 0;
  // Retains ownership of return value.

  virtual Any* value() const = 0;
  // Retains ownership of return value.

  virtual Flags flags() const = 0;

  virtual Boolean NP_is_nil() const = 0;
  virtual NamedValue_ptr NP_duplicate() = 0;

  static NamedValue_ptr _duplicate(NamedValue_ptr);
  static NamedValue_ptr _nil();

  static inline _CORBA_Boolean PR_is_valid(NamedValue_ptr p ) {
    return ((p) ? (p->pd_magic == PR_magic) : 1);
  }

  static _dyn_attr const _CORBA_ULong PR_magic;

protected:
  NamedValue() { pd_magic = PR_magic; }

private:
  _CORBA_ULong pd_magic;

  NamedValue(const NamedValue&);
  NamedValue& operator=(const NamedValue&);
};


//////////////////////////////////////////////////////////////////////
/////////////////////////////// NVList ///////////////////////////////
//////////////////////////////////////////////////////////////////////

class NVList;
typedef NVList* NVList_ptr;
typedef NVList_ptr NVListRef;
typedef _CORBA_PseudoObj_Var<NVList> NVList_var;
typedef _CORBA_PseudoObj_Out<NVList,NVList_var> NVList_out;

class NVList {
public:
  virtual ~NVList();

  virtual ULong count() const = 0;
  virtual NamedValue_ptr add(Flags) = 0;
  virtual NamedValue_ptr add_item(const char*, Flags) = 0;
  virtual NamedValue_ptr add_value(const char*, const Any&, Flags) = 0;
  virtual NamedValue_ptr add_item_consume(char*,Flags) = 0;
  virtual NamedValue_ptr add_value_consume(char*, Any*, Flags) = 0;
  virtual NamedValue_ptr item(ULong index) = 0;
  virtual void remove (ULong) = 0;

  virtual Boolean NP_is_nil() const = 0;
  virtual NVList_ptr NP_duplicate() = 0;

  static NVList_ptr _duplicate(NVList_ptr);
  static NVList_ptr _nil();

  // OMG Interface:

  OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(Bounds, _dyn_attr)

  static inline _CORBA_Boolean PR_is_valid(NVList_ptr p ) {
    return ((p) ? (p->pd_magic == PR_magic) : 1);
  }

  static _dyn_attr const _CORBA_ULong PR_magic;

protected:
  NVList() { pd_magic = PR_magic; }

private:
  _CORBA_ULong pd_magic;

  NVList(const NVList& nvl);
  NVList& operator=(const NVList&);
};

