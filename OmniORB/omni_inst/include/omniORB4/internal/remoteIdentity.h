// -*- Mode: C++; -*-
//                            Package   : omniORB2
// remoteIdentity.h           Created on: 16/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2002-2007 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//    Encapsulation of the identity of a remote object implementation.
//      
 
#ifndef __OMNIORB_REMOTEIDENTITY_H__
#define __OMNIORB_REMOTEIDENTITY_H__

#include <omniIdentity.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

class omniRemoteIdentity_RefHolder;

OMNI_NAMESPACE_END(omni)

class omniRemoteIdentity : public omniIdentity {
public:
  inline omniRemoteIdentity(omniIOR* ior, 
			    const CORBA::Octet* key,
			    CORBA::ULong keysize,
			    _OMNI_NS(Rope)* rope)
    : omniIdentity(key, keysize),
      pd_refCount(0),
      pd_ior(ior),
      pd_rope(rope)
    {
      ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
      ++identity_count;
    }
  // Consumes <ior> and <rope>.  Copies <key>.  Constructs an identity
  // with ref count of 0.

  virtual void dispatch(omniCallDescriptor&);
  virtual void gainRef(omniObjRef* obj = 0);
  virtual void loseRef(omniObjRef* obj = 0);
  virtual omniIdentity::equivalent_fn get_real_is_equivalent() const;
  // Overrides omniIdentity.

  inline _OMNI_NS(Rope)* rope() const { return pd_rope; }
  // This function is thread-safe.  Does not increment
  // the reference count of the rope.

  virtual void locateRequest(omniCallDescriptor&);
  // If this returns normally, then the object exists.
  // Throws OBJECT_NOT_EXIST, or omniORB::LOCATION_FORWARD
  // otherwise.
  // Caller must hold <internalLock>. On return or raised exception, the
  // lock is released.

  virtual _CORBA_Boolean inThisAddressSpace();
  // Override omniIdentity.

  void disconnect();
  // Override omniIdentity.
  

  virtual void* ptrToClass(int* cptr);
  static inline omniRemoteIdentity* downcast(omniIdentity* i) {
    return (omniRemoteIdentity*)i->ptrToClass(&_classid);
  }
  static _core_attr int _classid;
  // Dynamic casting mechanism.

private:
  friend class _OMNI_NS(omniRemoteIdentity_RefHolder);

  ~omniRemoteIdentity();

  omniRemoteIdentity(const omniRemoteIdentity&);
  omniRemoteIdentity& operator = (const omniRemoteIdentity&);
  // Not implemented.


  int              pd_refCount;
  omniIOR*         pd_ior;
  _OMNI_NS(Rope)*  pd_rope;
  // Immutable.

  static _CORBA_Boolean real_is_equivalent(const omniIdentity*,
					   const omniIdentity*);

};

#undef _core_attr

#endif  // __OMNIORB_REMOTEIDENTITY_H__
