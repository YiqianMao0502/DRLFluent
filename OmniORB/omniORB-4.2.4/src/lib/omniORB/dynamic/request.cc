// -*- Mode: C++; -*-
//                            Package   : omniORB
// request.cc                 Created on: 9/1998
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2003-2013 Apasphere Ltd
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
//   Implementation of CORBA::Request.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/objTracker.h>
#include <omniORB4/ami.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <request.h>
#include <context.h>
#include <string.h>
#include <omniORB4/callDescriptor.h>
#include <remoteIdentity.h>
#include <exceptiondefs.h>
#include <omniORB4/IOP_C.h>
#include <orbParameters.h>
#include <invoker.h>

OMNI_NAMESPACE_BEGIN(omni)


template <class T>
static inline typename T::_ptr_type
dup(typename T::_ptr_type p)
{
  return p ? T::_duplicate(p) : T::_nil();
}

DIICallDescriptor::
DIICallDescriptor(RequestImpl*             req,
                  const char*              op,
                  CORBA::NVList_ptr        arguments,
                  CORBA::NamedValue_ptr    result,
                  CORBA::ExceptionList_ptr exceptions,
                  CORBA::ContextList_ptr   contexts,
                  CORBA::Context_ptr       context)
: omniAsyncCallDescriptor(0, op, strlen(op) + 1, 0, 0, 0),
  pd_req(req),
  pd_environment(new EnvironmentImpl),
  pd_exceptions (dup<CORBA::ExceptionList>(exceptions)),
  pd_contexts   (dup<CORBA::ContextList>  (contexts)),
  pd_context    (dup<CORBA::Context>      (context))
{
  if (!arguments || CORBA::is_nil(arguments))
    pd_arguments = new NVListImpl;
  else
    pd_arguments = CORBA::NVList::_duplicate(arguments);

  if (!result || CORBA::is_nil(result)) {
    pd_result = new NamedValueImpl(CORBA::Flags(0));
    pd_result->value()->replace(CORBA::_tc_void, (void*)0);
  }
  else {
    pd_result = CORBA::NamedValue::_duplicate(result);
  }
}


void
DIICallDescriptor::
marshalArguments(cdrStream& s)
{
  CORBA::ULong num_args = pd_arguments->count();

  for (CORBA::ULong i = 0; i < num_args; i++){
    CORBA::NamedValue_ptr arg = pd_arguments->item(i);
    if (arg->flags() & CORBA::ARG_IN)
      arg->value()->NP_marshalDataOnly(s);
  }
  if (!CORBA::is_nil(pd_contexts)) {
    ContextListImpl* context_list = (ContextListImpl*)
      (CORBA::ContextList_ptr)pd_contexts;

    CORBA::Context::marshalContext(pd_context, context_list->NP_list(),
				   context_list->count(), s);
  }
}


void
DIICallDescriptor::
unmarshalReturnedValues(cdrStream& s)
{
  pd_result->value()->NP_unmarshalDataOnly(s);

  CORBA::ULong num_args = pd_arguments->count();

  for (CORBA::ULong i = 0; i < num_args; i++){
    CORBA::NamedValue_ptr arg = pd_arguments->item(i);
    if (arg->flags() & CORBA::ARG_OUT)
      arg->value()->NP_unmarshalDataOnly(s);
  }
}


void
DIICallDescriptor::
userException(cdrStream& s, IOP_C* iop_client, const char* repoId)
{
  CORBA::ULong exListLen = CORBA::is_nil(pd_exceptions) ?
                                 0 : pd_exceptions->count();

  // Search for a match in the exception list.
  for (CORBA::ULong i = 0; i < exListLen; i++){
    CORBA::TypeCode_ptr exType = pd_exceptions->item(i);

    if (omni::strMatch(repoId, exType->id())){
      // Unmarshal the exception into an Any.
      CORBA::Any* newAny = new CORBA::Any(exType, 0);
      try {
	newAny->NP_unmarshalDataOnly(s);
      }
      catch(...) {
	delete newAny;
	throw;
      }
      // Encapsulate this in an UnknownUserException, which is
      // placed into pd_environment.
      CORBA::UnknownUserException* ex =
	new CORBA::UnknownUserException(newAny);

      pd_environment->exception(ex);
      return;
    }
  }
  OMNIORB_THROW(UNKNOWN, UNKNOWN_UserException, CORBA::COMPLETED_MAYBE);
}


void
DIICallDescriptor::
completeCallback()
{
  if (exceptionOccurred() && !orbParameters::diiThrowsSysExceptions)
    pd_environment->exception(getException());

  omniAMI::DIIPollableImpl::_PD_instance._replyReady();

  {
    omni_tracedmutex_lock l(sd_lock);
    pd_do_callback = 0;
    if (pd_cond)
      pd_cond->broadcast();
  }

  pd_req->decrRefCount();
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

RequestImpl::RequestImpl(CORBA::Object_ptr target,
                         const char*       operation)

: pd_target(CORBA::Object::_duplicate(target)),
  pd_operation(CORBA::string_dup(operation)),
  pd_state(RS_READY),
  pd_cd(this, pd_operation, 0, 0, 0, 0, 0),
  pd_sysExceptionToThrow(0)
{
  if (CORBA::is_nil(target))
    OMNIORB_THROW(INV_OBJREF,
                  INV_OBJREF_InvokeOnNilObjRef,
                  CORBA::COMPLETED_NO);

  if (!operation || !*operation)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_NullStringUnexpected,
		  CORBA::COMPLETED_NO);
}


RequestImpl::RequestImpl(CORBA::Object_ptr     target,
                         const char*           operation,
			 CORBA::Context_ptr    context,
			 CORBA::NVList_ptr     arguments,
			 CORBA::NamedValue_ptr result)

: pd_target(CORBA::Object::_duplicate(target)),
  pd_operation(CORBA::string_dup(operation)),
  pd_state(RS_READY),
  pd_cd(this, pd_operation, arguments, result, 0, 0, context),
  pd_sysExceptionToThrow(0)
{
  if (CORBA::is_nil(target))
    OMNIORB_THROW(INV_OBJREF,
                  INV_OBJREF_InvokeOnNilObjRef,
                  CORBA::COMPLETED_NO);

  if (!operation || !*operation)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_NullStringUnexpected,
		  CORBA::COMPLETED_NO);
}


RequestImpl::RequestImpl(CORBA::Object_ptr        target,
                         const char*              operation,
			 CORBA::Context_ptr       context,
			 CORBA::NVList_ptr        arguments,
			 CORBA::NamedValue_ptr    result,
			 CORBA::ExceptionList_ptr exceptions,
			 CORBA::ContextList_ptr   contexts)

: pd_target(CORBA::Object::_duplicate(target)),
  pd_operation(CORBA::string_dup(operation)),
  pd_state(RS_READY),
  pd_cd(this, pd_operation, arguments, result, exceptions, contexts, context),
  pd_sysExceptionToThrow(0)

{
  if (CORBA::is_nil(target))
    OMNIORB_THROW(INV_OBJREF,
                  INV_OBJREF_InvokeOnNilObjRef,
                  CORBA::COMPLETED_NO);

  if (!operation || !*operation)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_NullStringUnexpected,
		  CORBA::COMPLETED_NO);
}


RequestImpl::~RequestImpl()
{
  if (pd_state == RS_DEFERRED) {
    omniORB::logs(1, "Warning: The application has not collected the "
                  "reponse of a deferred DII request. Use "
                  "Request::get_response() or poll_response().");
  }
  if (pd_sysExceptionToThrow)
    delete pd_sysExceptionToThrow;
}


CORBA::Object_ptr
RequestImpl::target() const
{
  return pd_target;
}


const char*
RequestImpl::operation() const
{
  return pd_operation;
}


CORBA::NVList_ptr
RequestImpl::arguments()
{
  if (pd_sysExceptionToThrow)  pd_sysExceptionToThrow->_raise();

  return pd_cd.arguments();
}


CORBA::NamedValue_ptr
RequestImpl::result()
{
  if (pd_sysExceptionToThrow)  pd_sysExceptionToThrow->_raise();

  return pd_cd.result();
}


CORBA::Environment_ptr
RequestImpl::env()
{
  if (pd_sysExceptionToThrow)  pd_sysExceptionToThrow->_raise();

  return pd_cd.environment();
}


CORBA::ExceptionList_ptr
RequestImpl::exceptions()
{
  return pd_cd.exceptions();
}


CORBA::ContextList_ptr
RequestImpl::contexts()
{
  return pd_cd.contexts();
}


CORBA::Context_ptr
RequestImpl::ctx() const
{
  return pd_cd.context();
}


void
RequestImpl::ctx(CORBA::Context_ptr context)
{
  if (!CORBA::Context::PR_is_valid(context))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidContext,
		  CORBA::COMPLETED_NO);

  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestConfiguredOutOfOrder,
		  CORBA::COMPLETED_NO);

  pd_cd.context(context);
}


CORBA::Any&
RequestImpl::add_in_arg()
{
  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestConfiguredOutOfOrder,
		  CORBA::COMPLETED_NO);

  return *(pd_cd.arguments()->add(CORBA::ARG_IN)->value());
}


CORBA::Any&
RequestImpl::add_in_arg(const char* name)
{
  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestConfiguredOutOfOrder,
		  CORBA::COMPLETED_NO);

  return *(pd_cd.arguments()->add_item(name, CORBA::ARG_IN)->value());
}


CORBA::Any&
RequestImpl::add_inout_arg()
{
  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestConfiguredOutOfOrder,
		  CORBA::COMPLETED_NO);

  return *(pd_cd.arguments()->add(CORBA::ARG_INOUT)->value());
}


CORBA::Any&
RequestImpl::add_inout_arg(const char* name)
{
  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestConfiguredOutOfOrder,
		  CORBA::COMPLETED_NO);

  return *(pd_cd.arguments()->add_item(name, CORBA::ARG_INOUT)->value());
}


CORBA::Any&
RequestImpl::add_out_arg()
{
  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestConfiguredOutOfOrder,
		  CORBA::COMPLETED_NO);

  return *(pd_cd.arguments()->add(CORBA::ARG_OUT)->value());
}


CORBA::Any&
RequestImpl::add_out_arg(const char* name)
{
  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestConfiguredOutOfOrder,
		  CORBA::COMPLETED_NO);

  return *(pd_cd.arguments()->add_item(name, CORBA::ARG_OUT)->value());
}


void
RequestImpl::set_return_type(CORBA::TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidTypeCode,
		  CORBA::COMPLETED_NO);

  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestConfiguredOutOfOrder,
		  CORBA::COMPLETED_NO);

  pd_cd.result()->value()->replace(tc, (void*)0);
}


CORBA::Any&
RequestImpl::return_value()
{
  if (pd_sysExceptionToThrow)  pd_sysExceptionToThrow->_raise();

  return *(pd_cd.result()->value());
}


void
RequestImpl::invoke()
{
  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestAlreadySent,
		  CORBA::COMPLETED_NO);

  try {
    pd_target->_PR_getobj()->_invoke(pd_cd);
  }
  catch (CORBA::SystemException& ex) {
    // Either throw system exceptions, or store in environment.
    pd_state = RS_DONE;

    if (orbParameters::diiThrowsSysExceptions) {
      pd_sysExceptionToThrow = CORBA::Exception::_duplicate(&ex);
      throw;
    }
    else
      pd_cd.environment()->exception(CORBA::Exception::_duplicate(&ex));
  }
  pd_state = RS_DONE;
}


void
RequestImpl::send_oneway()
{
  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestAlreadySent,
		  CORBA::COMPLETED_NO);

  try {
    pd_cd.set_oneway(1);
    pd_target->_PR_getobj()->_invoke(pd_cd);
  }
  catch(CORBA::SystemException& ex) {
    // Either throw system exceptions, or store in pd_environment.
    pd_state = RS_DONE;

    if (orbParameters::diiThrowsSysExceptions) {
      pd_sysExceptionToThrow = CORBA::Exception::_duplicate(&ex);
      throw;
    }
    else {
      pd_cd.environment()->exception(CORBA::Exception::_duplicate(&ex));
    }
  }
  pd_state = RS_DONE;
}


void
RequestImpl::send_deferred()
{
  if (pd_state != RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestAlreadySent,
		  CORBA::COMPLETED_NO);

  pd_state = RS_DEFERRED;
  incrRefCount(); // Make sure Request lives until invocation completes

  omniObjRef* o = pd_target->_PR_getobj();
  pd_target->_PR_getobj()->_invoke_async(&pd_cd);
}


void
RequestImpl::get_response()
{
  if (pd_state == RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestNotSentYet,
		  CORBA::COMPLETED_NO);

  if (pd_state == RS_DONE)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestIsSynchronous,
		  CORBA::COMPLETED_NO);

  if (pd_state == RS_DONE_DEFERRED)
    OMNIORB_THROW(BAD_INV_ORDER,
                  BAD_INV_ORDER_ResultAlreadyReceived,
                  CORBA::COMPLETED_NO);

  if (pd_state == RS_POLLED_DONE_DEFERRED)
    pd_state = RS_DONE_DEFERRED;

  if (pd_sysExceptionToThrow)  pd_sysExceptionToThrow->_raise();

  if (pd_state == RS_DONE_DEFERRED)
    return;

  pd_cd.waitForCallback();
  pd_state = RS_DONE_DEFERRED;
  omniAMI::DIIPollableImpl::_PD_instance._replyCollected();

  CORBA::Exception* ex = pd_cd.getException();
  if (ex) {
    // System exception occurred. Store it so later calls can rethrow
    // it, then throw it immediately. User exceptions are handled by
    // DIICallDescriptor::userException, so ex should only be a system
    // exception.
    pd_sysExceptionToThrow = CORBA::SystemException::_downcast(ex);
    OMNIORB_ASSERT(pd_sysExceptionToThrow);
    pd_sysExceptionToThrow->_raise();
  }
}


CORBA::Boolean
RequestImpl::poll_response()
{
  if (pd_state == RS_READY)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestNotSentYet,
		  CORBA::COMPLETED_NO);

  if (pd_state == RS_DONE)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_RequestIsSynchronous,
		  CORBA::COMPLETED_NO);

  if (pd_state == RS_DONE_DEFERRED || pd_state == RS_POLLED_DONE_DEFERRED)
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_ResultAlreadyReceived,
		  CORBA::COMPLETED_NO);

  if (!pd_cd.isCalledBack())
    return 0;

  pd_state = RS_POLLED_DONE_DEFERRED;
  omniAMI::DIIPollableImpl::_PD_instance._replyCollected();

  CORBA::Exception* ex = pd_cd.getException();
  if (ex) {
    pd_sysExceptionToThrow = CORBA::SystemException::_downcast(ex);
    OMNIORB_ASSERT(pd_sysExceptionToThrow);

    // XXX Opengroup vsOrb tests for poll_response to raise an
    //     exception when the invocation results in a system exception.
    if (orbParameters::diiThrowsSysExceptions)
      pd_sysExceptionToThrow->_raise();
  }
  return 1;
}

CORBA::Boolean
RequestImpl::NP_is_nil() const
{
  return 0;
}


CORBA::Request_ptr
RequestImpl::NP_duplicate()
{
  incrRefCount();
  return this;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static CORBA::Any dummy_any;

class omniNilRequest : public CORBA::Request, public omniTrackedObject {
public:
  virtual CORBA::Object_ptr target() const {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::Object::_nil();
  }
  virtual const char* operation() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::NVList_ptr arguments() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::NVList::_nil();
  }
  virtual CORBA::NamedValue_ptr result() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::NamedValue::_nil();
  }
  virtual CORBA::Environment_ptr env() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::Environment::_nil();
  }
  virtual CORBA::ExceptionList_ptr exceptions() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::ExceptionList::_nil();
  }
  virtual CORBA::ContextList_ptr contexts() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::ContextList::_nil();
  }
  virtual CORBA::Context_ptr ctx() const {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::Context::_nil();
  }
  virtual void ctx(CORBA::Context_ptr) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Any& add_in_arg() {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Any& add_in_arg(const char* name) {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Any& add_inout_arg() {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Any& add_inout_arg(const char* name) {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Any& add_out_arg() {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual CORBA::Any& add_out_arg(const char* name) {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual void set_return_type(CORBA::TypeCode_ptr tc) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Any& return_value() {
    _CORBA_invoked_nil_pseudo_ref();
    return dummy_any;
  }
  virtual void  invoke() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void  send_oneway() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void  send_deferred() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void  get_response() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Boolean poll_response() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual CORBA::Request_ptr NP_duplicate() {
    return _nil();
  }
};

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Request::~Request() { pd_magic = 0; }


CORBA::Request_ptr
CORBA::
Request::_duplicate(Request_ptr p)
{
  if (!PR_is_valid(p))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidRequest,
		  CORBA::COMPLETED_NO);

  if (!CORBA::is_nil(p))
    return p->NP_duplicate();
  else
    return _nil();
}


CORBA::Request_ptr
CORBA::
Request::_nil()
{
  static omniNilRequest* _the_nil_ptr = 0;
  if (!_the_nil_ptr) {
    omni::nilRefLock().lock();
    if (!_the_nil_ptr) {
      _the_nil_ptr = new omniNilRequest;
      registerTrackedObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////// CORBA::Object ///////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::release(CORBA::Request_ptr p)
{
  if (CORBA::Request::PR_is_valid(p) && !CORBA::is_nil(p))
    ((RequestImpl*)p)->decrRefCount();
}


void
CORBA::Object::_create_request(CORBA::Context_ptr    ctx,
			       const char*           operation,
			       CORBA::NVList_ptr     arg_list,
			       CORBA::NamedValue_ptr result,
			       CORBA::Request_out    request,
			       CORBA::Flags          req_flags)
{
  if (_NP_is_pseudo())
    OMNIORB_THROW(NO_IMPLEMENT,
                  NO_IMPLEMENT_DIIOnLocalObject,
                  CORBA::COMPLETED_NO);

  // NB. req_flags is ignored - ref. CORBA 2.2 section 20.28
  if (!CORBA::Context::PR_is_valid(ctx))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidContext,
		  CORBA::COMPLETED_NO);
  if (!operation)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_NullStringUnexpected,
		  CORBA::COMPLETED_NO);
  if (!CORBA::NVList::PR_is_valid(arg_list))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidNVList,
		  CORBA::COMPLETED_NO);
  if (!CORBA::NamedValue::PR_is_valid(result))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidNamedValue,
		  CORBA::COMPLETED_NO);

  request = new RequestImpl(this, operation, ctx, arg_list, result);
}


void 
CORBA::Object::_create_request(CORBA::Context_ptr       ctx,
                               const char*              operation,
			       CORBA::NVList_ptr        arg_list,
			       CORBA::NamedValue_ptr    result,
			       CORBA::ExceptionList_ptr exceptions,
			       CORBA::ContextList_ptr   ctxlist,
			       CORBA::Request_out       request,
			       CORBA::Flags             req_flags)
{
  if (_NP_is_pseudo())
    OMNIORB_THROW(NO_IMPLEMENT,
                  NO_IMPLEMENT_DIIOnLocalObject,
                  CORBA::COMPLETED_NO);

  // NB. req_flags is ignored - ref. CORBA 2.2 section 20.28
  if (!CORBA::Context::PR_is_valid(ctx))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidContext,
		  CORBA::COMPLETED_NO);
  if (!operation)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_NullStringUnexpected,
		  CORBA::COMPLETED_NO);
  if (!CORBA::NVList::PR_is_valid(arg_list))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidNVList,
		  CORBA::COMPLETED_NO);
  if (!CORBA::NamedValue::PR_is_valid(result))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidNamedValue,
		  CORBA::COMPLETED_NO);
  if (!CORBA::ExceptionList::PR_is_valid(exceptions))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidExceptionList,
		  CORBA::COMPLETED_NO);
  if (!CORBA::ContextList::PR_is_valid(ctxlist))
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidContextList,
		  CORBA::COMPLETED_NO);

  request = new RequestImpl(this, operation, ctx, arg_list, result,
			    exceptions, ctxlist);
}


CORBA::Request_ptr
CORBA::Object::_request(const char* operation) 
{
  if (_NP_is_pseudo())
    OMNIORB_THROW(NO_IMPLEMENT,
                  NO_IMPLEMENT_DIIOnLocalObject,
                  CORBA::COMPLETED_NO);

  if (!operation)
    OMNIORB_THROW(BAD_PARAM,
                  BAD_PARAM_NullStringUnexpected,
                  CORBA::COMPLETED_NO);

  return new RequestImpl(this, operation);
}
