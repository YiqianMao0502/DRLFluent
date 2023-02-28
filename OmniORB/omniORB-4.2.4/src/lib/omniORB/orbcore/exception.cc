// -*- Mode: C++; -*-
//                            Package   : omniORB
// exception.cc               Created on: 13/5/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2005-2014 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <excepthandler.h>
#include <omniORB4/omniObjRef.h>
#include <exceptiondefs.h>
#include <omniORB4/minorCode.h>

OMNI_USING_NAMESPACE(omni)

static CORBA::Boolean
omni_defaultTransientExcHandler(void*,
                                CORBA::ULong            n_retries,
				const CORBA::TRANSIENT& ex,
                                CORBA::Object_ptr       obj,
                                const char*             op);
static CORBA::Boolean
omni_defaultTimeoutExcHandler(void*, CORBA::ULong n_retries,
                              const CORBA::TIMEOUT& ex);
static CORBA::Boolean
omni_defaultCommFailureExcHandler(void*, CORBA::ULong n_retries,
				  const CORBA::COMM_FAILURE& ex);
static CORBA::Boolean
omni_defaultSystemExcHandler(void*, CORBA::ULong n_retries,
			     const CORBA::SystemException& ex);

static void* omni_globalTransientExcHandler =
                                       (void*)omni_defaultTransientExcHandler;
static void* omni_globalTimeoutExcHandler =
                                       (void*)omni_defaultTimeoutExcHandler;
static void* omni_globalCommFailureExcHandler =
                                       (void*)omni_defaultCommFailureExcHandler;
static void* omni_globalSystemExcHandler =
                                       (void*)omni_defaultSystemExcHandler;

static CORBA::Boolean omni_globalTransientExcHandlerExt   = 1;
static CORBA::Boolean omni_globalTimeoutExcHandlerExt     = 0;
static CORBA::Boolean omni_globalCommFailureExcHandlerExt = 0;
static CORBA::Boolean omni_globalSystemExcHandlerExt      = 0;

static void* omni_globalTransientExcHandlerCookie   = 0;
static void* omni_globalTimeoutExcHandlerCookie     = 0;
static void* omni_globalCommFailureExcHandlerCookie = 0;
static void* omni_globalSystemExcHandlerCookie      = 0;


omniExHandlers** omniExHandlers::Table = 0;
omni_tracedmutex omniExHandlers::TableLock("omniExHandlers::TableLock");


//
// Transient

static CORBA::Boolean
omni_defaultTransientExcHandler(void*,
				CORBA::ULong            n_retries,
				const CORBA::TRANSIENT& ex,
                                CORBA::Object_ptr       obj,
                                const char*             op)
{
  if (ex.minor() == TRANSIENT_FailedOnForwarded) {
    if (omniORB::trace(10)) {
      omniORB::logger log;
      if (op)
        log << "Invocation '" << op << "'";
      else
        log << "LocateRequest";

      log << " on a location forwarded object has failed. "
          << n_retries << " retries.\n";
    }
    unsigned long secs;
    secs = ((n_retries < 30) ? n_retries : 30);
    if (secs) omni_thread::sleep(secs,0);
    return 1;
  }
  return 0;
}

void
omniORB::installTransientExceptionHandler(void* cookie,
			       omniORB::transientExceptionHandler_t fn)
{
  omni_globalTransientExcHandler       = (void*)fn;
  omni_globalTransientExcHandlerCookie = cookie;
  omni_globalTransientExcHandlerExt    = 0;
}

void
omniORB::installTransientExceptionHandlerExt(void* cookie,
			       omniORB::transientExceptionHandlerExt_t fn)
{
  omni_globalTransientExcHandler       = (void*)fn;
  omni_globalTransientExcHandlerCookie = cookie;
  omni_globalTransientExcHandlerExt    = 1;
}

void
omniORB::installTransientExceptionHandler(CORBA::Object_ptr obj,
				   void* cookie,
				   omniORB::transientExceptionHandler_t fn)
{
  if (CORBA::is_nil(obj)) return;
  obj->_PR_getobj()->_transientExceptionHandler((void*)fn, cookie, 0);
}

void
omniORB::installTransientExceptionHandlerExt(CORBA::Object_ptr obj,
				   void* cookie,
				   omniORB::transientExceptionHandlerExt_t fn)
{
  if (CORBA::is_nil(obj)) return;
  obj->_PR_getobj()->_transientExceptionHandler((void*)fn, cookie, 1);
}


//
// Timeout

static CORBA::Boolean
omni_defaultTimeoutExcHandler(void*,
			      CORBA::ULong,
			      const CORBA::TIMEOUT&)
{
  return 0;
}

void
omniORB::installTimeoutExceptionHandler(void* cookie,
					omniORB::timeoutExceptionHandler_t fn)
{
  omni_globalTimeoutExcHandler       = (void*)fn;
  omni_globalTimeoutExcHandlerCookie = cookie;
  omni_globalTimeoutExcHandlerExt    = 0;
}

void
omniORB::installTimeoutExceptionHandlerExt(void* cookie,
                                       omniORB::timeoutExceptionHandlerExt_t fn)
{
  omni_globalTimeoutExcHandler       = (void*)fn;
  omni_globalTimeoutExcHandlerCookie = cookie;
  omni_globalTimeoutExcHandlerExt    = 1;
}

void
omniORB::installTimeoutExceptionHandler(CORBA::Object_ptr obj,
					void*             cookie,
					omniORB::timeoutExceptionHandler_t fn)
{
  if (CORBA::is_nil(obj)) return;
  obj->_PR_getobj()->_timeoutExceptionHandler((void*)fn, cookie, 0);
}

void
omniORB::installTimeoutExceptionHandlerExt(CORBA::Object_ptr obj,
                                      void* cookie,
                                      omniORB::timeoutExceptionHandlerExt_t fn)
{
  if (CORBA::is_nil(obj)) return;
  obj->_PR_getobj()->_timeoutExceptionHandler((void*)fn, cookie, 1);
}


//
// COMM_FAILURE

static CORBA::Boolean
omni_defaultCommFailureExcHandler(void*,
				  CORBA::ULong,
				  const CORBA::COMM_FAILURE&)
{
  return 0;
}

void
omniORB::installCommFailureExceptionHandler(void* cookie,
				  omniORB::commFailureExceptionHandler_t fn)
{
  omni_globalCommFailureExcHandler       = (void*)fn;
  omni_globalCommFailureExcHandlerCookie = cookie;
  omni_globalCommFailureExcHandlerExt    = 0;
}

void
omniORB::installCommFailureExceptionHandlerExt(void* cookie,
				  omniORB::commFailureExceptionHandlerExt_t fn)
{
  omni_globalCommFailureExcHandler       = (void*)fn;
  omni_globalCommFailureExcHandlerCookie = cookie;
  omni_globalCommFailureExcHandlerExt    = 1;
}

void
omniORB::installCommFailureExceptionHandler(CORBA::Object_ptr obj,
				   void* cookie,
				   omniORB::commFailureExceptionHandler_t fn)
{
  if (CORBA::is_nil(obj)) return;
  obj->_PR_getobj()->_commFailureExceptionHandler((void*)fn, cookie, 0);
}

void
omniORB::installCommFailureExceptionHandlerExt(CORBA::Object_ptr obj,
				   void* cookie,
				   omniORB::commFailureExceptionHandlerExt_t fn)
{
  if (CORBA::is_nil(obj)) return;
  obj->_PR_getobj()->_commFailureExceptionHandler((void*)fn, cookie, 1);
}


//
// SystemException

static CORBA::Boolean
omni_defaultSystemExcHandler(void*,
			     CORBA::ULong,
			     const CORBA::SystemException&)
{	
  return 0;
}


void
omniORB::installSystemExceptionHandler(void* cookie,
				       omniORB::systemExceptionHandler_t fn)
{
 omni_globalSystemExcHandler       = (void*)fn;
 omni_globalSystemExcHandlerCookie = cookie;
 omni_globalSystemExcHandlerExt    = 0;
}

void
omniORB::installSystemExceptionHandlerExt(void* cookie,
                                        omniORB::systemExceptionHandlerExt_t fn)
{
 omni_globalSystemExcHandler       = (void*)fn;
 omni_globalSystemExcHandlerCookie = cookie;
 omni_globalSystemExcHandlerExt    = 1;
}

void
omniORB::installSystemExceptionHandler(CORBA::Object_ptr obj,
				       void* cookie,
				       omniORB::systemExceptionHandler_t fn)
{
  if (CORBA::is_nil(obj)) return;
  obj->_PR_getobj()->_systemExceptionHandler((void*)fn, cookie, 0);
}

void
omniORB::installSystemExceptionHandlerExt(CORBA::Object_ptr obj,
                                        void* cookie,
                                        omniORB::systemExceptionHandlerExt_t fn)
{
  if (CORBA::is_nil(obj)) return;
  obj->_PR_getobj()->_systemExceptionHandler((void*)fn, cookie, 1);
}


//
// Callers

static inline CORBA::Object_ptr cObj(omniObjRef* obj)
{
  return (CORBA::Object_ptr)obj->_ptrToObjRef(CORBA::Object::_PD_repoId);
}

CORBA::Boolean
_omni_callTransientExceptionHandler(omniObjRef*             obj,
				    CORBA::ULong            nretries,
				    const CORBA::TRANSIENT& ex,
                                    const char*             op)
{
  CORBA::Boolean ext;
  void*          cookie  = 0;
  void*          handler = obj->_transientExceptionHandler(cookie, ext);

  if (!handler) {
    handler = omni_globalTransientExcHandler;
    cookie  = omni_globalTransientExcHandlerCookie;
    ext     = omni_globalTransientExcHandlerExt;
  }

  if (ext) {
    return (*(omniORB::transientExceptionHandlerExt_t)handler)
      (cookie, nretries, ex, cObj(obj), op);
  }
  else {
    return (*(omniORB::transientExceptionHandler_t)handler)
      (cookie, nretries, ex);
  }
}


CORBA::Boolean
_omni_callTimeoutExceptionHandler(omniObjRef*           obj,
				  CORBA::ULong          nretries,
				  const CORBA::TIMEOUT& ex,
                                  const char*           op)
{
  CORBA::Boolean ext;
  void*          cookie  = 0;
  void*          handler = obj->_timeoutExceptionHandler(cookie, ext);

  if (!handler) {
    handler = omni_globalTimeoutExcHandler;
    cookie  = omni_globalTimeoutExcHandlerCookie;
    ext     = omni_globalTimeoutExcHandlerExt;
  }

  if (ext) {
    return (*(omniORB::timeoutExceptionHandlerExt_t)handler)
      (cookie, nretries, ex, cObj(obj), op);
  }
  else {
    return (*(omniORB::timeoutExceptionHandler_t)handler)
      (cookie, nretries, ex);
  }
}


CORBA::Boolean
_omni_callCommFailureExceptionHandler(omniObjRef*                obj,
				      CORBA::ULong               nretries,
				      const CORBA::COMM_FAILURE& ex,
                                      const char*                op)
{
  CORBA::Boolean ext;
  void*          cookie  = 0;
  void*          handler = obj->_commFailureExceptionHandler(cookie, ext);

  if (!handler) {
    handler = omni_globalCommFailureExcHandler;
    cookie  = omni_globalCommFailureExcHandlerCookie;
    ext     = omni_globalCommFailureExcHandlerExt;
  }

  if (ext) {
    return (*(omniORB::commFailureExceptionHandlerExt_t)handler)
      (cookie, nretries, ex, cObj(obj), op);
  }
  else {
    return (*(omniORB::commFailureExceptionHandler_t)handler)
      (cookie, nretries, ex);
  }
}


CORBA::Boolean
_omni_callSystemExceptionHandler(omniObjRef*                   obj,
				 CORBA::ULong                  nretries,
				 const CORBA::SystemException& ex,
                                 const char*                   op)
{
  CORBA::Boolean ext;
  void*          cookie  = 0;
  void*          handler = obj->_systemExceptionHandler(cookie, ext);

  if (!handler) {
    handler = omni_globalSystemExcHandler;
    cookie  = omni_globalSystemExcHandlerCookie;
    ext     = omni_globalSystemExcHandlerExt;
  }

  if (ext) {
    return (*(omniORB::systemExceptionHandlerExt_t)handler)
      (cookie, nretries, ex, cObj(obj), op);
  }
  else {
    return (*(omniORB::systemExceptionHandler_t)handler)
      (cookie, nretries, ex);
  }
}


// This should really be extensible ...
static int exHandlersTableSize = 103;


#ifdef EXHANDLER_HASH
#  undef EXHANDLER_HASH
#endif

#define EXHANDLER_HASH(p) ((omni::ptr_arith_t) p % exHandlersTableSize)


omniExHandlers::omniExHandlers() :
  transient_hdr(0), transient_cookie(0),
  timeout_hdr(0),   timeout_cookie(0),
  commfail_hdr(0),  commfail_cookie(0),
  sysexcpt_hdr(0),  sysexcpt_cookie(0),
  transient_ext(0), timeout_ext(0), commfail_ext(0), sysexcpt_ext(0),
  objptr(0), next(0)
{
}


omniExHandlers_iterator::omniExHandlers_iterator()
{
  omniExHandlers::TableLock.lock();
}


omniExHandlers_iterator::~omniExHandlers_iterator()
{
  omniExHandlers::TableLock.unlock();
}


omniExHandlers*
omniExHandlers_iterator::find_or_create(omniObjRef* p)
{
  if (omniExHandlers::Table == 0) {
    omniExHandlers::Table = new omniExHandlers* [exHandlersTableSize];
    for( int i = 0; i < exHandlersTableSize; i++ )
      omniExHandlers::Table[i] = 0;
  }
  omniExHandlers* exlist = omniExHandlers::Table[EXHANDLER_HASH(p)];
  while (exlist) {
    if (exlist->objptr == p)
      break;
    exlist = exlist->next;
  }
  if (exlist) {
    return exlist;
  }
  else {
    omniExHandlers* np = new omniExHandlers;
    np->objptr = p;
    omniExHandlers** exlistp = &(omniExHandlers::Table[EXHANDLER_HASH(p)]);
    np->next = *exlistp;
    *exlistp = np;
    return np;
  }
}


omniExHandlers*
omniExHandlers_iterator::find(omniObjRef* p)
{
  if (omniExHandlers::Table == 0) return 0;
  omniExHandlers* exlist = omniExHandlers::Table[EXHANDLER_HASH(p)];
  while (exlist) {
    if (exlist->objptr == p)
      break;
    exlist = exlist->next;
  }
  return exlist;
}


void
omniExHandlers_iterator::remove(omniObjRef* p)
{
  if (omniExHandlers::Table == 0) return;
  omniExHandlers** exlistp = &(omniExHandlers::Table[EXHANDLER_HASH(p)]);
  while (*exlistp) {
    if ((*exlistp)->objptr == p) {
      omniExHandlers* exp = *exlistp;
      *exlistp = exp->next;
      delete exp;
      break;
    }
    else {
      exlistp = &((*exlistp)->next);
    }
  }
}

#undef EXHANDLER_HASH

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Boolean 
_CORBA_use_nil_ptr_as_nil_objref()
{
  omniORB::logs(10, "Warning: a nil (0) pointer is wrongly used as a "
		"nil object reference.");
  return 1;
}


void
_CORBA_new_operator_return_null()
{
  OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
}


void
_CORBA_bound_check_error()
{
  OMNIORB_THROW(BAD_PARAM,BAD_PARAM_IndexOutOfRange,CORBA::COMPLETED_NO);
}


void
_CORBA_marshal_sequence_range_check_error(cdrStream& s)
{
  OMNIORB_THROW(MARSHAL,MARSHAL_SequenceIsTooLong,
		(CORBA::CompletionStatus)s.completion());
}


void
_CORBA_invoked_nil_pseudo_ref()
{
  omniORB::logs(1, "Error: the application attempted to invoke an"
		" operation on a nil pseudo-object reference.");
  OMNIORB_THROW(INV_OBJREF,INV_OBJREF_InvokeOnNilObjRef, CORBA::COMPLETED_NO);
}


CORBA::Boolean
_CORBA_use_nil_ptr_as_nil_pseudo_objref(const char* objType)
{
  if (omniORB::trace(1)) {
    omniORB::logger l;
    l << "Warning: a nil (0) pointer is wrongly used as a "
      << "nil CORBA::" << objType << " object reference. "
      << "Use CORBA::" << objType << "::_nil()\n";
  }
  return 1;
}


void
_CORBA_invoked_nil_objref()
{
  omniORB::logs(1, "Error: the application attempted to invoke an "
		"operation on a nil reference.");

  OMNIORB_THROW(INV_OBJREF,INV_OBJREF_InvokeOnNilObjRef, CORBA::COMPLETED_NO);
}


void
_CORBA_bad_param_freebuf()
{
  omniORB::logs(1, "Error: an invalid buffer pointer is passed to freebuf "
		" of string or object sequence");
}
