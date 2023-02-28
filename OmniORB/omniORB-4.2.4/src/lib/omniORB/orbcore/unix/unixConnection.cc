// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixConnection.cc          Created on: 6 Aug 2001
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
#include <orbParameters.h>
#include <SocketCollection.h>
#include <unix/unixConnection.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>
#include <sys/un.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixConnection);

OMNI_NAMESPACE_BEGIN(omni)


/////////////////////////////////////////////////////////////////////////
int
unixConnection::Send(void* buf, size_t sz,
		     const omni_time_t& deadline) {

  if (sz > orbParameters::maxSocketSend)
    sz = orbParameters::maxSocketSend;

  int tx;

  do {
    struct timeval t;

    if (deadline) {
      if (tcpSocket::setTimeout(deadline, t)) {
	// Already timed out.
	return 0;
      }
      else {
        setNonBlocking();

        tx = tcpSocket::waitWrite(pd_socket, t);

	if (tx == 0) {
	  // Timed out
	  return 0;
	}
	else if (tx == RC_SOCKET_ERROR) {
	  if (ERRNO == RC_EINTR) {
	    continue;
          }
	  else {
	    return -1;
	  }
	}
      }
    }
    else {
      setBlocking();
    }

    // Reach here if we can write without blocking or we don't care if
    // we block here.
    if ((tx = ::send(pd_socket,(char*)buf,sz,0)) == RC_SOCKET_ERROR) {
      int err = ERRNO;
      if (RC_TRY_AGAIN(err))
	continue;
      else
	return -1;
    }
    else if (tx == 0)
      return -1;

    break;

  } while(1);

  return tx;
}

/////////////////////////////////////////////////////////////////////////
int
unixConnection::Recv(void* buf, size_t sz,
                     const omni_time_t& deadline) {

  if (sz > orbParameters::maxSocketRecv)
    sz = orbParameters::maxSocketRecv;

  int rx;

  do {
    if (pd_shutdown)
      return -1;

    struct timeval t;

    if (tcpSocket::setAndCheckTimeout(deadline, t)) {
      // Already timed out
      return 0;
    }

    if (t.tv_sec || t.tv_usec) {
      setNonBlocking();
      rx = tcpSocket::waitRead(pd_socket, t);

      if (rx == 0) {
	// Timed out
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
	continue;
#else
	return 0;
#endif
      }
      else if (rx == RC_SOCKET_ERROR) {
	if (ERRNO == RC_EINTR) {
	  continue;
        }
	else {
	  return -1;
	}
      }
    }
    else {
      setBlocking();
    }

    // Reach here if we can read without blocking or we don't care if
    // we block here.
    if ((rx = ::recv(pd_socket,(char*)buf,sz,0)) == RC_SOCKET_ERROR) {
      int err = ERRNO;
      if (RC_TRY_AGAIN(err))
	continue;
      else
	return -1;
    }
    else if (rx == 0)
      return -1;

    break;

  } while(1);

  return rx;
}

/////////////////////////////////////////////////////////////////////////
void
unixConnection::Shutdown() {
  SHUTDOWNSOCKET(pd_socket);
  pd_shutdown = 1;
}

/////////////////////////////////////////////////////////////////////////
const char*
unixConnection::myaddress() {
  return (const char*)pd_myaddress;
}

/////////////////////////////////////////////////////////////////////////
const char*
unixConnection::peeraddress() {
  return (const char*)pd_peeraddress;
}

/////////////////////////////////////////////////////////////////////////
unixConnection::unixConnection(SocketHandle_t sock, 
			       SocketCollection* belong_to,
			       const char* filename,
			       CORBA::Boolean isActive) : 
  SocketHolder(sock) {

  static CORBA::ULong suffix = 0;

  CORBA::String_var filename_1;
  filename_1 = CORBA::string_alloc(strlen(filename)+12);
  sprintf(filename_1,"%s %08x",filename,(unsigned int)++suffix);

  if (isActive) {
    pd_myaddress = unToString(filename_1);
    pd_peeraddress = unToString(filename);
  }
  else {
    pd_myaddress = unToString(filename);
    pd_peeraddress = unToString(filename_1);
  }

  tcpSocket::setCloseOnExec(sock);

  belong_to->addSocket(this);
}

/////////////////////////////////////////////////////////////////////////
unixConnection::~unixConnection() {
  clearSelectable();
  pd_belong_to->removeSocket(this);
  CLOSESOCKET(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
void
unixConnection::setSelectable(int now,
			      CORBA::Boolean data_in_buffer) {

  SocketHolder::setSelectable(now, data_in_buffer);
}

/////////////////////////////////////////////////////////////////////////
void
unixConnection::clearSelectable() {

  SocketHolder::clearSelectable();
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixConnection::isSelectable() {
  return pd_belong_to->isSelectable(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixConnection::Peek() {
  return SocketHolder::Peek();
}

/////////////////////////////////////////////////////////////////////////
char*
unixConnection::unToString(const char* filename) {

  const char* format = "giop:unix:%s";

  CORBA::ULong len = strlen(filename);
  if (len == 0) {
    filename = "<not bound>";
    len = strlen(filename);
  }
  len += strlen(format);
  CORBA::String_var addrstr(CORBA::string_alloc(len));
  sprintf(addrstr,format,(const char*)filename);
  return addrstr._retn();
}


OMNI_NAMESPACE_END(omni)
