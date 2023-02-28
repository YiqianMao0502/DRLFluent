// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpSocket.cc               Created on: 4 June 2010
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2010-2012 Apasphere Ltd.
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
//    *** PROPRIETARY INTERFACE ***
//    Utility functions for managing TCP sockets

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#include <SocketCollection.h>
#include <libcWrapper.h>
#include <orbParameters.h>
#include <objectAdapter.h>
#include <giopStrandFlags.h>
#include <tcpSocket.h>

#if defined(NTArchitecture)
#  include <libcWrapper.h>
#  include <ws2tcpip.h>
#endif

#if defined(__vxWorks__)
#  include "selectLib.h"
#endif

#ifdef OMNI_USE_CFNETWORK_CONNECT
#  include <CFNetwork/CFSocketStream.h>
#endif


OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
enum PassiveHost {
  EXPLICIT,
  PASSIVE,
  IPv4PASSIVE,
  IPv6PASSIVE
};

static SocketHandle_t
openSocket(const char*& 	      host,
	   PassiveHost&		      passive_host,
	   CORBA::UShort              port,
	   LibcWrapper::AddrInfo_var& aiv,
	   LibcWrapper::AddrInfo*&    ai)
{
  SocketHandle_t sock = RC_INVALID_SOCKET;

  do {
    aiv = LibcWrapper::getAddrInfo(host, port);
    ai  = aiv;

#if defined(OMNI_SUPPORT_IPV6)
    if (passive_host == PASSIVE) {
      // Prefer an IPv6 address if there is one
      LibcWrapper::AddrInfo* aip = ai;
      while (aip) {
	if (aip->addrFamily() == PF_INET6) {
	  ai = aip;
	  break;
	}
	aip = aip->next();
      }
    }
#endif
    if (ai == 0) {
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Cannot get the address of host " << host << ".\n";
      }
      return RC_INVALID_SOCKET;
    }

    sock = socket(ai->addrFamily(), SOCK_STREAM, 0);

    if (sock == RC_INVALID_SOCKET) {

      if (passive_host == PASSIVE) {
	omniORB::logs(2, "Unable to open socket for unspecified passive host. "
		      "Fall back to IPv4.");
	host = "0.0.0.0";
	passive_host = IPv4PASSIVE;
	continue;
      }
      else {
	omniORB::logs(1, "Unable to open required socket.");
	return RC_INVALID_SOCKET;
      }
    }

#if defined(OMNI_SUPPORT_IPV6) && defined(IPV6_V6ONLY)
#  if !defined(OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS)

    if (passive_host == PASSIVE) {
      // Attempt to turn IPV6_V6ONLY option off
      int valfalse = 0;
      omniORB::logs(10, "Attempt to set socket to listen on IPv4 and IPv6.");

      if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
		     (char*)&valfalse, sizeof(valfalse)) == RC_SOCKET_ERROR) {
	omniORB::logs(2, "Unable to set socket to listen on IPv4 and IPv6. "
		      "Fall back to just IPv4.");
	CLOSESOCKET(sock);
	sock = RC_INVALID_SOCKET;
	host = "0.0.0.0";
	passive_host = IPv4PASSIVE;
      }
    }
#  endif
#endif
  } while (sock == RC_INVALID_SOCKET);
  
  return sock;
}

static void
setSocketOptions(SocketHandle_t sock, CORBA::Boolean fixed_port)
{
  {
    // Prevent Nagle's algorithm
    int valtrue = 1;
    if (setsockopt(sock, IPPROTO_TCP,TCP_NODELAY,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {

      omniORB::logs(2, "Warning: failed to set TCP_NODELAY option.");
    }
  }

  if (orbParameters::socketSendBuffer != -1) {
    // Set the send buffer size
    int bufsize = orbParameters::socketSendBuffer;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF,
		   (char*)&bufsize, sizeof(bufsize)) == RC_SOCKET_ERROR) {

      omniORB::logs(2, "Warning: failed to set SO_SNDBUF option.");
    }
  }

  tcpSocket::setCloseOnExec(sock);

  if (fixed_port) {
    int valtrue = 1;
    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {

      omniORB::logs(2, "Warning: failed to set SO_REUSEADDR option.");
    }
  }
}


static CORBA::Boolean
doBind(SocketHandle_t sock, CORBA::UShort port, LibcWrapper::AddrInfo* ai)
{
  if (omniORB::trace(25)) {
    omniORB::logger log;
    CORBA::String_var addr(ai->asString());
    log << "Bind to address " << addr << " ";
    if (port)
      log << "port " << port << ".\n";
    else
      log << "ephemeral port.\n";
  }
  if (::bind(sock, ai->addr(), ai->addrSize()) == RC_SOCKET_ERROR)
    return 0;

  return 1;
}


SocketHandle_t
tcpSocket::Bind(const char*   	      	 host,
		CORBA::UShort 	      	 port_min,
		CORBA::UShort 	      	 port_max,
		const char*   	      	 transport_type,
		char*&  	         bound_host,
		CORBA::UShort&           bound_port,
		orbServer::EndpointList& endpoints)
{
  SocketHandle_t sock;
  PassiveHost passive_host;

  bound_host = 0;

  if (host && *host) {
    if (omni::strMatch(host, "0.0.0.0")) {
      passive_host = IPv4PASSIVE;
    }
#if defined(OMNI_SUPPORT_IPV6)
    else if (omni::strMatch(host, "::")) {
      passive_host = IPv6PASSIVE;
    }
#endif
    else {
      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << "Explicit bind to host " << host << ".\n";
      }
      passive_host = EXPLICIT;
    }
  }
  else {
    // No host specified. Try environment variable.
    host = getenv(OMNIORB_USEHOSTNAME_VAR);
    if (host) {
      if (omniORB::trace(5)) {
	omniORB::logger log;
	log << "Use hostname '" << host << "' from "
	    << OMNIORB_USEHOSTNAME_VAR << " environment variable.\n";
      }
      passive_host = EXPLICIT;
    }
    else {
      // No host -- use passive host
#if defined(OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS) || defined(IPV6_V6ONLY)
      host = 0;
      passive_host = PASSIVE;
#else
      host = "0.0.0.0";
      passive_host = IPv4PASSIVE;
#endif
    }
  }

  CORBA::UShort port, port_start;
  CORBA::Boolean fixed_port = 0;

  if (port_min == 0 && port_max == 0) {
    // Ephemeral port
    port = 0;
  }
  else if (port_min == port_max) {
    // Fixed port
    port = port_min;
    fixed_port = 1;
  }
  else {
    // Port range. Pick a random starting point.
    if (port_min > port_max)
      OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);

    port = port_min + LibcWrapper::Rand() % (port_max - port_min);
  }

  port_start = port;

  LibcWrapper::AddrInfo_var aiv;
  LibcWrapper::AddrInfo*    ai;

  //
  // Open socket and bind to a port

  do {
    sock = openSocket(host, passive_host, port, aiv, ai);
    
    if (sock == RC_INVALID_SOCKET)
      return RC_INVALID_SOCKET;

    setSocketOptions(sock, fixed_port);

    if (!doBind(sock, port, ai)) {
      CLOSESOCKET(sock);

      if (port_min != port_max) {
	CORBA::UShort fport = port;
	++port;
	if (port > port_max)
	  port = port_min;

	if (port == port_start) {
	  if (omniORB::trace(1)) {
	    omniORB::logger log;
	    CORBA::String_var addr(ai->asString());
	    log << "Failed to bind to address " << addr << " on any port in "
		<< "range " << port_min << "-" << port_max << "\n";
	  }
	  return RC_INVALID_SOCKET;
	}
	else {
	  if (omniORB::trace(25)) {
	    omniORB::logger log;
	    log << "Failed to bind to port " << fport << "; try " << port
		<< "...\n";
	  }
	  continue;
	}
      }
      else {
	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  CORBA::String_var addr(ai->asString());
	  log << "Failed to bind to address " << addr << " ";
	  if (port)
	    log << "port " << port << ". Address in use?\n";
	  else
	    log << "ephemeral port.\n";
	}
	return RC_INVALID_SOCKET;
      }
    }
    break;
  } while (1);

  //
  // Listen...
  
  if (listen(sock, orbParameters::listenBacklog) == RC_SOCKET_ERROR) {
    CLOSESOCKET(sock);
    omniORB::logs(1, "Failed to listen on socket.");
    return RC_INVALID_SOCKET;
  }

  //
  // Populate address details

  OMNI_SOCKADDR_STORAGE addr;
  SOCKNAME_SIZE_T l;
  l = sizeof(OMNI_SOCKADDR_STORAGE);

  if (getsockname(sock, (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    CLOSESOCKET(sock);
    omniORB::logs(1, "Failed to get socket name.");
    return RC_INVALID_SOCKET;
  }

  bound_port = addrToPort((struct sockaddr*)&addr);

  if (passive_host != EXPLICIT) {
    // Ask the transport for its list of interface addresses

    CORBA::ULong   addrs_len = 0;
    CORBA::Boolean set_host  = 0;

    const omnivector<const char*>* ifaddrs
      = giopTransportImpl::getInterfaceAddress(transport_type);

    if (ifaddrs && !ifaddrs->empty()) {
      // Transport successfully gave us a list of interface addresses

      const char* loopback4 = 0;
      const char* loopback6 = 0;

      omnivector<const char*>::const_iterator i;
      for (i = ifaddrs->begin(); i != ifaddrs->end(); i++) {

	if (passive_host == IPv4PASSIVE && !LibcWrapper::isip4addr(*i))
	  continue;

	if (passive_host == IPv6PASSIVE && !LibcWrapper::isip6addr(*i))
	  continue;

	if (omni::strMatch(*i, "127.0.0.1")) {
	  loopback4 = *i;
	  continue;
	}
	if (strncmp(*i, "127.", 4) == 0) {
	  // Anything else starting 127. is defined to be a loopback.
	  if (!loopback4)
	    loopback4 = *i;
	  continue;
	}
	if (omni::strMatch(*i, "::1")) {
	  loopback6 = *i;
	  continue;
	}
	endpoints.length(addrs_len + 1);
	endpoints[addrs_len++] = omniURI::buildURI(transport_type,
						   *i, bound_port);

	if (!set_host) {
	  bound_host = CORBA::string_dup(*i);
	  set_host = 1;
	}
      }
      if (!set_host) {
	// No suitable addresses other than the loopback.
	if (loopback4) {
	  endpoints.length(addrs_len + 1);
	  endpoints[addrs_len++] = omniURI::buildURI(transport_type,
						     loopback4,
						     bound_port);
	  bound_host = CORBA::string_dup(loopback4);
	  set_host = 1;
	}
	if (loopback6) {
	  endpoints.length(addrs_len + 1);
	  endpoints[addrs_len++] = omniURI::buildURI(transport_type,
						     loopback6,
						     bound_port);
	  if (!set_host) {
	    bound_host = CORBA::string_dup(loopback6);
	    set_host = 1;
	  }
	}
	if (!set_host) {
	  omniORB::logs(1, "No suitable address in the list of "
			"interface addresses.");
	  CLOSESOCKET(sock);
	  return RC_INVALID_SOCKET;
	}
      }
    }
    else {
      omniORB::logs(5, "No list of interface addresses; fall back to "
		    "system hostname.");
      char self[OMNIORB_HOSTNAME_MAX];

      if (gethostname(&self[0],OMNIORB_HOSTNAME_MAX) == RC_SOCKET_ERROR) {
	omniORB::logs(1, "Cannot get the name of this host.");
	CLOSESOCKET(sock);
	return RC_INVALID_SOCKET;
      }
      if (orbParameters::dumpConfiguration || omniORB::trace(10)) {
	omniORB::logger log;
	log << "My hostname is '" << self << "'.\n";
      }
      LibcWrapper::AddrInfo_var ai;
      ai = LibcWrapper::getAddrInfo(self, bound_port);
      if ((LibcWrapper::AddrInfo*)ai == 0) {
	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  log << "Cannot get the address of my hostname '"
	      << self << "'.\n";
	}
	CLOSESOCKET(sock);
	return RC_INVALID_SOCKET;
      }
      bound_host = ai->asString();
      endpoints.length(1);
      endpoints[0] = omniURI::buildURI(transport_type,
				       bound_host, bound_port);
    }
    if (omniORB::trace(1) &&
	(omni::strMatch(bound_host, "127.0.0.1") ||
	 omni::strMatch(bound_host, "::1"))) {

      omniORB::logger log;
      log << "Warning: the local loop back interface (" << bound_host
	  << ") is the only address available for this server.\n";
    }
  }
  else {
    // Specific host
    bound_host = CORBA::string_dup(host);
    endpoints.length(1);
    endpoints[0] = omniURI::buildURI(transport_type,
				     bound_host, bound_port);
  }

  OMNIORB_ASSERT(bound_host);
  OMNIORB_ASSERT(bound_port);

  return sock;
}

/////////////////////////////////////////////////////////////////////////

#ifndef OMNI_USE_CFNETWORK_CONNECT

static
SocketHandle_t
doConnect(const char*   	 host,
	  CORBA::UShort 	 port,
	  const omni_time_t&     deadline,
	  CORBA::ULong  	 strand_flags,
	  LibcWrapper::AddrInfo* ai,
	  CORBA::Boolean&        timed_out)
{
  SocketHandle_t sock;

  if ((sock = socket(ai->addrFamily(), SOCK_STREAM, 0)) == RC_INVALID_SOCKET) {
    tcpSocket::logConnectFailure("Failed to create socket", ai);
    return RC_INVALID_SOCKET;
  }

  if (!(strand_flags & GIOPSTRAND_ENABLE_TRANSPORT_BATCHING)) {
    // Prevent Nagle's algorithm
    int valtrue = 1;
    if (setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {
      tcpSocket::logConnectFailure("Failed to set TCP_NODELAY option", ai);
      CLOSESOCKET(sock);
      return RC_INVALID_SOCKET;
    }
  }
  else {
    omniORB::logs(25, "New TCP connection without NO_DELAY option.");
  }

  if (orbParameters::socketSendBuffer != -1) {
    // Set the send buffer size
    int bufsize = orbParameters::socketSendBuffer;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF,
		   (char*)&bufsize, sizeof(bufsize)) == RC_SOCKET_ERROR) {
      tcpSocket::logConnectFailure("Failed to set socket send buffer", ai);
      CLOSESOCKET(sock);
      return RC_INVALID_SOCKET;
    }
  }

#if !defined(USE_NONBLOCKING_CONNECT)

  if (::connect(sock,ai->addr(),ai->addrSize()) == RC_SOCKET_ERROR) {
    tcpSocket::logConnectFailure("Failed to connect", ai);
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }
  return sock;

#else
  if (tcpSocket::setNonBlocking(sock) == RC_INVALID_SOCKET) {
    tcpSocket::logConnectFailure("Failed to set socket to non-blocking mode",
				 ai);
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }
  if (::connect(sock,ai->addr(),ai->addrSize()) == RC_SOCKET_ERROR) {

    int err = ERRNO;
    if (err && err != RC_EINPROGRESS) {
      tcpSocket::logConnectFailure("Failed to connect", ai);
      CLOSESOCKET(sock);
      return RC_INVALID_SOCKET;
    }
  }

  struct timeval t;
  int rc;

  do {
    if (tcpSocket::setAndCheckTimeout(deadline, t)) {
      // Already timed out
      tcpSocket::logConnectFailure("Connect timed out", ai);
      CLOSESOCKET(sock);
      timed_out = 1;
      return RC_INVALID_SOCKET;
    }

    rc = tcpSocket::waitWrite(sock, t);

    if (rc == 0) {
      // Timed out
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      continue;
#else
      tcpSocket::logConnectFailure("Connect timed out", ai);
      CLOSESOCKET(sock);
      timed_out = 1;
      return RC_INVALID_SOCKET;
#endif
    }
    else if (rc == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR) {
	continue;
      }
      else {
	tcpSocket::logConnectFailure("Failed to connect "
                                     "(waiting for writable socket)", ai);
	CLOSESOCKET(sock);
	return RC_INVALID_SOCKET;
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
	tcpSocket::logConnectFailure("Failed to connect (no peer name)", ai);
	CLOSESOCKET(sock);
	return RC_INVALID_SOCKET;
      }
    }
    break;

  } while (1);

  if (tcpSocket::setBlocking(sock) == RC_INVALID_SOCKET) {
    tcpSocket::logConnectFailure("Failed to set socket to blocking mode", ai);
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }

  return sock;
#endif
}


SocketHandle_t
tcpSocket::Connect(const char*        host,
		   CORBA::UShort      port,
		   const omni_time_t& deadline,
		   CORBA::ULong       strand_flags,
		   CORBA::Boolean&    timed_out)
{
  OMNIORB_ASSERT(host);
  OMNIORB_ASSERT(port);

  LibcWrapper::AddrInfo_var aiv;
  aiv = LibcWrapper::getAddrInfo(host, port);

  LibcWrapper::AddrInfo* ai = aiv;

  if (ai == 0) {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Unable to resolve: " << host << "\n";
    }
    return RC_INVALID_SOCKET;
  }

  while (ai) {
    if (omniORB::trace(25)) {
      if (!LibcWrapper::isipaddr(host)) {
	omniORB::logger log;
	CORBA::String_var addr = ai->asString();
	log << "Name '" << host << "' resolved: " << addr << "\n";
      }
    }
    SocketHandle_t sock = doConnect(host, port, deadline,
				    strand_flags, ai, timed_out);
    if (sock != RC_INVALID_SOCKET)
      return sock;

    ai = ai->next();
  }
  return RC_INVALID_SOCKET;
}


#else // OMNI_USE_CFNETWORK_CONNECT


SocketHandle_t
tcpSocket::Connect(const char*        host,
		   CORBA::UShort      port,
		   const omni_time_t& deadline,
		   CORBA::ULong       strand_flags,
		   CORBA::Boolean&    timed_out)
{
  OMNIORB_ASSERT(host);
  OMNIORB_ASSERT(port);

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "Attempt to connect to " << host << ":" << port
	<< " with CFNetwork.\n";
  }

  CFStringRef host_str = CFStringCreateWithCString(NULL, host,
                                                   kCFStringEncodingASCII);
  CFWriteStreamRef wstream;

  // Create stream to host
  CFStreamCreatePairWithSocketToHost(NULL, host_str, port, NULL, &wstream);
  CFRelease(host_str);

  // Open it, causing the network connection to be created
  CFWriteStreamOpen(wstream);

  // Wait until it is connected. The socket is not available from the
  // stream until it has connected, so we have to wait with an
  // exponential back-off.
  CFStreamStatus status;
  omni_time_t    delay(0, 500000);        // 0.5 ms
  omni_time_t    max_delay(0, 100000000); // 100 ms

  while ((status = CFWriteStreamGetStatus(wstream)) == kCFStreamStatusOpening) {
    if (delay < max_delay)
      delay += delay;

    if (deadline) {
      omni_time_t now;
      omni_thread::get_time(now);
      if (now >= deadline) {
	tcpSocket::logConnectFailure("Connect timed out", host, port);
	CFRelease(wstream);
	timed_out = 1;
	return RC_INVALID_SOCKET;
      }
      if (deadline - now < delay) {
	delay = deadline - now;
	delay.ns /= 2;
      }
    }
    omni_thread::sleep(delay);
  }

  if (status != kCFStreamStatusOpen) {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Failed to open CFNetwork stream to "
          << host << ":" << port << "\n";
    }
    CFRelease(wstream);
    return RC_INVALID_SOCKET;
  }

  // Extract the native socket
  CFDataRef prop = (CFDataRef)CFWriteStreamCopyProperty(wstream,
                                          kCFStreamPropertySocketNativeHandle);
  CFSocketNativeHandle *sockp = (CFSocketNativeHandle *)CFDataGetBytePtr(prop);
  SocketHandle_t sock = *sockp;
  CFRelease(prop);

  // Tell the stream not to close the socket when we release it.
  CFWriteStreamSetProperty(wstream, kCFStreamPropertyShouldCloseNativeSocket,
                           kCFBooleanFalse);
  CFRelease(wstream);

  // Set socket options
  struct timeval t;
  int rc;

  if (!(strand_flags & GIOPSTRAND_ENABLE_TRANSPORT_BATCHING)) {
    // Prevent Nagle's algorithm
    int valtrue = 1;
    if (setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {
      tcpSocket::logConnectFailure("Failed to set TCP_NODELAY option",
                                   host, port);
      CLOSESOCKET(sock);
      return RC_INVALID_SOCKET;
    }
  }
  else {
    omniORB::logs(25, "New TCP connection without NO_DELAY option.");
  }

  if (orbParameters::socketSendBuffer != -1) {
    // Set the send buffer size
    int bufsize = orbParameters::socketSendBuffer;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF,
		   (char*)&bufsize, sizeof(bufsize)) == RC_SOCKET_ERROR) {
      tcpSocket::logConnectFailure("Failed to set socket send buffer",
                                   host, port);
      CLOSESOCKET(sock);
      return RC_INVALID_SOCKET;
    }
  }

  do {
    if (tcpSocket::setAndCheckTimeout(deadline, t)) {
      // Already timed out
      tcpSocket::logConnectFailure("Connect timed out", host, port);
      CLOSESOCKET(sock);
      timed_out = 1;
      return RC_INVALID_SOCKET;
    }

    rc = tcpSocket::waitWrite(sock, t);

    if (rc == 0) {
      // Timed out
      tcpSocket::logConnectFailure("Connect timed out", host, port);
      CLOSESOCKET(sock);
      timed_out = 1;
      return RC_INVALID_SOCKET;
    }
    else if (rc == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR) {
	continue;
      }
      else {
	tcpSocket::logConnectFailure("Failed to connect "
                                     "(waiting for writable socket)",
                                     host, port);
	CLOSESOCKET(sock);
	return RC_INVALID_SOCKET;
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
	tcpSocket::logConnectFailure("Failed to connect (no peer name)",
                                     host, port);
	CLOSESOCKET(sock);
	return RC_INVALID_SOCKET;
      }
    }
    break;

  } while (1);

  if (tcpSocket::setBlocking(sock) == RC_INVALID_SOCKET) {
    tcpSocket::logConnectFailure("Failed to set socket to blocking mode",
                                 host, port);
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }

  return sock;
}

#endif // OMNI_USE_CFNETWORK_CONNECT


/////////////////////////////////////////////////////////////////////////
int
tcpSocket::setBlocking(SocketHandle_t sock)
{
# if defined(__vxWorks__)
  int fl = FALSE;
  if (ioctl(sock, FIONBIO, (int)&fl) == ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# elif defined(__VMS)
  int fl = 0;
  if (ioctl(sock, FIONBIO, &fl) == RC_INVALID_SOCKET) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# elif defined(__WIN32__)
  u_long v = 0;
  if (ioctlsocket(sock,FIONBIO,&v) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# else
  int fl = 0;
  if (fcntl(sock,F_SETFL,fl) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# endif
}


/////////////////////////////////////////////////////////////////////////
int
tcpSocket::setNonBlocking(SocketHandle_t sock)
{
# if defined(__vxWorks__)
  int fl = TRUE;
  if (ioctl(sock, FIONBIO, (int)&fl) == ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# elif defined(__VMS)
  int fl = 1;
  if (ioctl(sock, FIONBIO, &fl) == RC_INVALID_SOCKET) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# elif defined(__WIN32__)
  u_long v = 1;
  if (ioctlsocket(sock,FIONBIO,&v) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# else
  int fl = O_NONBLOCK;
  if (fcntl(sock,F_SETFL,fl) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# endif
}


/////////////////////////////////////////////////////////////////////////
int
tcpSocket::setCloseOnExec(SocketHandle_t sock)
{
# if defined(__vxWorks__) || defined(__ETS_KERNEL__)
  // Not supported on vxWorks or ETS
  return 0;
# elif defined(__WIN32__)
  SetHandleInformation((HANDLE)sock, HANDLE_FLAG_INHERIT, 0);
  return 0;
# else
  int fl = FD_CLOEXEC;
  if (fcntl(sock,F_SETFD,fl) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# endif
}


/////////////////////////////////////////////////////////////////////////
char*
tcpSocket::addrToString(sockaddr* addr)
{
#if defined(HAVE_INET_NTOP)

  char dest[80];
  const char* addrstr;

  if (addr->sa_family == AF_INET) {
    sockaddr_in* addr_in = (sockaddr_in*)addr;
    addrstr = inet_ntop(AF_INET, &addr_in->sin_addr, dest, sizeof(dest));
  }
#if defined(OMNI_SUPPORT_IPV6)
  else {
    OMNIORB_ASSERT(addr->sa_family == AF_INET6);
    sockaddr_in6* addr_in6 = (sockaddr_in6*)addr;
    addrstr = inet_ntop(AF_INET6, &addr_in6->sin6_addr, dest, sizeof(dest));
  }
#endif
  OMNIORB_ASSERT(addrstr);

  return CORBA::string_dup(addrstr);

#elif defined (HAVE_GETNAMEINFO)

  char dest[80];
  socklen_t addrlen = 0;

  if (addr->sa_family == AF_INET) {
    addrlen = sizeof(sockaddr_in);
  }
#if defined(OMNI_SUPPORT_IPV6)
  else {
    OMNIORB_ASSERT(addr->sa_family == AF_INET6);
    addrlen = sizeof(sockaddr_in6);
  }
#endif
  OMNIORB_ASSERT(addrlen);

  int result = getnameinfo(addr, addrlen, dest, sizeof(dest), 0, 0,
			   NI_NUMERICHOST);
  if (result != 0) {
    omniORB::logs(1, "Unable to convert IP address to a string!");
    return CORBA::string_dup("**invalid**");
  }
  return CORBA::string_dup(dest);

#else

  OMNIORB_ASSERT(addr->sa_family == AF_INET);
  CORBA::ULong ipv4 = ((sockaddr_in*)addr)->sin_addr.s_addr;
  CORBA::ULong hipv4 = ntohl(ipv4);
  int ip1 = (int)((hipv4 & 0xff000000) >> 24);
  int ip2 = (int)((hipv4 & 0x00ff0000) >> 16);
  int ip3 = (int)((hipv4 & 0x0000ff00) >> 8);
  int ip4 = (int)((hipv4 & 0x000000ff));

  char* result = CORBA::string_alloc(16);
  sprintf(result,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
  return result;

#endif
}


/////////////////////////////////////////////////////////////////////////
char*
tcpSocket::addrToURI(sockaddr* addr, const char* prefix)
{
#if defined(HAVE_INET_NTOP)

  char dest[80];
  int port;
  const char* addrstr;

  if (addr->sa_family == AF_INET) {
    sockaddr_in* addr_in = (sockaddr_in*)addr;
    port = ntohs(addr_in->sin_port);
    addrstr = inet_ntop(AF_INET, &addr_in->sin_addr, dest, sizeof(dest));
  }
#if defined(OMNI_SUPPORT_IPV6)
  else if (addr->sa_family == AF_INET6) {
    sockaddr_in6* addr_in6 = (sockaddr_in6*)addr;
    port = ntohs(addr_in6->sin6_port);
    addrstr = inet_ntop(AF_INET6, &addr_in6->sin6_addr, dest, sizeof(dest));
  }
#endif
  else {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Unknown address family " << addr->sa_family << " in sockaddr.\n";
    }
    return CORBA::string_dup("**invalid**");
  }
  OMNIORB_ASSERT(addrstr);

  return omniURI::buildURI(prefix, addrstr, port);

#elif defined (HAVE_GETNAMEINFO)

  char dest[80];
  int port;
  socklen_t addrlen = 0;

  if (addr->sa_family == AF_INET) {
    addrlen = sizeof(sockaddr_in);
    sockaddr_in* addr_in = (sockaddr_in*)addr;
    port = ntohs(addr_in->sin_port);
  }
#if defined(OMNI_SUPPORT_IPV6)
  else if (addr->sa_family == AF_INET6) {
    addrlen = sizeof(sockaddr_in6);
    sockaddr_in6* addr_in6 = (sockaddr_in6*)addr;
    port = ntohs(addr_in6->sin6_port);
  }
#endif
  else {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Unknown address family " << addr->sa_family << " in sockaddr.\n";
    }
    return CORBA::string_dup("**invalid**");
  }
  OMNIORB_ASSERT(addrlen);

  int result = getnameinfo(addr, addrlen, dest, sizeof(dest), 0, 0,
			   NI_NUMERICHOST);
  if (result != 0) {
    omniORB::logs(1, "Unable to convert IP address to a string!");
    return CORBA::string_dup("**invalid**");
  }
  return omniURI::buildURI(prefix, dest, port);

#else

  OMNIORB_ASSERT(addr->sa_family == AF_INET);
  sockaddr_in* addr_in = (sockaddr_in*)addr;

  CORBA::ULong ipv4 = addr_in->sin_addr.s_addr;
  CORBA::ULong hipv4 = ntohl(ipv4);
  int ip1 = (int)((hipv4 & 0xff000000) >> 24);
  int ip2 = (int)((hipv4 & 0x00ff0000) >> 16);
  int ip3 = (int)((hipv4 & 0x0000ff00) >> 8);
  int ip4 = (int)((hipv4 & 0x000000ff));
  int port = ntohs(addr_in->sin_port);

  char* result = CORBA::string_alloc(strlen(prefix) + 24);
  sprintf(result,"%s%d.%d.%d.%d:%d",prefix, ip1, ip2, ip3, ip4, port);
  return result;

#endif
}


/////////////////////////////////////////////////////////////////////////
CORBA::UShort
tcpSocket::addrToPort(sockaddr* addr)
{
  if (addr->sa_family == AF_INET) {
    sockaddr_in* addr_in = (sockaddr_in*)addr;
    return ntohs(addr_in->sin_port);
  }
#if defined(OMNI_SUPPORT_IPV6)
  else {
    OMNIORB_ASSERT(addr->sa_family == AF_INET6);
    sockaddr_in6* addr_in6 = (sockaddr_in6*)addr;
    return ntohs(addr_in6->sin6_port);
  }
#else
  OMNIORB_ASSERT(0);
  return 0;
#endif
}


/////////////////////////////////////////////////////////////////////////
char*
tcpSocket::peerToURI(SocketHandle_t sock, const char* prefix)
{
  OMNI_SOCKADDR_STORAGE addr;
  SOCKNAME_SIZE_T l = sizeof(addr);
  if (getpeername(sock, (struct sockaddr*)&addr, &l) == RC_SOCKET_ERROR)
    return CORBA::string_dup("<unknown address>");
  
  return addrToURI((sockaddr*)&addr, prefix);
}


OMNI_NAMESPACE_END(omni)
