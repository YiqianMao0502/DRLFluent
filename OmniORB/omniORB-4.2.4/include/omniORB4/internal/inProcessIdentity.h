// -*- Mode: C++; -*-
//                            Package   : omniORB
// inProcessIdentity.h        Created on: 16/05/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2007 Apasphere Ltd
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
//
//   Identity for objects in the caller's address space which cannot
//   be called directly. This can be because they are not activated
//   yet, because they are using DSI, or because they are in a
//   different language to the caller.

#ifndef __OMNIORB_INPROCESSIDENTITY_H__
#define __OMNIORB_INPROCESSIDENTITY_H__

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

class omniInProcessIdentity_RefHolder;

OMNI_NAMESPACE_END(omni)


class omniInProcessIdentity : public omniIdentity {
public:
  inline ~omniInProcessIdentity() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    if (--identity_count == 0)
      lastIdentityHasBeenDeleted();
  }

  inline omniInProcessIdentity(omniObjKey& key)
    : omniIdentity(key),
      pd_refCount(0)
    {
      ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
      ++identity_count;
    }
  // May consume <key>.

  inline omniInProcessIdentity(const _CORBA_Octet* key, int keysize)
    : omniIdentity(key, keysize),
      pd_refCount(0)
    {
      ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
      ++identity_count;
    }
  // Copies <key>.

  virtual void dispatch(omniCallDescriptor&);
  virtual void gainRef(omniObjRef* obj = 0);
  virtual void loseRef(omniObjRef* obj = 0);

  virtual omniIdentity::equivalent_fn get_real_is_equivalent() const;
  // Shares omniLocalIdentity's is_equivalent function.

  virtual void locateRequest(omniCallDescriptor&);
  // If this returns normally, then the object exists.
  // Throws OBJECT_NOT_EXIST, or omniORB::LOCATION_FORWARD
  // otherwise.
  // Caller must hold <internalLock>. On return or raised exception, the
  // lock is released.

  virtual _CORBA_Boolean inThisAddressSpace();
  // Override omniIdentity.


  virtual void* ptrToClass(int* cptr);
  static inline omniInProcessIdentity* downcast(omniIdentity* i) {
    return (omniInProcessIdentity*)i->ptrToClass(&_classid);
  }
  static _core_attr int _classid;
  // Dynamic casting mechanism.

private:
  friend class _OMNI_NS(omniInProcessIdentity_RefHolder);

  omniInProcessIdentity(const omniInProcessIdentity&);
  omniInProcessIdentity& operator = (const omniInProcessIdentity&);
  // Not implemented.

  int pd_refCount;
};

#undef _core_attr

#endif // __OMNIORB_INPROCESSIDENTITY_H__
