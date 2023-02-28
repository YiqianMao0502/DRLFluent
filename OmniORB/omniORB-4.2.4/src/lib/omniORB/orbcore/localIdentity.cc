// -*- Mode: C++; -*-
//                            Package   : omniORB
// localIdentity.cc           Created on: 16/6/99
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
//

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <localIdentity.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <objectAdapter.h>
#include <exceptiondefs.h>
#include <orbParameters.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
OMNI_NAMESPACE_BEGIN(omni)

class omniLocalIdentity_RefHolder {
public:
  inline omniLocalIdentity_RefHolder(omniLocalIdentity* id) : pd_id(id) {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    pd_id->pd_nInvocations++;
  }

  inline ~omniLocalIdentity_RefHolder() {
    omni::internalLock->lock();
    pd_id->pd_nInvocations--;
    pd_id->pd_adapter->leaveAdapter();
    if (pd_id->pd_nInvocations > 0) {
      omni::internalLock->unlock();
      return;
    }
    // Object has been deactivated, and the last invocation
    // has completed.  Pass the object back to the adapter
    // so it can be etherealised.
    pd_id->adapter()->lastInvocationHasCompleted(pd_id);

    // lastInvocationHasCompleted() has released <omni::internalLock>.
  }

private:
  omniLocalIdentity* pd_id;
};

OMNI_NAMESPACE_END(omni)


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
OMNI_USING_NAMESPACE(omni)

void
omniLocalIdentity::dispatch(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(pd_adapter && pd_servant);

  if (!call_desc.op() && !pd_deactivated) {
    // LocateRequest. It's a local object and we know it's here.
    omni::internalLock->unlock();
    return;
  }

  if (pd_deactivated || !call_desc.haslocalCallFn()) {
    // This localIdentity is dead and unusable, or the call descriptor
    // is unable to do a direct local call (because it's a DII call).
    // Either way, replace the object reference's identity with an
    // inProcessIdentity and invoke on that.
    //
    // Note that in the case of a DII call, we have dropped the
    // localIdentity, meaning the next non-DII call will have to
    // re-discover it. We do it this way since if the application has
    // done one DII call, it's likely to do more, so it's best to
    // avoid repeatedly creating inProcessIdentities.

    if (omniORB::trace(15)) {
      omniORB::logger l;
      if (pd_deactivated)
	l << this << " is no longer active. Using in-process identity.\n";
      else
	l << this << " cannot be directly invoked upon. "
	  << "Using in-process identity.\n";
    }
    omniIdentity* id = omni::createInProcessIdentity(key(), keysize());
    call_desc.objref()->_setIdentity(id);
    id->dispatch(call_desc);
    return;
  }

  if (call_desc.containsValues() && orbParameters::copyValuesInLocalCalls) {
    // Must use a call handle to call via a memory stream.
    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "Local call on " << this << " involves valuetypes; call via a "
	<< "memory buffer.\n";
    }
    omniCallHandle call_handle(&call_desc, 0);
    dispatch(call_handle);
    return;
  }

  call_desc.localId(this);

  omniLocalIdentity_RefHolder rh(this);

  omni::localInvocationCount++;

#ifndef HAS_Cplusplus_catch_exception_by_base
  // The compiler cannot catch exceptions by base class, hence
  // we cannot trap invalid exceptions going through here.
  pd_adapter->dispatch(call_desc, this);

#else
  try { pd_adapter->dispatch(call_desc, this); }

  catch (CORBA::SystemException& ex) {
    throw;
  }
  catch (CORBA::UserException& ex) {
    call_desc.validateUserException(ex);
    throw;
  }
  catch (omniORB::LOCATION_FORWARD&) {
    throw;
  }

  catch (...) {
    if( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "Warning: method '" << call_desc.op() << "' raised an unknown "
	"exception (not a legal CORBA exception).\n";
    }
    OMNIORB_THROW(UNKNOWN,UNKNOWN_UserException, CORBA::COMPLETED_MAYBE);
  }
#endif
}


void
omniLocalIdentity::dispatch(omniCallHandle& handle)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(pd_adapter && pd_servant);

  handle.localId(this);

  omniLocalIdentity_RefHolder rh(this);

  omni::remoteInvocationCount++;

  pd_adapter->dispatch(handle, this);
}


void
omniLocalIdentity::gainRef(omniObjRef*)
{
  OMNIORB_ASSERT(0);
  // An omniLocalIdentity should never be used as the identity within
  // an object reference. omniObjTableEntry should be used instead.
}


void
omniLocalIdentity::loseRef(omniObjRef*)
{
  OMNIORB_ASSERT(0);
  // An omniLocalIdentity should never be used as the identity within
  // an object reference. omniObjTableEntry should be used instead.
}

omniIdentity::equivalent_fn
omniLocalIdentity::get_real_is_equivalent() const {
  return real_is_equivalent;
}

CORBA::Boolean
omniLocalIdentity::real_is_equivalent(const omniIdentity* id1,
				      const omniIdentity* id2) {

  const CORBA::Octet* key1 = id1->key();
  int keysize1             = id1->keysize();

  const CORBA::Octet* key2 = id2->key();
  int keysize2             = id2->keysize();

  if (keysize1 != keysize2 || memcmp((void*)key1,(void*)key2,keysize1) != 0)
      // Object keys do not match
      return 0;

  return 1;
}

_CORBA_Boolean
omniLocalIdentity::inThisAddressSpace()
{
  return 1;
}


void*
omniLocalIdentity::ptrToClass(int* cptr)
{
  if (cptr == &omniLocalIdentity::_classid) return (omniLocalIdentity*)this;
  if (cptr == &omniIdentity     ::_classid) return (omniIdentity*)     this;
  return 0;
}

int omniLocalIdentity::_classid;
