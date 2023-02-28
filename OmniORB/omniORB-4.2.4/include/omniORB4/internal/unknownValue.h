// -*- Mode: C++; -*-
//                            Package   : omniORB
// unknownValue.h             Created on: 2004/06/14
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2004-2005 Apasphere Ltd.
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
//    When we receive a valuetype inside an Any, if we do not have a
//    statically generated class for the value, we use an instance of
//    this class.
//

#ifndef __OMNI_UNKNOWNVALUE_H__
#define __OMNI_UNKNOWNVALUE_H__

#ifdef _dyn_attr
# error "A local CPP macro _dyn_attr has already been defined."
#endif

#if defined(_OMNIORB_DYNAMIC_LIBRARY)
#  define _dyn_attr
#else
#  define _dyn_attr _OMNIORB_NTDLL_IMPORT
#endif

#include <omniORB4/CORBA.h>
#include <omniORB4/anyStream.h>


OMNI_NAMESPACE_BEGIN(omni)

class UnknownValue : public virtual CORBA::DefaultValueRefCountBase {
public:

  UnknownValue(CORBA::TypeCode_ptr tc);
  virtual ~UnknownValue();

  static UnknownValue* _downcast(CORBA::ValueBase*);

#ifdef OMNI_HAVE_COVARIANT_RETURNS
  virtual UnknownValue* _copy_value();
#else
  virtual CORBA::ValueBase* _copy_value();
#endif

  // Overridden internal functions from ValueBase

  virtual const char* _NP_repositoryId() const;
  virtual const char* _NP_repositoryId(CORBA::ULong& hash) const;
  virtual const _omni_ValueIds* _NP_truncatableIds() const;
  virtual CORBA::Boolean _NP_custom() const ;

  virtual void* _ptrToValue(const char* repoId);

  virtual void _PR_marshal_state(cdrStream& s) const;
  virtual void _PR_unmarshal_state(cdrStream& s);

  static _dyn_attr const char* _PD_repoId;

  cdrAnyMemoryStream  pd_mbuf;
  // Marshalled state members of the value.

private:
  CORBA::TypeCode_var pd_tc;
  // TypeCode of this value.

  CORBA::ULong        pd_hash;
  // Hash value of repository id in TypeCode.

  UnknownValue(const UnknownValue &);
  // Internal copy constructor.

  void operator=(const UnknownValue &);
  // Not implemented
};


OMNI_NAMESPACE_END(omni)

#undef _dyn_attr

#endif // __OMNI_UNKNOWNVALUE_H__
