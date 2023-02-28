// -*- Mode: C++; -*-
//                            Package   : omniORB
// codeSets.cc                Created on: 18/10/2000
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//    Base implementation of code set conversion functions
//

#include <omniORB4/CORBA.h>
#include <omniORB4/omniInterceptors.h>
#include <omniORB4/linkHacks.h>
#include <codeSetUtil.h>
#include <initialiser.h>
#include <giopStreamImpl.h>
#include <giopRope.h>
#include <GIOP_C.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <stdio.h>


//
// Make sure built-in code set modules are always linked

OMNI_FORCE_LINK(CS_8859_1);
OMNI_FORCE_LINK(CS_UTF_8);
OMNI_FORCE_LINK(CS_UTF_16);


OMNI_NAMESPACE_BEGIN(omni)

//
// Configuration options
//

omniCodeSet::NCS_C* orbParameters::nativeCharCodeSet = 0;
//  set the native code set for char and string
//

omniCodeSet::NCS_W* orbParameters::nativeWCharCodeSet = 0;
//  set the native code set for wchar and wstring
//

omniCodeSet::TCS_C* orbParameters::anyCharCodeSet = 0;
//  set the preferred code set for char data inside anys
//

omniCodeSet::TCS_W* orbParameters::anyWCharCodeSet = 0;
//  set the preferred code set for wchar data inside anys
//

omniCodeSet::TCS_C* orbParameters::defaultCharCodeSet = 0;
//  set the code set for char data sent to servers that have not specified one
//

omniCodeSet::TCS_W* orbParameters::defaultWCharCodeSet = 0;
//  set the code set for wchar data sent to servers that have not specified one
//


//
// Linked lists of code set objects
//

static omniCodeSet::NCS_C*& ncs_c_head() {
  static omniCodeSet::NCS_C* head_ = 0;
  return head_;
}
static omniCodeSet::NCS_W*& ncs_w_head() {
  static omniCodeSet::NCS_W* head_ = 0;
  return head_;
}
static omniCodeSet::TCS_C*& tcs_c_head() {
  static omniCodeSet::TCS_C* head_ = 0;
  return head_;
}
static omniCodeSet::TCS_W*& tcs_w_head() {
  static omniCodeSet::TCS_W* head_ = 0;
  return head_;
}

static inline _CORBA_Boolean
versionMatch(GIOP::Version v1, GIOP::Version v2)
{
  return (v1.major == v2.major && v1.minor == v2.minor);
}

//
// Code set id constants
//

// Ones tagged with // ?? are not in the DCE code set registry, but
// are inferred from the id allocation pattern.

const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_1  = 0x00010001;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_2  = 0x00010002;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_3  = 0x00010003;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_4  = 0x00010004;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_5  = 0x00010005;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_6  = 0x00010006;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_7  = 0x00010007;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_8  = 0x00010008;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_9  = 0x00010009;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_10 = 0x0001000a;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_11 = 0x0001000b; // ??
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_13 = 0x0001000d; // ??
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_14 = 0x0001000e; // ??
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_15 = 0x0001000f; // ??
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_16 = 0x00010010; // ??
const CONV_FRAME::CodeSetId omniCodeSet::ID_UCS_2   = 0x00010102;
const CONV_FRAME::CodeSetId omniCodeSet::ID_UCS_4   = 0x00010106;
const CONV_FRAME::CodeSetId omniCodeSet::ID_UTF_8   = 0x05010001;
const CONV_FRAME::CodeSetId omniCodeSet::ID_UTF_16  = 0x00010109;
const CONV_FRAME::CodeSetId omniCodeSet::ID_CP1250  = 0x100204e2;
const CONV_FRAME::CodeSetId omniCodeSet::ID_CP1251  = 0x100204e3;
const CONV_FRAME::CodeSetId omniCodeSet::ID_CP1252  = 0x100204e4;
const CONV_FRAME::CodeSetId omniCodeSet::ID_CP1253  = 0x100204e5;
const CONV_FRAME::CodeSetId omniCodeSet::ID_CP1254  = 0x100204e6;
const CONV_FRAME::CodeSetId omniCodeSet::ID_CP1255  = 0x100204e7;
const CONV_FRAME::CodeSetId omniCodeSet::ID_CP1256  = 0x100204e8;
const CONV_FRAME::CodeSetId omniCodeSet::ID_CP1257  = 0x100204e9;
const CONV_FRAME::CodeSetId omniCodeSet::ID_CP1258  = 0x100204ea; // ??
const CONV_FRAME::CodeSetId omniCodeSet::ID_037     = 0x10020025;
const CONV_FRAME::CodeSetId omniCodeSet::ID_500     = 0x100201f8;
const CONV_FRAME::CodeSetId omniCodeSet::ID_EDF_4   = 0x10040366;


//
// GIOP version constants
//

const GIOP::Version omniCodeSetUtil::GIOP10 = { 1,0 };
const GIOP::Version omniCodeSetUtil::GIOP11 = { 1,1 };
const GIOP::Version omniCodeSetUtil::GIOP12 = { 1,2 };

omniCodeSet::NCS_C*
omniCodeSet::getNCS_C(CONV_FRAME::CodeSetId id)
{
  omniCodeSet::Base* cs;
  for (cs = ncs_c_head(); cs; cs = cs->pd_next) {
    if (cs->id() == id)
      return (NCS_C*)cs;
  }
  return 0;
}

omniCodeSet::NCS_W*
omniCodeSet::getNCS_W(CONV_FRAME::CodeSetId id)
{
  omniCodeSet::Base* cs;
  for (cs = ncs_w_head(); cs; cs = cs->pd_next) {
    if (cs->id() == id)
      return (NCS_W*)cs;
  }
  return 0;
}

omniCodeSet::TCS_C*
omniCodeSet::getTCS_C(CONV_FRAME::CodeSetId id, GIOP::Version v)
{
  omniCodeSet::Base*  cs;
  omniCodeSet::TCS_C* tcs;
  for (cs = tcs_c_head(); cs; cs = cs->pd_next) {
    tcs = (omniCodeSet::TCS_C*)cs;
    if (tcs->id() == id && versionMatch(tcs->giopVersion(), v))
      return tcs;
  }
  return 0;
}

omniCodeSet::TCS_W*
omniCodeSet::getTCS_W(CONV_FRAME::CodeSetId id, GIOP::Version v)
{
  omniCodeSet::Base*  cs;
  omniCodeSet::TCS_W* tcs;
  for (cs = tcs_w_head(); cs; cs = cs->pd_next) {
    tcs = (omniCodeSet::TCS_W*)cs;
    if (tcs->id() == id && versionMatch(tcs->giopVersion(), v))
      return tcs;
  }
  return 0;
}

omniCodeSet::NCS_C*
omniCodeSet::getNCS_C(const char* name)
{
  omniCodeSet::Base* cs;
  for (cs = ncs_c_head(); cs; cs = cs->pd_next) {
    if (omni::strMatch(cs->name(), name))
      return (NCS_C*)cs;
  }
  return 0;
}

omniCodeSet::NCS_W*
omniCodeSet::getNCS_W(const char* name)
{
  omniCodeSet::Base* cs;
  for (cs = ncs_w_head(); cs; cs = cs->pd_next) {
    if (omni::strMatch(cs->name(), name))
      return (NCS_W*)cs;
  }
  return 0;
}

omniCodeSet::TCS_C*
omniCodeSet::getTCS_C(const char* name, GIOP::Version v)
{
  omniCodeSet::Base*  cs;
  omniCodeSet::TCS_C* tcs;
  for (cs = tcs_c_head(); cs; cs = cs->pd_next) {
    tcs = (omniCodeSet::TCS_C*)cs;
    if (omni::strMatch(cs->name(), name) &&
	versionMatch(tcs->giopVersion(), v))
      return tcs;
  }
  return 0;
}

omniCodeSet::TCS_W*
omniCodeSet::getTCS_W(const char* name, GIOP::Version v)
{
  omniCodeSet::Base*  cs;
  omniCodeSet::TCS_W* tcs;
  for (cs = tcs_w_head(); cs; cs = cs->pd_next) {
    tcs = (omniCodeSet::TCS_W*)cs;
    if (omni::strMatch(cs->name(), name) &&
	versionMatch(tcs->giopVersion(), v))
      return tcs;
  }
  return 0;
}



void
omniCodeSet::registerNCS_C(omniCodeSet::NCS_C* cs)
{
  cs->pd_next  = ncs_c_head();
  ncs_c_head() = cs;
}

void
omniCodeSet::registerNCS_W(omniCodeSet::NCS_W* cs)
{
  cs->pd_next  = ncs_w_head();
  ncs_w_head() = cs;
}

void
omniCodeSet::registerTCS_C(omniCodeSet::TCS_C* cs)
{
  cs->pd_next  = tcs_c_head();
  tcs_c_head() = cs;
}

void
omniCodeSet::registerTCS_W(omniCodeSet::TCS_W* cs)
{
  cs->pd_next  = tcs_w_head();
  tcs_w_head() = cs;
}

//
// Code set base constructors
//

omniCodeSet::NCS_C::NCS_C(CONV_FRAME::CodeSetId id,
			  const char* name, Kind kind)

  : Base(id, name, kind)
{
}

omniCodeSet::TCS_C::TCS_C(CONV_FRAME::CodeSetId id,
			  const char* name, Kind kind,
			  GIOP::Version giopVersion)

  : Base(id, name, kind), pd_giopVersion(giopVersion)
{
}

omniCodeSet::NCS_W::NCS_W(CONV_FRAME::CodeSetId id,
			  const char* name, Kind kind)

  : Base(id, name, kind)
{
}

omniCodeSet::TCS_W::TCS_W(CONV_FRAME::CodeSetId id,
			  const char* name, Kind kind,
			  GIOP::Version giopVersion)

  : Base(id, name, kind), pd_giopVersion(giopVersion)
{
}

//
// Default implementations of fast marshalling functions just return false
//

_CORBA_Boolean
omniCodeSet::TCS_C::fastMarshalChar(cdrStream&    stream,
				    NCS_C*        ncs,
				    _CORBA_Char   c)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C::fastMarshalString(cdrStream&    stream,
				      NCS_C*        ncs,
				      _CORBA_ULong  bound,
				      _CORBA_ULong  len,
				      const char*   s)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C::fastUnmarshalChar(cdrStream&    stream,
				      NCS_C*        ncs,
				      _CORBA_Char&  c)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C::fastUnmarshalString(cdrStream&    stream,
					NCS_C*        ncs,
					_CORBA_ULong  bound,
					_CORBA_ULong& length,
					char*&        s)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_W::fastMarshalWChar(cdrStream&    stream,
				     NCS_W*        ncs,
				     _CORBA_WChar  c)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_W::fastMarshalWString(cdrStream&          stream,
				       NCS_W*              ncs,
				       _CORBA_ULong        bound,
				       _CORBA_ULong        len,
				       const _CORBA_WChar* s)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_W::fastUnmarshalWChar(cdrStream&    stream,
				       NCS_W*        ncs,
				       _CORBA_WChar& c)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_W::fastUnmarshalWString(cdrStream&     stream,
					 NCS_W*         ncs,
					 _CORBA_ULong   bound,
					 _CORBA_ULong&  length,
					 _CORBA_WChar*& s)
{
  return 0;
}


void
CONV_FRAME::
CodeSetComponent::operator>>=(cdrStream& s) const
{
  native_code_set >>= s;
  conversion_code_sets >>= s;
}

void
CONV_FRAME::
CodeSetComponent::operator<<=(cdrStream& s)
{
  native_code_set <<= s;
  conversion_code_sets <<= s;
}

void
CONV_FRAME::
CodeSetComponentInfo::operator>>=(cdrStream& s) const
{
  ForCharData >>= s;
  ForWcharData >>= s;
}

void
CONV_FRAME::
CodeSetComponentInfo::operator<<=(cdrStream& s)
{
  ForCharData <<= s;
  ForWcharData <<= s;
}


OMNI_NAMESPACE_END(omni)


//
// Codeset components in IORs
//

OMNI_USING_NAMESPACE(omni)

static void
initialise_my_code_set()
{
  CONV_FRAME::CodeSetComponentInfo info;

  // Could put more conversion_code_sets but our default is sufficent for
  // most cases.
  info.ForCharData.native_code_set = orbParameters::nativeCharCodeSet->id();
  info.ForCharData.conversion_code_sets.length(1);
  info.ForCharData.conversion_code_sets[0] = omniCodeSet::ID_UTF_8;

  info.ForWcharData.native_code_set = orbParameters::nativeWCharCodeSet->id();
  info.ForWcharData.conversion_code_sets.length(1);
  info.ForWcharData.conversion_code_sets[0] = omniCodeSet::ID_UTF_16;

  omniIOR::add_TAG_CODE_SETS(info);
}


void
omniIOR::unmarshal_TAG_CODE_SETS(const IOP::TaggedComponent& c, omniIOR& ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_CODE_SETS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CONV_FRAME::CodeSetComponentInfo info;
  info <<= e;

  // Pick a char code set convertor
  omniCodeSet::TCS_C* tcs_c = 0;
  if (info.ForCharData.native_code_set) {
    tcs_c = omniCodeSet::getTCS_C(info.ForCharData.native_code_set,
				  ior.getIORInfo()->version());
  }
  if (!tcs_c) {
    CORBA::ULong total = info.ForCharData.conversion_code_sets.length();
    for (CORBA::ULong index=0; index < total; index++) {
      tcs_c = omniCodeSet::getTCS_C(
		      info.ForCharData.conversion_code_sets[index],
		      ior.getIORInfo()->version());
      if (tcs_c) break;
    }
  }
  if (!tcs_c) {
    // The server either specified code sets we do not know, or did
    // not specify any at all. We use the fallback code set.
    tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_UTF_8,
				  ior.getIORInfo()->version());
  }
  ior.getIORInfo()->TCS_C(tcs_c);

  // Pick a wchar code set convertor
  omniCodeSet::TCS_W* tcs_w = 0;
  if (info.ForWcharData.native_code_set) {
    tcs_w = omniCodeSet::getTCS_W(info.ForWcharData.native_code_set,
				  ior.getIORInfo()->version());
  }
  if (!tcs_w) {
    CORBA::ULong total = info.ForWcharData.conversion_code_sets.length();
    for (CORBA::ULong index=0; index < total; index++) {
      tcs_w = omniCodeSet::getTCS_W(
		       info.ForWcharData.conversion_code_sets[index],
		       ior.getIORInfo()->version());
      if (tcs_w) break;
    }
  }
  if (!tcs_w) {
    // The server either specified code sets we do not know, or did
    // not specify any at all. We use the fallback code set.
    tcs_w = omniCodeSet::getTCS_W(omniCodeSet::ID_UTF_16,
				  ior.getIORInfo()->version());
  }
  ior.getIORInfo()->TCS_W(tcs_w);
}

static  const char* not_specified = "not specified";
static  const char* not_supported = "0xXXXXXXXX";

static void write_codeset_name(char* buf, const char* cname, 
			       CONV_FRAME::CodeSetId id)
{
  if (strcmp(cname,not_supported))
    strcat(buf,cname);
  else {
    char* p = buf + strlen(buf);
    sprintf(p,"0x%08lx",(unsigned long)id);
  }
}


//
// Client side interceptor for code set service context
//

static
CORBA::Boolean
setCodeSetServiceContext(omniInterceptors::clientSendRequest_T::info_T& info)
{
  omniCodeSet::TCS_C* tcs_c;
  omniCodeSet::TCS_W* tcs_w;

  giopStrand& d = info.giop_c.strand();

  if (d.tcs_selected) {
    // giopStream::acquireClient never gives out the same strand
    // to run 2 different GIOP versions.
    //OMNIORB_ASSERT(d.version.major == ver.major && 
    //               d.version.minor == ver.minor);
    info.giop_c.TCS_C(d.tcs_c);
    info.giop_c.TCS_W(d.tcs_w);
    return 1;
    
    // Notice that we do not check the chosen convertors against the IOR
    // of the object. In fact, the IOR of the object may specify a set
    // of codesets that does not contain the ones we have just selected!
    // This is possible because our selection might be based on the IOR
    // of another object in the same server and a perverse server may put
    // into the 2 IORs a different set of codesets.
  }

  GIOP::Version ver = info.giop_c.version();
  if (ver.minor < 1) {
    // Code set service context is only defined from GIOP 1.1 onwards,
    // so here we do not attempt to set a codeset service context.
    d.tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
    d.tcs_w = 0;
    d.tcs_selected = 1;
    info.giop_c.TCS_C(d.tcs_c);
    info.giop_c.TCS_W(d.tcs_w);
    return 1;
  }

  // Get codeset information from the IOR.
  const omniIOR::IORInfo* ior_info = info.giop_c.ior()->getIORInfo();
  tcs_c = ior_info->TCS_C();
  tcs_w = ior_info->TCS_W();

  if (!(tcs_c || tcs_w)) {
    // The IOR did not specify anything, so according to the CORBA
    // specification, we should use the default ISO-8859-1 for char
    // and nothing for wchar, as below. We allow the defaults to be
    // overridden so we can gracefully handle badly behaved servers.
    tcs_c = orbParameters::defaultCharCodeSet;
    tcs_w = orbParameters::defaultWCharCodeSet;

    if (omniORB::trace(25) && (tcs_c || tcs_w)) {
      omniORB::logger log;
      log << "Use default codesets for server that did not specify any.\n";
    }
  }

  if (tcs_c) {
    d.tcs_c = tcs_c;
    d.tcs_w = tcs_w;
    d.version = ver;
    d.tcs_selected = 1;

    info.giop_c.TCS_C(tcs_c);
    info.giop_c.TCS_W(tcs_w);

    // Send the codeset service context
    cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
    tcs_c->id() >>= s;
    if (tcs_w) {
      tcs_w->id() >>= s;
    }
    else {
      operator>>=(CORBA::ULong(0),s);
    }

    CORBA::Octet* data;
    CORBA::ULong max,datalen;
    s.getOctetStream(data,max,datalen);

    CORBA::ULong len = info.service_contexts.length() + 1;
    info.service_contexts.length(len);
    info.service_contexts[len-1].context_id = IOP::CodeSets;
    info.service_contexts[len-1].context_data.replace(max,datalen,data,1);

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Send codeset service context: ("
	  << tcs_c->name() << "," << ((tcs_w) ? tcs_w->name() : "none")
	  << ")\n";
    }
    return 1;
  }
  else {
    // The server has not supplied any code set information.
    // Use the default code set.
    tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
    info.giop_c.TCS_C(tcs_c);
    info.giop_c.TCS_W(0);
    return 1;
  }
}

//
// Server side interceptor for code set service context
//

static
CORBA::Boolean
getCodeSetServiceContext(omniInterceptors::serverReceiveRequest_T::info_T& info)
{
  giopStrand&   d   = info.giop_s.strand();
  GIOP::Version ver = info.giop_s.version();

  if (ver.minor < 1) {
    // Code set service context is only defined from  GIOP 1.1 onwards
    if (!d.tcs_selected) {
      d.tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
      d.tcs_w = 0;
    }
    info.giop_s.TCS_C(d.tcs_c);
    info.giop_s.TCS_W(d.tcs_w);
    return 1;
  }

  omniCodeSet::TCS_C* tcs_c = d.tcs_c;
  omniCodeSet::TCS_W* tcs_w = d.tcs_w;

  IOP::ServiceContextList& svclist = info.giop_s.service_contexts();
  CORBA::ULong total = svclist.length();
  for (CORBA::ULong index = 0; index < total; index++) {
    if (svclist[index].context_id == IOP::CodeSets) {
      cdrEncapsulationStream e(svclist[index].context_data.get_buffer(),
			       svclist[index].context_data.length(),1);

      CONV_FRAME::CodeSetId tcs_cid, tcs_wid;
      tcs_cid <<= e;
      tcs_wid <<= e;
      if (tcs_cid) {
	tcs_c = omniCodeSet::getTCS_C(tcs_cid,ver);
      }
      else {
	// Client do not specify char TCS. Use default.
	tcs_c =  omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
      }
      if (tcs_wid) {
	tcs_w = omniCodeSet::getTCS_W(tcs_wid,ver);
      }
      else {
	// Client do not specify wchar TCS.
	tcs_w = 0;
      }
      d.version = ver;
      d.tcs_c = tcs_c;
      d.tcs_w = tcs_w;
      d.tcs_selected = 1;

      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << "Receive codeset service context and set TCS to ("
	    << ((tcs_c) ? tcs_c->name() : "none") << "," 
	    << ((tcs_w) ? tcs_w->name() : "none") << ")\n";
      }
      break;
    }
  }

  if (!d.tcs_selected) {
    // In the absence of any codeset negotiation, we choose 
    // ISO-8859-1 as the transmission code set for char
    d.version.major = ver.major; d.version.minor = ver.minor;
    tcs_c = d.tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
    tcs_w = d.tcs_w = 0;
    d.tcs_selected = 1;
  }

  if (d.version.major != ver.major || d.version.minor != ver.minor) {

    // We are in murky water here. The strand has previously been used
    // for one GIOP version and now a request of another GIOP version
    // is coming in. Does the codeset negotiation established for
    // the previous request also apply to this one?
    //
    // Our interpretation is that the previous codeset negotiation only
    // applies to the GIOP version it negotiates for. For the current
    // request, we treat the situation as if no negotiation has taken place
    // at all.
    tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
    tcs_w = 0;
  }

  info.giop_s.TCS_C(tcs_c);
  info.giop_s.TCS_W(tcs_w);

  return 1;
}

/////////////////////////////////////////////////////////////////////////////
static const char* unknown_code_set_msg = "Unknown code set name";

class nativeCharCodeSetHandler : public orbOptions::Handler {
public:

  nativeCharCodeSetHandler() : 
    orbOptions::Handler("nativeCharCodeSet",
			"nativeCharCodeSet = <code set name, e.g. ISO-8859-1>",
			1,
			"-ORBnativeCharCodeSet <code set name, e.g. ISO-8859-1>") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    omniCodeSet::NCS_C* v = omniCodeSet::getNCS_C(value);
    if (!v) {
      throw orbOptions::BadParam(key(),value,unknown_code_set_msg);
    }
    orbParameters::nativeCharCodeSet = v;
  }

  void dump(orbOptions::sequenceString& result) {

    const char* v;
    if (orbParameters::nativeCharCodeSet)
      v = orbParameters::nativeCharCodeSet->name();
    else
      v = "nil";
    orbOptions::addKVString(key(),v,result);
  }
};

static nativeCharCodeSetHandler nativeCharCodeSetHandler_;

/////////////////////////////////////////////////////////////////////////////
class nativeWCharCodeSetHandler : public orbOptions::Handler {
public:

  nativeWCharCodeSetHandler() : 
    orbOptions::Handler("nativeWCharCodeSet",
			"nativeWCharCodeSet = <code set name, e.g. UTF-16>",
			1,
			"-ORBnativeWCharCodeSet <code set name, e.g. UTF-16>") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    omniCodeSet::NCS_W* v = omniCodeSet::getNCS_W(value);
    if (!v) {
      throw orbOptions::BadParam(key(),value,unknown_code_set_msg);
    }
    orbParameters::nativeWCharCodeSet = v;
  }

  void dump(orbOptions::sequenceString& result) {

    const char* v;
    if (orbParameters::nativeWCharCodeSet)
      v = orbParameters::nativeWCharCodeSet->name();
    else
      v = "nil";
    orbOptions::addKVString(key(),v,result);
  }
};

static nativeWCharCodeSetHandler nativeWCharCodeSetHandler_;


/////////////////////////////////////////////////////////////////////////////
class defaultCharCodeSetHandler : public orbOptions::Handler {
public:

  defaultCharCodeSetHandler() : 
    orbOptions::Handler("defaultCharCodeSet",
			"defaultCharCodeSet = <code set name, e.g. ISO-8859-1>",
			1,
			"-ORBdefaultCharCodeSet <code set name, e.g. ISO-8859-1>") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    omniCodeSet::TCS_C* v = omniCodeSet::getTCS_C(value, omniCodeSetUtil::GIOP12);
    if (!v) {
      throw orbOptions::BadParam(key(),value,unknown_code_set_msg);
    }
    orbParameters::defaultCharCodeSet = v;
  }

  void dump(orbOptions::sequenceString& result) {

    const char* v;
    if (orbParameters::defaultCharCodeSet)
      v = orbParameters::defaultCharCodeSet->name();
    else
      v = "nil";
    orbOptions::addKVString(key(),v,result);
  }
};

static defaultCharCodeSetHandler defaultCharCodeSetHandler_;

/////////////////////////////////////////////////////////////////////////////
class defaultWCharCodeSetHandler : public orbOptions::Handler {
public:

  defaultWCharCodeSetHandler() : 
    orbOptions::Handler("defaultWCharCodeSet",
			"defaultWCharCodeSet = <code set name, e.g. UTF-16>",
			1,
			"-ORBdefaultWCharCodeSet <code set name, e.g. UTF-16>") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    omniCodeSet::TCS_W* v = omniCodeSet::getTCS_W(value, omniCodeSetUtil::GIOP12);
    if (!v) {
      throw orbOptions::BadParam(key(),value,unknown_code_set_msg);
    }
    orbParameters::defaultWCharCodeSet = v;

    if (!orbParameters::defaultCharCodeSet) {
      // If the wchar codeset is selected, we must have a char codeset.
      orbParameters::defaultCharCodeSet =
        omniCodeSet::getTCS_C(omniCodeSet::ID_UTF_8, omniCodeSetUtil::GIOP12);
    }
  }

  void dump(orbOptions::sequenceString& result) {

    const char* v;
    if (orbParameters::defaultWCharCodeSet)
      v = orbParameters::defaultWCharCodeSet->name();
    else
      v = "nil";
    orbOptions::addKVString(key(),v,result);
  }
};

static defaultWCharCodeSetHandler defaultWCharCodeSetHandler_;



//
// Module initialiser
//

void
omniCodeSet::logCodeSets()
{
  omniCodeSet::Base* cs;
  GIOP::Version v;

  {
    omniORB::logger l;
    l << "Native char code sets:";
    for (cs = ncs_c_head(); cs; cs = cs->pd_next) l << " " << cs->name();
    l << ".\n";
  }
  {
    omniORB::logger l;
    l << "Transmission char code sets:";
    for (cs = tcs_c_head(); cs; cs = cs->pd_next) {
      v = ((omniCodeSet::TCS_C*)cs)->giopVersion();
      l << " " << cs->name()
	<< "(" << ((int)v.major) << "." << ((int)v.minor) << ")";
    }
    l << ".\n";
  }
  {
    omniORB::logger l;
    l << "Native wide char code sets:";
    for (cs = ncs_w_head(); cs; cs = cs->pd_next) l << " " << cs->name();
    l << ".\n";
  }
  {
    omniORB::logger l;
    l << "Transmission wide char code sets:";
    for (cs = tcs_w_head(); cs; cs = cs->pd_next) {
      v = ((omniCodeSet::TCS_W*)cs)->giopVersion();
      l << " " << cs->name()
	<< "(" << ((int)v.major) << "." << ((int)v.minor) << ")";
    }
    l << ".\n";
  }
}


OMNI_NAMESPACE_BEGIN(omni)

class omni_codeSet_initialiser : public omniInitialiser {
public:
  omni_codeSet_initialiser() {
    orbOptions::singleton().registerHandler(nativeCharCodeSetHandler_);
    orbOptions::singleton().registerHandler(nativeWCharCodeSetHandler_);
    orbOptions::singleton().registerHandler(defaultCharCodeSetHandler_);
    orbOptions::singleton().registerHandler(defaultWCharCodeSetHandler_);
  }

  void attach() {
    if (!orbParameters::nativeCharCodeSet)
      orbParameters::nativeCharCodeSet =
        omniCodeSet::getNCS_C(omniCodeSet::ID_8859_1);

    if (!orbParameters::nativeWCharCodeSet)
      orbParameters::nativeWCharCodeSet =
        omniCodeSet::getNCS_W(omniCodeSet::ID_UTF_16);

    GIOP::Version ver = giopStreamImpl::maxVersion()->version();

    if (!orbParameters::anyCharCodeSet)
      orbParameters::anyCharCodeSet =
	omniCodeSet::getTCS_C(orbParameters::nativeCharCodeSet->id(), ver);

    if (!orbParameters::anyWCharCodeSet)
      orbParameters::anyWCharCodeSet =
	omniCodeSet::getTCS_W(orbParameters::nativeWCharCodeSet->id(), ver);

    // Create the tagged component for all IORs created by this ORB.
    initialise_my_code_set();

    // install interceptors
    omniInterceptors* interceptors = omniORB::getInterceptors();
    interceptors->clientSendRequest.add(setCodeSetServiceContext);
    interceptors->serverReceiveRequest.add(getCodeSetServiceContext);

    if (omniORB::trace(15)) {
      omniCodeSet::logCodeSets();
    }
  }
  void detach() {}
};


static omni_codeSet_initialiser initialiser;

omniInitialiser& omni_codeSet_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
