// -*- Mode: C++; -*-
//                            Package   : omniORB
// ior.cc                     Created on: 5/7/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
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
//	*** PROPRIETARY INTERFACE ***
//

#include <omniORB4/CORBA.h>
#include <omniORB4/omniInterceptors.h>
#include <omniORB4/omniURI.h>
#include <exceptiondefs.h>
#include <initialiser.h>
#include <giopBiDir.h>
#include <SocketCollection.h>
#include <orbParameters.h>
#include <stdio.h>

OMNI_USING_NAMESPACE(omni)

void
IOP::TaggedProfile::operator>>= (cdrStream &s) const {
    tag >>= s;
    profile_data >>= s;
}


void
IOP::TaggedProfile::operator<<= (cdrStream &s) {
  tag <<= s;
  profile_data <<= s;
}


void
IOP::TaggedComponent::operator>>= (cdrStream& s) const {
  tag >>= s;
  component_data >>= s;
}

void
IOP::TaggedComponent::operator<<= (cdrStream& s) {
  tag <<= s;
  component_data <<= s;
}


void
IOP::ServiceContext::operator>>= (cdrStream& s) const {
  context_id >>= s;
  context_data >>= s;
}

void
IOP::ServiceContext::operator<<= (cdrStream& s) {
  context_id <<= s;
  context_data <<= s;
}

void
IOP::IOR::operator<<= (cdrStream& s) {
  type_id = unmarshaltype_id(s);
  profiles <<= s;
}

void
IOP::IOR::operator>>= (cdrStream& s) {
  type_id >>= s;
  profiles >>= s;
}


char*
IOP::IOR::unmarshaltype_id(cdrStream& s) {
  CORBA::ULong idlen;
  CORBA::String_var id;

  idlen <<= s;

  if (!s.checkInputOverrun(1,idlen))
    OMNIORB_THROW(MARSHAL,MARSHAL_SequenceIsTooLong,
		  (CORBA::CompletionStatus)s.completion());

  switch (idlen) {

  case 0:
#ifdef NO_SLOPPY_NIL_REFERENCE
    OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		  (CORBA::CompletionStatus)s.completion());
#else
    // According to the CORBA specification 2.0 section 10.6.2:
    //   Null object references are indicated by an empty set of
    //   profiles, and by a NULL type ID (a string which contain
    //   only *** a single terminating character ***).
    //
    // Therefore the idlen should be 1.
    // Visibroker for C++ (Orbeline) 2.0 Release 1.51 gets it wrong
    // and sends out a 0 len string.
    // We quietly accept it here. Turn this off by defining
    //   NO_SLOPPY_NIL_REFERENCE
    id = CORBA::string_alloc(1);
    ((char*)id)[0] = '\0';
#endif
    break;

  case 1:
    id = CORBA::string_alloc(1);
    ((char*)id)[0] = s.unmarshalOctet();
    if (((char*)id)[0] != '\0')
      OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		    (CORBA::CompletionStatus)s.completion());
    idlen = 0;
    break;

  default:
    id = CORBA::string_alloc(idlen);
    s.get_octet_array((CORBA::Octet*)((const char*)id), idlen);
    if( ((char*)id)[idlen - 1] != '\0' )
      OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		    (CORBA::CompletionStatus)s.completion());
    break;
  }

  return id._retn();
}

void
IIOP::Address::operator>>= (cdrStream& s) const {
  s.marshalRawString(host);
  port >>= s;
}

void
IIOP::Address::operator<<= (cdrStream& s) {
  host = s.unmarshalRawString();
  port <<= s;
}

void
IIOP::encodeProfile(const IIOP::ProfileBody& body,IOP::TaggedProfile& profile)
{
  profile.tag = IOP::TAG_INTERNET_IOP;

  {
    cdrEncapsulationStream s((CORBA::ULong)0,1);
    s.marshalOctet(body.version.major);
    s.marshalOctet(body.version.minor);
    s.marshalRawString(body.address.host);
    body.address.port >>= s;
    body.object_key >>= s;

    if (body.version.minor > 0) {
      CORBA::ULong total = body.components.length();
      total >>= s;
      for (CORBA::ULong index=0; index < total; index++) {
	body.components[index] >>= s;
      }
    }

    _CORBA_Octet* p;
    CORBA::ULong max;
    CORBA::ULong len;
    s.getOctetStream(p,max,len);
    profile.profile_data.replace(max,len,p,1);
  }
}

void
IIOP::encodeMultiComponentProfile(const IOP::MultipleComponentProfile& body,
				  IOP::TaggedProfile& profile)
{
  profile.tag = IOP::TAG_MULTIPLE_COMPONENTS;

  {
    cdrEncapsulationStream s((CORBA::ULong)0,1);
    CORBA::ULong total = body.length();
    if (total) {
      total >>= s;
      for (CORBA::ULong index=0; index < total; index++) {
	body[index] >>= s;
      }
    }
    _CORBA_Octet* p;
    CORBA::ULong max;
    CORBA::ULong len;
    s.getOctetStream(p,max,len);
    profile.profile_data.replace(max,len,p,1);
  }
}


void
IIOP::unmarshalProfile(const IOP::TaggedProfile& profile,
		       IIOP::ProfileBody& body)
{
  OMNIORB_ASSERT(profile.tag == IOP::TAG_INTERNET_IOP);

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1);

  body.version.major = s.unmarshalOctet();
  body.version.minor = s.unmarshalOctet();

  if (body.version.major != 1)
    OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);

  body.address.host = s.unmarshalRawString();
  body.address.port <<= s;
  body.object_key <<= s;

  if (body.version.minor > 0) {
    CORBA::ULong total;
    total <<= s;
    if (total) {
      if (!s.checkInputOverrun(1,total))
	OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
      body.components.length(total);
      for (CORBA::ULong index=0; index<total; index++) {
	body.components[index] <<= s;
      }
    }
  }

  // Check that the profile body ends here.
  if (s.checkInputOverrun(1,1)) {
    if (orbParameters::strictIIOP) {
      omniORB::logs(10, "IIOP Profile has garbage at end");
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
    }
    else
      omniORB::logs(1, "Warning: IIOP Profile has garbage at end. Ignoring.");
  }
}

void
IIOP::unmarshalMultiComponentProfile(const IOP::TaggedProfile& profile,
				     IOP::MultipleComponentProfile& body)
{
  OMNIORB_ASSERT(profile.tag == IOP::TAG_MULTIPLE_COMPONENTS);

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1);

  CORBA::ULong newitems;
  newitems <<= s;
  if (newitems) {
    if (!s.checkInputOverrun(1,newitems))
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);

    CORBA::ULong oldlen = body.length();
    body.length(oldlen + newitems);
    for (CORBA::ULong index = oldlen; index < oldlen+newitems; index++) {
      body[index] <<= s;
    }
  }
  // Check that the profile body ends here.
  if (s.checkInputOverrun(1,1)) {
    if (orbParameters::strictIIOP) {
      omniORB::logs(10, "Multi-component profile has garbage at end");
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
    }
    else
      omniORB::logs(1, "Warning: Multi-component profile has "
		    "garbage at end. Ignoring.");
  }
}


void
IIOP::unmarshalObjectKey(const IOP::TaggedProfile& profile,
			 _CORBA_Unbounded_Sequence_Octet& key)
{
  OMNIORB_ASSERT(profile.tag == IOP::TAG_INTERNET_IOP);

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1);

  CORBA::ULong len;
  CORBA::UShort port;

  // skip version
  s.skipInput(2);

  // skip address & port
  len <<= s;
  s.skipInput(len);
  port <<= s;

  len <<= s; // Get object key length

  if (len > profile.profile_data.length())
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
                  CORBA::COMPLETED_NO);

  if (s.readOnly()) {
    CORBA::Octet* p = (CORBA::Octet*)((omni::ptr_arith_t)s.bufPtr() +
				      s.currentInputPtr());
    key.replace(len,len,p,0);
  }
  else {
    // If the cdrEncapsulationStream had to copy the profile data, we
    // have to copy it _again_ here, otherwise it will be out of scope
    // before the key is used.
    key.length(len);
    s.get_octet_array(key.NP_data(), len);
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_ORB_TYPE(const IOP::TaggedComponent& c, omniIOR& ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_ORB_TYPE);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);
  CORBA::ULong v;
  v <<= e;
  ior.getIORInfo()->orbType(v);
}


void
omniIOR::unmarshal_TAG_ALTERNATE_IIOP_ADDRESS(const IOP::TaggedComponent& c, omniIOR& ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_ALTERNATE_IIOP_ADDRESS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  IIOP::Address v;
  v.host = e.unmarshalRawString();
  v.port <<= e;
  giopAddress* address = giopAddress::fromTcpAddress(v);
  if (address == 0) return;
  ior.getIORInfo()->addresses().push_back(address);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_SSL_SEC_TRANS(const IOP::TaggedComponent& c,
				     omniIOR& ior) {

  OMNIORB_ASSERT(c.tag == IOP::TAG_SSL_SEC_TRANS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CORBA::UShort target_supports, target_requires, port;

  try {
    switch (c.component_data.length()) {
      // Remember this is an encapsulation, so the length includes the
      // first endian octet plus the necessary paddings after it
    case 8:
      {
	// This is the standard format
	target_supports <<= e;
	target_requires <<= e;
	port <<= e;
	break;
      }
    default:
      {
	omniORB::logs(1, " decode TAG_SSL_SEC_TRANS "
		      "Warning: Wrong component size. Attempt to decode "
                      "it as the Visibroker non-compilant format");
	CORBA::ULong v;
	v <<= e; target_supports = (CORBA::UShort)v;
	v <<= e; target_requires = (CORBA::UShort)v;
	port <<= e;
	break;
      }
    }
  }
  catch (...) {
    omniORB::logs(1," decode TAG_SSL_SEC_TRANS "
		  "Warning: fail to decode the component. The format neither "
                  "conforms to the standard or is Visibroker proprietary.");
    return;
  }

  giopAddressList& addresses = ior.getIORInfo()->addresses();
  // The first address in the list is the host port combo stored in the
  // IOR's address field. We have to copy the host name from there.
  const char* tcpaddr = 0;
  giopAddressList::iterator i, last;
  i    = addresses.begin();
  last = addresses.end();
  for (; i != last; i++) {
    if (omni::strMatch((*i)->type(),"giop:tcp")) {
      tcpaddr = (*i)->address();
      break;
    }
  }
  if (tcpaddr == 0) return;

  CORBA::UShort     tcp_port;
  CORBA::String_var tcp_host = omniURI::extractHostPort(tcpaddr+9, tcp_port);

  IIOP::Address ssladdr;
  ssladdr.host = tcp_host._retn();
  ssladdr.port = port;
  giopAddress* address = giopAddress::fromSslAddress(ssladdr);
  // If we do not have ssl transport linked the return value will be 0
  if (address == 0) return;
  ior.getIORInfo()->addresses().push_back(address);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void
omniIOR::unmarshal_TAG_CSI_SEC_MECH_LIST(const IOP::TaggedComponent& c,
					 omniIOR& ior) {

  OMNIORB_ASSERT(c.tag == IOP::TAG_CSI_SEC_MECH_LIST);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CORBA::Boolean stateful = e.unmarshalBoolean();

  CORBA::ULong mech_count;
  mech_count <<= e;

  if (mech_count > c.component_data.length())
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
                  CORBA::COMPLETED_NO);

  for (CORBA::ULong mech_idx = 0; mech_idx != mech_count; ++mech_idx) {
    CORBA::UShort target_requires;

    CORBA::UShort as_target_supports, as_target_requires;
    _CORBA_Unbounded_Sequence_Octet as_client_authentication_mech;
    _CORBA_Unbounded_Sequence_Octet as_target_name;

    CORBA::UShort sas_target_supports, sas_target_requires;
    CORBA::ULong sas_privilege_authorities_len;
    _CORBA_Unbounded_Sequence<_CORBA_Unbounded_Sequence_Octet> sas_supported_naming_mechanisms;
    CORBA::ULong sas_supported_identity_types;

    // CompoundSecMech structure
    target_requires <<= e;

    IOP::TaggedComponent transport_mech;
    transport_mech <<= e;

    // as_context_mech member
    as_target_supports <<= e;
    as_target_requires <<= e;
    as_client_authentication_mech <<= e;
    as_target_name <<= e;

    // sas_context_mech member
    sas_target_supports <<= e;
    sas_target_requires <<= e;
    sas_privilege_authorities_len <<= e;

    if (sas_privilege_authorities_len > transport_mech.component_data.length())
      OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
                    CORBA::COMPLETED_NO);

    for (CORBA::ULong pi = 0; pi != sas_privilege_authorities_len; ++pi) {
      CORBA::ULong syntax;
      _CORBA_Unbounded_Sequence_Octet name;
      
      syntax <<= e;
      name   <<= e;
    }
    sas_supported_naming_mechanisms <<= e;
    sas_supported_identity_types <<= e;

    if (as_target_requires  == 0 &&
	sas_target_requires == 0 &&
	transport_mech.tag  == IOP::TAG_TLS_SEC_TRANS) {

      // No higher-level requirements and a TLS transport tag -- we
      // can support this component.
      CORBA::UShort tls_target_supports, tls_target_requires;
      CORBA::ULong addresses_len;

      cdrEncapsulationStream tls_e(transport_mech.component_data.get_buffer(),
				   transport_mech.component_data.length(),1);
      
      tls_target_supports <<= tls_e;
      tls_target_requires <<= tls_e;
      addresses_len <<= tls_e;

      if (addresses_len > transport_mech.component_data.length())
        OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
                      CORBA::COMPLETED_NO);
      
      for (CORBA::ULong ai = 0; ai != addresses_len; ++ai) {
	IIOP::Address ssladdr;

	ssladdr.host = tls_e.unmarshalRawString();
	ssladdr.port <<= tls_e;

	giopAddress* address = giopAddress::fromSslAddress(ssladdr);
	// If we do not have ssl transport linked the return value will be 0

	if (address == 0) return;
	ior.getIORInfo()->addresses().push_back(address);
      }
    }
  }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_OMNIORB_BIDIR(const IOP::TaggedComponent& c,
				     omniIOR& ior) {
  
  OMNIORB_ASSERT(c.tag == IOP::TAG_OMNIORB_BIDIR);
  OMNIORB_ASSERT(ior.pd_iorInfo);

  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  char* sendfrom = e.unmarshalRawString();

  BiDirInfo* info = new BiDirInfo(sendfrom);
  
  omniIOR::IORExtraInfoList& infolist = ior.pd_iorInfo->extraInfo();
  CORBA::ULong index = infolist.length();
  infolist.length(index+1);
  infolist[index] = (omniIOR::IORExtraInfo*)info;
}

void
omniIOR::add_TAG_OMNIORB_BIDIR(const char* sendfrom,omniIOR& ior) {

  cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
  s.marshalRawString(sendfrom);

  IOP::MultipleComponentProfile body;
  body.length(1);
  body[0].tag = IOP::TAG_OMNIORB_BIDIR;
  CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
  body[0].component_data.replace(max,len,p,1);

  CORBA::ULong index = ior.pd_iopProfiles->length();
  ior.pd_iopProfiles->length(index+1);
  IIOP::encodeMultiComponentProfile(body,ior.pd_iopProfiles[index]);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_OMNIORB_UNIX_TRANS(const IOP::TaggedComponent& c,
					  omniIOR& ior) {
  
  OMNIORB_ASSERT(c.tag == IOP::TAG_OMNIORB_UNIX_TRANS);
  OMNIORB_ASSERT(ior.pd_iorInfo);

  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CORBA::String_var host;
  host = e.unmarshalRawString();
  CORBA::String_var filename;
  filename = e.unmarshalRawString();

  // Check if we are on the same host and hence can use unix socket.
  char self[OMNIORB_HOSTNAME_MAX];
  if (gethostname(&self[0],OMNIORB_HOSTNAME_MAX) == RC_SOCKET_ERROR) {
    self[0] = '\0';
    omniORB::logs(1, "Cannot get the name of this host.");
  }
  if (strcmp(self,host) != 0) return;

  const char* format = "giop:unix:%s";

  CORBA::ULong len = strlen(filename);
  if (len == 0) return;
  len += strlen(format);
  CORBA::String_var addrstr(CORBA::string_alloc(len));
  sprintf(addrstr,format,(const char*)filename);
  
  giopAddress* address = giopAddress::str2Address(addrstr);
  // If we do not have unix transport linked the return value will be 0
  if (address == 0) return;
  ior.getIORInfo()->addresses().push_back(address);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_OMNIORB_PERSISTENT_ID(const IOP::TaggedComponent& c,
					     omniIOR& ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_OMNIORB_PERSISTENT_ID);

  CORBA::ULong len = orbParameters::persistentId.length();

  if (len && len == c.component_data.length()) {

    const CORBA::Octet* a = c.component_data.get_buffer();
    const CORBA::Octet* b = orbParameters::persistentId.get_buffer();
    for (CORBA::ULong i=0; i < len; i++) {
      if (*a++ != *b++)
	return;
    }
    
    omniIOR::IORExtraInfoList& extra = ior.pd_iorInfo->extraInfo();
    CORBA::ULong index = extra.length();
    extra.length(index+1);
    extra[index] = new IORExtraInfo(IOP::TAG_OMNIORB_PERSISTENT_ID);
  }
}


static void
logPersistentIdentifier()
{
  omniORB::logger l;
  l << "Persistent server identifier: ";

  int c, n;
  for (CORBA::ULong i=0; i < orbParameters::persistentId.length(); i++) {
    c = orbParameters::persistentId[i];
    n = (c & 0xf0) >> 4;
    if (n >= 10)
      l << (char)('a' + n - 10);
    else
      l << (char)('0' + n);

    n = c & 0xf;
    if (n >= 10)
      l << (char)('a' + n - 10);
    else
      l << (char)('0' + n);
  }
  l << "\n";
}  


void
omniORB::
setPersistentServerIdentifier(const _CORBA_Unbounded_Sequence_Octet& id)
{
  if (orbParameters::persistentId.length()) {
    // Once set, it must not be changed
    OMNIORB_THROW(INITIALIZE, INITIALIZE_FailedLoadLibrary,
		  CORBA::COMPLETED_NO);
  }

  orbParameters::persistentId = id;

  if (omniORB::trace(10)) {
    logPersistentIdentifier();
  }
}




OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// For the TAGs that the ORB will look at, add a handler to the following
// table.
//
static struct {
  IOP::ComponentId id;
  void (*fn)(const IOP::TaggedComponent&,omniIOR&);
} componentUnmarshalHandlers[] = {
  // This table must be arranged in ascending order of IOP::ComponentId

  { IOP::TAG_ORB_TYPE,
    omniIOR::unmarshal_TAG_ORB_TYPE },

  { IOP::TAG_CODE_SETS,
    omniIOR::unmarshal_TAG_CODE_SETS },

  { IOP::TAG_POLICIES, 0 },

  { IOP::TAG_ALTERNATE_IIOP_ADDRESS,
    omniIOR::unmarshal_TAG_ALTERNATE_IIOP_ADDRESS },

  { IOP::TAG_COMPLETE_OBJECT_KEY, 0 },
  { IOP::TAG_ENDPOINT_ID_POSITION, 0 },
  { IOP::TAG_LOCATION_POLICY, 0 },
  { IOP::TAG_ASSOCIATION_OPTIONS, 0 },
  { IOP::TAG_SEC_NAME, 0 },
  { IOP::TAG_SPKM_1_SEC_MECH, 0 },
  { IOP::TAG_SPKM_2_SEC_MECH, 0 },
  { IOP::TAG_KERBEROSV5_SEC_MECH, 0 },
  { IOP::TAG_CSI_ECMA_SECRET_SEC_MECH, 0 },
  { IOP::TAG_CSI_ECMA_HYBRID_SEC_MECH, 0 },

  { IOP::TAG_SSL_SEC_TRANS,
    omniIOR::unmarshal_TAG_SSL_SEC_TRANS },

  { IOP::TAG_CSI_ECMA_PUBLIC_SEC_MECH, 0 },
  { IOP::TAG_GENERIC_SEC_MECH, 0 },
  { IOP::TAG_FIREWALL_TRANS, 0 },
  { IOP::TAG_SCCP_CONTACT_INFO, 0 },
  { IOP::TAG_JAVA_CODEBASE, 0 },

  { IOP::TAG_CSI_SEC_MECH_LIST,
    omniIOR::unmarshal_TAG_CSI_SEC_MECH_LIST },

  { IOP::TAG_DCE_STRING_BINDING, 0 },
  { IOP::TAG_DCE_BINDING_NAME, 0 },
  { IOP::TAG_DCE_NO_PIPES, 0 },
  { IOP::TAG_DCE_SEC_MECH, 0 },
  { IOP::TAG_INET_SEC_TRANS, 0 },

  { IOP::TAG_PRIMARY, 0 },
  { IOP::TAG_HEARTBEAT_ENABLED, 0 },

  { IOP::TAG_OMNIORB_BIDIR,
    omniIOR::unmarshal_TAG_OMNIORB_BIDIR },

  { IOP::TAG_OMNIORB_UNIX_TRANS,
    omniIOR::unmarshal_TAG_OMNIORB_UNIX_TRANS },

  { IOP::TAG_OMNIORB_PERSISTENT_ID,
    omniIOR::unmarshal_TAG_OMNIORB_PERSISTENT_ID },

  { 0xffffffff, 0 }
};

static int tablesize = 0;

OMNI_NAMESPACE_END(omni)

/////////////////////////////////////////////////////////////////////////////
//            EndPoints and default IOR contents                           //
/////////////////////////////////////////////////////////////////////////////

OMNI_NAMESPACE_BEGIN(omni)

typedef _CORBA_Unbounded_Sequence_Octet          OctetSeq;
typedef _CORBA_Unbounded_Sequence<OctetSeq>      OctetSeqSeq;
typedef _CORBA_Unbounded_Sequence<IIOP::Address> AddressSeq;

class IORPublish {
public:
  IIOP::Address  address;
  OctetSeqSeq    alternative_addrs;
  OctetSeqSeq    ssl_addrs;
  OctetSeqSeq    unix_addrs;
  OctetSeq       csi_component;
  AddressSeq     tls_addrs;
  CORBA::UShort  tls_supports;
  CORBA::UShort  tls_requires;
  CORBA::Boolean csi_enabled;

  inline IORPublish() : tls_supports(0), tls_requires(0), csi_enabled(0)
  {
    address.port = 0;
  }
};

static IORPublish my_eps;

static OctetSeq my_code_set;
static OctetSeq my_orb_type;

_CORBA_Unbounded_Sequence_Octet orbParameters::persistentId;

OMNI_NAMESPACE_END(omni)


IORPublish*
omniPolicy::EndPointPublishPolicy::getEPs()
{
  if (!pd_eps) {
    omniORB::logs(20, "Override published endpoints:");

    pd_eps = new IORPublish;

    for (CORBA::ULong idx=0; idx != pd_value.length(); ++idx) {
      const char* ep = pd_value[idx];

      if (omniORB::trace(20)) {
        omniORB::logger log;
        log << "  override endpoint " << idx << ": '" << ep << "'\n";
      }
      giopEndpoint::addToIOR(pd_value[idx], pd_eps);
    }
  }
  return pd_eps;
}

omniPolicy::EndPointPublishPolicy::~EndPointPublishPolicy()
{
  if (pd_eps)
    delete pd_eps;
}


/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_IIOP_ADDRESS(const IIOP::Address& address, IORPublish* eps)
{
  if (!eps)
    eps = &my_eps;

  if (eps->address.port == 0) {
    eps->address = address;
  }
  else {
    add_TAG_ALTERNATE_IIOP_ADDRESS(address, eps);
  }
}

/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_TAG_CODE_SETS(const CONV_FRAME::CodeSetComponentInfo& info)
{
  cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
  info >>= s;

  CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
  my_code_set.replace(max,len,p,1);
}

/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_TAG_ALTERNATE_IIOP_ADDRESS(const IIOP::Address& address,
                                        IORPublish*          eps)
{
  if (!eps)
    eps = &my_eps;

  cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
  s.marshalRawString(address.host);
  address.port >>= s;

  CORBA::ULong index = eps->alternative_addrs.length();
  eps->alternative_addrs.length(index+1);

  s.setOctetSeq(eps->alternative_addrs[index]);
}

/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_TAG_SSL_SEC_TRANS(const IIOP::Address& address,
			       CORBA::UShort        supports,
                               CORBA::UShort        requires,
                               IORPublish*          eps)
{
  if (!eps)
    eps = &my_eps;

  {
    // Add to list of TLS addresses
    CORBA::ULong length = eps->tls_addrs.length();
    eps->tls_addrs.length(length+1);

    eps->tls_addrs[length] = address;
    eps->tls_supports      = supports;
    eps->tls_requires      = requires;
  }

  if (strlen(eps->address.host) == 0) {
    eps->address.host = address.host;
  }
  else if (strcmp(eps->address.host, address.host) != 0) {
    // The address does not match the IIOP address. Cannot add as an
    // SSL address. Enable the minimal CSI support.
    eps->csi_enabled = 1;
    return;
  }

  cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
  supports >>= s;
  requires >>= s;
  address.port >>= s;

  CORBA::ULong index = eps->ssl_addrs.length();
  eps->ssl_addrs.length(index+1);

  s.setOctetSeq(eps->ssl_addrs[index]);
}

/////////////////////////////////////////////////////////////////////////////
static
void add_TAG_CSI_SEC_MECH_LIST(const _CORBA_Unbounded_Sequence<IIOP::Address>& addrs,
			       CORBA::UShort supports,
                               CORBA::UShort requires,
                               IORPublish*   eps)
{
  if (!eps)
    eps = &my_eps;

  // Anyone would think this structure was designed by committee...

  if (omniORB::trace(10)) {
    omniORB::logger log;
    log << "Create CSIv2 security mechanism list for " << addrs.length()
	<< " addresses.\n";
  }

  cdrEncapsulationStream stream(CORBA::ULong(0),CORBA::Boolean(1));

  CORBA::UShort zeroUShort = 0;
  CORBA::ULong  zeroULong  = 0;

  // struct CompoundSecMechList {
  //   boolean stateful;
  //   CompoundSecMechanisms mechanism_list;
  // };
  stream.marshalBoolean(0);
  CORBA::ULong mechanism_count = 1;
  mechanism_count >>= stream;

  // struct CompoundSecMech {
  //   AssociationOptions taget_requires;
  //   IOP::TaggedComponent transport_mech;
  //   AS_ContextSec as_context_mech;
  //   SAS_ContextSec sas_context_mech;
  // };

  requires >>= stream;

  IOP::TaggedComponent transport_mech;
  transport_mech.tag = IOP::TAG_TLS_SEC_TRANS;

  cdrEncapsulationStream mech_stream(CORBA::ULong(0),CORBA::Boolean(1));

  supports >>= mech_stream;
  requires >>= mech_stream;
  addrs    >>= mech_stream;

  {
    CORBA::Octet* p;
    CORBA::ULong max, len;
    mech_stream.getOctetStream(p,max,len);
    transport_mech.component_data.replace(max, len, p, 1);
  }
  transport_mech >>= stream;

  // struct AS_ContextSec {
  //   AssociationOptions target_supports;
  //   AssociationOptions target_requires;
  //   CSI::OID client_authentication_mech;
  //   CSI::GSS_NT_ExportedName target_name;
  // };
  zeroUShort >>= stream;
  zeroUShort >>= stream;
  zeroULong  >>= stream;
  zeroULong  >>= stream;

  // struct SAS_ContextSec {
  //   AssociationOptions target_supports;
  //   AssociationOptions target_requires;
  //   ServiceConfigurationList privilege_authorities;
  //   CSI::OIDList supported_naming_mechanisms;
  //   CSI::IdentityTokenType supported_identity_types;
  // };
  zeroUShort >>= stream;
  zeroUShort >>= stream;
  zeroULong  >>= stream;
  zeroULong  >>= stream;
  zeroULong  >>= stream;

  stream.setOctetSeq(eps->csi_component);
}


/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_TAG_OMNIORB_UNIX_TRANS(const char* filename,
                                    IORPublish* eps)
{
  if (!eps)
    eps = &my_eps;

  OMNIORB_ASSERT(filename && strlen(filename) != 0);

  char self[OMNIORB_HOSTNAME_MAX];
  if (gethostname(&self[0],OMNIORB_HOSTNAME_MAX) == RC_SOCKET_ERROR) {
    omniORB::logs(1, "Cannot get the name of this host.");
    self[0] = '\0';
  }

  if (strlen(eps->address.host) == 0) {
    eps->address.host = (const char*) self;
  }

  cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));

  s.marshalRawString(self);
  s.marshalRawString(filename);

  CORBA::ULong index = eps->unix_addrs.length();
  eps->unix_addrs.length(index+1);
  
  s.setOctetSeq(eps->unix_addrs[index]);
}


OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
insertSupportedComponents(omniInterceptors::encodeIOR_T::info_T& info)
{
  IORPublish* eps = &my_eps;

  const GIOP::Version&           v        = info.iiop.version;
  IOP::MultipleComponentProfile& cs       = info.iiop.components;
  const CORBA::PolicyList*       policies = info.hints.policies;

  // Is there an endpoint publishing override policy?
  if (policies) {
    for (CORBA::ULong idx=0; idx != policies->length(); ++idx) {
      CORBA::Policy_ptr policy = (*policies)[idx];

      if (policy->policy_type() == omniPolicy::ENDPOINT_PUBLISH_POLICY_TYPE) {
        omniPolicy::EndPointPublishPolicy_var epp 
          = omniPolicy::EndPointPublishPolicy::_narrow(policy);
        
        if (CORBA::is_nil(epp))
          OMNIORB_THROW(INV_POLICY, INV_POLICY_InvalidPolicyType,
                        CORBA::COMPLETED_NO);

        eps = epp->getEPs();
        break;
      }
    }
  }

  if (strlen(info.iiop.address.host) == 0) {
    if (strlen(eps->address.host) == 0) {
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
    }
    info.iiop.address = eps->address;
  }

  if ((v.major > 1 || v.minor >= 1) && my_orb_type.length()) {
    // 1.1 or later, Insert ORB TYPE
    IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
    c.tag = IOP::TAG_ORB_TYPE;
    CORBA::ULong max, len;
    max = my_orb_type.maximum();
    len = my_orb_type.length();
    c.component_data.replace(max,len,my_orb_type.get_buffer(),0);
  }

  if ((v.major > 1 || v.minor >= 2) && my_code_set.length()) {
    // 1.2 or later, Insert CODE SET
    IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
    c.tag = IOP::TAG_CODE_SETS;
    CORBA::ULong max, len;
    max = my_code_set.maximum();
    len = my_code_set.length();
    c.component_data.replace(max,len,my_code_set.get_buffer(),0);
  }

  if (v.major > 1 || v.minor >= 2) {
    // 1.2 or later, Insert ALTERNATIVE IIOP ADDRESS
    for (CORBA::ULong index = 0;
	 index < eps->alternative_addrs.length(); index++) {

      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c.tag = IOP::TAG_ALTERNATE_IIOP_ADDRESS;
      CORBA::ULong max, len;
      max = eps->alternative_addrs[index].maximum();
      len = eps->alternative_addrs[index].length();
      c.component_data.replace(max,len,
			       eps->alternative_addrs[index].get_buffer(),0);
    }
  }

  if (v.major > 1 || v.minor >= 1) {
    // 1.1 or later, Insert SSL_SEC_TRANS
    for (CORBA::ULong index = 0;
	 index < eps->ssl_addrs.length(); index++) {

      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c.tag = IOP::TAG_SSL_SEC_TRANS;
      CORBA::ULong max, len;
      max = eps->ssl_addrs[index].maximum();
      len = eps->ssl_addrs[index].length();
      c.component_data.replace(max,len,
			       eps->ssl_addrs[index].get_buffer(),0);
    }
  }

  if (v.major > 1 || v.minor >= 1) {
    // 1.1 or later, Insert CSI_SEC_MECH_LIST
    if (eps->csi_enabled) {
      if (!eps->csi_component.length()) {
	add_TAG_CSI_SEC_MECH_LIST(eps->tls_addrs,
				  eps->tls_supports, eps->tls_requires, eps);
      }
      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c.tag = IOP::TAG_CSI_SEC_MECH_LIST;

      CORBA::ULong max, len;
      max = eps->csi_component.maximum();
      len = eps->csi_component.length();
      c.component_data.replace(max,len,
			       eps->csi_component.get_buffer(),0);
    }
  }

  if (v.major > 1 || v.minor >= 2) {
    // 1.2 or later, Insert omniORB unix transport
    for (CORBA::ULong index = 0;
	 index < eps->unix_addrs.length(); index++) {

      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c.tag = IOP::TAG_OMNIORB_UNIX_TRANS;
      CORBA::ULong max, len;
      max = eps->unix_addrs[index].maximum();
      len = eps->unix_addrs[index].length();
      c.component_data.replace(max,len,
			       eps->unix_addrs[index].get_buffer(),0);
    }
  }

  if (v.major > 1 || v.minor >= 1) {
    // 1.1 or later, insert omniORB persistent id
    if (orbParameters::persistentId.length()) {
      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c.tag = IOP::TAG_OMNIORB_PERSISTENT_ID;
      c.component_data.replace(orbParameters::persistentId.maximum(),
			       orbParameters::persistentId.length(),
			       orbParameters::persistentId.get_buffer(), 0);
    }
  }

  return (info.default_only ? 0 : 1);
}

/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
extractSupportedComponents(omniInterceptors::decodeIOR_T::info_T& info)
{
  if (!info.has_iiop_body) return 1;

  omniIOR::IORInfo& iorInfo = *(info.ior.getIORInfo());

  iorInfo.version(info.iiop.version);

  giopAddress* address = giopAddress::fromTcpAddress(info.iiop.address);
  if (address)
    iorInfo.addresses().push_back(address);

  if (!tablesize) {
    while (componentUnmarshalHandlers[tablesize].id != 0xffffffff) tablesize++;
  }

  const IOP::MultipleComponentProfile& components = info.iiop.components;

  CORBA::ULong total = components.length();
  for (CORBA::ULong index = 0; index < total; index++) {

    int top = tablesize;
    int bottom = 0;

    do {
      int i = (top + bottom) >> 1;
      IOP::ComponentId id = componentUnmarshalHandlers[i].id;
      if (id == components[index].tag) {
	if (componentUnmarshalHandlers[i].fn) {
	  componentUnmarshalHandlers[i].fn(components[index],info.ior);
	}
	break;
      }
      else if (id > components[index].tag) {
	top = i;
      }
      else {
	bottom = i + 1;
      }
    } while (top != bottom);
  }
  return 1;
}


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_ior_initialiser : public omniInitialiser {
public:
  omni_ior_initialiser() {}

  void attach() {
    my_eps.address.port = 0;

    omniORB::getInterceptors()->encodeIOR.add(insertSupportedComponents);
    omniORB::getInterceptors()->decodeIOR.add(extractSupportedComponents);

    cdrEncapsulationStream s(8,1);
    omniORB_TAG_ORB_TYPE >>= s;
    _CORBA_Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
    my_orb_type.replace(max,len,p,1);

    if (omniORB::trace(10) && orbParameters::persistentId.length()) {
      logPersistentIdentifier();
    }

  }

  void detach() {
    omniORB::getInterceptors()->encodeIOR.remove(insertSupportedComponents);
    omniORB::getInterceptors()->decodeIOR.remove(extractSupportedComponents);

    _CORBA_Unbounded_Sequence_Octet::freebuf(my_orb_type.get_buffer(1));

    my_eps.alternative_addrs.length(0);
    my_eps.ssl_addrs.length(0);
    my_eps.unix_addrs.length(0);
    my_eps.csi_component.length(0);
    my_eps.tls_addrs.length(0);
    my_eps.csi_enabled = 0;
  }

};

static omni_ior_initialiser initialiser;

omniInitialiser& omni_ior_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
