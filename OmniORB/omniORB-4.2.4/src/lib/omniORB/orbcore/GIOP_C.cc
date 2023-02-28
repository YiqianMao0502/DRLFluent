// -*- Mode: C++; -*-
//                            Package   : omniORB
// GIOP_C.cc                  Created on: 08/02/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2011 Apasphere Ltd
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
//	*** PROPRIETARY INTERFACE ***
//	

#include <omniORB4/CORBA.h>
#include <giopRope.h>
#include <giopStrand.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <GIOP_C.h>
#include <exceptiondefs.h>
#include <orbParameters.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/minorCode.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
GIOP_C::GIOP_C(giopRope* r,giopStrand* s) : giopStream(s), 
					    pd_state(IOP_C::UnUsed),
					    pd_calldescriptor(0),
					    pd_ior(0),
					    pd_rope(r),
					    pd_replyStatus(GIOP::NO_EXCEPTION),
					    pd_locateStatus(GIOP::OBJECT_HERE)
{
}

////////////////////////////////////////////////////////////////////////
GIOP_C::~GIOP_C() {
}

////////////////////////////////////////////////////////////////////////
void*
GIOP_C::ptrToClass(int* cptr)
{
  if (cptr == &GIOP_C    ::_classid) return (GIOP_C*)    this;
  if (cptr == &giopStream::_classid) return (giopStream*)this;
  if (cptr == &cdrStream ::_classid) return (cdrStream*) this;

  return 0;
}
int GIOP_C::_classid;

////////////////////////////////////////////////////////////////////////
void
GIOP_C::initialise(const omniIOR* i, 
		   const CORBA::Octet* k,
		   int ksz,
		   omniCallDescriptor* calldesc)
{
  giopStream::reset();
  pd_strand->stopIdleCounter();
  state(IOP_C::Idle);
  ior(i);
  calldescriptor(calldesc);
  setDeadline(calldesc->getDeadline());
  key(k);
  keysize(ksz);
  requestId(pd_strand->newSeqNumber());
  TCS_C(0);
  TCS_W(0);
}

////////////////////////////////////////////////////////////////////////
void
GIOP_C::cleanup() {
  giopStream::reset();
  state(IOP_C::UnUsed);
  ior(0);
  calldescriptor(0);
}

////////////////////////////////////////////////////////////////////////
void
GIOP_C::InitialiseRequest() {

  OMNIORB_ASSERT(pd_state  == IOP_C::Idle);
  OMNIORB_ASSERT(pd_calldescriptor);
  OMNIORB_ASSERT(pd_ior);

  pd_state = IOP_C::RequestInProgress;
  impl()->outputMessageBegin(this,impl()->marshalRequestHeader);
  calldescriptor()->marshalArguments(*this);
  impl()->outputMessageEnd(this);
  clearValueTracker();
  pd_state = IOP_C::WaitingForReply;
  pd_strand->first_call = 0;
}

////////////////////////////////////////////////////////////////////////
GIOP::ReplyStatusType
GIOP_C::ReceiveReply() {

  OMNIORB_ASSERT(pd_state == IOP_C::WaitingForReply);

  if (calldescriptor()->is_oneway()) {
    pd_state = IOP_C::ReplyIsBeingProcessed;
    return GIOP::NO_EXCEPTION;
  }

  impl()->inputMessageBegin(this,impl()->unmarshalReplyHeader);

  pd_state = IOP_C::ReplyIsBeingProcessed;

  GIOP::ReplyStatusType rc = replyStatus();
  if (rc == GIOP::SYSTEM_EXCEPTION) { 
    if (omniORB::traceInvocationReturns) {
      omniORB::logger l;
      l << "Finish '" << calldescriptor()->op() << "' (system exception)\n";
    }
    UnMarshallSystemException();
    // never reaches here
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
void
GIOP_C::RequestCompleted(CORBA::Boolean skip) {

  OMNIORB_ASSERT(pd_state == IOP_C::ReplyIsBeingProcessed);

  clearValueTracker();

  if (!calldescriptor() || !calldescriptor()->is_oneway()) {
    impl()->inputMessageEnd(this,skip);
  }
  pd_strand->first_use = 0;
  pd_state = IOP_C::Idle;
}

////////////////////////////////////////////////////////////////////////
GIOP::LocateStatusType
GIOP_C::IssueLocateRequest() {

  OMNIORB_ASSERT(pd_state  == IOP_C::Idle);
  OMNIORB_ASSERT(pd_ior);

  pd_state = IOP_C::RequestInProgress;
  impl()->sendLocateRequest(this);
  pd_state = IOP_C::WaitingForReply;
  impl()->inputMessageBegin(this,impl()->unmarshalLocateReply);

  pd_state = IOP_C::ReplyIsBeingProcessed;

  GIOP::LocateStatusType rc = locateStatus();
  if (rc == GIOP::LOC_SYSTEM_EXCEPTION) {
    UnMarshallSystemException();
    // never reaches here
  }
  return rc;
}


////////////////////////////////////////////////////////////////////////
void
GIOP_C::UnMarshallSystemException()
{

#define CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION(_ex) \
  if (strcmp("IDL:omg.org/CORBA/" #_ex ":1.0",(const char*)repoid) == 0) \
    OMNIORB_THROW(_ex, minorcode, (CORBA::CompletionStatus) status);

  // Real code begins here
  cdrStream& s = *this;

  CORBA::String_var repoid;

  repoid = s.unmarshalRawString();

  CORBA::ULong minorcode;
  CORBA::ULong status;
  minorcode <<= s;
  status <<= s;

  clearValueTracker();

  impl()->inputMessageEnd(this,0);
  pd_strand->first_use = 0;
  pd_state = IOP_C::Idle;

  switch (status) {
  case CORBA::COMPLETED_YES:
  case CORBA::COMPLETED_NO:
  case CORBA::COMPLETED_MAYBE:
    break;
  default:
    OMNIORB_THROW(UNKNOWN,UNKNOWN_SystemException,CORBA::COMPLETED_YES);
  };

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION)

  // If none of the above matched
  OMNIORB_THROW(UNKNOWN,UNKNOWN_SystemException,CORBA::COMPLETED_YES);

#undef CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION
}

////////////////////////////////////////////////////////////////////////
void
GIOP_C::notifyCommFailure(CORBA::Boolean  heldlock,
			  CORBA::ULong&   minor,
			  CORBA::Boolean& retry) {

  OMNIORB_ASSERT(pd_calldescriptor);

  if (pd_strand->first_use || orbParameters::immediateRopeSwitch) {
    const giopAddress* firstaddr = pd_calldescriptor->firstAddressUsed();
    const giopAddress* currentaddr; 

    if (!firstaddr || !pd_rope->hasAddress(firstaddr)) {
      firstaddr = pd_strand->address;
      pd_calldescriptor->firstAddressUsed(firstaddr);
      currentaddr = firstaddr;
      pd_calldescriptor->currentAddress(currentaddr);
    }
    else {
      currentaddr = pd_calldescriptor->currentAddress();
    }

    if (pd_strand->orderly_closed && !orbParameters::immediateRopeSwitch) {
      // Strand was closed before / during our request. Retry with the
      // same address.
      retry = 1;
    }
    else {
      currentaddr = pd_rope->notifyCommFailure(currentaddr,heldlock);
      pd_calldescriptor->currentAddress(currentaddr);

      if (currentaddr == firstaddr) {
        // Run out of addresses to try.
        pd_rope->resetAddressOrder(heldlock, pd_strand);
	retry = 0;
	pd_calldescriptor->firstAddressUsed(0);
	pd_calldescriptor->currentAddress(0);
      }
      else {
	// Retry will use the next address in the list.
	retry = 1;
      }
    }
  }
  else if (pd_strand->isBiDir() && 
	   pd_strand->isClient() && 
	   pd_strand->biDir_has_callbacks) {

    // when the connection is used bidirectionally, the call back
    // objects at the other end will not be able to call us.
    // The application may want to know this. We
    // should not silently retry and reconnect again because the
    // callback objects would not use the new connection.
    retry = 0;
  }
  else {
    // Strand has been re-used from a previous invocation. Have
    // another go with a new strand in case something was broken in
    // the current one.
    pd_rope->resetAddressOrder(heldlock, pd_strand);
    retry = 1;
  }

  switch (pd_state) {
  case IOP_C::RequestInProgress:
    minor = COMM_FAILURE_MarshalArguments;
    break;
  case IOP_C::WaitingForReply:
    minor = COMM_FAILURE_WaitingForReply;
    break;
  case IOP_C::ReplyIsBeingProcessed:
    minor = COMM_FAILURE_UnMarshalResults;
    break;
  default:
    minor = TRANSIENT_ConnectionClosed;
    break;
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
GIOP_C::completion() {
  if (pd_state == IOP_C::WaitingForReply) {
    return (CORBA::ULong)CORBA::COMPLETED_MAYBE;
  }
  else if (pd_state == IOP_C::ReplyIsBeingProcessed) {
    return (CORBA::ULong)CORBA::COMPLETED_YES;
  }
  else {
    return (CORBA::ULong)CORBA::COMPLETED_NO;
  }
}

OMNI_NAMESPACE_END(omni)


