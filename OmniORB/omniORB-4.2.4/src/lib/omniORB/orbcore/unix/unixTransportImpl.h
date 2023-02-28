// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixTransportImpl.h        Created on: 6 Aug 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2013 Apasphere Ltd
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

#ifndef __UNIXTRANSPORTIMPL_H__
#define __UNIXTRANSPORTIMPL_H__

OMNI_NAMESPACE_BEGIN(omni)

class unixTransportImpl : public giopTransportImpl {
 public:

  unixTransportImpl();
  ~unixTransportImpl();
  
  giopEndpoint*  toEndpoint(const char* param);
  giopAddress*   toAddress(const char* param);
  CORBA::Boolean isValid(const char* param);
  CORBA::Boolean addToIOR(const char* param, IORPublish* eps);
  const omnivector<const char*>* getInterfaceAddress();

 private:
  unixTransportImpl(const unixTransportImpl&);
  unixTransportImpl& operator=(const unixTransportImpl&);
};

OMNI_NAMESPACE_END(omni)

#endif // __UNIXTRANSPORTIMPL_H__
