// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniObjRef.cc              Created on: 26/2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2002-2011 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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

#include <omniORB4/CORBA.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/omniInterceptors.h>
#include <objectTable.h>
#include <objectAdapter.h>
#include <excepthandler.h>
#include <exceptiondefs.h>
#include <objectStub.h>
#include <giopStrand.h>
#include <giopStream.h>
#include <omniCurrent.h>
#include <poaimpl.h>
#include <initialiser.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <shutdownIdentity.h>
#include <invoker.h>
#include <interceptors.h>

OMNI_USING_NAMESPACE(omni)


static omniObjRef* objref_list = 0;
// Linked list of all non-nil object references.
//  Protected by <omni::objref_rc_lock>.


////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////

CORBA::Boolean orbParameters::verifyObjectExistsAndType = 1;
//  If the value of this variable is 0 then the ORB will not
//  send a GIOP LOCATE_REQUEST message to verify the existence of
//  the object prior to the first invocation. Setting this variable
//  if the other end is a buggy ORB that cannot handle GIOP
//  LOCATE_REQUEST. 
//
//  Valid values = 0 or 1

CORBA::Boolean orbParameters::copyValuesInLocalCalls = 1;
//  If the value of this variable is TRUE, valuetypes used in local
//  calls are properly copied, to retain local/remote transparency.
//  This involves copying all operation parameters / return values,
//  and it thus quite time consuming. If this parameter is set to
//  FALSE, valuetypes in local calls are not copied.
//
//  Valid values = 0 or 1

CORBA::Boolean orbParameters::resetTimeOutOnRetries = 0;
// If the value of this variable is TRUE, the call timeout is reset
// when an exception handler causes a call to be retried. If it is
// FALSE, the timeout is not reset, and therefore applies to the call
// as a whole, rather than each individual call attempt.


////////////////////////////////////////////////////////////////////////////
//             Object type and identity                                   //
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
const char*
omniObjRef::_localServantTarget()
{
  return pd_intfRepoId;
}


CORBA::Boolean
omniObjRef::_real_is_a(const char* repoId)
{
  if( !repoId )  return 0;

  if( _ptrToObjRef(repoId) )                             return 1;
  if( omni::ptrStrMatch(repoId, pd_mostDerivedRepoId) )  return 1;

  // Reach here because pd_flags.type_verified == 0, and we could not
  // verify the inheritance relationship using compile-time information.
  // Thus we ask our implementation if it is an instance of the given
  // type.

  return _remote_is_a(repoId);
}


void*
omniObjRef::_realNarrow(const char* repoId)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(repoId && *repoId);

  // Attempt to narrow the reference using static type info.
  void* target = _ptrToObjRef(repoId);

  if( target ) {
    omni::internalLock->lock();

    omniLocalIdentity* lid = omniLocalIdentity::downcast(_identity());
    if (!lid ||
	(lid && !lid->deactivated() && lid->servant() &&
	 lid->servant()->_ptrToInterface(repoId))) {

      // This object reference can be used directly.
      omni::internalLock->unlock();
      omni::duplicateObjRef(this);
    }
    else {
      // The interface type is OK, but we can't use this objref.
      // Create a new objref based on the IOR
      omni::internalLock->unlock();

      omniObjRef* objref;
      omniIOR*    ior;

      {
	omni_tracedmutex_lock sync(*omniIOR::lock);
	ior = pd_ior->duplicateNoLock();
      }

      {
	omni_tracedmutex_lock sync(*omni::internalLock);
	objref = omni::createObjRef(repoId,ior,1,0);
	objref->pd_flags.forward_location = pd_flags.forward_location;
	objref->pd_flags.type_verified = 1;
	objref->pd_flags.object_exists = 1;
      }

      if( objref ) {
	target = objref->_ptrToObjRef(repoId);
	OMNIORB_ASSERT(target);
      }
    }
  }
  else {
    // Either:
    //  1. This proxy object was created for an object type for which we
    //     do not have an authoritative answer to its inheritance relation.
    //  2. The object's actual most derived type has been changed to a
    //     more derived type after this object reference is generated.
    //     Since the type ID in the IOR is only considered as a hint,
    //     we cannot give up without asking the object about it.
    // Use _is_a() to query the object to find out if <repoId> is the
    // interface repo id of a base type of this object.

    if( _real_is_a(repoId) ) {
      // OK, the actual object does support the interface <repoId>.
      // However <repoId> is not necassarily a base type of our
      // most derived id -- since the implementation may have been
      // replaced by one of a more derived type.  In this case the
      // narrow will fail, since we can't be expected to know this.

      omniObjRef* objref;

      omniIOR* ior;

      {
	omni_tracedmutex_lock sync(*omniIOR::lock);
	ior = pd_ior->duplicateNoLock();
      }

      {
	omni_tracedmutex_lock sync(*omni::internalLock);
	objref = omni::createObjRef(repoId,ior,1,_identity());

        if (objref) {
          objref->pd_flags.forward_location = pd_flags.forward_location;
          objref->pd_flags.type_verified    = 1;
          objref->pd_flags.object_exists    = 1;
        }
      }

      if (objref) {
	target = objref->_ptrToObjRef(repoId);
	OMNIORB_ASSERT(target);
      }
    }
  }
  return target;
}


void*
omniObjRef::_uncheckedNarrow(const char* repoId)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(repoId && *repoId);

  // Attempt to narrow the reference using static type info.
  void* target = _ptrToObjRef(repoId);

  if( target ) {
    omni::internalLock->lock();

    omniLocalIdentity* lid = omniLocalIdentity::downcast(_identity());
    if (!lid ||
	(lid && !lid->deactivated() && lid->servant() &&
	 lid->servant()->_ptrToInterface(repoId))) {

      // This object reference can be used directly.
      omni::internalLock->unlock();
      omni::duplicateObjRef(this);
    }
    else {
      omni::internalLock->unlock();
      target = 0;
    }
  }
  if( !target ) {
    // Create a new objref based on the IOR

    omniObjRef* objref;
    omniIOR*    ior;

    {
      omni_tracedmutex_lock sync(*omniIOR::lock);
      ior = pd_ior->duplicateNoLock();
    }
    {
      omni_tracedmutex_lock sync(*omni::internalLock);
      objref = omni::createObjRef(repoId,ior,1,0);
      objref->pd_flags.forward_location = pd_flags.forward_location;
      objref->pd_flags.type_verified = 1;
    }
    target = objref->_ptrToObjRef(repoId);
  }
  OMNIORB_ASSERT(target);
  return target;
}



void
omniObjRef::_assertExistsAndTypeVerified()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  // NB. We don't lock here to protect <pd_flags> while reading.  This
  // just means that there could potentially be multiple threads doing
  // the following work simultaneously.  A waste, but not a disaster.
  // We do need to lock when writing of course.

  if( !pd_flags.type_verified ) {

    if( !_remote_is_a(pd_intfRepoId) ) {
      if( omniORB::trace(1) ) {
	omniORB::logger log;
	log << "The object with the repository ID: "
	    << pd_mostDerivedRepoId
	    << " returns FALSE to the query _is_a(\""
	    << pd_intfRepoId << "\"). A CORBA::INV_OBJREF is raised.\n";
      }
      OMNIORB_THROW(INV_OBJREF,INV_OBJREF_InterfaceMisMatch,
		    CORBA::COMPLETED_NO);
    }
    {
      omni::internalLock->lock();
      pd_flags.type_verified = 1;
      pd_flags.object_exists = 1;
      omni::internalLock->unlock();
    }
    return;
  }

  if( !pd_flags.object_exists ) {

    _locateRequest();
    {
      omni::internalLock->lock();
      pd_flags.object_exists = 1;
      omni::internalLock->unlock();
    }

  }
}

CORBA::Boolean 
omniObjRef::__is_equivalent(omniObjRef* o_obj)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  {
    omni_tracedmutex_lock sync(*omni::internalLock);

    return pd_id->is_equivalent(o_obj->pd_id);
  }
}

CORBA::ULong
omniObjRef::__hash(CORBA::ULong maximum)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  const CORBA::Octet* key;
  int keysize;

  {
    omni_tracedmutex_lock sync(*omni::internalLock);
    key = pd_id->key();
    keysize = pd_id->keysize();

    return CORBA::ULong(omni::hash(key, keysize) % maximum);
  }
}


//////////////////////////////////////////////////////////////////////
CORBA::Boolean
omniObjRef::_remote_is_a(const char* a_repoId)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  omni_is_a_CallDesc call_desc("_is_a", sizeof("_is_a"), a_repoId);

  _invoke(call_desc, 0);
  return call_desc.result;
}

//////////////////////////////////////////////////////////////////////
CORBA::Boolean
omniObjRef::_remote_non_existent()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  omni_non_existent_CallDesc call_desc("_non_existent",
				       sizeof("_non_existent"));

  _invoke(call_desc, 0);
  return call_desc.result;
}

////////////////////////////////////////////////////////////////////////////
//             Exception handlers                                         //
////////////////////////////////////////////////////////////////////////////

void*
omniObjRef::_transientExceptionHandler(void*& cookie, _CORBA_Boolean& ext)
{
  if (pd_flags.transient_exception_handler) {
    omniExHandlers_iterator iter;
    omniExHandlers* hp = iter.find(this);
    if (hp != 0) {
      cookie = hp->transient_cookie;
      ext    = hp->transient_ext;
      return   hp->transient_hdr;
    }
  }
  return 0;
}


void
omniObjRef::_transientExceptionHandler(void* new_handler, void* cookie,
                                       _CORBA_Boolean ext)
{
  omniExHandlers_iterator iter;
  omniExHandlers* hp = iter.find_or_create(this);

  hp->transient_cookie = cookie;
  hp->transient_ext    = ext;
  hp->transient_hdr    = new_handler;

  pd_flags.transient_exception_handler = 1;
}


void*
omniObjRef::_timeoutExceptionHandler(void*& cookie, _CORBA_Boolean& ext)
{
  if (pd_flags.timeout_exception_handler) {
    omniExHandlers_iterator iter;
    omniExHandlers* hp = iter.find(this);
    if (hp != 0) {
      cookie = hp->timeout_cookie;
      ext    = hp->timeout_ext;
      return   hp->timeout_hdr;
    }
  }
  return 0;
}


void
omniObjRef::_timeoutExceptionHandler(void* new_handler, void* cookie,
                                     _CORBA_Boolean ext)
{
  omniExHandlers_iterator iter;
  omniExHandlers* hp = iter.find_or_create(this);

  hp->timeout_cookie = cookie;
  hp->timeout_ext    = ext;
  hp->timeout_hdr    = new_handler;

  pd_flags.timeout_exception_handler = 1;
}


void*
omniObjRef::_commFailureExceptionHandler(void*& cookie, _CORBA_Boolean& ext)
{
  if (pd_flags.commfail_exception_handler) {
    omniExHandlers_iterator iter;
    omniExHandlers* hp = iter.find(this);
    if (hp != 0) {
      cookie = hp->commfail_cookie;
      ext    = hp->commfail_ext;
      return   hp->commfail_hdr;
    }
  }
  return 0;
}


void
omniObjRef::_commFailureExceptionHandler(void* new_handler, void* cookie,
                                         _CORBA_Boolean ext)
{
  omniExHandlers_iterator iter;
  omniExHandlers* hp = iter.find_or_create(this);

  hp->commfail_cookie = cookie;
  hp->commfail_ext    = ext;
  hp->commfail_hdr    = new_handler;

  pd_flags.commfail_exception_handler = 1;
}


void*
omniObjRef::_systemExceptionHandler(void*& cookie, _CORBA_Boolean& ext)
{
  if (pd_flags.system_exception_handler) {
    omniExHandlers_iterator iter;
    omniExHandlers* hp = iter.find(this);
    if (hp != 0) {
      cookie = hp->sysexcpt_cookie;
      ext    = hp->sysexcpt_ext;
      return   hp->sysexcpt_hdr;
    }
  }
  return 0;
}


void
omniObjRef::_systemExceptionHandler(void* new_handler, void* cookie,
                                    _CORBA_Boolean ext)
{
  omniExHandlers_iterator iter;
  omniExHandlers* hp = iter.find_or_create(this);

  hp->sysexcpt_cookie = cookie;
  hp->sysexcpt_ext    = ext;
  hp->sysexcpt_hdr    = new_handler;

  pd_flags.system_exception_handler = 1;
}


////////////////////////////////////////////////////////////////////////////
//             Object lifecycle                                           //
////////////////////////////////////////////////////////////////////////////

omniObjRef::~omniObjRef()
{
  if (pd_refCount)
    omniORB::logs(1, "Error: an object reference has been explicity deleted.");

  if (!pd_ior) return; // Nil

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  {
    omni_tracedmutex_lock sync(*omni::objref_rc_lock);
    *pd_prev = pd_next;
    if (pd_next) pd_next->pd_prev = pd_prev;
  }

  if (pd_flags.static_repoId) {
    if (pd_mostDerivedRepoId != pd_intfRepoId)
      delete [] pd_mostDerivedRepoId;
  }
  else {
    if( pd_intfRepoId != pd_mostDerivedRepoId )
      delete[] pd_intfRepoId;
    if( pd_mostDerivedRepoId )  delete[] pd_mostDerivedRepoId;
  }

  if (pd_ior) pd_ior->release();

  OMNIORB_ASSERT(pd_id == 0);
}


omniObjRef::omniObjRef()
  : pd_refCount(0),
    pd_mostDerivedRepoId(0),
    pd_intfRepoId(0),
    pd_ior(0),
    pd_id(0),
    pd_next(0),
    pd_prev(0)
{
  // Nil objref.
  pd_flags.orb_shutdown = 0;
}


omniObjRef::omniObjRef(const char* intfRepoId, omniIOR* ior,
		       omniIdentity* id, _CORBA_Boolean static_repoId)
  : pd_refCount(1),
    pd_ior(ior),
    pd_id(id)
{
  OMNIORB_ASSERT(intfRepoId);
  OMNIORB_ASSERT(ior);
  OMNIORB_ASSERT(id);

  if (static_repoId) {
    pd_intfRepoId = (char*)intfRepoId; // Excuse the dodgy cast :-)
  }
  else {
    pd_intfRepoId = new char[strlen(intfRepoId) + 1];
    strcpy(pd_intfRepoId, intfRepoId);
  }

  if( omni::ptrStrMatch(intfRepoId, ior->repositoryID()) ) {
    pd_mostDerivedRepoId = pd_intfRepoId;
  }
  else {
    pd_mostDerivedRepoId = new char[strlen(ior->repositoryID()) + 1];
    strcpy(pd_mostDerivedRepoId, ior->repositoryID());
  }

  {
    omni_tracedmutex_lock sync(*omni::objref_rc_lock);
    pd_next = objref_list;
    pd_prev = &objref_list;
    if (pd_next) pd_next->pd_prev = &pd_next;
    objref_list = this;
  }

  pd_flags.forward_location            = 0;
  pd_flags.type_verified               = 1;
  pd_flags.object_exists               = omniObjTableEntry::downcast(id) ? 1:0;
  pd_flags.transient_exception_handler = 0;
  pd_flags.timeout_exception_handler   = 0;
  pd_flags.commfail_exception_handler  = 0;
  pd_flags.system_exception_handler    = 0;
  pd_flags.static_repoId               = static_repoId;
  pd_flags.orb_shutdown                = 0;
}

void
omniObjRef::_shutdown()
{
  omni_tracedmutex_lock sync(*omni::internalLock);
  omni_tracedmutex_lock sync2(*omni::objref_rc_lock);

  int i=0;

  for (omniObjRef* o = objref_list; o; o = o->pd_next, i++)
    o->_disable();

  if (omniORB::trace(15)) {
    omniORB::logger l;
    l << i << " object reference" << (i == 1 ? "" : "s")
      << " present at ORB shutdown.\n";
  }
}

void
omniObjRef::_disable()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "Disable ObjRef(" << pd_mostDerivedRepoId << ") " << pd_id << "\n";
  }
  _setIdentity(omniShutdownIdentity::singleton());
  pd_flags.orb_shutdown = 1;
}


////////////////////////////////////////////////////////////////////////////
//             Invocation                                                 //
////////////////////////////////////////////////////////////////////////////

void
omniObjRef::_invoke(omniCallDescriptor& call_desc, CORBA::Boolean do_assert)
{
#define	RECOVER_FORWARD do {\
  omni::revertToOriginalProfile(this); \
  CORBA::TRANSIENT ex2(TRANSIENT_FailedOnForwarded, ex.completed()); \
  if (!_omni_callTransientExceptionHandler(this, retries++, ex2,\
                                           call_desc.op())) \
    throw; \
} while(0)

  int retries = 0;
#if defined(__DECCXX) && __DECCXX_VER < 60300000
  // Work-around for bug in Compaq C++ optimiser
  volatile
#endif
  int fwd = 0;

  if (_is_nil())
    _CORBA_invoked_nil_objref();

  call_desc.objref(this);

  omniCurrent* current;
  omni_time_t  abs_time;

  while (1) {

    CORBA::Boolean required_retry      = 0;
    CORBA::Boolean retry_after_timeout = 0;

    if (orbParameters::verifyObjectExistsAndType && do_assert)
      _assertExistsAndTypeVerified();

    if (!abs_time) {
      if (pd_timeout) {
	omni_thread::get_time(abs_time, pd_timeout);
      }
      else if (orbParameters::supportPerThreadTimeOut &&
	       (current = omniCurrent::get()) &&
	       (current->timeout())) {
    
	if (current->timeout_absolute()) {
	  abs_time = current->timeout();
	}
	else {
	  omni_thread::get_time(abs_time, current->timeout());
	}
      }
      else if (orbParameters::clientCallTimeOutPeriod) {

	omni_thread::get_time(abs_time,
			      orbParameters::clientCallTimeOutPeriod);
      }
      call_desc.setDeadline(abs_time);
    }

    try {
      omni::internalLock->lock();

      fwd = pd_flags.forward_location;

      _identity()->dispatch(call_desc);
      return;
    }
    catch (const giopStream::CommFailure& ex) {
      if (ex.retry()) {
	required_retry = 1;
      }
      else {
	if (fwd) {
	  omni::revertToOriginalProfile(this);
	  CORBA::TRANSIENT ex2(TRANSIENT_FailedOnForwarded, ex.completed());

	  if (!_omni_callTransientExceptionHandler(this, retries++, ex2,
                                                   call_desc.op())) {

	    if (is_COMM_FAILURE_minor(ex.minor())) {
	      OMNIORB_THROW(COMM_FAILURE,ex.minor(),ex.completed());
	    }
	    else if (!orbParameters::throwTransientOnTimeOut &&
		     ex.minor() == TRANSIENT_CallTimedout) {
	      OMNIORB_THROW(TIMEOUT,ex.minor(),ex.completed());
	    }
	    else {
	      OMNIORB_THROW(TRANSIENT,ex.minor(),ex.completed());
	    }
	  }
	}
	else if (is_COMM_FAILURE_minor(ex.minor())) {

	  CORBA::COMM_FAILURE ex2(ex.minor(), ex.completed());
	  if (!_omni_callCommFailureExceptionHandler(this, retries++, ex2,
                                                     call_desc.op())) {
	    OMNIORB_THROW(COMM_FAILURE,ex.minor(),ex.completed());
	  }
	}
	else if (!orbParameters::throwTransientOnTimeOut &&
		 ex.minor() == TRANSIENT_CallTimedout) {

	  CORBA::TIMEOUT ex2(ex.minor(), ex.completed());
	  if (!_omni_callTimeoutExceptionHandler(this, retries++, ex2,
                                                 call_desc.op())) {
	    OMNIORB_THROW(TIMEOUT,ex.minor(),ex.completed());
	  }
	  retry_after_timeout = 1;
	}
	else {
	  CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	  if (!_omni_callTransientExceptionHandler(this, retries++, ex2,
                                                   call_desc.op())) {
	    OMNIORB_THROW(TRANSIENT,ex.minor(),ex.completed());
	  }
	}
      }
    }
    catch (CORBA::COMM_FAILURE& ex) {
      if (fwd) {
	RECOVER_FORWARD;
      }
      else if (!_omni_callCommFailureExceptionHandler(this, retries++, ex,
                                                      call_desc.op()))
	OMNIORB_THROW(COMM_FAILURE,ex.minor(),ex.completed());
    }
    catch (CORBA::TRANSIENT& ex) {
      if (!_omni_callTransientExceptionHandler(this, retries++, ex,
                                               call_desc.op()))
	OMNIORB_THROW(TRANSIENT,ex.minor(),ex.completed());
    }
    catch (CORBA::TIMEOUT& ex) {
      if (!_omni_callTimeoutExceptionHandler(this, retries++, ex,
                                             call_desc.op()))
	OMNIORB_THROW(TIMEOUT,ex.minor(),ex.completed());
      retry_after_timeout = 1;
    }
    catch (CORBA::OBJECT_NOT_EXIST& ex) {
      if (fwd) {
	RECOVER_FORWARD;
      }
      else if (!_omni_callSystemExceptionHandler(this, retries++, ex,
                                                 call_desc.op()))
	OMNIORB_THROW(OBJECT_NOT_EXIST,ex.minor(),ex.completed());
    }
    catch (CORBA::SystemException& ex) {
      if (!_omni_callSystemExceptionHandler(this, retries++, ex,
                                            call_desc.op()))
	throw;
    }
    catch (omniORB::LOCATION_FORWARD& ex) {
      if (CORBA::is_nil(ex.get_obj())) {
	CORBA::TRANSIENT ex2(TRANSIENT_NoUsableProfile, CORBA::COMPLETED_NO);
	omniORB::logs(10, "Received GIOP::LOCATION_FORWARD message that"
		      " contains a nil object reference.");
	if (!_omni_callTransientExceptionHandler(this, retries++, ex2,
                                                 call_desc.op()))
	  OMNIORB_THROW(TRANSIENT,ex2.minor(),ex2.completed());
      }
      omni::locationForward(this,ex.get_obj()->_PR_getobj(),ex.is_permanent());
    }
    
    if (!required_retry &&
	(retry_after_timeout || orbParameters::resetTimeOutOnRetries)) {

      // Reset the timeout next time around
      abs_time.assign(0,0);
    }
  }
}

void
omniObjRef::_locateRequest()
{
  omniCallDescriptor call_desc(0,0,0,0,0,0,0);
  _invoke(call_desc, 0);
}


////////////////////////////////////////////////////////////////////////////
//             Asynchronous invocation                                    //
////////////////////////////////////////////////////////////////////////////

OMNI_NAMESPACE_BEGIN(omni)

class AsyncRequest : public omniTask {
public:
  inline AsyncRequest(omniObjRef*              objref,
		      omniAsyncCallDescriptor* call_desc,
		      const omni_time_t&       thread_timeout,
		      CORBA::Boolean           timeout_absolute)
    : omniTask(omniTask::AnyTime, omniTask::ClientInvocation),
      pd_objref(objref),
      pd_callDescriptor(call_desc),
      pd_threadTimeout(thread_timeout),
      pd_timeoutAbsolute(timeout_absolute)
  {
    duplicateObjRef(objref);
  }

  void execute();

protected:
  virtual ~AsyncRequest();

private:
  omniObjRef*              pd_objref;
  omniAsyncCallDescriptor* pd_callDescriptor;
  omni_time_t              pd_threadTimeout;
  CORBA::Boolean           pd_timeoutAbsolute;

  // Not implemented
  AsyncRequest(const AsyncRequest&);
  AsyncRequest& operator=(const AsyncRequest&);
};


void
AsyncRequest::execute()
{
  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "Asynchronous invoke '" << pd_callDescriptor->op() << "'...\n";
  }

  try {
    if (pd_threadTimeout) {
      omniCurrent* current = omniCurrent::get();

      if (!current)
        OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PerThreadTimeoutWithNoCurrent,
                      CORBA::COMPLETED_NO);

      if (pd_timeoutAbsolute)
        current->setDeadline(pd_threadTimeout);
      else
        current->setTimeout(pd_threadTimeout);
    }

    pd_objref->_invoke(*pd_callDescriptor);

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Asynchronous invoke '" << pd_callDescriptor->op()
	  << "' done\n";
    }
  }
  catch (const CORBA::SystemException& ex) {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Asynchronous invoke '" << pd_callDescriptor->op()
	  << "' raised CORBA::" << ex._name() << "\n";
    }
    pd_callDescriptor->storeException(ex);
  }
  catch (const CORBA::UserException& ex) {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Asynchronous invoke '" << pd_callDescriptor->op()
	  << "' raised " << ex._name() << "\n";
    }
    pd_callDescriptor->storeException(ex);
  }
  catch (...) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Unexpected exception performing asynchronous request '"
	  << pd_callDescriptor->op() << "'\n";
    }
    CORBA::UNKNOWN ex(UNKNOWN_UserException, CORBA::COMPLETED_NO);
    pd_callDescriptor->storeException(ex);
  }

  try {
    pd_callDescriptor->setComplete();
  }
  catch (const CORBA::UserException& ex) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Asynchronous complete callback for '" << pd_callDescriptor->op()
	  << "' raised " << ex._name() << "\n";
    }
  }
  catch (const CORBA::SystemException& ex) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Asynchronous complete callback for '" << pd_callDescriptor->op()
	  << "' raised CORBA::" << ex._name() << "\n";
    }
  }
  catch (...) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Unexpected exception performing complete callback for "
	  << "asynchronous request '" << pd_callDescriptor->op() << "'\n";
    }
  }

  delete this;
}

AsyncRequest::~AsyncRequest()
{
  releaseObjRef(pd_objref);
}

OMNI_NAMESPACE_END(omni)


void
omniObjRef::_invoke_async(omniAsyncCallDescriptor* call_desc)
{
  // Store objref in call descriptor straight away so calling code can
  // request it from the AMI poller.
  call_desc->objref(this);

  omni_time_t    thread_timeout;
  CORBA::Boolean timeout_absolute = 0;

  if (!pd_timeout && orbParameters::supportPerThreadTimeOut) {
    omniCurrent* current;
    if ((current = omniCurrent::get())) {
      thread_timeout   = current->timeout();
      timeout_absolute = current->timeout_absolute();
    }
  }
  AsyncRequest* req = new AsyncRequest(this, call_desc,
				       thread_timeout, timeout_absolute);
  orbAsyncInvoker->insert(req);
}


////////////////////////////////////////////////////////////////////////////
//             IORs and marshalling                                       //
////////////////////////////////////////////////////////////////////////////

omniIOR* 
omniObjRef::_getIOR()
{
  omni_tracedmutex_lock sync(*omniIOR::lock);
  // Must hold mutex before reading pd_ior.
  return pd_ior->duplicateNoLock();
}

void
omniObjRef::_marshal(omniObjRef* objref, cdrStream& s)
{
  if (!objref || objref->_is_nil()) {
    ::operator>>= ((CORBA::ULong)1,s);
    s.marshalOctet('\0');
    ::operator>>= ((CORBA::ULong) 0,s);
    return;
  }

  if (objref->pd_flags.orb_shutdown)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_ORBHasShutdown,
		  (CORBA::CompletionStatus)s.completion());

  omniIOR_var ior;
  {
    omni_tracedmutex_lock sync(*omniIOR::lock);
    // Must hold mutex before reading pd_ior.
    ior =  objref->pd_ior->duplicateNoLock();
  }

  s.marshalRawString(ior->repositoryID());
  (const IOP::TaggedProfileList&)ior->iopProfiles() >>= s;

  // Provide interim BiDir GIOP support. If this is a giopStream and this
  // is the client side of a bidirectional stream, mark the stream to
  // indicate that an object reference has been sent to the other end.
  // We assume all these references are callback objects.
  // The reason for doing so is that if subsequently this connection is
  // broken, the callback objects would not be able to callback to us.
  // We want the application to know about this.
  giopStream* gs = giopStream::downcast(&s);
  if (gs) {
    giopStrand& g = gs->strand();
    if (g.isBiDir() && g.isClient()) {
      g.biDir_has_callbacks = 1;
    }
  }
}

char*
omniObjRef::_toString(omniObjRef* objref)
{
  cdrMemoryStream buf(CORBA::ULong(0),CORBA::Boolean(1));
  buf.marshalOctet(omni::myByteOrder);
  _marshal(objref,buf);

  // turn the encapsulation into a hex string with "IOR:" prepended
  buf.rewindInputPtr();
  size_t s = buf.bufSize();
  CORBA::Char * data = (CORBA::Char *)buf.bufPtr();

  char *result = new char[4+s*2+1];
  result[4+s*2] = '\0';
  result[0] = 'I';
  result[1] = 'O';
  result[2] = 'R';
  result[3] = ':';
  for (int i=0; i < (int)s; i++) {
    int j = 4 + i*2;
    int v = (data[i] & 0xf0);
    v = v >> 4;
    if (v < 10)
      result[j] = '0' + v;
    else
      result[j] = 'a' + (v - 10);
    v = ((data[i] & 0xf));
    if (v < 10)
      result[j+1] = '0' + v;
    else
      result[j+1] = 'a' + (v - 10);
  }
  return result;
}


omniObjRef*
omniObjRef::_unMarshal(const char* repoId, cdrStream& s)
{
  CORBA::String_var id;
  IOP::TaggedProfileList_var profiles;

  id = IOP::IOR::unmarshaltype_id(s);
  
  profiles = new IOP::TaggedProfileList();
  (IOP::TaggedProfileList&)profiles <<= s;

  if (profiles->length() == 0 && strlen(id) == 0) {
    // This is a nil object reference
    return 0;
  }
  // It is possible that we reach here with the id string = '\0'.
  // That is alright because the actual type of the object will be
  // verified using _is_a() at the first invocation on the object.
  //
  // Apparently, some ORBs such as ExperSoft's do that. Furthermore,
  // this has been accepted as a valid behaviour in GIOP 1.1/IIOP 1.1.
  // 
  omniIOR* ior = new omniIOR(id._retn(),profiles._retn());

  // Provide interim BiDir GIOP support. Check the stream where this IOR
  // comes from. If it is a giopStream and this is the server side of
  // a bidirectional stream, add the component tag TAG_OMNIORB_BIDIR to the
  // ior's IOP profile list. The component will be decoded in createObjRef.
  //
  // In the next revision to bidir giop, as documented in OMG doc. 2001-06-04 
  // and if it ever gets adopted in a future GIOP version, the tag component
  // TAG_BI_DIR_GIOP will be embedded in the IOR and this step will be
  // redundent.
  giopStream* gs = giopStream::downcast(&s);
  if (gs) {
    giopStrand& g = gs->strand();
    if (g.isBiDir() && !g.isClient()) {
      // Check the POA policy to see if the servant's POA is willing
      // to use bidirectional on its callback objects.
      omniCurrent* current = omniCurrent::get();
      omniCallDescriptor* desc = ((current)? current->callDescriptor() : 0);
      if (desc && desc->poa() && desc->poa()->acceptBiDirectional()) {
	const char* sendfrom = g.connection->peeraddress();
	omniIOR::add_TAG_OMNIORB_BIDIR(sendfrom,*ior);
      }
    }
  }

  omniObjRef* objref = omni::createObjRef(repoId,ior,0);
  return objref;
}

omniObjRef*
omniObjRef::_fromString(const char* str)
{
  size_t s = (str ? strlen(str) : 0);
  if (s<4)
    OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
  const char *p = str;
  if (!((p[0] == 'I' || p[0] == 'i') &&
	(p[1] == 'O' || p[1] == 'o') &&
	(p[2] == 'R' || p[2] == 'r') &&
	(p[3] == ':')))
    OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);

  s = (s-4)/2;  // how many octets are there in the string
  p += 4;

  cdrMemoryStream buf((CORBA::ULong)s,0);

  for (int i=0; i<(int)s; i++) {
    int j = i*2;
    CORBA::Octet v=0;
    
    if (p[j] >= '0' && p[j] <= '9') {
      v = ((p[j] - '0') << 4);
    }
    else if (p[j] >= 'a' && p[j] <= 'f') {
      v = ((p[j] - 'a' + 10) << 4);
    }
    else if (p[j] >= 'A' && p[j] <= 'F') {
      v = ((p[j] - 'A' + 10) << 4);
    }
    else
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);

    if (p[j+1] >= '0' && p[j+1] <= '9') {
      v += (p[j+1] - '0');
    }
    else if (p[j+1] >= 'a' && p[j+1] <= 'f') {
      v += (p[j+1] - 'a' + 10);
    }
    else if (p[j+1] >= 'A' && p[j+1] <= 'F') {
      v += (p[j+1] - 'A' + 10);
    }
    else
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
    buf.marshalOctet(v);
  }

  buf.rewindInputPtr();
  CORBA::Boolean b = buf.unmarshalBoolean();
  buf.setByteSwapFlag(b);

  return _unMarshal(CORBA::Object::_PD_repoId,buf);
}

void
omniObjRef::_setIdentity(omniIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if (id != pd_id) {
    if (pd_id) pd_id->loseRef(this);
    pd_id = id;
    if (pd_id) pd_id->gainRef(this);
  }
}

void
omniObjRef::_enableShortcut(omniServant*, const _CORBA_Boolean*)
{
  // Default does nothing
}


/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////


void
omniObjRef::_NP_disconnect()
{
  omni::internalLock->lock();
  _identity()->disconnect();
}


OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////////
class verifyObjectExistsAndTypeHandler : public orbOptions::Handler {
public:

  verifyObjectExistsAndTypeHandler() : 
    orbOptions::Handler("verifyObjectExistsAndType",
			"verifyObjectExistsAndType = 0 or 1",
			1,
			"-ORBverifyObjectExistsAndType < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::verifyObjectExistsAndType = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::verifyObjectExistsAndType,
			     result);
  }
};

static verifyObjectExistsAndTypeHandler verifyObjectExistsAndTypeHandler_;


/////////////////////////////////////////////////////////////////////////////
class copyValuesInLocalCallsHandler : public orbOptions::Handler {
public:

  copyValuesInLocalCallsHandler() : 
    orbOptions::Handler("copyValuesInLocalCalls",
			"copyValuesInLocalCalls = 0 or 1",
			1,
			"-ORBcopyValuesInLocalCalls < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::copyValuesInLocalCalls = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::copyValuesInLocalCalls,
			     result);
  }
};

static copyValuesInLocalCallsHandler copyValuesInLocalCallsHandler_;


/////////////////////////////////////////////////////////////////////////////
class resetTimeOutOnRetriesHandler : public orbOptions::Handler {
public:

  resetTimeOutOnRetriesHandler() : 
    orbOptions::Handler("resetTimeOutOnRetries",
			"resetTimeOutOnRetries = 0 or 1",
			1,
			"-ORBresetTimeOutOnRetries < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::resetTimeOutOnRetries = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::resetTimeOutOnRetries,
			     result);
  }
};

static resetTimeOutOnRetriesHandler resetTimeOutOnRetriesHandler_;


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
class omni_ObjRef_initialiser : public omniInitialiser {
public:

  omni_ObjRef_initialiser() {
    orbOptions::singleton().registerHandler(verifyObjectExistsAndTypeHandler_);
    orbOptions::singleton().registerHandler(copyValuesInLocalCallsHandler_);
    orbOptions::singleton().registerHandler(resetTimeOutOnRetriesHandler_);
  }

  void attach() { }
  void detach() { }
};


static omni_ObjRef_initialiser initialiser;

omniInitialiser& omni_ObjRef_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
