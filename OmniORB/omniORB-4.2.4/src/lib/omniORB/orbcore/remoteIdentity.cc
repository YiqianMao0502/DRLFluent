// -*- Mode: C++; -*-
//                            Package   : omniORB
// remoteIdentity.cc          Created on: 16/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2002-2010 Apasphere Ltd
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

#include <remoteIdentity.h>
#include <omniORB4/omniTransport.h>
#include <omniORB4/IOP_C.h>
#include <omniORB4/callDescriptor.h>
#include <dynamicLib.h>
#include <exceptiondefs.h>
#include <giopStream.h>

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniRemoteIdentity_RefHolder {
public:
  inline omniRemoteIdentity_RefHolder(omniRemoteIdentity* id) : pd_id(id) {
    pd_id->pd_refCount++;
    omni::internalLock->unlock();
  }

  inline ~omniRemoteIdentity_RefHolder() {
    omni::internalLock->lock();
    if (--pd_id->pd_refCount == 0) delete pd_id;
    omni::internalLock->unlock();
  }

private:
  omniRemoteIdentity* pd_id;
};

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
omniRemoteIdentity::dispatch(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniRemoteIdentity_RefHolder rh(this);
  // omni::internalLock has been released by RefHolder constructor

  if (!call_desc.op()) {
    locateRequest(call_desc);
    return;
  }

  if (omniORB::traceInvocations) {
    omniORB::logger l;
    l << "Invoke '" << call_desc.op() << "' on remote: " << this << '\n';
  }

  IOP_C_Holder iop_client(pd_ior,key(),keysize(),pd_rope,&call_desc);
  cdrStream& s = iop_client->getStream();

 again:
  call_desc.initialiseCall(s);

  iop_client->InitialiseRequest();

  // Wait for the reply.
  GIOP::ReplyStatusType rc = iop_client->ReceiveReply();

  switch (rc) {
  case GIOP::NO_EXCEPTION:
    // Unmarshal any result and out/inout arguments.
    call_desc.unmarshalReturnedValues(s);
    iop_client->RequestCompleted();

    if (omniORB::traceInvocationReturns) {
      omniORB::logger l;
      l << "Return '" << call_desc.op() << "' on remote: " << this << '\n';
    }
    break;

  case GIOP::USER_EXCEPTION:
    {
      if (omniORB::traceInvocationReturns) {
	omniORB::logger l;
	l << "Finish '" << call_desc.op() << "' (user exception)\n";
      }
      // Retrieve the Interface Repository ID of the exception.
      CORBA::String_var repoId(s.unmarshalRawString());
      call_desc.userException(iop_client->getStream(), &(IOP_C&)iop_client,
			      repoId);
      // Usually, the userException() call throws a user exception or
      // a system exception. In the DII case, it just stores the
      // exception and returns.

      break;
    }

  case GIOP::LOCATION_FORWARD:
  case GIOP::LOCATION_FORWARD_PERM:
    {
      CORBA::Object_var obj(CORBA::Object::_unmarshalObjRef(s));
      iop_client->RequestCompleted();
      if (omniORB::traceInvocationReturns) {
	omniORB::logger l;
	l << "Finish '" << call_desc.op() << "' (location forward)\n";
      }
      throw omniORB::LOCATION_FORWARD(obj._retn(),
			       (rc == GIOP::LOCATION_FORWARD_PERM) ? 1 : 0);
    }

  case GIOP::NEEDS_ADDRESSING_MODE:
    {
      GIOP::AddressingDisposition v;
      v <<= s;
      pd_ior->addr_mode(v);
      iop_client->RequestCompleted();
      if (omniORB::traceInvocationReturns) {
	omniORB::logger l;
	l << "Finish '" << call_desc.op() << "' (needs addressing mode)\n";
      }
      if (omniORB::trace(10)) {
	omniORB::logger log;
	log << "Remote invocation: GIOP::NEEDS_ADDRESSING_MODE: "
	    << (int) v << " retry request.\n";
      }
      goto again;
    }

  case GIOP::SYSTEM_EXCEPTION:
    OMNIORB_ASSERT(0);
    break;

  }
}


void
omniRemoteIdentity::gainRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  pd_refCount++;
}


void
omniRemoteIdentity::loseRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if( --pd_refCount > 0 )  return;

  delete this;
}


void
omniRemoteIdentity::locateRequest(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  if( omniORB::trace(10) || omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "LocateRequest to remote: " << this << '\n';
  }

  IOP_C_Holder iop_client(pd_ior,key(),keysize(),pd_rope,&call_desc);
  cdrStream& s = ((IOP_C&)iop_client).getStream();

  GIOP::LocateStatusType rc;

 again:
  switch( (rc = iop_client->IssueLocateRequest()) ) {
  case GIOP::OBJECT_HERE:
    iop_client->RequestCompleted();
    if (omniORB::traceInvocationReturns) {
      omniORB::logger l;
      l << "Return LocateRequest to remote: " << this << '\n';
    }
    break;

  case GIOP::UNKNOWN_OBJECT:
    iop_client->RequestCompleted();
    if (omniORB::traceInvocationReturns) {
      omniORB::logger l;
      l << "Return LocateRequest to remote: " << this << " (unknown object)\n";
    }
    OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_NoMatch,
		  CORBA::COMPLETED_NO);
    break;        // dummy break

  case GIOP::OBJECT_FORWARD:
  case GIOP::OBJECT_FORWARD_PERM:
    {
      CORBA::Object_var obj(CORBA::Object::_unmarshalObjRef(s));
      iop_client->RequestCompleted();
      if (omniORB::traceInvocationReturns) {
	omniORB::logger l;
	l << "Finish LocateRequest (object forward)\n";
      }
      throw omniORB::LOCATION_FORWARD(obj._retn(),
				      (rc == GIOP::OBJECT_FORWARD_PERM) ? 1 : 0);
    }

  case GIOP::LOC_NEEDS_ADDRESSING_MODE:
    {
      GIOP::AddressingDisposition v;
      v <<= s;
      pd_ior->addr_mode(v);
      iop_client->RequestCompleted();
      if (omniORB::traceInvocationReturns) {
	omniORB::logger l;
	l << "Finish LocateRequest (needs addressing mode)\n";
      }
      if (omniORB::trace(10)) {
	omniORB::logger log;
	log << "Remote locateRequest: GIOP::NEEDS_ADDRESSING_MODE: "
	    << (int) v << " retry request.\n";
      }
      goto again;
    }

  case GIOP::LOC_SYSTEM_EXCEPTION:
    OMNIORB_ASSERT(0);
    break;
  }
}


omniRemoteIdentity::~omniRemoteIdentity()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniORB::logs(15, "omniRemoteIdentity deleted.");
  pd_rope->decrRefCount();
  pd_ior->release();

  if (--identity_count == 0)
    lastIdentityHasBeenDeleted();
}

omniIdentity::equivalent_fn
omniRemoteIdentity::get_real_is_equivalent() const {
  return real_is_equivalent;
}

CORBA::Boolean
omniRemoteIdentity::real_is_equivalent(const omniIdentity* id1,
				       const omniIdentity* id2) {

  omniRemoteIdentity* rid1 = (omniRemoteIdentity*)id1;
  omniRemoteIdentity* rid2 = (omniRemoteIdentity*)id2;

  if (rid1->pd_rope != rid2->pd_rope) return 0;

  const CORBA::Octet* key1 = rid1->key();
  int keysize1             = rid1->keysize();

  const CORBA::Octet* key2 = rid2->key();
  int keysize2             = rid2->keysize();

  if (keysize1 != keysize2 || memcmp((void*)key1,(void*)key2,keysize1) != 0)
      // Object keys do not match
      return 0;

  return 1;
}

_CORBA_Boolean
omniRemoteIdentity::inThisAddressSpace()
{
  return 0;
}


void
omniRemoteIdentity::disconnect()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniRemoteIdentity_RefHolder rh(this);
  // omni::internalLock has been released by RefHolder constructor

  pd_rope->disconnect();
}


void*
omniRemoteIdentity::ptrToClass(int* cptr)
{
  if (cptr == &omniRemoteIdentity::_classid) return (omniRemoteIdentity*)this;
  if (cptr == &omniIdentity      ::_classid) return (omniIdentity*)      this;
  return 0;
}

int omniRemoteIdentity::_classid;
