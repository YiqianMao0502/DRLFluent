// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixAddress.cc             Created on: 6 Aug 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2006-2013 Apasphere Ltd
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

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <unix/unixConnection.h>
#include <unix/unixAddress.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>
#include <sys/un.h>

#if defined(USE_FAKE_INTERRUPTABLE_RECV)
#  include <orbParameters.h>
#endif

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixAddress);

#ifndef AF_LOCAL
#  ifdef  AF_UNIX
#    define AF_LOCAL AF_UNIX
#  endif
#endif

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
unixAddress::unixAddress(const char* filename) {

  pd_filename = (const char*) filename;
  pd_address_string = unixConnection::unToString(filename);
}

/////////////////////////////////////////////////////////////////////////
const char*
unixAddress::type() const {
  return "giop:unix";
}

/////////////////////////////////////////////////////////////////////////
const char*
unixAddress::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
unixAddress::duplicate() const {
  return new unixAddress(pd_filename);
}

/////////////////////////////////////////////////////////////////////////
giopActiveConnection*
unixAddress::Connect(const omni_time_t& deadline,
		     CORBA::ULong    	strand_flags,
		     CORBA::Boolean& 	timed_out) const {

  struct sockaddr_un raddr;
  int                rc;
  SocketHandle_t     sock;

  if ((sock = socket(AF_LOCAL,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return 0;
  }

  memset((void*)&raddr, 0, sizeof(raddr));
  raddr.sun_family = AF_LOCAL;
  strncpy(raddr.sun_path, pd_filename, sizeof(raddr.sun_path) - 1);

#if !defined(USE_NONBLOCKING_CONNECT)

  if (::connect(sock, (struct sockaddr *)&raddr,
                sizeof(raddr)) == RC_SOCKET_ERROR) {

    omniORB::logs(25, "Failed to connect to Unix socket.");
    CLOSESOCKET(sock);
    return 0;
  }

#else

  if (tcpSocket::setNonBlocking(sock) == RC_INVALID_SOCKET) {
    omniORB::logs(25, "Failed to set Unix socket to non-blocking mode.");
    CLOSESOCKET(sock);
    return 0;
  }

  if (::connect(sock,(struct sockaddr *)&raddr,
                sizeof(raddr)) == RC_SOCKET_ERROR) {

    int err = ERRNO;
    if (err && err != RC_EINPROGRESS) {
      omniORB::logs(25, "Failed to connect to Unix socket.");
      CLOSESOCKET(sock);
      return 0;
    }
  }

  struct timeval t;

  do {
    if (tcpSocket::setAndCheckTimeout(deadline, t)) {
      // Already timed out
      omniORB::logs(25, "Timed out connecting to Unix socket.");
      CLOSESOCKET(sock);
      timed_out = 1;
      return 0;
    }

    rc = tcpSocket::waitWrite(sock, t);

    if (rc == 0) {
      // Timed out
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      continue;
#else
      omniORB::logs(25, "Timed out connecting to Unix socket.");
      CLOSESOCKET(sock);
      timed_out = 1;
      return 0;
#endif
    }
    else if (rc == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR) {
	continue;
      }
      else {
        omniORB::logs(25, "Failed to connect to Unix socket "
                      "(waiting for writable socket)");
	CLOSESOCKET(sock);
	return 0;
      }
    }

    // Check to make sure that the socket is connected.
    OMNI_SOCKADDR_STORAGE peer;
    SOCKNAME_SIZE_T len = sizeof(peer);
    rc = getpeername(sock, (struct sockaddr*)&peer, &len);

    if (rc == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR) {
	continue;
      }
      else {
	omniORB::logs(25, "Failed to connect to Unix socket (no peer name).");
	CLOSESOCKET(sock);
	return 0;
      }
    }
    break;

  } while (1);
#endif

  if (tcpSocket::setBlocking(sock) == RC_INVALID_SOCKET) {
    omniORB::logs(25, "Failed to set Unix socket to blocking mode");
    CLOSESOCKET(sock);
    return 0;
  }
  return new unixActiveConnection(sock, pd_filename);
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixAddress::Poke() const {

  struct sockaddr_un raddr;
  int  rc;
  SocketHandle_t sock;

  if ((sock = socket(AF_LOCAL,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return 0;
  }

  memset((void*)&raddr,0,sizeof(raddr));
  raddr.sun_family = AF_LOCAL;
  strncpy(raddr.sun_path, pd_filename, sizeof(raddr.sun_path) - 1);


#if defined(USE_NONBLOCKING_CONNECT)

  if (tcpSocket::setNonBlocking(sock) == RC_INVALID_SOCKET) {
    CLOSESOCKET(sock);
    return 0;
  }

#endif

  if (::connect(sock,(struct sockaddr *)&raddr,
                sizeof(raddr)) == RC_SOCKET_ERROR) {

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
