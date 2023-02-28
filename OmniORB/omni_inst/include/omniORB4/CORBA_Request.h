// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Request.h            Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
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
//    CORBA::Request
//

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////////// Request //////////////////////////////
//////////////////////////////////////////////////////////////////////

class Request {
public:
  virtual ~Request();

  virtual Object_ptr        target() const = 0;
  virtual const char*       operation() const = 0;
  virtual NVList_ptr        arguments() = 0;
  virtual NamedValue_ptr    result() = 0;
  virtual Environment_ptr   env() = 0;
  virtual ExceptionList_ptr exceptions() = 0;
  virtual ContextList_ptr   contexts() = 0;
  virtual Context_ptr       ctx() const = 0;
  virtual void              ctx(Context_ptr) = 0;

  virtual Any& add_in_arg() = 0;
  virtual Any& add_in_arg(const char* name) = 0;
  virtual Any& add_inout_arg() = 0;
  virtual Any& add_inout_arg(const char* name) = 0;
  virtual Any& add_out_arg() = 0;
  virtual Any& add_out_arg(const char* name) = 0;

  virtual void set_return_type(TypeCode_ptr tc) = 0;
  virtual Any& return_value() = 0;

  virtual void    invoke() = 0;
  virtual void    send_oneway() = 0;
  virtual void    send_deferred() = 0;
  virtual void    get_response() = 0;
  virtual Boolean poll_response() = 0;

  virtual Boolean NP_is_nil() const = 0;
  virtual Request_ptr NP_duplicate() = 0;

  static Request_ptr _duplicate(Request_ptr);
  static Request_ptr _nil();

  static inline _CORBA_Boolean PR_is_valid(Request_ptr p ) {
    return ((p) ? (p->pd_magic == PR_magic) : 1);
  }

  static _dyn_attr const _CORBA_ULong PR_magic;

protected:
  Request() { pd_magic = PR_magic; }

private:
  _CORBA_ULong pd_magic;

  Request(const Request&);
  Request& operator=(const Request&);
};


