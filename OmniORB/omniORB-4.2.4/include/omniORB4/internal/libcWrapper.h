// -*- Mode: C++; -*-
//                            Package   : omniORB
// LibcWrapper.h              Created on: 19/3/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2010 Apasphere Ltd
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
//    Wrapper for libc functions which are non-reentrant / non-portable
//

#ifndef __LIBCWRAPPER_H__
#define __LIBCWRAPPER_H__

#if defined(__WIN32__)
#  define FD_SETSIZE 2048
#  include <winsock2.h>
#else
#  include <netdb.h>
#endif

OMNI_NAMESPACE_BEGIN(omni)

class LibcWrapper {
public:
  class AddrInfo;

  static unsigned int Rand();
  // Thread-safe pseudo-random number

  static void SRand(unsigned int seed);
  // Set seed for Rand().

  static CORBA::Boolean isip4addr(const char* node);
  // True if node is an IPv4 address.

  static CORBA::Boolean isip6addr(const char* node);
  // True if node is an IPv6 address.

  static CORBA::Boolean isipaddr(const char* node);
  // True if node is an IPv4 or v6 address.

  static AddrInfo* getAddrInfo(const char* node, CORBA::UShort port);
  // Return an AddrInfo object for the specified node and port. If
  // node is zero, address is INADDR_ANY. If node is invalid, returns
  // zero.

  static void freeAddrInfo(AddrInfo* ai);
  // Release the AddrInfo object returned by getAddrInfo(), and any in
  // its linked list.

  class AddrInfo {
  public:
    AddrInfo() {}

    virtual ~AddrInfo();

    virtual struct sockaddr* addr() = 0;
    // sockaddr struct suitable for passing to bind(), connect()

    virtual int addrSize() = 0;
    // size of sockaddr struct returned.

    virtual int addrFamily() = 0;
    // protocol / address family of the sockaddr.

    virtual char* asString() = 0;
    // String form of address. Free with CORBA::string_free().

    virtual char* name() = 0;
    // Name relating to address. Returns zero if name cannot be found.

    virtual AddrInfo* next() = 0;
    // Linked list of AddrInfos for multi-homed hosts.

  private:
    // Not implemented:
    AddrInfo(const AddrInfo&);
    AddrInfo& operator=(const AddrInfo&);
  };

  class AddrInfo_var {
    // Partial _var type.
  public:
    inline AddrInfo_var() : pd_ai(0) {}
    inline AddrInfo_var(AddrInfo* ai) : pd_ai(ai) {}
    inline ~AddrInfo_var() {
      if (pd_ai) LibcWrapper::freeAddrInfo(pd_ai);
    }
    inline AddrInfo_var& operator=(AddrInfo* ai) {
      if (pd_ai) LibcWrapper::freeAddrInfo(pd_ai);
      pd_ai = ai;
      return *this;
    }
    inline AddrInfo* operator->() const { return pd_ai; }
    inline operator AddrInfo*() const   { return pd_ai; }
    inline AddrInfo* in() const         { return pd_ai; }
  private:
    AddrInfo* pd_ai;
  };
};


OMNI_NAMESPACE_END(omni)

#if defined(_MSC_VER) && !defined(_WINSTATIC)
#  if defined(_OMNIORB_LIBRARY)
#    define _NT_DLL_ATTR __declspec(dllexport)
#  else
#    define _NT_DLL_ATTR __declspec(dllimport)
#  endif
#endif

#ifndef _NT_DLL_ATTR
#  define _NT_DLL_ATTR
#endif

#ifndef HAVE_STRCASECMP
int _NT_DLL_ATTR strcasecmp(const char *s1, const char *s2);
#endif

#ifndef HAVE_STRNCASECMP
int _NT_DLL_ATTR strncasecmp(const char *s1, const char *s2,size_t n);
#endif

#endif // __LIBCWRAPPER_H__
