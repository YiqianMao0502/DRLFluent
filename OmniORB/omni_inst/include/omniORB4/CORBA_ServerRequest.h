// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_ServerRequest.h      Created on: 2001/08/17
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
//    CORBA::ServerRequest
//

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////// ServerRequest ///////////////////////////
//////////////////////////////////////////////////////////////////////

class ServerRequest {
public:
  virtual const char* operation() = 0;
  virtual void arguments(NVList_ptr& parameters) = 0;
  virtual Context_ptr ctx() = 0;
  virtual void set_result(const Any& value) = 0;
  virtual void set_exception(const Any& value) = 0;

protected:
  inline ServerRequest() {}
  virtual ~ServerRequest();

private:
  ServerRequest(const ServerRequest&);
  ServerRequest& operator=(const ServerRequest&);
};
