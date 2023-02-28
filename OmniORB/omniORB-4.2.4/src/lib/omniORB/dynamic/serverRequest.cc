// -*- Mode: C++; -*-
//                            Package   : omniORB
// serverRequest.cc           Created on: 9/1998
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2003 Apasphere Ltd
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
//   Implementation of CORBA::ServerRequest.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <omniORB4/IOP_S.h>
#include <dynamicImplementation.h>
#include <pseudo.h>
#include <context.h>
#include <exceptiondefs.h>
#include <poacurrentimpl.h>


CORBA::ServerRequest::~ServerRequest()  {}

OMNI_NAMESPACE_BEGIN(omni)


////////////////////////////////////////////////////////////////////////
omniServerRequest::omniServerRequest(omniCallHandle& handle) 
  : pd_state(SR_READY), pd_handle(handle)
{
  pd_calldesc =
    new serverRequestCallDescriptor(handle.operation_name(),
				    strlen(handle.operation_name()));
}


////////////////////////////////////////////////////////////////////////
omniServerRequest::~omniServerRequest()  {
  if (pd_calldesc) {
    delete pd_calldesc;
    pd_calldesc = 0;
  }
}


////////////////////////////////////////////////////////////////////////
const char*
omniServerRequest::operation()
{
  return pd_handle.operation_name();
}


////////////////////////////////////////////////////////////////////////
void
omniServerRequest::arguments(CORBA::NVList_ptr& parameters)
{
  if( pd_state != SR_READY ) {
    pd_state = SR_DSI_ERROR;
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_ArgumentsCalledOutOfOrder,
		  CORBA::COMPLETED_NO);
  }
  if( CORBA::is_nil(parameters) ) {
    pd_state = SR_DSI_ERROR;
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidNVList,
		  CORBA::COMPLETED_NO);
  }
  pd_state = SR_ERROR;

  pd_calldesc->pd_params = parameters;

  if (pd_handle.iop_s()) {
    pd_handle.iop_s()->ReceiveRequest(*((omniCallDescriptor*)pd_calldesc));
  }
  else {
    // In process call -- use a memory stream
    cdrMemoryStream stream;
    pd_handle.call_desc()->initialiseCall(stream);
    pd_handle.call_desc()->marshalArguments(stream);

    if (stream.valueTracker()) {
      delete stream.valueTracker();
      stream.valueTracker(0);
    }
    pd_calldesc->unmarshalArguments(stream);
  }
  pd_state = SR_GOT_PARAMS;
}


////////////////////////////////////////////////////////////////////////
CORBA::Context_ptr
omniServerRequest::ctx()
{
  // Returns a nil context if no context information supplied.

  if( pd_state != SR_GOT_PARAMS ) {
    pd_state = SR_DSI_ERROR;
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_CtxCalledOutOfOrder,
		  CORBA::COMPLETED_NO);
  }

  pd_state = SR_GOT_CTX;

  return pd_calldesc->pd_context;
}


////////////////////////////////////////////////////////////////////////
void
omniServerRequest::set_result(const CORBA::Any& value)
{
  if( CORBA::is_nil(pd_calldesc->pd_context) ) {
    if( pd_state != SR_GOT_PARAMS && pd_state != SR_GOT_CTX ) {
      pd_state = SR_DSI_ERROR;
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_SetResultCalledOutOfOrder,
		    CORBA::COMPLETED_NO);
    }
  }
  else {
    if( pd_state != SR_GOT_CTX ) {
      pd_state = SR_DSI_ERROR;
      OMNIORB_THROW(MARSHAL,
		    MARSHAL_ServerRequestWrongOrder,
		    CORBA::COMPLETED_NO);
    }
  }
  pd_calldesc->pd_result = value;
  pd_state = SR_GOT_RESULT;
}


////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean isASystemException(const char* repoId) {
#define TEST_IS_A_SYSEXCEPTION(name) \
  if (strcmp("IDL:omg.org/CORBA/" #name ":1.0",repoId) == 0) return 1;
  OMNIORB_FOR_EACH_SYS_EXCEPTION(TEST_IS_A_SYSEXCEPTION)
  return 0;
#undef TEST_IS_A_SYSEXCEPTION
}

////////////////////////////////////////////////////////////////////////
void
omniServerRequest::set_exception(const CORBA::Any& value)
{
  CORBA::TypeCode_var tc = value.type();
  while( tc->kind() == CORBA::tk_alias )
    tc = tc->content_type();

  if( tc->kind() != CORBA::tk_except )
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_NotAnException,
		  CORBA::COMPLETED_NO);

  switch( pd_state ) {
  case SR_GOT_PARAMS:
  case SR_GOT_CTX:
  case SR_GOT_RESULT:
  case SR_EXCEPTION:
  case SR_ERROR:
    break;

  case SR_READY:
    {
      if (isASystemException(tc->id())) {
	pd_handle.SkipRequestBody();
	break;
      }
      pd_state = SR_DSI_ERROR;
    }
  case SR_DSI_ERROR:
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_ErrorInDynamicImplementation,
		  CORBA::COMPLETED_NO);
  }

  pd_calldesc->pd_exception = value;
  pd_state = SR_EXCEPTION;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

serverRequestCallDescriptor::
serverRequestCallDescriptor(const char* op,size_t oplen)
  : omniCallDescriptor(0, op, oplen, 0, 0, 0, 1)
{ }

void
serverRequestCallDescriptor::
unmarshalArguments(cdrStream& s)
{
  CORBA::ULong num_args = pd_params->count();

  for( CORBA::ULong i = 0; i < num_args; i++){
    CORBA::NamedValue_ptr arg = pd_params->item(i);
    if( arg->flags() & CORBA::ARG_IN )
      arg->value()->NP_unmarshalDataOnly(s);
  }

  // If there is no space left for context info...
  if ( s.checkInputOverrun(1,4) ) {
    pd_context = CORBA::Context::unmarshalContext(s);
  }
}

/////////////////////////////////////////////////////////////////////
void
serverRequestCallDescriptor::
marshalReturnedValues(cdrStream& s)
{
  pd_result.NP_marshalDataOnly(s);
  for( CORBA::ULong j = 0; j < pd_params->count(); j++ ){
    CORBA::NamedValue_ptr arg = pd_params->item(j);
    if( arg->flags() & CORBA::ARG_OUT )
      arg->value()->NP_marshalDataOnly(s);
  }
}



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class FromAnyUserException : public CORBA::UserException {
public:
  virtual ~FromAnyUserException() {}

  FromAnyUserException(const CORBA::Any& v,const char* id) :
      value(v), repoid(id) {}

  const char* _NP_repoId(int* size) const {
    *size = strlen(repoid) + 1;
    return repoid;
  }

  void _NP_marshal(cdrStream& s) const {
    value.NP_marshalDataOnly(s);
  }

private:

  // We don't expect any of these functions to be called.
  // Any call to these functions is a bug!
  void _raise() const {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "Wrong usage of class FromAnyUserException");
  }
  CORBA::Exception* _NP_duplicate() const {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "Wrong usage of class FromAnyUserException");
  }
  const char* _NP_typeId() const {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "Wrong usage of class FromAnyUserException");
  }

private:
  const CORBA::Any& value;
  const char* repoid;

  FromAnyUserException();
  FromAnyUserException(const FromAnyUserException&);
  FromAnyUserException& operator=(const FromAnyUserException&);
};

////////////////////////////////////////////////////////////////////////
void
omniServerRequest::do_reply()
{
  switch ( pd_state ) {
  case omniServerRequest::SR_GOT_PARAMS:
  case omniServerRequest::SR_GOT_RESULT:
    {
      if (pd_handle.iop_s()) {
	pd_handle.iop_s()->SendReply();
      }
      else {
	cdrMemoryStream stream;
	pd_calldesc->marshalReturnedValues(stream);
	if (stream.valueTracker()) {
	  delete stream.valueTracker();
	  stream.valueTracker(0);
	}
	pd_handle.call_desc()->unmarshalReturnedValues(stream);
      }
      break;
    }
  case omniServerRequest::SR_EXCEPTION:  // User & System exception
    {
      CORBA::TypeCode_var tc = pd_calldesc->pd_exception.type();
      const char* repoid = tc->id();

#     define TEST_AND_EXTRACT_SYSEXCEPTION(name) \
      if ( strcmp("IDL:omg.org/CORBA/" #name ":1.0",repoid) == 0 ) { \
        const CORBA::name* ex; \
 	pd_calldesc->pd_exception >>= ex; \
        if (pd_handle.iop_s()) { \
 	  pd_handle.iop_s()->SendException((CORBA::name*)ex); \
 	  return; \
        } \
        else { \
          ex->_raise(); \
        } \
      }
      OMNIORB_FOR_EACH_SYS_EXCEPTION(TEST_AND_EXTRACT_SYSEXCEPTION)
#     undef TEST_AND_EXTRACT_SYSEXCEPTION

      FromAnyUserException ex(pd_calldesc->pd_exception,repoid);

      if (pd_handle.iop_s()) {
	pd_handle.iop_s()->SendException(&ex);
      }
      else {
	cdrMemoryStream stream;
	ex._NP_marshal(stream);
	if (stream.valueTracker()) {
	  delete stream.valueTracker();
	  stream.valueTracker(0);
	}
	pd_handle.call_desc()->userException(stream, 0, repoid);
      }
      break;
    }
  default:
    // Never reach here.
    break;
  }
}


OMNI_NAMESPACE_END(omni)
