// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslAddress.cc              Created on: 29 May 2001
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

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <omniORB4/omniURI.h>
#include <orbParameters.h>
#include <giopStrandFlags.h>
#include <SocketCollection.h>
#include <tcpSocket.h>
#include <omniORB4/sslContext.h>
#include <ssl/sslConnection.h>
#include <ssl/sslAddress.h>
#include <openssl/err.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(sslAddress);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
sslAddress::sslAddress(const IIOP::Address& address, sslContext* ctx) : 
  pd_address(address), pd_ctx(ctx) {

  pd_address_string = omniURI::buildURI("giop:ssl", address.host, address.port);
}

/////////////////////////////////////////////////////////////////////////
const char*
sslAddress::type() const {
  return "giop:ssl";
}

/////////////////////////////////////////////////////////////////////////
const char*
sslAddress::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
const char*
sslAddress::host() const {
  return pd_address.host;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
sslAddress::duplicate() const {
  return new sslAddress(pd_address,pd_ctx);
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
sslAddress::duplicate(const char* host) const {
  IIOP::Address addr;
  addr.host = host;
  addr.port = pd_address.port;

  return new sslAddress(addr, pd_ctx);
}


/////////////////////////////////////////////////////////////////////////
giopActiveConnection*
sslAddress::Connect(const omni_time_t& deadline,
		    CORBA::ULong       strand_flags,
		    CORBA::Boolean&    timed_out) const {

  if (pd_address.port == 0) return 0;

  SocketHandle_t sock = tcpSocket::Connect(pd_address.host, pd_address.port,
					   deadline, strand_flags, timed_out);
  if (sock == RC_SOCKET_ERROR)
    return 0;

  if (tcpSocket::setNonBlocking(sock) == RC_INVALID_SOCKET) {
    tcpSocket::logConnectFailure("Failed to set socket to non-blocking mode",
				 pd_address.host, pd_address.port);
    CLOSESOCKET(sock);
    return 0;
  }

  ::SSL* ssl = SSL_new(pd_ctx->get_SSL_CTX());
  SSL_set_fd(ssl, sock);
  SSL_set_connect_state(ssl);

  if (!LibcWrapper::isipaddr(pd_address.host))
    SSL_set_tlsext_host_name(ssl, (const char*)pd_address.host);
  
  struct timeval t;
  int rc;

  // Do the SSL handshake...
  while (1) {

    if (tcpSocket::setAndCheckTimeout(deadline, t)) {
      // Already timed out.
      tcpSocket::logConnectFailure("Timed out before SSL handshake",
				   pd_address.host, pd_address.port);
      SSL_free(ssl);
      CLOSESOCKET(sock);
      timed_out = 1;
      return 0;
    }

    int result = SSL_connect(ssl);
    int code   = SSL_get_error(ssl, result);

    switch(code) {
    case SSL_ERROR_NONE:
      {
	if (tcpSocket::setBlocking(sock) == RC_INVALID_SOCKET) {
	  tcpSocket::logConnectFailure("Failed to set socket to blocking mode",
				       pd_address.host, pd_address.port);
	  SSL_free(ssl);
	  CLOSESOCKET(sock);
	  return 0;
	}
	return new sslActiveConnection(sock,ssl);
      }

    case SSL_ERROR_WANT_READ:
      {
	rc = tcpSocket::waitRead(sock, t);
	if (rc == 0) {
	  // Timeout
#if !defined(USE_FAKE_INTERRUPTABLE_RECV)
	  tcpSocket::logConnectFailure("Timed out during SSL handshake",
				       pd_address.host, pd_address.port);
	  SSL_free(ssl);
	  CLOSESOCKET(sock);
	  timed_out = 1;
	  return 0;
#endif
	}
	continue;
      }

    case SSL_ERROR_WANT_WRITE:
      {
	rc = tcpSocket::waitWrite(sock, t);
	if (rc == 0) {
	  // Timeout
#if !defined(USE_FAKE_INTERRUPTABLE_RECV)
	  tcpSocket::logConnectFailure("Timed out during SSL handshake",
				       pd_address.host, pd_address.port);
	  SSL_free(ssl);
	  CLOSESOCKET(sock);
	  timed_out = 1;
	  return 0;
#endif
	}
	continue;
      }

    case SSL_ERROR_SYSCALL:
      {
	if (ERRNO == RC_EINTR)
	  continue;
      }
      // otherwise falls through
    case SSL_ERROR_SSL:
      {
	if (omniORB::trace(10)) {
	  omniORB::logger log;
	  char buf[128];
	  ERR_error_string_n(ERR_get_error(),buf,128);
	  log << "OpenSSL error connecting to " << pd_address.host
	      << ": " << (const char*) buf << "\n";
	}
	SSL_free(ssl);
	CLOSESOCKET(sock);
	return 0;
      }
    default:
      OMNIORB_ASSERT(0);
    }
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslAddress::Poke() const {

  SocketHandle_t sock;

  if (pd_address.port == 0) return 0;

  LibcWrapper::AddrInfo_var ai;
  ai = LibcWrapper::getAddrInfo(pd_address.host, pd_address.port);

  if ((LibcWrapper::AddrInfo*)ai == 0)
    return 0;

  if ((sock = socket(ai->addrFamily(), SOCK_STREAM,0)) == RC_INVALID_SOCKET)
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
  // we've done enough to poke the endpoint. We do not bother with the
  // SSL handshake, so the accepting thread will get an error when it
  // tries to do the SSL accept.
  CLOSESOCKET(sock);
  return 1;
}


OMNI_NAMESPACE_END(omni)
