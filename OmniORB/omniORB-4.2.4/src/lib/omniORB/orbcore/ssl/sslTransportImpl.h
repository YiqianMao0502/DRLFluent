// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslTransportImpl.h         Created on: 29 May 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2011-2013 Apasphere Ltd.
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

#ifndef __SSLTRANSPORTIMPL_H__
#define __SSLTRANSPORTIMPL_H__

class sslContext;

OMNI_NAMESPACE_BEGIN(omni)

class sslTransportImpl : public giopTransportImpl {
 public:

  giopEndpoint*  toEndpoint(const char* param);
  giopAddress*   toAddress(const char* param);
  CORBA::Boolean isValid(const char* param);
  CORBA::Boolean addToIOR(const char* param, IORPublish* eps);
  sslContext*    getContext() const { return pd_ctx; }
  const omnivector<const char*>* getInterfaceAddress();

  sslTransportImpl(sslContext* ctx);
  ~sslTransportImpl();

  static omni_time_t sslAcceptTimeOut;

 private:

  sslContext*  pd_ctx;


  sslTransportImpl();
  sslTransportImpl(const sslTransportImpl&);
  sslTransportImpl& operator=(const sslTransportImpl&);
};

OMNI_NAMESPACE_END(omni)

#endif // __SSLTRANSPORTIMPL_H__
