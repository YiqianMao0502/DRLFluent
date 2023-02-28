// -*- Mode: C++; -*-
//                            Package   : omniORB
// libcWrapper.cc             Created on: 19/3/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2007 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//	Wrapper for libc functions which are non-reentrant
//

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if !defined(HAVE_STRCASECMP) || !defined(HAVE_STRNCASECMP)
#  include <ctype.h>  //  for toupper and tolower.
#endif

#include <SocketCollection.h>

#ifdef HAVE_GETADDRINFO
#  ifdef __WIN32__
#    include <ws2tcpip.h>
#  else
#    include <sys/types.h>
#    include <sys/socket.h>
#    include <netdb.h>
#  endif
#endif

#define HAVE_GETHOSTBYADDR

#ifdef __vxWorks__
#  include <hostLib.h>
#  include <resolvLib.h>
#  undef HAVE_GETHOSTBYADDR
#endif

#include <libcWrapper.h>

OMNI_NAMESPACE_BEGIN(omni)

//
// Pseudo-random numbers
//

#ifdef HAVE_RAND_R
static unsigned int seed=1;

unsigned int LibcWrapper::Rand()
{
  return rand_r(&seed);
}

void LibcWrapper::SRand(unsigned int s)
{
  seed = s;
}

#else
static omni_tracedmutex rand_lock("rand_lock");

unsigned int LibcWrapper::Rand()
{
  omni_tracedmutex_lock l(rand_lock);
  return rand();
}

void LibcWrapper::SRand(unsigned int s)
{
  srand(s);
}

#endif  


//
// IP address validation
//

static inline CORBA::Boolean
checkNumber(const char* num, int base, long int max)
{
  char* end;

  long int val = strtoul(num, &end, base);
  if (val > max || *end != '\0')
    return 0;
  return 1;
}

CORBA::Boolean LibcWrapper::isip4addr(const char* node)
{
  // Test if string node is an IPv4 address
  // Return: 0: not ip address,  1: is ip address

  const char* c;
  char  buf[4];
  int   bi   = 0;
  int   dots = 0;

  for (c=node; *c; ++c) {
    if (*c == '.') {
      ++dots;
      if (bi == 0)
	return 0;

      buf[bi] = '\0';
      bi = 0;
      if (!checkNumber(buf, 10, 255))
	return 0;
    }
    else {
      if (*c < '0' || *c > '9')
	return 0;

      buf[bi++] = *c;
      if (bi == 4)
	return 0;
    }
  }
  if (dots != 3)
    return 0;

  buf[bi] = '\0';
  if (!checkNumber(buf, 10, 255))
    return 0;

  return 1;
}

CORBA::Boolean LibcWrapper::isip6addr(const char* node)
{
  // Test if string node is an IPv6 address
  // Return: 0: not ip address,  1: is ip address

  const char*    c;
  char           buf[16];
  int            bi         = 0;
  int            colons     = 0;
  int            dots       = 0;
  CORBA::Boolean seen_blank = 0;

  for (c=node; *c; ++c) {
    if (*c == ':') {
      ++colons;

      if (bi) {
	buf[bi] = '\0';
	bi = 0;
	if (!checkNumber(buf, 16, 0xffff))
	  return 0;
      }
      else if (c == node) {
        // First character is a colon. Only valid if the next one is a
        // colon too.
        if (*(c+1) != ':')
          return 0;
      }
      else {
        if (seen_blank) {
          // Only one :: permitted
          return 0;
        }
        seen_blank = 1;
      }
    }
    else if ((*c >= '0' && *c <= '9') ||
	     (*c >= 'a' && *c <= 'f') ||
	     (*c >= 'A' && *c <= 'F')) {
      buf[bi++] = *c;
      if (dots == 0) {
	if (bi == 5) {
	  return 0;
	}
      }
      else {
	if (bi == 16) {
	  return 0;
	}
      }
    }
    else if (*c == '.') {
      ++dots;
      buf[bi++] = *c;
      if (bi == 16)
	return 0;
    }
    else {
      return 0;
    }
  }
  if (seen_blank) {
    if (colons < 2 || colons > 7)
      return 0;
  }
  else {
    if (colons != (dots ? 6 : 7))
      return 0;
  }

  if (bi == 0) {
    // No number at the end. Only valid if it ends ::
    const char* t = c-2;
    return (t[0] == ':' && t[1] == ':');
  }

  buf[bi] = '\0';
  if (dots) {
    // buf should contain an IPv4 address
    if (dots == 3) {
      return isip4addr(buf);
    }
    else {
      return 0;
    }
  }
  else {
    return checkNumber(buf, 16, 0xffff);
  }
}

CORBA::Boolean LibcWrapper::isipaddr(const char* node)
{
  return isip4addr(node) || isip6addr(node);
}


//
// AddrInfo
//

LibcWrapper::AddrInfo::~AddrInfo() {}


#ifdef HAVE_GETADDRINFO

class FullAddrInfo : public LibcWrapper::AddrInfo
{
public:
  FullAddrInfo(struct addrinfo* info, CORBA::Boolean do_free)
    : pd_addrinfo(info), pd_next(0), pd_free(do_free)
  { }

  virtual ~FullAddrInfo();

  virtual struct sockaddr* addr();
  virtual int addrSize();
  virtual int addrFamily();
  virtual char* asString();
  virtual char* name();
  virtual LibcWrapper::AddrInfo* next();

private:
  struct addrinfo* pd_addrinfo;
  FullAddrInfo*    pd_next;     // Next in linked list
  CORBA::Boolean   pd_free;     // If true, pd_addrinfo should be freed on
				// destruction.
};

LibcWrapper::AddrInfo* LibcWrapper::getAddrInfo(const char*   node,
						CORBA::UShort port)
{
  char service[6];
  sprintf(service, "%d", (int)port);

  struct addrinfo hints;
  hints.ai_flags     = 0;
#ifdef OMNI_SUPPORT_IPV6
  hints.ai_family    = PF_UNSPEC;
#else
  hints.ai_family    = PF_INET;
#endif
  hints.ai_socktype  = SOCK_STREAM;
  hints.ai_protocol  = 0;
  hints.ai_addrlen   = 0;
  hints.ai_addr      = 0;
  hints.ai_canonname = 0;
  hints.ai_next      = 0;

  if (node == 0) {
    hints.ai_flags |= AI_PASSIVE;
  }
  else if (omni::strMatch(node, "0.0.0.0") ||
	   omni::strMatch(node, "::")) {
    hints.ai_flags |= AI_PASSIVE | AI_NUMERICHOST;
  }
  else if (LibcWrapper::isipaddr(node)) {
    hints.ai_flags |= AI_NUMERICHOST;
  }

  struct addrinfo* info;

  int result = getaddrinfo(node, service, &hints, &info);

  if (result != 0) {
    if (omniORB::trace(2)) {
      omniORB::logger l;
      l << "getaddrinfo failed for node '" << node << "', port " << port
	<< ": " << gai_strerror(result) << "\n";
    }
    return 0;
  }
  return new FullAddrInfo(info, 1);
}

void
LibcWrapper::freeAddrInfo(LibcWrapper::AddrInfo* ai)
{
  delete ai;
}

FullAddrInfo::~FullAddrInfo()
{
  if (pd_free && pd_addrinfo)
    freeaddrinfo(pd_addrinfo);

  if (pd_next)
    delete pd_next;
}


struct sockaddr*
FullAddrInfo::addr()
{
  OMNIORB_ASSERT(pd_addrinfo);
  return pd_addrinfo->ai_addr;
}

int
FullAddrInfo::addrSize()
{
  OMNIORB_ASSERT(pd_addrinfo);
  return pd_addrinfo->ai_addrlen;
}

int
FullAddrInfo::addrFamily()
{
  OMNIORB_ASSERT(pd_addrinfo);
  return pd_addrinfo->ai_addr->sa_family;
}

char*
FullAddrInfo::asString()
{
  char host[NI_MAXHOST];

  OMNIORB_ASSERT(pd_addrinfo);

  while (1) {
    int result = getnameinfo(pd_addrinfo->ai_addr, pd_addrinfo->ai_addrlen,
			     host, NI_MAXHOST, 0, 0, NI_NUMERICHOST);
    if (result == 0) {
      return CORBA::string_dup(host);
    }
    else if (result == EAI_AGAIN) {
      continue;
    }
    else {
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "Error calling getnameinfo: " << result << "\n";
      }
      return CORBA::string_dup("**invalid**");
    }
  }
}

char*
FullAddrInfo::name()
{
  char host[NI_MAXHOST];

  OMNIORB_ASSERT(pd_addrinfo);

  while (1) {
    int result = getnameinfo(pd_addrinfo->ai_addr, pd_addrinfo->ai_addrlen,
			     host, NI_MAXHOST, 0, 0, NI_NAMEREQD);
    if (result == 0) {
      return CORBA::string_dup(host);
    }
    else if (result == EAI_NONAME) {
      return 0;
    }
    else if (result == EAI_AGAIN) {
      continue;
    }
    else {
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "Error calling getnameinfo: " << result << "\n";
      }
      return 0;
    }
  }
}

LibcWrapper::AddrInfo*
FullAddrInfo::next()
{
  OMNIORB_ASSERT(pd_addrinfo);

  if (!pd_next && pd_addrinfo->ai_next)
    pd_next = new FullAddrInfo(pd_addrinfo->ai_next, 0);

  return pd_next;
}


#else // no HAVE_GETADDRINFO


//
// AddrInfo without getaddrinfo()
//

static omni_tracedmutex non_reentrant("non_reentrant");

class IP4AddrInfo : public LibcWrapper::AddrInfo
{
public:
  IP4AddrInfo(const char* name, CORBA::ULong ip4addr, CORBA::UShort port);
  virtual ~IP4AddrInfo();

  virtual struct sockaddr* addr();
  virtual int addrSize();
  virtual int addrFamily();
  virtual char* asString();
  virtual char* name();
  virtual LibcWrapper::AddrInfo* next();

private:
  struct sockaddr_in pd_addr;
  CORBA::String_var  pd_name;
};

IP4AddrInfo::IP4AddrInfo(const char* name,
			 CORBA::ULong ip4addr,
			 CORBA::UShort port)
{
  if (name)
    pd_name = CORBA::string_dup(name);

  pd_addr.sin_family      = INETSOCKET;
  pd_addr.sin_addr.s_addr = ip4addr;
  pd_addr.sin_port        = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  pd_addr.sin_len = sizeof(struct sockaddr_in);
#endif
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_ZERO
  memset((void *)&pd_addr.sin_zero, 0, sizeof(pd_addr.sin_zero));
#endif
}

IP4AddrInfo::~IP4AddrInfo() {}

struct sockaddr*
IP4AddrInfo::addr()
{
  return (struct sockaddr*)&pd_addr;
}

int
IP4AddrInfo::addrSize()
{
  return sizeof(struct sockaddr_in);
}

int
IP4AddrInfo::addrFamily()
{
  return PF_INET;
}

char*
IP4AddrInfo::asString()
{
  CORBA::ULong hipv4 = ntohl(pd_addr.sin_addr.s_addr);
  int ip1 = (int)((hipv4 & 0xff000000) >> 24);
  int ip2 = (int)((hipv4 & 0x00ff0000) >> 16);
  int ip3 = (int)((hipv4 & 0x0000ff00) >> 8);
  int ip4 = (int)((hipv4 & 0x000000ff));

  char* result = CORBA::string_alloc(16);
  sprintf(result,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
  return result;
}

char*
IP4AddrInfo::name()
{
  if ((const char*)pd_name) {
    return CORBA::string_dup(pd_name);
  }
#ifdef HAVE_GETHOSTBYADDR
  omni_tracedmutex_lock sync(non_reentrant);
  struct hostent* ent = gethostbyaddr((const char*)&pd_addr.sin_addr.s_addr,
				      sizeof(struct sockaddr_in),
				      AF_INET);
  if (ent)
    return CORBA::string_dup(ent->h_name);
#endif
  return 0;
}


LibcWrapper::AddrInfo*
IP4AddrInfo::next()
{
  return 0; // Not implemented
}


static inline CORBA::ULong hostent_to_ip4(struct hostent* entp)
{
  CORBA::ULong ip4;
  memcpy((void*)&ip4, (void*)entp->h_addr_list[0], sizeof(CORBA::ULong));
  return ip4;
}


LibcWrapper::AddrInfo* LibcWrapper::getAddrInfo(const char*   node,
						CORBA::UShort port)
{
  if (!node) {
    // No node address -- used to bind to INADDR_ANY
    return new IP4AddrInfo(0, INADDR_ANY, port);
  }
  if (isip4addr(node)) {
    // Already an IPv4 address.
    CORBA::ULong ip4 = inet_addr(node);

    if (ip4 == RC_INADDR_NONE)
      return 0;
    else
      return new IP4AddrInfo(0, ip4, port);
  }

#if defined(__sunos__) && __OSVERSION__ >= 5

  // Use gethostbyname_r() on Solaris 2

  struct hostent ent;
  char* buffer = new char[256];
  int buflen = 256;
  int rc;
  IP4AddrInfo* ret;

again:
  if (gethostbyname_r(node,&ent,buffer,buflen,&rc) == 0) {
    if (errno == ERANGE) {
      // buffer is too small to store the result, try again
      delete [] buffer;
      buflen = buflen * 2;
      buffer = new char [buflen];
      goto again;
    }
    else {
      ret = 0;
    }
  }
  else {
    ret = new IP4AddrInfo(ent.h_name, hostent_to_ip4(&ent), port);
  }
  delete [] buffer;
  return ret;

#elif defined(__irix__) && __OSVERSION__ >= 6

  // Use gethostbyname_r() on Irix 6.5

  struct hostent ent;
  char* buffer = new char[256];
  int buflen = 256;
  int rc;
  IP4AddrInfo* ret;

again:
  if (gethostbyname_r(node,&ent,buffer,buflen,&rc) == 0) {
    if (errno == ERANGE) {
      // buffer is too small to store the result, try again
      delete [] buffer;
      buflen = buflen * 2;
      buffer = new char [buflen];
      goto again;
    }
    else {
      ret = 0;
    }
  }
  else {
    ret = new IP4AddrInfo(ent.h_name, hostent_to_ip4(&ent), port);
  }
  delete [] buffer;
  return ret;

#elif defined(__linux__) && __OSVERSION__ >= 2 && !defined(__cygwin__)

  // Use gethostbyname_r() on Linux 

  struct hostent ent;
  char* buffer = new char[256];
  int buflen = 256;
  int rc, retValue;
  struct hostent *hp;
  IP4AddrInfo* ret;

again:
  retValue = gethostbyname_r(node,&ent,buffer,buflen,&hp,&rc);
  if (hp == 0) {
    if (retValue == ERANGE) {
      // buffer is too small to store the result, try again
      delete [] buffer;
      buflen = buflen * 2;
      buffer = new char [buflen];
      goto again;
    }
    else {
      ret = 0;
    }
  }
  else {
    ret = new IP4AddrInfo(ent.h_name, hostent_to_ip4(&ent), port);
  }
  delete [] buffer;
  return ret;



#elif defined(__osf1__)

  // Use gethostbyname_r() on Digital UNIX

  struct hostent ent;
  char* buffer = new char[sizeof(struct hostent_data)];
  memset((void*)buffer,0,sizeof(struct hostent_data));
  // XXX Is it possible that the pointer buffer is at a wrong alignment
  //     for a struct hostent_data?

  IP4AddrInfo* ret;

  if (gethostbyname_r(node,&ent,(struct hostent_data *)buffer) < 0)
    ret = 0;
  else
    ret = new IP4AddrInfo(ent.h_name, hostent_to_ip4(&ent), port);

  delete [] buffer;
  return ret;

#elif defined(__hpux__)

# if __OSVERSION__ >= 11
  // gethostbyname is thread safe and reentrant under HPUX 11.00
  struct hostent *hp;
  if ((hp = ::gethostbyname(node)) == NULL) {
    return 0;
  }
  return new IP4AddrInfo(0, hostent_to_ip4(hp), port);

# else
  // Use gethostbyname_r() on HPUX 10.20
  //  int gethostbyname_r(const char *node, struct hostent *result,
  //                       struct hostent_data *buffer);
  // -1 = Error, 0 is OK

  struct hostent ent;
  char* buffer = new char[sizeof(hostent_data)];
  memset((void*)buffer,0,sizeof(hostent_data));
  IP4AddrInfo* ret;

  if (gethostbyname_r(node,&ent,(hostent_data*)buffer) == -1)
    ret = 0;
  else
    ret = new IP4AddrInfo(ent.h_name, hostent_to_ip4(&ent), port);

  delete [] buffer;
  return ret;

# endif

#elif defined(__vxWorks__)
  int ip4 = hostGetByName(const_cast<char*>(node)); // grep /etc/hosts
  if (ip4 == ERROR) return 0;
  return new IP4AddrInfo(0, ip4, port);

#else
  // Use non-reentrant gethostbyname()
  omni_tracedmutex_lock sync(non_reentrant);

  struct hostent *hp = ::gethostbyname(node);
  
# ifdef __atmos__
  if (hp <= 0)
    return 0
# else
  if (hp == NULL)
    return 0;
# endif

  return new IP4AddrInfo(hp->h_name, hostent_to_ip4(hp), port);
#endif
}

void LibcWrapper::freeAddrInfo(LibcWrapper::AddrInfo* ai)
{
  delete ai;
}

#endif  // HAVE_GETADDRINFO

OMNI_NAMESPACE_END(omni)


//
// strcasecmp / strncasecmp
//

#ifndef HAVE_STRCASECMP
int
strcasecmp(const char *s1, const char *s2)
{
  if( s1 == s2 )  return 0;

  while( *s1 && tolower(*s1) == tolower(*s2) )
    s1++, s2++;

  return (int)*s1 - *s2;
}
#endif


#ifndef HAVE_STRNCASECMP
int
strncasecmp(const char *s1, const char *s2, size_t n)
{
  if( s1 == s2 || !n )  return 0;

  while( --n && *s1 && tolower(*s1) == tolower(*s2) )
    s1++, s2++;

  return (int)*s1 - *s2;
}
#endif

