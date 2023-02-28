// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpSocket.h                Created on: 4 June 2010
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2010-2011 Apasphere Ltd.
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

#ifndef __TCPSOCKET_h__
#define __TCPSOCKET_h__

#include <orbParameters.h>
#include <libcWrapper.h>
#include <omniORB4/omniServer.h>

////////////////////////////////////////////////////////////////////////
//  Platform feature selection

#if !defined(OMNI_DISABLE_IPV6) && defined(HAVE_STRUCT_SOCKADDR_IN6) && defined(HAVE_STRUCT_SOCKADDR_STORAGE) && defined(HAVE_GETADDRINFO) && defined(HAVE_GETNAMEINFO)
#  define OMNI_SUPPORT_IPV6
#  define OMNI_SOCKADDR_STORAGE sockaddr_storage
#else
#  define OMNI_SOCKADDR_STORAGE sockaddr_in
#endif

#define SOCKNAME_SIZE_T OMNI_SOCKNAME_SIZE_T
#define USE_NONBLOCKING_CONNECT
#define OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS
#define OMNIORB_HOSTNAME_MAX 512

#ifdef HAVE_POLL
#   define USE_POLL
#endif

// Darwin implementation of poll() appears to be broken
#if defined(__darwin__)
#   undef USE_POLL
#endif

#if defined(__hpux__)
#   if __OSVERSION__ < 11
#       undef USE_POLL
#   endif
#   define USE_FAKE_INTERRUPTABLE_RECV
#endif

#if defined(__WIN32__)
#   define USE_FAKE_INTERRUPTABLE_RECV
#   undef OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS
#endif

#if defined(__freebsd__) || defined(__netbsd__)
#   undef OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS
#endif

// By default, Linux does accept IPv4 connections on IPv6, but some
// distributions misconfigure it not to.
#if defined(__linux__) || defined(IPV6_V6ONLY)
#   undef OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS
#endif


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//             win32 API
//
#if defined(__WIN32__)

#  include <sys/types.h>

#  if defined(OMNI_SUPPORT_IPV6)
#    include <ws2tcpip.h>
#    include <Wspiapi.h>
#    if !defined(IPV6_V6ONLY)
#      define IPV6_V6ONLY 27  // Defined to this on Vista
#    endif
#  endif

#  define RC_INADDR_NONE       INADDR_NONE
#  define RC_INVALID_SOCKET    INVALID_SOCKET
#  define RC_SOCKET_ERROR      SOCKET_ERROR
#  define INETSOCKET           PF_INET
#  define CLOSESOCKET(sock)    closesocket(sock)
#  define SHUTDOWNSOCKET(sock) ::shutdown(sock,2)
#  define ERRNO                ::WSAGetLastError()
#  define RC_EINPROGRESS       WSAEWOULDBLOCK
#  define RC_EINTR             WSAEINTR
#  define RC_EBADF             WSAENOTSOCK

#  define RC_TRY_AGAIN(err)       ((err == WSAEINTR) || (err == WSAEWOULDBLOCK))

#else

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//             unix(ish)
//
#  if defined(__vxWorks__)
#    include <sockLib.h>
#    include <hostLib.h>
#    include <ioLib.h>
#    include <iosLib.h>
#    include <netinet/tcp.h>
#  else
#    include <sys/time.h>
#  endif
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#  include <sys/types.h>
#  include <errno.h>
#  include <libcWrapper.h>

#  if defined(USE_POLL)
#    include <poll.h>
#  endif

#  include <fcntl.h>

#  if defined (__uw7__)
#    ifdef shutdown
#      undef shutdown
#    endif
#  endif

#  if defined(__VMS) && defined(USE_tcpSocketVaxRoutines)
#    include "tcpSocketVaxRoutines.h"
#    undef accept
#    undef recv
#    undef send
#    define accept(a,b,c) tcpSocketVaxAccept(a,b,c)
#    define recv(a,b,c,d) tcpSocketVaxRecv(a,b,c,d)
#    define send(a,b,c,d) tcpSocketVaxSend(a,b,c,d)
#  endif

#  ifdef __rtems__
extern "C" int select (int,fd_set*,fd_set*,fd_set*,struct timeval *);
#  endif

#  define RC_INADDR_NONE     ((CORBA::ULong)-1)
#  define RC_INVALID_SOCKET  (-1)
#  define RC_SOCKET_ERROR    (-1)
#  define INETSOCKET         AF_INET
#  define CLOSESOCKET(sock)  close(sock)

#  if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#    define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#  elif defined(__osf1__) && defined(__alpha__)
#    define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#  else
     // XXX none of the above, calling shutdown() may not have the
     // desired effect.
#    define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#  endif

#  define ERRNO              errno
#  define RC_EINTR           EINTR
#  define RC_EINPROGRESS     EINPROGRESS
#  if defined (__vxWorks__)
#    define RC_EBADF         S_iosLib_INVALID_FILE_DESCRIPTOR  
#  else
#    define RC_EBADF         EBADF
#  endif
#  define RC_EAGAIN          EAGAIN

#  define RC_TRY_AGAIN(err) ((err == EINTR) ||\
                             (err == EWOULDBLOCK) ||\
                             (err == EAGAIN))

#endif

#if defined(NEED_GETHOSTNAME_PROTOTYPE)
extern "C" int gethostname(char *name, int namelen);
#endif


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

OMNI_NAMESPACE_BEGIN(omni)

#if defined(__WIN32__)
typedef SOCKET SocketHandle_t;
#else
typedef int SocketHandle_t;
#endif

class tcpSocket {
public:

  static SocketHandle_t Bind(const char*   	      host,
			     CORBA::UShort 	      port_min,
			     CORBA::UShort 	      port_max,
			     const char*   	      transport_type,
			     char*&  	              bound_host,
			     CORBA::UShort&           bound_port,
			     orbServer::EndpointList& endpoints);
  // Create a socket and bind() and listen().
  //
  // If host is null or empty string, bind to all interfaces;
  // otherwise bind to the specified interface.
  //
  // If port_min and port_max are zero, bind to an ephemeral port; if
  // they are non-zero and equal, bind to the specified port, setting
  // the SO_REUSEADDR socket option; if port_max > port_min, bind to
  // one of the ports in the specified range.
  //
  // transport_type is the URI prefix for the requesting transport,
  // e.g. giop:tcp
  //
  // Returns the bound socket, or RC_INVALID_SOCKET on error.
  //
  // bound_host is set to the chosen host address. Caller frees.
  //
  // bound_port is set to the chosen port.
  //
  // endpoints is populated with all the endpoints that result from
  // the socket.


  static SocketHandle_t Connect(const char*   	   host,
				CORBA::UShort 	   port,
				const omni_time_t& deadline,
				CORBA::ULong  	   strand_flags,
				CORBA::Boolean&    timed_out);
  // Connect to specified host and port.
  //
  // If deadline is set, connect attempt can time out.
  //
  // strand_flags contains additional requirements on the connection.
  // See giopStrandFlags.h.
  //
  // Returns bound socket, or RC_INVALID_SOCKET on error or timeout.
  // On timeout, timed_out is set true.


  static inline void
  logConnectFailure(const char*            message,
		    LibcWrapper::AddrInfo* ai)
  {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      CORBA::String_var addr = ai->asString();
      log << message << ": " << addr;
      
      CORBA::UShort port = addrToPort(ai->addr());
      if (port)
	log << ":" << port;

      log << "\n";
    }
  }

  static inline void
  logConnectFailure(const char*   message,
		    const char*   host,
		    CORBA::UShort port=0)
  {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << message << ": " << host;
      if (port)
	log << ":" << port;
      log << "\n";
    }
  }


  static inline int setAndCheckTimeout(const omni_time_t& deadline,
				       struct timeval&    t)
  {
    if (deadline) {
      if (setTimeout(deadline, t)) {
        // Already timed out.
	return 1;
      }
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      if (t.tv_sec > (time_t)orbParameters::scanGranularity) {
	t.tv_sec = (time_t)orbParameters::scanGranularity;
      }
#endif
    }
    else {
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      t.tv_sec  = (time_t)orbParameters::scanGranularity;
      t.tv_usec = 0;
#else
      t.tv_sec = t.tv_usec = 0;
#endif
    }
    return 0;
  }


  static inline int waitWrite(SocketHandle_t sock, struct timeval& t)
  {
    int rc;

#if defined(USE_POLL)
    struct pollfd fds;
    fds.fd = sock;
    fds.events = POLLOUT;
    int timeout = t.tv_sec*1000+((t.tv_usec+999)/1000);
    if (timeout == 0) timeout = -1;
    rc = poll(&fds, 1, timeout);
    if (rc > 0 && fds.revents & POLLERR) {
      rc = RC_SOCKET_ERROR;
    }
#else

#  if !defined(__WIN32__)
    if (sock >= FD_SETSIZE)
      return RC_SOCKET_ERROR;
#  endif

    fd_set fds, efds;
    FD_ZERO(&fds);
    FD_ZERO(&efds);
    FD_SET(sock,&fds);
    FD_SET(sock,&efds);
    struct timeval* tp = &t;
    if (t.tv_sec == 0 && t.tv_usec == 0) tp = 0;
    rc = select((int)sock+1, 0, &fds, &efds, tp);
#endif
    return rc;
  }

  static inline int waitRead(SocketHandle_t sock, struct timeval& t)
  {
    int rc;

#if defined(USE_POLL)
    struct pollfd fds;
    fds.fd = sock;
    fds.events = POLLIN;
    int timeout = t.tv_sec*1000+((t.tv_usec+999)/1000);
    if (timeout == 0) timeout = -1;
    rc = poll(&fds, 1, timeout);
    if (rc > 0 && fds.revents & POLLERR) {
      rc = RC_SOCKET_ERROR;
    }
#else

#  if !defined(__WIN32__)
    if (sock >= FD_SETSIZE)
      return RC_SOCKET_ERROR;
#  endif

    fd_set fds, efds;
    FD_ZERO(&fds);
    FD_ZERO(&efds);
    FD_SET(sock,&fds);
    FD_SET(sock,&efds);
    struct timeval* tp = &t;
    if (t.tv_sec == 0 && t.tv_usec == 0) tp = 0;
    rc = select((int)sock+1, &fds, 0, &efds, tp);
#endif
    return rc;
  }

  static inline CORBA::Boolean
  setTimeout(const omni_time_t& deadline, struct timeval& t)
  {
    if (!deadline) {
      // Avoid get_time call which is expensive on some platforms.
      t.tv_sec = t.tv_usec = 0;
      return 0;
    }

    omni_time_t now;
    omni_thread::get_time(now);

    if (deadline <= now) {
      t.tv_sec = t.tv_usec = 0;
      return 1;
    }
    else {
      omni_time_t diff(deadline);
      diff -= now;
      t.tv_sec  = diff.s;
      t.tv_usec = diff.ns / 1000;
      return 0;
    }
  }
  
  static int setBlocking(SocketHandle_t sock);
  static int setNonBlocking(SocketHandle_t sock);
  static int setCloseOnExec(SocketHandle_t sock);


  static char* addrToString(sockaddr* addr);
  // Return string form of an IP address in dotted decimal or
  // colon-separated hex.

  static char* addrToURI(sockaddr* addr, const char* prefix);
  // Return URI for address, with specified prefix.

  static CORBA::UShort addrToPort(sockaddr* addr);
  // Return port number in address.

  static char* peerToURI(SocketHandle_t sock, const char* prefix);
  // Return URI for peer on socket.

};


OMNI_NAMESPACE_END(omni);


#endif // __TCPSOCKET_h__
