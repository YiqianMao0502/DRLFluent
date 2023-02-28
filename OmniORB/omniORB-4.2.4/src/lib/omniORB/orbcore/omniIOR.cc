// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniIOR.cc                 Created on: 19/09/2000
//                            Author    : Sai-Lai Lo
//
//    Copyright (C) 2005-2013 Apasphere Ltd
//    Copyright (C) 1999-2000 AT&T Laboratories, Cambridge
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
//

#include <omniORB4/CORBA.h>
#include <omniIdentity.h>
#include <initialiser.h>
#include <giopStreamImpl.h>
#include <omniORB4/omniInterceptors.h>
#include <interceptors.h>
#include <objectTable.h>
#include <giopRope.h>

omni_tracedmutex* omniIOR::lock = 0;

/////////////////////////////////////////////////////////////////////////////
omniIOR::omniIOR(char* repoId, IOP::TaggedProfileList* iop) : 
  pd_iopProfiles(iop),
  pd_addr_selected_profile_index(-1),
  pd_addr_mode(GIOP::KeyAddr), 
  pd_iorInfo(0),
  pd_refCount(1)
{
  pd_repositoryID = repoId;
}

/////////////////////////////////////////////////////////////////////////////
omniIOR::omniIOR(char* repoId, IOP::TaggedProfile* iop, CORBA::ULong niops,
		 CORBA::ULong selected_profile_index) :
  pd_addr_selected_profile_index((CORBA::Long)selected_profile_index),
  pd_addr_mode(GIOP::KeyAddr), 
  pd_iorInfo(0),
  pd_refCount(1)
{    
  pd_repositoryID = repoId;
  pd_iopProfiles  = new IOP::TaggedProfileList(niops,niops,iop,1);
}

/////////////////////////////////////////////////////////////////////////////
omniIOR::omniIOR(const char* repoId, const _CORBA_Octet* key, int keysize,
		 const omniIORHints& hints) :
  pd_iopProfiles(0),
  pd_addr_selected_profile_index(-1),
  pd_addr_mode(GIOP::KeyAddr), 
  pd_iorInfo(0),
  pd_refCount(1)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  pd_repositoryID = repoId;   // copied.

  IIOP::ProfileBody iiop;

  iiop.version = _OMNI_NS(giopStreamImpl)::maxVersion()->version();

  iiop.object_key.replace((CORBA::ULong)keysize, (CORBA::ULong)keysize,
			  (CORBA::Octet*)key, 0);
  // Forgive the dodgy cast to remove the const from the key. The
  // object_key sequence is only used to convert to an encoded
  // profile, without modification.

  pd_iopProfiles = new IOP::TaggedProfileList();
  {
    _OMNI_NS(omniInterceptors)::encodeIOR_T::info_T info(*this,iiop,hints,0);
    _OMNI_NS(omniInterceptorP)::visit(info);
  }

  if (strlen(iiop.address.host) == 0) {
    // Do not encode the IIOP profile if no IIOP address is set.
    if (iiop.components.length()) {
      // If there are tagged components, encode these as 
      // TAG_MULTIPLE_COMPONENTS.
      CORBA::ULong last = pd_iopProfiles->length();
      pd_iopProfiles->length(last+1);
      IIOP::encodeMultiComponentProfile(iiop.components,pd_iopProfiles[last]);
    }
    // In this case the interceptor(s) must have set up a valid profile!
    OMNIORB_ASSERT(pd_addr_selected_profile_index >= 0);
  }
  else {
    CORBA::ULong last = pd_iopProfiles->length();
    pd_iopProfiles->length(last+1);
    IIOP::encodeProfile(iiop,pd_iopProfiles[last]);
    pd_addr_selected_profile_index = last;
  }
}

/////////////////////////////////////////////////////////////////////////////
omniIOR::omniIOR(const char* repoId, 
		 const _CORBA_Unbounded_Sequence_Octet& key,
		 const IIOP::Address* addrs, CORBA::ULong naddrs,
		 GIOP::Version ver, interceptorOption callInterceptors,
                 const IOP::MultipleComponentProfile* tagged_components) :
  pd_iopProfiles(0),
  pd_addr_selected_profile_index(-1),
  pd_addr_mode(GIOP::KeyAddr), 
  pd_iorInfo(0),
  pd_refCount(1)
{
  pd_repositoryID = repoId;

  IIOP::ProfileBody iiop;

  iiop.version = ver;
  iiop.object_key.replace(key.length(),key.length(),
			  (CORBA::Octet*)key.get_buffer(),0);
  iiop.address = addrs[0]; 
  if (naddrs > 1 && (ver.major > 1 || ver.minor > 0)) {
    for (CORBA::ULong index = 1; index < naddrs; index++) {

      cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
      s.marshalRawString(addrs[index].host);
      addrs[index].port >>= s;
      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(iiop.components);
      c.tag = IOP::TAG_ALTERNATE_IIOP_ADDRESS;
      CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
      c.component_data.replace(max,len,p,1);
    }
  }

  pd_iopProfiles = new IOP::TaggedProfileList();

  IOP::MultipleComponentProfile& cs = iiop.components;

  if (tagged_components) {
    for (CORBA::ULong i = 0; i < tagged_components->length(); ++i) {
      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c = (*tagged_components)[i];
    }
  }
  if (callInterceptors != NoInterceptor) {
    omniIORHints hints(0);
    _OMNI_NS(omniInterceptors)::encodeIOR_T::info_T info(*this,iiop,hints,
				(callInterceptors == DefaultInterceptors));
    _OMNI_NS(omniInterceptorP)::visit(info);
  }

  {
    CORBA::ULong last = pd_iopProfiles->length();

    pd_iopProfiles->length(last + 1);
    IIOP::encodeProfile(iiop,pd_iopProfiles[last]);

    if (naddrs > 1 && ver.major == 1 && ver.minor == 0) {
      // In IIOP 1.0, there is no list of tagged components in the
      // IIOP profile, so we create a MultipleComponentProfile for the
      // extra addresses.
      IOP::MultipleComponentProfile components(naddrs - 1);
      components.length(naddrs - 1);
      for (CORBA::ULong index = 1; index < naddrs; index++) {
	cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
	s.marshalRawString(addrs[index].host);
	addrs[index].port >>= s;
	IOP::TaggedComponent& c = components[index-1];
	c.tag = IOP::TAG_ALTERNATE_IIOP_ADDRESS;
	CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
	c.component_data.replace(max,len,p,1);
      }
      pd_iopProfiles->length(last + 2);
      IIOP::encodeMultiComponentProfile(components,pd_iopProfiles[last+1]);
    }
    pd_addr_selected_profile_index = last;
  }
}

/////////////////////////////////////////////////////////////////////////////
omniIOR::~omniIOR()
{
  OMNIORB_ASSERT(pd_refCount <= 0);
  if (pd_iorInfo) {
    delete pd_iorInfo;
    pd_iorInfo = 0;
  }
}


/////////////////////////////////////////////////////////////////////////////
omniIOR*
omniIOR::duplicateNoLock()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniIOR::lock, 1);
  OMNIORB_ASSERT(pd_refCount > 0);
  pd_refCount++;
  return this;
}

/////////////////////////////////////////////////////////////////////////////
omniIOR*
omniIOR::duplicate()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniIOR::lock, 0);
  omni_tracedmutex_lock sync(*omniIOR::lock);
  return duplicateNoLock();
}

/////////////////////////////////////////////////////////////////////////////
void
omniIOR::releaseNoLock()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniIOR::lock, 1);
  if (--pd_refCount <= 0)
    delete this;
}

/////////////////////////////////////////////////////////////////////////////
void
omniIOR::release()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniIOR::lock, 0);
  omni_tracedmutex_lock sync(*omniIOR::lock);
  releaseNoLock();
}


/////////////////////////////////////////////////////////////////////////////
void
omniIOR::marshalIORAddressingInfo(cdrStream& s) const
{
  OMNIORB_ASSERT(pd_addr_selected_profile_index >= 0);
  
  const IOP::TaggedProfileList& profiles = pd_iopProfiles;

  pd_addr_mode >>= s;
  if (pd_addr_mode == GIOP::ProfileAddr) {
    profiles[pd_addr_selected_profile_index] >>= s;
  }
  else if (pd_addr_mode == GIOP::ReferenceAddr) {
    pd_addr_selected_profile_index >>= s;
    IOP::IOR ior;
    ior.type_id = pd_repositoryID;
    ior.profiles.replace(profiles.maximum(),
			 profiles.length(),
			 (IOP::TaggedProfile*)profiles.get_buffer(),0);
    ior >>= s;
  }
  else {
    OMNIORB_ASSERT(profiles[pd_addr_selected_profile_index].tag == IOP::TAG_INTERNET_IOP);
    IIOP::ProfileBody iiop;
    IIOP::unmarshalProfile(profiles[pd_addr_selected_profile_index],iiop);
    iiop.object_key >>= s;
  }
}

/////////////////////////////////////////////////////////////////////////////
IOP::TaggedComponent&
omniIOR::newIIOPtaggedComponent(IOP::MultipleComponentProfile& p)
{
  CORBA::ULong len = p.length() + 1;

  // Increase the buffer length in steps, to avoid frequent reallocation.
  if (p.maximum() < len)
    p.length(p.maximum() + 8);

  p.length(len);

  return p[len-1];
}


/////////////////////////////////////////////////////////////////////////////
void
omniIOR::decodeIOPprofile(const IIOP::ProfileBody& iiop) {

  OMNIORB_ASSERT(pd_iorInfo == 0);
  
  pd_iorInfo = new IORInfo();
  // Call interceptors
  {
    _OMNI_NS(omniInterceptors)::decodeIOR_T::info_T info(iiop,*this,1);
    _OMNI_NS(omniInterceptorP)::visit(info);
  }
}


/////////////////////////////////////////////////////////////////////////////
omniIOR::IORInfo*
omniIOR::getIORInfo() const
{
  if (!pd_iorInfo) {

    omni_tracedmutex_lock sync(*omniIOR::lock);

    CORBA::Boolean is_iiop = 0;

    if (!pd_iorInfo) {
      IIOP::ProfileBody iiop;
      const IOP::TaggedProfileList& profiles = pd_iopProfiles;

      if (pd_addr_selected_profile_index >= 0) {
	is_iiop = (profiles[pd_addr_selected_profile_index].tag
		   == IOP::TAG_INTERNET_IOP);

	if (is_iiop)
	  IIOP::unmarshalProfile(profiles[pd_addr_selected_profile_index],
				 iiop);

	for (CORBA::ULong index = 0; index < profiles.length(); index++) {
	  if (profiles[index].tag == IOP::TAG_MULTIPLE_COMPONENTS) {
	    IIOP::unmarshalMultiComponentProfile(profiles[index],
						 iiop.components);
	    is_iiop = 1;
	  }
	}
      }
      omniIOR* p = (omniIOR*)this;
      p->pd_iorInfo = new IORInfo();
      {
	_OMNI_NS(omniInterceptors)::decodeIOR_T::info_T info(iiop,*p,is_iiop);
	_OMNI_NS(omniInterceptorP)::visit(info);
      }
    }
  }
  return pd_iorInfo;
}

/////////////////////////////////////////////////////////////////////////////
omniIOR::IORInfo::IORInfo()
  : pd_tcs_c(0),
    pd_tcs_w(0),
    pd_flags(0)
{
  pd_version.major = 0;
  pd_version.minor = 0;
}

/////////////////////////////////////////////////////////////////////////////
omniIOR::IORInfo::~IORInfo() {

  _OMNI_NS(giopAddressList)::iterator i, last;
  i    = pd_addresses.begin();
  last = pd_addresses.end();
  for (; i != last; i++) {
    delete (*i);
    (*i) = 0;
  }
  for (CORBA::ULong index=0; index < pd_extra_info.length(); index++) {
    delete pd_extra_info[index];
    pd_extra_info[index] = 0;
  }
}

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
OMNI_NAMESPACE_BEGIN(omni)

class omni_omniIOR_initialiser : public omniInitialiser {
public:

  void attach() {
    if (!omniIOR::lock) omniIOR::lock = new omni_tracedmutex("omniIOR::lock");
  }

  void detach() {
    // omniIOR::lock is deleted by final clean-up in omniInternal.cc
  }
};

static omni_omniIOR_initialiser initialiser;

omniInitialiser& omni_omniIOR_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
