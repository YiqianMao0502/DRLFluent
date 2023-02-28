// -*- Mode: C++; -*-
//                            Package   : omniORB
// GIOP_S.cc                  Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2011 Apasphere Ltd
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
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <initRefs.h>
#include <exceptiondefs.h>
#include <objectTable.h>
#include <objectAdapter.h>
#include <giopStrand.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <invoker.h>
#include <giopServer.h>
#include <giopWorker.h>
#include <GIOP_S.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/omniInterceptors.h>
#include <interceptors.h>
#include <poaimpl.h>
#include <orbParameters.h>

#ifdef HAVE_STD
#include <memory>
#endif

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
GIOP_S_Holder::GIOP_S_Holder(giopStrand* g, giopWorker* work) : pd_strand(g) {
  pd_iop_s = g->acquireServer(work);
}


////////////////////////////////////////////////////////////////////////
GIOP_S_Holder::~GIOP_S_Holder() {
  if (pd_iop_s) pd_strand->releaseServer(pd_iop_s);
}

////////////////////////////////////////////////////////////////////////
GIOP_S::GIOP_S(giopStrand* g) : giopStream(g),
				pd_state(UnUsed),
				pd_worker(0),
				pd_calldescriptor(0),
				pd_requestType(GIOP::MessageError),
				pd_operation((char*)pd_op_buffer),
				pd_principal(pd_pr_buffer),
				pd_principal_len(0),
				pd_response_expected(1),
				pd_result_expected(1)
{
}

////////////////////////////////////////////////////////////////////////
GIOP_S::GIOP_S(const GIOP_S& src) : giopStream(src.pd_strand),
				    pd_state(UnUsed),
				    pd_worker(0),
				    pd_calldescriptor(0),
				    pd_requestType(GIOP::MessageError),
				    pd_operation((char*)pd_op_buffer),
				    pd_principal(pd_pr_buffer),
				    pd_principal_len(0),
				    pd_response_expected(1),
				    pd_result_expected(1)
{
}

////////////////////////////////////////////////////////////////////////
GIOP_S::~GIOP_S() {
  if (pd_operation != (char*)pd_op_buffer) delete [] pd_operation;
  if (pd_principal != pd_pr_buffer) delete [] pd_principal;
}

////////////////////////////////////////////////////////////////////////
void*
GIOP_S::ptrToClass(int* cptr)
{
  if (cptr == &GIOP_S    ::_classid) return (GIOP_S*)    this;
  if (cptr == &giopStream::_classid) return (giopStream*)this;
  if (cptr == &cdrStream ::_classid) return (cdrStream*) this;

  return 0;
}
int GIOP_S::_classid;


////////////////////////////////////////////////////////////////////////
static inline void
setServerDeadline(GIOP_S* giop_s)
{
  if (orbParameters::serverCallTimeOutPeriod) {

    omni_time_t deadline;
    omni_thread::get_time(deadline, orbParameters::serverCallTimeOutPeriod);
    giop_s->setDeadline(deadline);
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
GIOP_S::dispatcher() {

  OMNIORB_ASSERT(pd_state == Idle);

  try {

    pd_state = WaitForRequestHeader;
    calldescriptor(0);
    requestId(0xffffffff);

    impl()->inputMessageBegin(this,impl()->unmarshalWildCardRequestHeader);

    {
      omni_tracedmutex_lock sync(*omniTransportLock);
      pd_state = RequestHeaderIsBeingProcessed;
      if (!pd_strand->stopIdleCounter()) {
	// This strand has been expired by the scavenger. Don't
	// process this call.
	omniORB::logs(5, "Connection closed by scavenger. Dispatch aborted.");
	pd_strand->state(giopStrand::DYING);
	return 0;
      }
    }
    setServerDeadline(this);

    if (pd_requestType == GIOP::Request) {
      return handleRequest();
    }
    else if (pd_requestType == GIOP::LocateRequest) {
      return handleLocateRequest();
    }
    else if (pd_requestType == GIOP::CancelRequest) {
      return handleCancelRequest();
    }
    else {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "Unexpected message type (" << (CORBA::ULong) pd_requestType
	  << ") received by a server thread at "
	  << __FILE__ << ": line " << __LINE__ << "\n";
      }
      return 0;
    }
  }
  catch(const giopStream::CommFailure&) {
    // Connection has been closed.
    return 0;
  }

#ifndef HAS_Cplusplus_catch_exception_by_base
#  define CATCH_AND_HANDLE(name) \
  catch(CORBA::name& ex) { \
    impl()->sendMsgErrorMessage(this, &ex); \
    return 0; \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CATCH_AND_HANDLE)

#  undef CATCH_AND_HANDLE
#endif

  catch(CORBA::SystemException& ex) {
    impl()->sendMsgErrorMessage(this, &ex);
    return 0;
  }

  catch(const omniORB::fatalException& ex) {
    if( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "omniORB fatalException caught by a server thread at "
	<< ex.file() << ": line "
	<< ex.line() << ", message: "
	<< ex.errmsg() << "\n";
    }
    return 0;
  }
  catch (...) {
    if ( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "Unknown exception caught by a server thread at "
	<< __FILE__ << ": line " << __LINE__ << "\n";
    }
    return 0;
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
GIOP_S::handleRequest() {

  try {

    impl()->unmarshalRequestHeader(this);

    pd_state = RequestIsBeingProcessed;

    {
      omniInterceptors::serverReceiveRequest_T::info_T info(*this);
      omniInterceptorP::visit(info);
    }

    // Create a callHandle object
    omniCallHandle call_handle(this, pd_worker->selfThread());

    // Can we find the object in the local object table?
    if (keysize() < 0)
      OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_NoMatch,
		    CORBA::COMPLETED_NO);

    CORBA::ULong hash = omni::hash(key(), keysize());

    omni::internalLock->lock();
    omniLocalIdentity* id;
    id = omniObjTable::locateActive(key(), keysize(), hash, 1);

    if( id ) {
      id->dispatch(call_handle);
      return 1;
    }

    omni::internalLock->unlock();

    // Can we create a suitable object on demand?

    omniObjAdapter_var adapter(omniObjAdapter::getAdapter(key(),keysize()));

    if( adapter ) {
      adapter->dispatch(call_handle, key(), keysize());
      return 1;
    }

    // Or is it the bootstrap agent?

    if( keysize() == 4 && !memcmp(key(), "INIT", 4) &&
	omniInitialReferences::invoke_bootstrap_agentImpl(call_handle) )
      return 1;

    // Oh dear.

    if (omniObjAdapter::isDeactivating())
      OMNIORB_THROW(OBJ_ADAPTER,
		    OBJ_ADAPTER_POAUnknownAdapter,
		    CORBA::COMPLETED_NO);
    else
      OMNIORB_THROW(OBJECT_NOT_EXIST,
		    OBJECT_NOT_EXIST_NoMatch,
		    CORBA::COMPLETED_NO);
  }
  catch(omniORB::LOCATION_FORWARD& ex) {
    // This is here to provide a convenient way to implement
    // location forwarding. The object implementation can throw
    // a location forward exception to re-direct the request
    // to another location.

    if( omniORB::traceInvocations ) {
      omniORB::logger l;
      l << "Implementation of '" << operation()
	<< "' generated LOCATION_FORWARD.\n";
    }

    CORBA::Object_var release_it(ex.get_obj());
    if (pd_state == RequestIsBeingProcessed) {
      SkipRequestBody();
    }
    if (response_expected()) {
      setServerDeadline(this);
      impl()->sendLocationForwardReply(this,release_it,ex.is_permanent());
    }
    // If the client does not expect a response, we quietly drop
    // the location forward.
  }
  catch (const terminateProcessing&) {
  }

#define MARSHAL_USER_EXCEPTION() do { \
  if (pd_state == RequestIsBeingProcessed) { \
    SkipRequestBody(); \
  } \
  if (response_expected()) { \
    setServerDeadline(this); \
    if (calldescriptor()) { \
      int i, repoid_size;  \
      const char* repoid = ex._NP_repoId(&repoid_size); \
      for( i = 0; i < pd_n_user_excns; i++ ) \
	if( omni::strMatch(pd_user_excns[i], repoid) ) { \
	  impl()->sendUserException(this,ex); \
	  break; \
	} \
      if( i == pd_n_user_excns ) { \
	if( omniORB::trace(1) ) { \
	  omniORB::logger l; \
	  l << "Warning: method '" << operation() \
	    << "' on: " << pd_key \
	    << " raised the exception: " << repoid << '\n'; \
	} \
	CORBA::UNKNOWN sex(UNKNOWN_UserException, \
			   (CORBA::CompletionStatus) completion()); \
	impl()->sendSystemException(this,sex); \
      } \
    } \
    else { \
      impl()->sendUserException(this,ex); \
    } \
  } \
} while (0)

#define MARSHAL_SYSTEM_EXCEPTION() do { \
    setServerDeadline(this); \
    if (pd_state == RequestIsBeingProcessed) { \
      SkipRequestBody(); \
    } \
    if (pd_state == WaitForRequestHeader || \
	pd_state == RequestHeaderIsBeingProcessed ) { \
      impl()->sendMsgErrorMessage(this, &ex); \
      return 0; \
    } else if (response_expected()) { \
      impl()->sendSystemException(this,ex); \
    } \
} while (0) 

# ifndef HAS_Cplusplus_catch_exception_by_base

  // We have to catch each type of system exception separately
  // here to support compilers which cannot catch more derived
  // types.
#   define CATCH_AND_MARSHAL(name)  \
  catch (CORBA::name& ex) {  \
    MARSHAL_SYSTEM_EXCEPTION(); \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CATCH_AND_MARSHAL)

#    undef CATCH_AND_MARSHAL

  catch(omniORB::StubUserException& uex) {
    CORBA::UserException& ex = *((CORBA::UserException*)uex.ex());
    MARSHAL_USER_EXCEPTION();
    delete uex.ex();  // ?? Possible memory leak?
  }

#endif


  catch(CORBA::SystemException& ex) {
    MARSHAL_SYSTEM_EXCEPTION();
    // If the client does not expect a response, we quietly drop
    // the system exception.
  }
  catch(CORBA::UserException& ex) {
    MARSHAL_USER_EXCEPTION();
  }
#undef MARSHAL_USER_EXCEPTION
#undef MARSHAL_SYSTEM_EXCEPTION

  catch(const giopStream::CommFailure&) {
    throw;
  }

#ifdef HAVE_STD
  catch (const std::bad_alloc&) {
    // We keep logging as simple as possible to avoid too much allocation.
    omniORB::logs(1, "Error: upcall raised std::bad_alloc.");

    if (response_expected()) {
      CORBA::NO_MEMORY ex(NO_MEMORY_BadAlloc,
			  (CORBA::CompletionStatus)completion());
      impl()->sendSystemException(this,ex);
    }
  }
  
  catch (const std::exception& std_ex) {
    if (omniORB::trace(1)) {
      omniORB::logger l;
      l << "Warning: method '" << operation() << "' raised an unexpected "
	<< "std::exception (not a CORBA exception): "
	<< std_ex.what() << "\n";
    }
    if (response_expected()) {
      CORBA::UNKNOWN ex(UNKNOWN_UserException,
			(CORBA::CompletionStatus) completion());
      impl()->sendSystemException(this,ex);
    }
  }
#endif // HAVE_STD

  catch (const omni_thread_fatal& thr_ex) {
    if (omniORB::trace(1)) {
      omniORB::logger l;
      l << "Warning: method '" << operation() << "' raised an "
	<< "omni_thread_fatal exception (error " << thr_ex.error << ").\n";
    }
    if (response_expected()) {
      CORBA::UNKNOWN ex(UNKNOWN_OmniThreadException,
			(CORBA::CompletionStatus) completion());
      impl()->sendSystemException(this,ex);
    }
  }

  catch(...) {
    if (omniORB::trace(1)) {
      omniORB::logger l;
      l << "Warning: method '" << operation() << "' raised an unexpected "
	"exception (not a CORBA exception).\n";
    }
    if (response_expected()) {
      CORBA::UNKNOWN ex(UNKNOWN_UserException,
			(CORBA::CompletionStatus) completion());
      setServerDeadline(this);
      impl()->sendSystemException(this,ex);
    }
  }
  pd_state = ReplyCompleted;

  clearValueTracker();
  clearDeadline();
  return 1;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
GIOP_S::handleLocateRequest() {
  try {

    impl()->unmarshalLocateRequest(this);

    pd_state = RequestIsBeingProcessed;

    // Here we notify the giopServer that this thread has finished
    // reading the request. The server may want to keep a watch on
    // any more request coming in on the connection while this
    // thread does the upcall.

    CORBA::Boolean data_in_buffer = 0;
    if (pd_rdlocked) {
      // This is the thread that is reading from the connection. We
      // check if we have previously queued giopStream_Buffers on the
      // connection. In other words, we might have previously read
      // too much stuff out of the connection and these data belong to
      // other requests. If that is the case, we notify the giopServer
      // that there are already buffers waiting to be read.
      giopStrand& s = strand();
      data_in_buffer = ((s.head) ? 1 : 0);
    }
    pd_worker->server()->notifyWkPreUpCall(pd_worker,data_in_buffer);

    impl()->inputMessageEnd(this,0);
    
    pd_state = WaitingForReply;

    omniORB::logs(10, "Handling a GIOP LOCATE_REQUEST.");

    GIOP::LocateStatusType status = GIOP::UNKNOWN_OBJECT;

    if (keysize() > 0) {
      CORBA::ULong hash = omni::hash(key(), keysize());
      omni_tracedmutex_lock sync(*omni::internalLock);
      omniLocalIdentity* id;
      id = omniObjTable::locateActive(key(), keysize(), hash, 1);
      if( id )  status = GIOP::OBJECT_HERE;
     }
    if ( status == GIOP::UNKNOWN_OBJECT && keysize() > 0 ) {
      // We attempt to find the object adapter (activate it if necassary)
      // and ask it if the object exists, or if it has the *capability*
      // to activate such an object.  ie. is it able to do object loading
      // on demand?

      omniObjAdapter_var adapter(omniObjAdapter::getAdapter(key(),keysize()));
      if( adapter && adapter->objectExists(key(),keysize()) )
	status = GIOP::OBJECT_HERE;
    }
    if ( status == GIOP::UNKNOWN_OBJECT && 
	 keysize() == 4 && !memcmp(key(), "INIT", 4) &&
	 omniInitialReferences::is_bootstrap_agentImpl_initialised() ) {
      status = GIOP::OBJECT_HERE;
    }

    setServerDeadline(this);
    impl()->sendLocateReply(this,status,CORBA::Object::_nil(),0);
  }
  catch (omniORB::LOCATION_FORWARD& lf) {
    CORBA::Object_var release_it(lf.get_obj());
    setServerDeadline(this);
    impl()->sendLocateReply(this,
			    lf.is_permanent() ? GIOP::OBJECT_FORWARD_PERM :
			                        GIOP::OBJECT_FORWARD,
                            release_it,0);
  }

#define MARSHAL_SYSTEM_EXCEPTION() do { \
    setServerDeadline(this); \
    if (pd_state == RequestIsBeingProcessed) { \
      SkipRequestBody(); \
    } \
    if (pd_state == WaitForRequestHeader || \
        pd_state == RequestHeaderIsBeingProcessed) { \
      impl()->sendMsgErrorMessage(this, &ex); \
      return 0; \
    } else if (response_expected()) { \
      impl()->sendSystemException(this,ex); \
    } \
} while (0) 

# ifndef HAS_Cplusplus_catch_exception_by_base

  // We have to catch each type of system exception separately
  // here to support compilers which cannot catch more derived
  // types.
#   define CATCH_AND_MARSHAL(name)  \
  catch (CORBA::name& ex) {  \
    MARSHAL_SYSTEM_EXCEPTION(); \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CATCH_AND_MARSHAL)

#    undef CATCH_AND_MARSHAL
#endif

  catch(CORBA::SystemException& ex) {
    MARSHAL_SYSTEM_EXCEPTION();
  }

  pd_state = ReplyCompleted;

  clearValueTracker();
  clearDeadline();
  return 1;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
GIOP_S::handleCancelRequest() {
  // We do not have the means to asynchronously abort the execution of
  // an upcall by another thread. Therefore it is not possible to
  // cancel a request that has already been in progress. The best we
  // can do is prevent the reply from happening.
  omniORB::logs(5, "Received a CancelRequest message.");
  pd_state = WaitingForReply;
  response_expected(0);
  clearDeadline();
  return 1;
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::ReceiveRequest(omniCallDescriptor& desc) {

  OMNIORB_ASSERT(pd_state == RequestIsBeingProcessed);

  calldescriptor(&desc);

  // When a user exception is throw by the stub code, the
  // call descriptor could have been deallocated before the
  // catch frame for the user exception is reached. Therefore
  // we store the user exception signatures inside our own
  // private states.
  pd_n_user_excns = desc.n_user_excns();
  pd_user_excns = desc.user_excns();

  cdrStream& s = *this;
  desc.unmarshalArguments(s);
  pd_state = WaitingForReply;

  clearValueTracker();

  // Here we notify the giopServer that this thread has finished
  // reading the request. The server may want to keep a watch on
  // any more request coming in on the connection while this
  // thread does the upcall.

  CORBA::Boolean data_in_buffer = 0;
  if (pd_rdlocked) {
    // This is the thread that is reading from the connection. We
    // check if we have previously queued giopStream_Buffers on the
    // connection. In other words, we might have previously read
    // too much stuff out of the connection and these data belong to
    // other requests. If that is the case, we notify the giopServer
    // that there are already buffers waiting to be read.
    giopStrand& s = strand();
    data_in_buffer = ((s.head) ? 1 : 0);
  }
  pd_worker->server()->notifyWkPreUpCall(pd_worker,data_in_buffer);

  impl()->inputMessageEnd(this,0);


  // Check if this call comes in from a bidirectional connection.
  // If so check if the servant's POA policy allows this.
  giopStrand& g = strand();
  if (g.isBiDir() && g.isClient()) {
    if (!(pd_calldescriptor->poa() &&
	  pd_calldescriptor->poa()->acceptBiDirectional())) {
      OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_BiDirNotAllowed,
		    CORBA::COMPLETED_NO);
    }
  }
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::SkipRequestBody() {

  OMNIORB_ASSERT(pd_state == RequestIsBeingProcessed);

  pd_state = WaitingForReply;

  CORBA::Boolean data_in_buffer = 0;
  if (pd_rdlocked) {
    giopStrand& s  = strand();
    data_in_buffer = ((s.head) ? 1 : 0);
  }
  pd_worker->server()->notifyWkPreUpCall(pd_worker,data_in_buffer);

  impl()->inputMessageEnd(this,1);
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::SendReply() {

  OMNIORB_ASSERT(pd_state == WaitingForReply);

  if (!response_expected()) {
    pd_state = ReplyCompleted;
    clearDeadline();
    return;
  }

  pd_service_contexts.length(0);

  setServerDeadline(this);

  if (omniInterceptorP::serverSendReply) {
    omniInterceptors::serverSendReply_T::info_T info(*this);
    omniInterceptorP::visit(info);
  }
  pd_state = ReplyIsBeingComposed;
  impl()->outputMessageBegin(this,impl()->marshalReplyHeader);
  cdrStream& s = *this;
  calldescriptor()->marshalReturnedValues(s);
  impl()->outputMessageEnd(this);
  pd_state = ReplyCompleted;

  clearValueTracker();
  clearDeadline();
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::SendException(CORBA::Exception* ex) {

  OMNIORB_ASSERT(pd_state == WaitingForReply);

  if (!response_expected()) throw terminateProcessing();

  setServerDeadline(this);

  int idsize;
  const char* repoid = ex->_NP_repoId(&idsize);

# define TEST_AND_MARSHAL_SYSEXCEPTION(name) \
  if ( strcmp("IDL:omg.org/CORBA/" #name ":1.0",repoid) == 0 ) { \
    impl()->sendSystemException(this,*((CORBA::SystemException*)ex)); \
    pd_state = ReplyCompleted; \
    return; \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(TEST_AND_MARSHAL_SYSEXCEPTION)
# undef TEST_AND_MARSHAL_SYSEXCEPTION

  // this is not a system exception, treat it as a user exception
  // we do not check if this is a valid user exception for this operation.
  // caller should have checked this or else the user exception should have
  // been thrown as a C++ exception and got handled by the catch clause in
  // handleRequest.
  impl()->sendUserException(this,*((CORBA::UserException*)ex));
  pd_state = ReplyCompleted;

  clearValueTracker();
  clearDeadline();
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::notifyCommFailure(CORBA::Boolean,
			  CORBA::ULong& minor,
			  CORBA::Boolean& retry) {
  retry = 0;

  if (pd_state == WaitForRequestHeader ||
      pd_state == RequestIsBeingProcessed) {
    minor = COMM_FAILURE_UnMarshalArguments;
  }
  else if (pd_state == WaitingForReply) {
    minor = COMM_FAILURE_WaitingForReply;
  }
  else if (pd_state == ReplyIsBeingComposed) {
    minor = COMM_FAILURE_MarshalResults;
  }
  else {
    minor = TRANSIENT_ConnectionClosed;
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
GIOP_S::completion() {
  if (pd_state == WaitingForReply) {
    return (CORBA::ULong)CORBA::COMPLETED_MAYBE;
  }
  else if (pd_state == ReplyIsBeingComposed) {
    return (CORBA::ULong)CORBA::COMPLETED_YES;
  }
  else {
    return (CORBA::ULong)CORBA::COMPLETED_NO;
  }
}

////////////////////////////////////////////////////////////////////////
const char*
GIOP_S::operation_name() const {
  return operation();
}

////////////////////////////////////////////////////////////////////////
void
GIOP_S::unmarshalIORAddressingInfo() {
  GIOP::AddressingDisposition vp;
  CORBA::ULong   vl;

  resetKey();

  cdrStream& s = *this;

  vp <<= s;
  if (vp == GIOP::KeyAddr) {
    vl <<= s;
    if (!s.checkInputOverrun(1,vl)) {
      OMNIORB_THROW(MARSHAL,MARSHAL_SequenceIsTooLong,
		    (CORBA::CompletionStatus)completion());
    }
    keysize((int)vl);
    s.get_octet_array(key(),vl);
  }
  else {
    
    GIOP::IORAddressingInfo& ta = pd_target_address;
    if (vp == GIOP::ProfileAddr) {
      ta.ior.profiles.length(1);
      ta.ior.profiles[0] <<= s;
      ta.selected_profile_index = 0;
    }
    else {
      // GIOP::ReferenceAddr
      ta.selected_profile_index <<= s;
      ta.ior <<= s;
    }
    if (ta.selected_profile_index >= ta.ior.profiles.length() ||
	ta.ior.profiles[ta.selected_profile_index].tag != 
	                                              IOP::TAG_INTERNET_IOP) {
      if ( omniORB::trace(25) ) {
	omniORB::logger l;
	l << "unmarshal corrupted targetAddress at "
	  << __FILE__ << " line no. " << __LINE__ << "\n";
      }
      OMNIORB_THROW(BAD_PARAM,BAD_PARAM_IndexOutOfRange,
		    (CORBA::CompletionStatus)completion());
    }

    IIOP::ProfileBody decodedBody;
    IIOP::unmarshalProfile(ta.ior.profiles[ta.selected_profile_index],
			   decodedBody);

#if 0 // XXX Not finalise yet
    _OMNI_NS(giopAddressList) addresses;
    IIOP::extractAddresses(decodedBody,addresses);
    if ( isLocal(addresses) ) {
      keysize((int)decodedBody.object_key.length());
      memcpy((void*)key(),decodedBody.object_key.get_buffer(),keysize());
    }
    // XXX delete all the addresses.
#else
    OMNIORB_ASSERT(0);
#endif

    // Reach here either we have got the key of the target object
    // or we have the target address info in targetAddress().
    //
    if (keysize() < 0) {
      // We couldn't decode the target address to a local object key. Unless
      // an interceptor can decode it further, this request will be rejected.
      if ( omniORB::trace(25) ) {
	omniORB::logger l;
	l << "ProfileAddr or ReferenceAddr addresses unknown target at "
	  << __FILE__ << " line no. " << __LINE__ << "\n";
      }
    }
  }

}

OMNI_NAMESPACE_END(omni)
