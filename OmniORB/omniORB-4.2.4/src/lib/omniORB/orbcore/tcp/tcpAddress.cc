// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpAddress.cc              Created on: 19 Mar 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2013 Apasphere Ltd
//    Copyright (C) 2001      AT&T Laboratories Cambridge
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

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <omniORB4/omniURI.h>
#include <orbParameters.h>
#include <giopStrandFlags.h>
#include <tcpSocket.h>
#include <tcp/tcpConnection.h>
#include <tcp/tcpAddress.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>

#if defined(__vxWorks__)
#  include "selectLib.h"
#endif

OMNI_EXPORT_LINK_FORCE_SYMBOL(tcpAddress);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
tcpAddress::tcpAddress(const IIOP::Address& address) : pd_address(address) {

  pd_address_string = omniURI::buildURI("giop:tcp", address.host, address.port);
}


/////////////////////////////////////////////////////////////////////////
const char*
tcpAddress::type() const {
  return "giop:tcp";
}

/////////////////////////////////////////////////////////////////////////
const char*
tcpAddress::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
const char*
tcpAddress::host() const {
  return pd_address.host;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
tcpAddress::duplicate() const {
  return new tcpAddress(pd_address);
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
tcpAddress::duplicate(const char* host) const {
  IIOP::Address addr;
  addr.host = host;
  addr.port = pd_address.port;

  return new tcpAddress(addr);
}

/////////////////////////////////////////////////////////////////////////
giopActiveConnection*
tcpAddress::Connect(const omni_time_t& deadline,
		    CORBA::ULong       strand_flags,
		    CORBA::Boolean&    timed_out) const {

  if (pd_address.port == 0) return 0;

  SocketHandle_t sock = tcpSocket::Connect(pd_address.host, pd_address.port,
					   deadline, strand_flags, timed_out);
  if (sock == RC_SOCKET_ERROR)
    return 0;

  return new tcpActiveConnection(sock);
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpAddress::Poke() const {

  SocketHandle_t sock;

  if (pd_address.port == 0) return 0;

  LibcWrapper::AddrInfo_var ai;
  ai = LibcWrapper::getAddrInfo(pd_address.host, pd_address.port);

  if ((LibcWrapper::AddrInfo*)ai == 0)
    return 0;

  if ((sock = socket(ai->addrFamily(),SOCK_STREAM,0)) == RC_INVALID_SOCKET)
    return 0;

#if defined(USE_NONBLOCKING_CONNECT)

  if (tcpSocket::setNonBlocking(sock) == RC_INVALID_SOCKET) {
    CLOSESOCKET(sock);
    return 0;
  }

#endif

  if (::connect(sock,ai->addr(),ai->addrSize()) == RC_SOCKET_ERROR) {

    if (ERRNO != RC_EINPROGRESS) {
      CLOSESOCKET(sock);
      return 0;
    }
  }

  // The connect has not necessarily completed by this stage, but
  // we've done enough to poke the endpoint.
  CLOSESOCKET(sock);
  return 1;
}

OMNI_NAMESPACE_END(omni)
