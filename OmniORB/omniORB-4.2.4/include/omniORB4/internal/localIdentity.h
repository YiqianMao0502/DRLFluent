// -*- Mode: C++; -*-
//                            Package   : omniORB2
// localIdentity.h            Created on: 16/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2003-2010 Apasphere Ltd
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
//    Encapsulation of the identity of a local object implementation.
//

#ifndef __OMNIORB_LOCALIDENTITY_H__
#define __OMNIORB_LOCALIDENTITY_H__

#include <omniIdentity.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

class omniServant;
class omniObjRef;
class omniCallHandle;

OMNI_NAMESPACE_BEGIN(omni)
class omniObjAdapter;
class omniLocalIdentity_RefHolder;
OMNI_NAMESPACE_END(omni)

class omniLocalIdentity : public omniIdentity {
public:
  inline ~omniLocalIdentity() {}

  inline omniLocalIdentity(omniObjKey& key,
			   omniServant* servant,
			   _OMNI_NS(omniObjAdapter)* adapter)

    : omniIdentity(key),
      pd_nInvocations(1),
      pd_servant(servant),
      pd_adapter(adapter),
      pd_deactivated(0)
    {}
  // May consume <key> (if it is bigger than inline key buffer).
  // Constructs an identity with ref count of 1.

  inline omniLocalIdentity(const _CORBA_Octet* key, int keysize,
			   omniServant* servant,
			   _OMNI_NS(omniObjAdapter)* adapter)

    : omniIdentity(key, keysize),
      pd_nInvocations(1),
      pd_servant(servant),
      pd_adapter(adapter),
      pd_deactivated(0)
    {}
  // Copies <key>.  Constructs an identity with ref count
  // of 1.

  virtual void dispatch(omniCallDescriptor&);
  virtual void gainRef(omniObjRef* obj = 0);
  virtual void loseRef(omniObjRef* obj = 0);
protected:
  virtual omniIdentity::equivalent_fn get_real_is_equivalent() const;
  // Override omniIdentity.

public:
  void dispatch(omniCallHandle&);
  // Dispatches a remote invocation.  Grabs a reference
  // to this identity, and dispatches the call to the
  // object adapter (releasing the reference before
  // returning).
  //  Must hold <omni::internalLock> on entry.  It is not held
  // on exit.

  inline omniServant*              servant() const { return pd_servant; }
  inline _OMNI_NS(omniObjAdapter)* adapter() const { return pd_adapter; }
  inline int                       is_idle() const { return !pd_nInvocations;}
  inline _CORBA_Boolean        deactivated() const { return pd_deactivated; }
  // For each of the above the ownership of the returned value
  // is the responsibility of this object.  No reference counts
  // are incremented.

  inline void setServant(omniServant* servant, _OMNI_NS(omniObjAdapter)* oa) {
    OMNIORB_ASSERT(!pd_servant);  OMNIORB_ASSERT(servant);
    OMNIORB_ASSERT(!pd_adapter && oa);
    pd_servant = servant;
    pd_adapter = oa;
  }

  virtual _CORBA_Boolean inThisAddressSpace();
  // Override omniIdentity.


  virtual void* ptrToClass(int* cptr);
  static inline omniLocalIdentity* downcast(omniIdentity* i) {
    return (omniLocalIdentity*)i->ptrToClass(&_classid);
  }
  static _core_attr int _classid;
  // Dynamic casting mechanism.


  inline const _CORBA_Boolean* p_deactivated() const { return &pd_deactivated;}
  // This evil construction is used in the local shortcut
  // implementation to detect when the local identity has been
  // deactivated and the local shortcut can no longer be used. It is,
  // of course, totally unthreadsafe.

  static _CORBA_Boolean real_is_equivalent(const omniIdentity*,
					   const omniIdentity*);

protected:
  int pd_nInvocations;
  // This count gives the number of method calls in progress
  // on this object.  When it goes to zero, we check to see
  // if anyone is interested in such an event.
  //  To prevent this overhead when we are not interested in
  // invocations being finished, we add one to this value, so
  // that it never goes to zero.  The initial value is 1 for
  // this reason.  deactivate() decrements this value, so that
  // the adapter will be told when there are no invocations.

  omniServant* pd_servant;
  // Nil if this object is not yet incarnated, but once set
  // is immutable.
  //  The object adapter is responsible for managing the
  // etherealisation of the servant.

  _OMNI_NS(omniObjAdapter)* pd_adapter;
  // Nil if this object is not yet incarnated, but once set
  // is immutable.  We cannot have a pointer to the adapter
  // before the object is incarnated, since the adapter itself
  // may not yet exist either.
  //  We do not hold a reference to this adapter, since it will
  // (must!) outlive this object.

  _CORBA_Boolean pd_deactivated;
  // True if this localIdentity is no longer active, and cannot be
  // used for invocations.

private:
  friend class _OMNI_NS(omniLocalIdentity_RefHolder);

  omniLocalIdentity(const omniLocalIdentity&);
  omniLocalIdentity& operator = (const omniLocalIdentity&);
  // Not implemented.
};

#undef _core_attr

#endif  // __OMNIORB_LOCALIDENTITY_H__
