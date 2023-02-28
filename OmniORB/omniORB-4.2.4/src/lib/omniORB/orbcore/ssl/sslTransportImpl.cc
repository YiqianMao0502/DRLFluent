// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslTransportImpl.cc        Created on: 29 May 2001
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
#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/giopEndpoint.h>
#include <omniORB4/omniURI.h>
#include <objectAdapter.h>
#include <SocketCollection.h>
#include <omniORB4/sslContext.h>
#include <ssl/sslConnection.h>
#include <ssl/sslAddress.h>
#include <ssl/sslEndpoint.h>
#include <ssl/sslTransportImpl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <orbOptions.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(sslAddress);
OMNI_FORCE_LINK(sslConnection);
OMNI_FORCE_LINK(sslContext);
OMNI_FORCE_LINK(sslEndpoint);
OMNI_FORCE_LINK(sslActive);

OMNI_EXPORT_LINK_FORCE_SYMBOL(omnisslTP);


OMNI_NAMESPACE_BEGIN(omni)


/////////////////////////////////////////////////////////////////////////
omni_time_t sslTransportImpl::sslAcceptTimeOut(10);


/////////////////////////////////////////////////////////////////////////
sslTransportImpl::sslTransportImpl(sslContext* ctx) : 
  giopTransportImpl("giop:ssl"), pd_ctx(ctx) {
}

/////////////////////////////////////////////////////////////////////////
sslTransportImpl::~sslTransportImpl() {
}

/////////////////////////////////////////////////////////////////////////
giopEndpoint*
sslTransportImpl::toEndpoint(const char* param)
{
  if (!omniURI::validHostPortRange(param))
    return 0;

  return (giopEndpoint*)(new sslEndpoint(param, pd_ctx));
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslTransportImpl::isValid(const char* param) {
  
  return omniURI::validHostPort(param);
}

/////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
parseAddress(const char* param, IIOP::Address& address) {

  char* host = omniURI::extractHostPort(param, address.port);
  if (!host)
    return 0;

  address.host = host;
  return 1;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
sslTransportImpl::toAddress(const char* param) {

  IIOP::Address address;
  if (parseAddress(param,address)) {
    return (giopAddress*)(new sslAddress(address,pd_ctx));
  }
  else {
    return 0;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslTransportImpl::addToIOR(const char* param, IORPublish* eps) {

  IIOP::Address address;
  if (parseAddress(param,address)) {
    // XXX, hard-wired security options to:
    //       Integrity (0x02) Confidentiality (0x04) |
    //       EstablishTrustInTarget (0x20) | EstablishTrustInClient (0x40)
    // In future, this should be expanded configurable options.
    omniIOR::add_TAG_SSL_SEC_TRANS(address, 0x66, 0x66, eps);
    return 1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
const omnivector<const char*>* 
sslTransportImpl::getInterfaceAddress() {
  return giopTransportImpl::getInterfaceAddress("giop:tcp");
}


/////////////////////////////////////////////////////////////////////////////
class sslCAFileHandler : public orbOptions::Handler {
public:

  sslCAFileHandler() : 
    orbOptions::Handler("sslCAFile",
			"sslCAFile = <certificate authority file>",
			1,
			"-ORBsslCAFile <certificate authority file>") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {    
    sslContext::certificate_authority_file = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
			    sslContext::certificate_authority_file ?
			    sslContext::certificate_authority_file : "<unset>",
			    result);
  }
};

static sslCAFileHandler sslCAFileHandler_;


/////////////////////////////////////////////////////////////////////////////
class sslCAPathHandler : public orbOptions::Handler {
public:

  sslCAPathHandler() : 
    orbOptions::Handler("sslCAPath",
			"sslCAPath = <certificate authority path>",
			1,
			"-ORBsslCAPath <certificate authority path>") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {    
    sslContext::certificate_authority_path = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
			    sslContext::certificate_authority_path ?
			    sslContext::certificate_authority_path : "<unset>",
			    result);
  }
};

static sslCAPathHandler sslCAPathHandler_;


/////////////////////////////////////////////////////////////////////////////
class sslKeyFileHandler : public orbOptions::Handler {
public:

  sslKeyFileHandler() : 
    orbOptions::Handler("sslKeyFile",
			"sslKeyFile = <key file>",
			1,
			"-ORBsslKeyFile <key file>") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {    
    sslContext::key_file = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
			    sslContext::key_file ?
			    sslContext::key_file : "<unset>",
			    result);
  }
};

static sslKeyFileHandler sslKeyFileHandler_;


/////////////////////////////////////////////////////////////////////////////
class sslKeyPasswordHandler : public orbOptions::Handler {
public:

  sslKeyPasswordHandler() : 
    orbOptions::Handler("sslKeyPassword",
			"sslKeyPassword = <key file password>",
			1,
			"-ORBsslKeyPassword <key file password>") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {    
    sslContext::key_file_password = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
			    sslContext::key_file_password ? "****" : "<unset>",
			    result);
  }
};

static sslKeyPasswordHandler sslKeyPasswordHandler_;


/////////////////////////////////////////////////////////////////////////////
class sslVerifyModeHandler : public orbOptions::Handler {
public:

  sslVerifyModeHandler() : 
    orbOptions::Handler("sslVerifyMode",
			"sslVerifyMode = <mode>",
			1,
			"-ORBsslVerifyMode < \"none\" | \"peer[,fail][,once]\" >")
  {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {    
    if (!strcmp(value, "none")) {
      sslContext::verify_mode = 0;
      return;
    }

    int mode = 0;
    CORBA::String_var val(value); // Copy
    char* valc = (char*)val;
    char* c;

    while (*valc) {
      for (c=valc; *c && *c != ','; ++c) {}

      if (*c == ',')
	*c++ = '\0';

      if (!strcmp(valc, "peer"))
	mode |= SSL_VERIFY_PEER;

      else if (!strcmp(valc, "fail"))
	mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

      else if (!strcmp(valc, "once"))
	mode |= SSL_VERIFY_CLIENT_ONCE;

      else
	throw orbOptions::BadParam(key(), valc, "Invalid verify option");
      
      valc = c;
    }
    sslContext::verify_mode = mode;
  }

  void dump(orbOptions::sequenceString& result)
  {
    char buf[20];

    buf[0] = '\0';

    if (sslContext::verify_mode & SSL_VERIFY_PEER)
      strcpy(buf, "peer");
    else
      strcpy(buf, "none");
      
    if (sslContext::verify_mode & SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
      strcat(buf, ",fail");

    if (sslContext::verify_mode & SSL_VERIFY_CLIENT_ONCE)
      strcat(buf, ",once");

    orbOptions::addKVString(key(), buf, result);
  }
};

static sslVerifyModeHandler sslVerifyModeHandler_;


/////////////////////////////////////////////////////////////////////////////
class sslAcceptTimeOutHandler : public orbOptions::Handler {
public:

  sslAcceptTimeOutHandler() : 
    orbOptions::Handler("sslAcceptTimeOut",
			"sslAcceptTimeOut = n >= 0 in msecs",
			1,
			"-ORBsslAcceptTimeOut < n >= 0 in msecs >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 "Expect n >= 0 in msecs");
    }
    sslTransportImpl::sslAcceptTimeOut.assign(v / 1000, (v % 1000) * 1000000);
  }

  void dump(orbOptions::sequenceString& result) {
    CORBA::ULong v = sslTransportImpl::sslAcceptTimeOut.s * 1000 +
      sslTransportImpl::sslAcceptTimeOut.ns / 1000000;

    orbOptions::addKVULong(key(),v,result);
  }

};

static sslAcceptTimeOutHandler sslAcceptTimeOutHandler_;



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
static sslTransportImpl* _the_sslTransportImpl = 0;

class omni_sslTransport_initialiser : public omniInitialiser {
public:

  omni_sslTransport_initialiser() {
    orbOptions::singleton().registerHandler(sslCAFileHandler_);
    orbOptions::singleton().registerHandler(sslCAPathHandler_);
    orbOptions::singleton().registerHandler(sslKeyFileHandler_);
    orbOptions::singleton().registerHandler(sslKeyPasswordHandler_);
    orbOptions::singleton().registerHandler(sslVerifyModeHandler_);
    orbOptions::singleton().registerHandler(sslAcceptTimeOutHandler_);
    omniInitialiser::install(this);
  }

  void attach() {
    if (_the_sslTransportImpl) return;

    if (!sslContext::singleton) {

      if (omniORB::trace(5)) {
	omniORB::logger log;
	log << "No SSL context object supplied. Attempt to create one "
	    << "with the default constructor.\n";
      }
      if (!(sslContext::certificate_authority_file ||
            sslContext::certificate_authority_path)) {

	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  log << "Warning: SSL CA certificate location is not set. "
	      << "SSL transport disabled.\n";
	}
	return;
      }
      
      // Create the default singleton
      sslContext::singleton =
	new sslContext(sslContext::certificate_authority_file,
		       sslContext::key_file,
		       sslContext::key_file_password);
    }
    sslContext::singleton->internal_initialise();
    _the_sslTransportImpl = new sslTransportImpl(sslContext::singleton);
  }

  void detach() { 
    if (_the_sslTransportImpl) delete _the_sslTransportImpl;
    _the_sslTransportImpl = 0;
    if (sslContext::singleton) delete sslContext::singleton;
    sslContext::singleton = 0;
  }


};

static omni_sslTransport_initialiser initialiser;

OMNI_NAMESPACE_END(omni)
