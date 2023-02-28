// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopEndpoint.cc            Created on: 29 Mar 2001
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

#include <omniORB4/CORBA.h>
#include <omniORB4/omniTransport.h>
#include <omniORB4/giopEndpoint.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/linkHacks.h>
#include <omniORB4/omniInterceptors.h>
#include <interceptors.h>
#include <initialiser.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <SocketCollection.h>
#include <transportRules.h>
#include <stdio.h>

//
// Make sure built-in transports are always linked

OMNI_FORCE_LINK(tcpTransportImpl);

#if defined(UnixArchitecture)
OMNI_FORCE_LINK(unixTransportImpl);
#endif

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
static giopTransportImpl*& implHead() {
  static giopTransportImpl* head_ = 0;
  return head_;
}


////////////////////////////////////////////////////////////////////////
static
giopTransportImpl*
matchType(const char* uri,const char*& param,CORBA::Boolean allowShortHand) {

  const char* protocol = uri;
  param = 0;

  CORBA::String_var expanded;
  if (allowShortHand) {
    const char* p1 = strchr(uri,':');
    // Either the uri is of the form ":xxxx:xxxx" or "xxx::xxxx". Do we
    // have to expand?
    if (p1 && (uri == p1 || *(p1+1) == ':')) {
      const char* p2 = strchr(p1+1,':');
      if (p2) {
	CORBA::ULong len = (p1 - uri) + (p2 - p1) + sizeof("giop:tcp");
	expanded = CORBA::string_alloc(len);
	if (uri == p1) {
	  strcpy(expanded,"giop");
	}
	else {
	  strncpy(expanded,uri,(p1-uri));
	  ((char*) expanded)[p1-uri] = '\0';
	}
	if (p2 == p1+1) {
	  strcat(expanded,":tcp");
	}
	else {
	  len = strlen(expanded);
	  strncat(expanded,p1,(p2-p1));
	  ((char*)expanded)[len+(p2-p1)] = '\0';
	}
	protocol = expanded;
	param = p2 + 1;
      }
    }
  }

  giopTransportImpl* impl = implHead();

  while (impl) {
    int len = strlen(impl->type);
    if (strncmp(protocol,impl->type,len) == 0) {
      if (param == 0) {
	param = uri + len + 1;
	if (*(param-1) != ':') impl = 0;
      }
      break;
    }
    impl = impl->next;
  }
  return impl;
}


////////////////////////////////////////////////////////////////////////
giopAddress*
giopAddress::str2Address(const char* address) {

  const char* param;
  giopTransportImpl* impl = matchType(address,param,1);
  if (impl) {
    return impl->toAddress(param);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
const char*
giopAddress::host() const {
  return 0;
}


////////////////////////////////////////////////////////////////////////
giopAddress*
giopAddress::duplicate(const char* host) const {
  return duplicate();
}


////////////////////////////////////////////////////////////////////////
giopAddress*
giopAddress::fromTcpAddress(const IIOP::Address& addr) {

  CORBA::String_var addrstr = omniURI::buildURI("giop:tcp",
						addr.host, addr.port);
  return giopAddress::str2Address(addrstr);
}

////////////////////////////////////////////////////////////////////////
giopAddress*
giopAddress::fromSslAddress(const IIOP::Address& addr) {

  CORBA::String_var addrstr = omniURI::buildURI("giop:ssl",
						addr.host, addr.port);
  return giopAddress::str2Address(addrstr);
}

////////////////////////////////////////////////////////////////////////
giopEndpoint*
giopEndpoint::str2Endpoint(const char* endpoint) {

  const char* param;
  giopTransportImpl* impl = matchType(endpoint,param,1);
  if (impl) {
    return impl->toEndpoint(param);
  }
  return 0;

}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopEndpoint::strIsValidEndpoint(const char* endpoint) {

  const char* param;
  giopTransportImpl* impl = matchType(endpoint,param,0);
  if (impl) {
    return impl->isValid(param);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopEndpoint::addToIOR(const char* endpoint, IORPublish* eps) {

  const char* param;
  giopTransportImpl* impl = matchType(endpoint, param, 0);
  if (impl) {
    return impl->addToIOR(param, eps);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
giopTransportImpl::giopTransportImpl(const char* t) : type(t), next(0) {
  giopTransportImpl** pp = &implHead();
  while (*pp) pp = &((*pp)->next);
  *pp = this;
}

////////////////////////////////////////////////////////////////////////
giopTransportImpl::~giopTransportImpl() {

  giopTransportImpl** pp = &implHead();
  while (*pp && *pp != this) pp = &((*pp)->next);

  if (*pp == this) *pp = this->next;
}

////////////////////////////////////////////////////////////////////////
giopTransportImpl*
giopTransportImpl::str2Transport(const char* endpoint) {

  const char* param;
  return matchType(endpoint,param,1);
}

////////////////////////////////////////////////////////////////////////
void
giopConnection::incrRefCount() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);
  OMNIORB_ASSERT(pd_refcount >= 0);
  pd_refcount++;
}

////////////////////////////////////////////////////////////////////////
int
giopConnection::decrRefCount(CORBA::Boolean forced) {

  if (!forced) {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);
  }
  int rc = --pd_refcount;
  OMNIORB_ASSERT(rc >= 0);
  if (rc == 0)
    delete this;
  return rc;
}

////////////////////////////////////////////////////////////////////////
const char*
giopConnection::peeridentity() {
  return 0;
}


////////////////////////////////////////////////////////////////////////
void*
giopConnection::peerdetails() {
  return 0;
}


////////////////////////////////////////////////////////////////////////
_CORBA_Boolean
giopConnection::gatekeeperCheck(giopStrand* strand)
{
  CORBA::StringSeq actions;
  CORBA::ULong matchedRule;

  CORBA::Boolean acceptconnection;
  CORBA::Boolean dumprule = 0;
  const char* why;

  if (!(acceptconnection = transportRules::serverRules().
	match(peeraddress(), actions, matchedRule)) ) {

    why = (const char*) "no matching rule is found";
  }

  if (acceptconnection) {
    acceptconnection = 0;

    CORBA::ULong i;
    const char* transport = strchr(peeraddress(), ':');
    OMNIORB_ASSERT(transport);
    transport++;
    i = 0;
    for (i = 0; i < actions.length(); i++ ) {
      size_t len = strlen(actions[i]);
      if (strncmp(actions[i], transport, len) == 0) {
	acceptconnection = 1;
	break;
      }
      else if (strcmp(actions[i], "none") == 0) {
	why = (const char*) "no connection is granted by this rule: ";
	dumprule = 1;
	break;
      }
    }
    if (i == actions.length()) {
      why = (const char*) "the transport type is not in this rule: ";
      dumprule = 1;
    }
  }

  if (acceptconnection && omniInterceptorP::serverAcceptConnection) {
    omniInterceptors::serverAcceptConnection_T::info_T info(*strand);
    omniInterceptorP::visit(info);
    if (info.reject) {
      acceptconnection = 0;
      why = info.why ? info.why : (const char*)"an interceptor rejected it";
    }
  }

  if (!acceptconnection) {
    if (omniORB::trace(2)) {
      omniORB::logger log;
      log << "Connection from " << peeraddress() << " is rejected because "
	  << why;

      if (dumprule) {
	CORBA::String_var rule;
	rule = transportRules::serverRules().dumpRule(matchedRule);
	log << "\"" << (const char*) rule << "\"";
      }
      log << "\n";
    }
    return 0;
  }

  if (!gatekeeperCheckSpecific(strand))
    return 0;

  if (omniORB::trace(5)) {
    CORBA::String_var rule;
    rule = transportRules::serverRules().dumpRule(matchedRule);

    omniORB::logger log;
    log << "Accepted connection from " 
	<< peeraddress()
	<< " because of this rule: \""
	<< (const char*) rule << "\"\n";
  }
  return 1;
}


////////////////////////////////////////////////////////////////////////
_CORBA_Boolean
giopConnection::gatekeeperCheckSpecific(giopStrand* strand)
{
  // Can be overridden by derived class.
  return 1;
}


////////////////////////////////////////////////////////////////////////
const omnivector<const char*>*
giopTransportImpl::getInterfaceAddress(const char* t) {

  giopTransportImpl* impl = implHead();

  while (impl) {
    if ( strcmp(t,impl->type) == 0 )
      return impl->getInterfaceAddress();
    impl = impl->next;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
void
giopTransportImpl::initialise() {
}

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
#ifdef __HP_aCC
CORBA::String_var orbParameters::unixTransportDirectory =
                                                (const char*)"/tmp/omni-%u";
#else
CORBA::String_var orbParameters::unixTransportDirectory(
		                                (const char*)"/tmp/omni-%u");
#endif

//  Applies to the server side. Determine the directory in which
//  the unix domain socket is to be created.
//
//  Valid values = a valid pathname for a directory.
//                 %u in the string will be expanded into the user name

CORBA::UShort   orbParameters::unixTransportPermission = 0777;
//  Applies to the server side. Determine the permission mode bits
//  the unix domain socket is set to.
//
//  Valid values = unix permission mode bits in octal radix (e.g. 0755)


/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class unixTransportDirectoryHandler : public orbOptions::Handler {
public:

  unixTransportDirectoryHandler() :
    orbOptions::Handler("unixTransportDirectory",
			"unixTransportDirectory = <dir name>",
			1,
			"-ORBunixTransportDirectory <dir name>") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    orbParameters::unixTransportDirectory = value;
  }

  void dump(orbOptions::sequenceString& result) {

    CORBA::String_var kv;
    CORBA::ULong l;

    const char* format = "unixTransportDirectory = %s";

    l = strlen(format) + strlen(orbParameters::unixTransportDirectory);
    kv = CORBA::string_alloc(l);
    sprintf(kv,format,(const char*)orbParameters::unixTransportDirectory);

    l = result.length();
    result.length(l+1);
    result[l] = kv._retn();
  }
};

static unixTransportDirectoryHandler unixTransportDirectoryHandler_;

/////////////////////////////////////////////////////////////////////////////
class unixTransportPermissionHandler : public orbOptions::Handler {
public:

  unixTransportPermissionHandler() :
    orbOptions::Handler("unixTransportPermission",
			"unixTransportPermission = <mode bits in octal radix>",
			1,
			"-ORBunixTransportPermission <mode bits in octal radix>") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    unsigned int v;
    if( sscanf(value,"%o",&v) != 1 ) {
      throw orbOptions::BadParam(key(),value,
				 "Invalid value, expect mode bits in octal radix");
    }
    orbParameters::unixTransportPermission = v;
  }

  void dump(orbOptions::sequenceString& result) {

    CORBA::String_var kv;
    CORBA::ULong l;

    const char* format = "unixTransportPermission = %4o";

    l = strlen(format) + 10;
    kv = CORBA::string_alloc(l);
    sprintf(kv,format,orbParameters::unixTransportPermission);

    l = result.length();
    result.length(l+1);
    result[l] = kv._retn();
  }
};

static unixTransportPermissionHandler unixTransportPermissionHandler_;


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
class omni_giopEndpoint_initialiser : public omniInitialiser {
public:

  omni_giopEndpoint_initialiser() {
    orbOptions::singleton().registerHandler(unixTransportDirectoryHandler_);
    orbOptions::singleton().registerHandler(unixTransportPermissionHandler_);
  }

  void attach() {
    static CORBA::Boolean once = 0;
    if (once) return;

#ifdef __WIN32__
    // Initialize WinSock:

    WORD versionReq;
    WSADATA wData;
#ifdef __ETS_KERNEL__
    versionReq = MAKEWORD(1, 1);  // ETS kernel only supports 1.1
#else
    versionReq = MAKEWORD(2, 0);  // Must use 2.0 in order to use
                                  // SIO_ADDRESS_LIST_QUERY
#endif

    int rc = WSAStartup(versionReq, &wData);

    if (rc != 0) {
      // Couldn't find a usable DLL.
      OMNIORB_THROW(INITIALIZE,INITIALIZE_FailedLoadLibrary,
		    CORBA::COMPLETED_NO);
    }

    // Confirm that the returned Windows Sockets DLL supports 2.0

#ifdef __ETS_KERNEL__
    // Confirm that the returned Windows Sockets DLL supports 1.1

    if ( LOBYTE( wData.wVersion ) != 1 ||
         HIBYTE( wData.wVersion ) != 1 )  {
#else
    if ( LOBYTE( wData.wVersion ) != 2 ||
         HIBYTE( wData.wVersion ) != 0 )  {
#endif

      // Couldn't find a usable DLL
      WSACleanup();
      OMNIORB_THROW(INITIALIZE,INITIALIZE_FailedLoadLibrary,
		    CORBA::COMPLETED_NO);
    }
#endif

    giopTransportImpl* impl = implHead();
    while (impl) {
      impl->initialise();
      impl = impl->next;
    }
  }
  void detach() { }
};


static omni_giopEndpoint_initialiser initialiser;

omniInitialiser& omni_giopEndpoint_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
