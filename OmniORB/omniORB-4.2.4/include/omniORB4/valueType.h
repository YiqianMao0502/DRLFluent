// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueType.h                Created on: 2003/08/22
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003-2004 Apasphere Ltd.
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
//    ValueType definitions
//

#ifndef __OMNI_VALUETYPE_H__
#define __OMNI_VALUETYPE_H__

#include <omniORB4/CORBA.h>
#include <omniORB4/valueFactoryManager.h>


struct _omni_ValueId {
  const char*  repoId;
  CORBA::ULong hashval;
};

struct _omni_ValueIds {
  CORBA::Long    idcount;
  CORBA::ULong   hashval;
  _omni_ValueId* repoIds;
};


class omniValueType {
public:
  static void marshal(CORBA::ValueBase* val,
		      const char* repoId,
		      cdrStream& stream);
  // Marshal the value, in a context where the IDL specifies the value
  // has type given by <repoId>.

  static CORBA::ValueBase* unmarshal(const char* repoId, CORBA::ULong hashval,
				     CORBA::TypeCode_ptr tc,
				     cdrStream& stream);
  // Unmarshal a value, in a context where the IDL specifies the value
  // has type given by <repoId>, with hash <hashval>.
  //
  // If the TypeCode is present, it is the TypeCode of a value inside
  // an Any. When this function is called by static stubs, the
  // TypeCode pointer is zero, not nil, for efficiency and cleanliness
  // in the stubs.

  static CORBA::ValueBase* 
  handleIncompatibleValue(const char* repoId,
			  CORBA::ULong hashval,
			  CORBA::ValueBase* val,
			  CORBA::CompletionStatus completion);
  // When the stubs unmarshal a value, the value returned by unmarshal
  // above may not downcast to the required valuetype class. This can
  // happen for two reasons. One, the application registered a factory
  // that did the wrong thing, in which case we throw BAD_PARAM. Two,
  // the value has already been unmarshalled in an Any at a time when
  // no factory was registered, meaning that it is an instance of
  // UnknownValue. By the time the application tries to extract if
  // from the Any, it may have registered a suitable factory, so we
  // can now convert the UnknownValue into a suitable value. If that
  // succeeds, we return a new value that can be downcast to the
  // required valuetype class. If that fails, we throw MARSHAL.
  //
  // Whatever happens, the input value is consumed.

  static inline CORBA::ULong hash_id(const char* id)
  {
    CORBA::ULong n = 0;
    while (*id)  n = ((n << 5) ^ (n >> 27)) ^ *id++;
    return n;
  }

  static inline CORBA::ULong hash_ids(const _omni_ValueIds* ids)
  {
    CORBA::ULong n = 0;
    
    for (CORBA::Long i=0; i < ids->idcount; i++)
      n = ((n << 5) ^ (n >> 27)) ^ ids->repoIds[i].hashval;

    return n;
  }
};


#endif // __OMNI_VALUETYPE_H__
