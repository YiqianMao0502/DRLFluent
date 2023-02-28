// -*- Mode: C++; -*-
//                            Package   : omniORB
// request.h                  Created on: 9/1998
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
//

#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <omniORB4/callDescriptor.h>
#include <pseudo.h>


OMNI_NAMESPACE_BEGIN(omni)

class RequestImpl;

class DIICallDescriptor : public omniAsyncCallDescriptor {
public:
  DIICallDescriptor(RequestImpl*             req,
                    const char*              op,
                    CORBA::NVList_ptr        arguments,
                    CORBA::NamedValue_ptr    result,
                    CORBA::ExceptionList_ptr exceptions,
                    CORBA::ContextList_ptr   contexts,
                    CORBA::Context_ptr       context);

  void marshalArguments(cdrStream& stream);
  void unmarshalReturnedValues(cdrStream& stream);
  void userException(cdrStream& stream, IOP_C* iop_client, const char* repoid);
  void completeCallback();

  inline CORBA::NVList_ptr        arguments()   const { return pd_arguments; }
  inline CORBA::NamedValue_ptr    result()      const { return pd_result; }
  inline CORBA::Environment_ptr   environment() const { return pd_environment; }
  inline CORBA::Context_ptr       context()     const { return pd_context; }

  inline CORBA::ExceptionList_ptr exceptions()
  {
    if (CORBA::is_nil(pd_exceptions)) pd_exceptions = new ExceptionListImpl;
    return pd_exceptions;
  }

  inline CORBA::ContextList_ptr contexts()
  {
    if (CORBA::is_nil(pd_contexts)) pd_contexts = new ContextListImpl;
    return pd_contexts;
  }

  inline void context(CORBA::Context_ptr ctx)
  {
    pd_context = CORBA::Context::_duplicate(ctx);
  }

  inline _CORBA_Boolean isCalledBack()
  {
    omni_tracedmutex_lock l(sd_lock);
    return !pd_do_callback;
  }

  inline void waitForCallback()
  {
    omni_tracedmutex_lock l(sd_lock);
    
    if (!pd_do_callback)
      return;

    if (!pd_cond)
      pd_cond = new omni_tracedcondition(&sd_lock,
                                         "DIICallDescriptor::pd_cond");

    while (pd_do_callback)
      pd_cond->wait();
  }

private:
  RequestImpl*             pd_req;
  CORBA::NVList_var        pd_arguments;
  CORBA::NamedValue_var    pd_result;
  CORBA::Environment_var   pd_environment;
  CORBA::ExceptionList_var pd_exceptions;  // may be nil
  CORBA::ContextList_var   pd_contexts;    // may be nil
  CORBA::Context_var       pd_context;     // may be nil
};


class RequestImpl : public CORBA::Request, public PseudoObjBase {
public:
  RequestImpl(CORBA::Object_ptr target, const char* operation);

  RequestImpl(CORBA::Object_ptr     target,
              const char*           operation,
	      CORBA::Context_ptr    context,
              CORBA::NVList_ptr     arguments,
	      CORBA::NamedValue_ptr result);

  RequestImpl(CORBA::Object_ptr        target,
              const char*              operation,
	      CORBA::Context_ptr       context,
              CORBA::NVList_ptr        arguments,
	      CORBA::NamedValue_ptr    result,
	      CORBA::ExceptionList_ptr exceptions,
	      CORBA::ContextList_ptr   contexts);

  virtual ~RequestImpl();

  virtual CORBA::Object_ptr        target() const;
  virtual const char*              operation() const;
  virtual CORBA::NVList_ptr        arguments();
  virtual CORBA::NamedValue_ptr    result();
  virtual CORBA::Environment_ptr   env();
  virtual CORBA::ExceptionList_ptr exceptions();
  virtual CORBA::ContextList_ptr   contexts();
  virtual CORBA::Context_ptr       ctx() const;
  virtual void                     ctx(CORBA::Context_ptr);

  virtual CORBA::Any& add_in_arg();
  virtual CORBA::Any& add_in_arg(const char* name);
  virtual CORBA::Any& add_inout_arg();
  virtual CORBA::Any& add_inout_arg(const char* name);
  virtual CORBA::Any& add_out_arg();
  virtual CORBA::Any& add_out_arg(const char* name);

  virtual void        set_return_type(CORBA::TypeCode_ptr tc);
  virtual CORBA::Any& return_value();

  virtual void 	      invoke();
  virtual void 	      send_oneway();
  virtual void 	      send_deferred();
  virtual void 	      get_response();

  virtual CORBA::Boolean     poll_response();
  virtual CORBA::Boolean     NP_is_nil() const;
  virtual CORBA::Request_ptr NP_duplicate();

  inline void storeExceptionInEnv() {
    pd_cd.environment()->exception(pd_sysExceptionToThrow);
    pd_sysExceptionToThrow = 0;
  }

private:
  enum State {
    RS_READY,                // Not used yet
    RS_DONE,                 // Result got synchronously
    RS_DEFERRED,             // Sent deferred
    RS_POLLED_DONE_DEFERRED, // Result got with poll_response
    RS_DONE_DEFERRED         // Result got with get_response
  };

  CORBA::Object_var pd_target;
  CORBA::String_var pd_operation;
  State             pd_state;
  DIICallDescriptor pd_cd;
  CORBA::Exception* pd_sysExceptionToThrow;
  // If non-zero, then the exception should be thrown next
  // time one of the public methods (other than poll_response)
  // is called.

  friend class DIICallDescriptor;
};

OMNI_NAMESPACE_END(omni)


#endif  // __REQUEST_H__
