// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpTransportImpl.cc        Created on: 29 Mar 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
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

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <omniORB4/omniURI.h>
#include <objectAdapter.h>
#include <SocketCollection.h>
#include <tcpSocket.h>
#include <tcp/tcpConnection.h>
#include <tcp/tcpAddress.h>
#include <tcp/tcpEndpoint.h>
#include <tcp/tcpTransportImpl.h>
#include <orbParameters.h>

#if defined(UnixArchitecture)
#  include <sys/ioctl.h>
#  include <net/if.h>
#endif

#if defined(HAVE_IFADDRS_H)
#  include <ifaddrs.h>
#endif
 
#if defined(NTArchitecture)
#  include <libcWrapper.h>
#  include <ws2tcpip.h>
#endif

#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(tcpAddress);
OMNI_FORCE_LINK(tcpConnection);
OMNI_FORCE_LINK(tcpEndpoint);
OMNI_FORCE_LINK(tcpActive);

OMNI_EXPORT_LINK_FORCE_SYMBOL(tcpTransportImpl);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
tcpTransportImpl::tcpTransportImpl() : giopTransportImpl("giop:tcp") {
}

/////////////////////////////////////////////////////////////////////////
tcpTransportImpl::~tcpTransportImpl() {
  omnivector<const char*>::iterator i = ifAddresses.begin();
  omnivector<const char*>::iterator last = ifAddresses.end();
  while ( i != last ) {
    char* p = (char*)(*i);
    CORBA::string_free(p);
    i++;
  }
}

/////////////////////////////////////////////////////////////////////////
giopEndpoint*
tcpTransportImpl::toEndpoint(const char* param)
{
  if (!omniURI::validHostPortRange(param))
    return 0;

  return (giopEndpoint*)(new tcpEndpoint(param));
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpTransportImpl::isValid(const char* param) {

  return omniURI::validHostPortRange(param);
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpTransportImpl::parseAddress(const char* param, IIOP::Address& address) {

  char* host = omniURI::extractHostPort(param, address.port);
  if (!host)
    return 0;

  address.host = host;
  return 1;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
tcpTransportImpl::toAddress(const char* param) {

  IIOP::Address address;
  if (parseAddress(param,address)) {
    return (giopAddress*)(new tcpAddress(address));
  }
  else {
    return 0;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpTransportImpl::addToIOR(const char* param, IORPublish* eps) {

  IIOP::Address address;

  if (parseAddress(param,address)) {
    omniIOR::add_IIOP_ADDRESS(address, eps);
    return 1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
#if   defined(__vxWorks__)
static void vxworks_get_ifinfo(omnivector<const char*>& ifaddrs);
#elif defined(HAVE_IFADDRS_H)
static void ifaddrs_get_ifinfo(omnivector<const char*>& addrs);
#elif defined(UnixArchitecture)
static void unix_get_ifinfo(omnivector<const char*>& ifaddrs);
#elif defined(NTArchitecture)
static void win32_get_ifinfo(omnivector<const char*>& ifaddrs);
static void win32_get_ifinfo4(omnivector<const char*>& ifaddrs);
static void win32_get_ifinfo6(omnivector<const char*>& ifaddrs);
#endif

/////////////////////////////////////////////////////////////////////////
void
tcpTransportImpl::initialise() {
  if (!ifAddresses.empty()) return;

#if   defined(__vxWorks__)
  vxworks_get_ifinfo(ifAddresses);
#elif defined(HAVE_IFADDRS_H)
  ifaddrs_get_ifinfo(ifAddresses);
#elif defined(UnixArchitecture)
  unix_get_ifinfo(ifAddresses);
#elif defined(NTArchitecture)
  win32_get_ifinfo(ifAddresses);
#endif

  if ( orbParameters::dumpConfiguration || omniORB::trace(20) ) {
    omniORB::logger log;
    omnivector<const char*>::iterator i    = ifAddresses.begin();
    omnivector<const char*>::iterator last = ifAddresses.end();
    log << "My addresses are: \n";
    while ( i != last ) {
      log << "omniORB: " << (const char*)(*i) << "\n";
      i++;
    }
  }
}

/////////////////////////////////////////////////////////////////////////
const omnivector<const char*>*
tcpTransportImpl::getInterfaceAddress() {
  return &ifAddresses;
}

/////////////////////////////////////////////////////////////////////////
const tcpTransportImpl _the_tcpTransportImpl;

/////////////////////////////////////////////////////////////////////////
#if defined(HAVE_IFADDRS_H)
static
void ifaddrs_get_ifinfo(omnivector<const char*>& addrs)
{
  struct ifaddrs *ifa_list;

  if ( getifaddrs(&ifa_list) < 0 ) {
    if ( omniORB::trace(1) ) {
       omniORB::logger log;
       log << "Warning: getifaddrs() failed.\n"
           << "Unable to obtain the list of all interface addresses.\n";
    }
    return;
  }

  struct ifaddrs *p;

#if defined(OMNI_SUPPORT_IPV6)
  // Work out which network interface is the loopback
  CORBA::String_var ip6_loopback_if;

  for (p = ifa_list; p != 0; p = p->ifa_next) {
    if (p->ifa_addr && p->ifa_flags & IFF_UP) {
      if (p->ifa_addr->sa_family == AF_INET6) {
        if (IN6_IS_ADDR_LOOPBACK(&((sockaddr_in6*)p->ifa_addr)->sin6_addr)) {
	  ip6_loopback_if = CORBA::string_dup(p->ifa_name);
	  break;
	}
      }
    }
  }
#endif

  for (p = ifa_list; p != 0; p = p->ifa_next) {
    if (p->ifa_addr && p->ifa_flags & IFF_UP) {
      if (p->ifa_addr->sa_family == AF_INET) {
        CORBA::String_var s = tcpSocket::addrToString(p->ifa_addr);
        addrs.push_back(s._retn());
      }
#if defined(OMNI_SUPPORT_IPV6)
      else if (p->ifa_addr->sa_family == AF_INET6) {

        if (IN6_IS_ADDR_LINKLOCAL(&((sockaddr_in6*)p->ifa_addr)->sin6_addr)) {
	  if (orbParameters::dumpConfiguration || omniORB::trace(20)) {
	    omniORB::logger log;
	    CORBA::String_var ls = tcpSocket::addrToString(p->ifa_addr);
	    log << "Skip link local address " << ls
		<< " on interface " << p->ifa_name << ".\n";
	  }
          continue;
	}

	if ((const char*)ip6_loopback_if &&
	    !strcmp((const char*)ip6_loopback_if, p->ifa_name)) {

	  if (!IN6_IS_ADDR_LOOPBACK(&((sockaddr_in6*)p->ifa_addr)->sin6_addr)){
	    if (orbParameters::dumpConfiguration || omniORB::trace(20)) {
	      omniORB::logger log;
	      CORBA::String_var ls = tcpSocket::addrToString(p->ifa_addr);
	      log << "Skip address " << ls << " on loopback interface "
		  << ip6_loopback_if << ".\n";
	    }
	    continue;
	  }
	}
        CORBA::String_var s = tcpSocket::addrToString(p->ifa_addr);
        addrs.push_back(s._retn());
      }
#endif
    }
  }
  freeifaddrs(ifa_list);
}

#elif defined(UnixArchitecture) && !defined(__vxWorks__)

#  if defined(HAVE_STRUCT_LIFCONF) && defined(OMNI_SUPPORT_IPV6)
#    ifdef __sunos__
#      undef ifc_buf
#      undef ifc_req
#      undef ifr_addr
#      define sa_family ss_family
#    endif
#    define OMNI_SIOCGIFCONF SIOCGLIFCONF
#    define ifconf   lifconf
#    define ifreq    lifreq
#    define ifc_len  lifc_len
#    define ifc_buf  lifc_buf
#    define ifc_req  lifc_req
#    define ifr_addr lifr_addr
#    define ifr_name lifr_name
#    define USING_LIFCONF
#  else
#    ifdef __aix__
#      define OMNI_SIOCGIFCONF OSIOCGIFCONF
#    else
#      define OMNI_SIOCGIFCONF SIOCGIFCONF
#    endif
#  endif

static
void unix_get_ifinfo(omnivector<const char*>& addrs)
{
  SocketHandle_t sock;

  sock = socket(INETSOCKET,SOCK_STREAM,0);

  int lastlen = 0;
  int len = 100 * sizeof(struct ifreq);
  struct ifconf ifc;
  // struct ifconf and ifreq are defined in net/if.h

  while ( 1 ) {
    // There is no way to know for sure the buffer is big enough to get
    // the info for all the interfaces. We work around this by calling
    // the ioctl 2 times and increases the buffer size in the 2nd call.
    // If both calls return the info with the same size, we know we have
    // got all the interfaces.
    char* buf = (char*) malloc(len);
    ifc.ifc_len = len;
    ifc.ifc_buf = buf;

#ifdef USING_LIFCONF
    ifc.lifc_family = AF_UNSPEC;
    ifc.lifc_flags  = 0;
#endif

    if ( ioctl(sock, OMNI_SIOCGIFCONF, &ifc) < 0 ) {
      if ( errno != EINVAL || lastlen != 0 ) {
	if ( omniORB::trace(1) ) {
	  omniORB::logger log;
	  log << "Warning: ioctl SIOCGIFCONF failed.\n"
	      << "Unable to obtain the list of all interface addresses.\n";
	}
	free(buf);
	return;
      }
    }
    else {
      if ( ifc.ifc_len == lastlen )
	break; // Success, len has not changed.
      lastlen = ifc.ifc_len;
    }
    len += 10 * sizeof(struct ifreq);
    free(buf);
  }
  close(sock);

  int total = ifc.ifc_len / sizeof(struct ifreq);
  struct ifreq* ifr = ifc.ifc_req;

#if defined(OMNI_SUPPORT_IPV6)
  // Work out which network interface is the loopback
  CORBA::String_var ip6_loopback_if;

  for (int i = 0; i < total; i++) {
    if (ifr[i].ifr_addr.sa_family == AF_INET6) {
      struct sockaddr_in6* iaddr6 = (struct sockaddr_in6*)&ifr[i].ifr_addr;

      if (IN6_IS_ADDR_LOOPBACK(&iaddr6->sin6_addr)) {
	ip6_loopback_if = CORBA::string_dup(ifr[i].ifr_name);
	break;
      }
    }
  }
#endif

  for (int i = 0; i < total; i++) {

    if ( ifr[i].ifr_addr.sa_family == AF_INET ) {
      struct sockaddr_in* iaddr = (struct sockaddr_in*)&ifr[i].ifr_addr;

      if ( iaddr->sin_addr.s_addr != 0 ) {
	CORBA::String_var s = tcpSocket::addrToString((sockaddr*)iaddr);
	addrs.push_back(s._retn());
      }
    }
#if defined(OMNI_SUPPORT_IPV6)
    else if ( ifr[i].ifr_addr.sa_family == AF_INET6 ) {
      struct sockaddr_in6* iaddr6 = (struct sockaddr_in6*)&ifr[i].ifr_addr;

      if (IN6_IS_ADDR_UNSPECIFIED(&iaddr6->sin6_addr))
	continue;

      if (IN6_IS_ADDR_LINKLOCAL(&iaddr6->sin6_addr)) {
	if (orbParameters::dumpConfiguration || omniORB::trace(20)) {
	  omniORB::logger log;
	  CORBA::String_var ls;
	  ls = tcpSocket::addrToString((sockaddr*)iaddr6);
	  log << "Skip link local address " << ls
	      << " on interface " << ifr[i].ifr_name << ".\n";
	}
	continue;
      }

      if ((const char*)ip6_loopback_if &&
	  !strcmp((const char*)ip6_loopback_if, ifr[i].ifr_name)) {

	if (!IN6_IS_ADDR_LOOPBACK(&iaddr6->sin6_addr)) {
	  if (orbParameters::dumpConfiguration || omniORB::trace(20)) {
	    omniORB::logger log;
	    CORBA::String_var ls;
	    ls = tcpSocket::addrToString((sockaddr*)iaddr6);
	    log << "Skip address " << ls << " on loopback interface "
		<< ip6_loopback_if << ".\n";
	  }
	  continue;
	}
      }
      CORBA::String_var s = tcpSocket::addrToString((sockaddr*)iaddr6);
      addrs.push_back(s._retn());
    }
#endif
  }
  free(ifc.ifc_buf);
}

/////////////////////////////////////////////////////////////////////////
#elif defined(__vxWorks__)
void vxworks_get_ifinfo(omnivector<const char*>& ifaddrs)
{
  const int iMAX_ADDRESS_ENTRIES = 50;
  // Max. number of interface addresses.  There is 1 link layer address
  // (AF_LINK) and at least 1 internet address (more if ifAddrAdd has been
  // called) per configured network interface

  const int iMAX_IFREQ_SIZE=36;
  // ifreq entries have a name field (16 bytes) plus an address field,
  // AF_LINK addresses (sockaddr_dl = 20 bytes)
  // AF_INET addresses (sockaddr_in = 16 bytes)

  // buffer into which to copy retieved configuration
  char buffer[iMAX_ADDRESS_ENTRIES * iMAX_IFREQ_SIZE];

  struct ifconf ifc; // used to retrieve interface configuration
  struct ifreq *ifr; // interface structure pointer

  int s;             // socket file descriptor
  int entryLength;   // size of ifreq entry

  char ifreqBuf[iMAX_IFREQ_SIZE]; // buffer for checking flags
  struct sockaddr_dl *pDataLinkAddr;
  int offset;

  // create socket to issue ioctl call
  if ((s = socket (AF_INET, SOCK_DGRAM,0)) == ERROR) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Warning : socket (AF_INET, SOCK_DGRAM,0) failed. "
	  << "Unable to obtain the list of all interface addresses.\n";
    }
    return;
  }

  // set up interface request structure array
  ifc.ifc_len = sizeof (buffer);
  ifc.ifc_buf = (char *)buffer;

  // get configuration
  if (ioctl (s, SIOCGIFCONF, (int)&ifc) < 0) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Warning: ioctl SIOCGICONF failed. "
	  << "Unable to obtain the list of all interface addresses.\n";
    }
    close (s);
    return;
  }

  ifr = ifc.ifc_req;

  // ioctl call changes ifc_len to the actual total bytes copied
  entryLength = ifc.ifc_len;

  for (entryLength = ifc.ifc_len; entryLength > 0;) {
    offset = sizeof (ifr->ifr_name) + ifr->ifr_addr.sa_len;
    bcopy ((caddr_t)ifr, ifreqBuf, offset);

    if (ioctl (s, SIOCGIFFLAGS, (int)ifreqBuf) < 0) {
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Warning: ioctl SIOCGIFFLAGS failed. "
	    << "Unable to obtain the list of all interface addresses.\n";
      }
      close (s);
      return;
    }

    if (((struct ifreq *)ifreqBuf)->ifr_flags & IFF_UP) {
      if (ifr->ifr_addr.sa_family == AF_INET) {
	// AF_INET entries are of type sockaddr_in = 16 bytes
	struct sockaddr_in* iaddr = (struct sockaddr_in*) &(ifr->ifr_addr);
	CORBA::String_var s;
	s = tcpSocket::addrToString((sockaddr*)iaddr);
	ifaddrs.push_back(s._retn());
      }
    }      
    // ifreq structures have variable lengths
    entryLength -= offset;
    ifr = (struct ifreq *)((char *)ifr + offset);
  }
  close (s);
}

#endif


/////////////////////////////////////////////////////////////////////////
#if defined(NTArchitecture)

#if defined(__ETS_KERNEL__)
extern "C" int WSAAPI ETS_WSAIoctl(
  SOCKET s,
  DWORD dwIoControlCode,
  LPVOID lpvInBuffer,
  DWORD cbInBuffer,
  LPVOID lpvOutBuffer,
  DWORD cbOutBuffer,
  LPDWORD lpcbBytesReturned,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);
#define WSAIoctl ETS_WSAIoctl
#endif

void win32_get_ifinfo(omnivector<const char*>& ifaddrs)
{
  win32_get_ifinfo4(ifaddrs);
  win32_get_ifinfo6(ifaddrs);
}


static
void win32_get_ifinfo4(omnivector<const char*>& ifaddrs)
{
  // Get Windows IPv4 interfaces.

  SocketHandle_t sock;

  sock = socket(INETSOCKET,SOCK_STREAM,0);

  INTERFACE_INFO info[64];  // Assume max 64 interfaces
  DWORD retlen;
  
  if ( WSAIoctl(sock, SIO_GET_INTERFACE_LIST, NULL,0,
                (LPVOID)&info, sizeof(info), (LPDWORD)&retlen,
		NULL,NULL) == SOCKET_ERROR ) {
    if ( omniORB::trace(2) ) {
      omniORB::logger log;
      int err = WSAGetLastError();
      log << "Warning: WSAIoctl SIO_GET_INTERFACE_LIST failed.\n"
	  << "Unable to obtain the list of all IPv4 interface addresses.\n"
	  << "WSAGetLastError() = " << err << "\n";
    }
    return;
  }
  CLOSESOCKET(sock);

  int numAddresses = retlen / sizeof(INTERFACE_INFO);
  for (int i = 0; i < numAddresses; i++) {
    // Only add the address if the interface is running
    if (info[i].iiFlags & IFF_UP) {
      if (info[i].iiAddress.Address.sa_family == INETSOCKET) {
	struct sockaddr* addr = (struct sockaddr*)&info[i].iiAddress.AddressIn;
	CORBA::String_var s = tcpSocket::addrToString(addr);

	// Just to catch us out, Windows ME apparently sometimes
	// returns 0.0.0.0 as one of its interfaces...
	if (omni::strMatch((const char*)s, "0.0.0.0")) continue;

	ifaddrs.push_back(s._retn());
      }
    }
  }
}


static
void win32_get_ifinfo6(omnivector<const char*>& ifaddrs)
{
  // Get Windows IPv6 interfaces.

#ifdef OMNI_SUPPORT_IPV6

  SocketHandle_t sock;
  int err;

  sock = socket(AF_INET6,SOCK_STREAM,0);

  if (sock == RC_INVALID_SOCKET) {
    err = WSAGetLastError();
    if (omniORB::trace(2)) {
      omniORB::logger log;
      log << "Warning: unable to create an IPv6 socket. "
	  << "Unable to obtain the list of IPv6 interface addresses ("
	  << err << ").\n";
    }
    return;
  }

  int buflen = sizeof(SOCKET_ADDRESS_LIST) + 63 * sizeof(SOCKET_ADDRESS);
  char *buffer = new char[buflen];
  LPSOCKET_ADDRESS_LIST addrlist = (LPSOCKET_ADDRESS_LIST)buffer;
  DWORD retlen;
  
  if ( WSAIoctl(sock, SIO_ADDRESS_LIST_QUERY, NULL,0,
                (LPVOID)addrlist, buflen, (LPDWORD)&retlen,
		NULL,NULL) == SOCKET_ERROR ) {

    err = WSAGetLastError();
    delete[] buffer;
    if ( omniORB::trace(2) ) {
      omniORB::logger log;
      log << "Warning: WSAIoctl SIO_ADDRESS_LIST_QUERY failed. "
	  << "Unable to obtain the list of IPv6 interface addresses ("
	  << err << ").\n";
    }
    return;
  }
  CLOSESOCKET(sock);

  for (int i = 0; i < addrlist->iAddressCount; i++) {
    // Only add the address if the interface is running
    struct sockaddr* addr = addrlist->Address[i].lpSockaddr;

    if (addr->sa_family == AF_INET6) {
      // Skip link-local addresses.
      if (IN6_IS_ADDR_LINKLOCAL(&((sockaddr_in6*)addr)->sin6_addr))
	continue;

      CORBA::String_var s = tcpSocket::addrToString(addr);

      // Windows appends interface details to site local addresses.
      // Strip anything starting with a % sign.
      char* ppos = strchr((char*)s, '%');
      if (ppos)
	*ppos = '\0';

      ifaddrs.push_back(s._retn());
    }
  }
  delete [] buffer;

#endif // OMNI_SUPPORT_IPV6
}

#endif

OMNI_NAMESPACE_END(omni)
