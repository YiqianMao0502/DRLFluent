// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpTransportImpl.h         Created on: 19 Mar 2001
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

#ifndef __TCPTRANSPORTIMPL_H__
#define __TCPTRANSPORTIMPL_H__

OMNI_NAMESPACE_BEGIN(omni)

class tcpTransportImpl : public giopTransportImpl {
 public:

  tcpTransportImpl();
  ~tcpTransportImpl();
  
  giopEndpoint*  toEndpoint(const char* param);
  giopAddress*   toAddress(const char* param);
  CORBA::Boolean isValid(const char* param);
  CORBA::Boolean addToIOR(const char* param, IORPublish* eps);
  const omnivector<const char*>* getInterfaceAddress();
  void initialise();

  static CORBA::Boolean parseAddress(const char* param,
				     IIOP::Address& address);
  // parse the string of the form <host>:<port> to an IIOP address.
  // return 0 if the string is not in the valid format.

 private:
  omnivector<const char*> ifAddresses;

  tcpTransportImpl(const tcpTransportImpl&);
  tcpTransportImpl& operator=(const tcpTransportImpl&);
};

OMNI_NAMESPACE_END(omni)

#endif // __TCPTRANSPORTIMPL_H__
