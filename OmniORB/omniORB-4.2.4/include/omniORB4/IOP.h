// -*- Mode: C++; -*-
//                            Package   : omniORB
// IOP.h                      Created on: 8/2/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2013 Apasphere Ltd
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
//       C++ mapping of the OMG IOP module
//       Reference: CORBA V2.0 10.6.2
//	

#ifndef __OMNIORB_IOP_H__
#define __OMNIORB_IOP_H__

class IOP {
public:
  
  typedef _CORBA_ULong ProfileId;
  static _core_attr const   ProfileId    TAG_INTERNET_IOP;
  static _core_attr const   ProfileId    TAG_MULTIPLE_COMPONENTS;
  static _core_attr const   ProfileId    TAG_SCCP_IOP;

  struct TaggedProfile {
    ProfileId tag;
    _CORBA_Unbounded_Sequence_Octet profile_data;

    void operator>>= (cdrStream &s) const;
    void operator<<= (cdrStream &s);
  };

  typedef _CORBA_Unbounded_Sequence<TaggedProfile> TaggedProfileList;

  class TaggedProfileList_out;
  class TaggedProfileList_var {
  public:
    typedef TaggedProfileList _Tseq;
    typedef TaggedProfileList_var _T_var;

    inline TaggedProfileList_var() : pd_seq(0) {}
    inline TaggedProfileList_var(_Tseq* s) : pd_seq(s) {}
    inline TaggedProfileList_var(const _T_var& sv) {
      if( sv.pd_seq ) {
	pd_seq = new _Tseq;
	*pd_seq = *sv.pd_seq;
      } else
	pd_seq = 0;
    }
    inline ~TaggedProfileList_var() { if( pd_seq ) delete pd_seq; }

    inline _T_var& operator = (_Tseq* s) {
      if( pd_seq )  delete pd_seq;
      pd_seq = s;
      return *this;
    }
    inline _T_var& operator = (const _T_var& sv) {
      if( sv.pd_seq ) {
	if( !pd_seq )  pd_seq = new _Tseq;
	*pd_seq = *sv.pd_seq;
      } else if( pd_seq ) {
	delete pd_seq;
	pd_seq = 0;
      }
      return *this;
    }

    inline TaggedProfile& operator [] (_CORBA_ULong i) {   return (*pd_seq)[i]; }
    inline _Tseq* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
    inline operator _Tseq& () const { return *pd_seq; }
#else
    inline operator const _Tseq& () const { return *pd_seq; }
    inline operator _Tseq& () { return *pd_seq; }
#endif

    inline const _Tseq& in() const { return *pd_seq; }
    inline _Tseq& inout() { return *pd_seq; }
    inline _Tseq*& out() { if (pd_seq) { delete pd_seq; pd_seq = 0; } return pd_seq; }
    inline _Tseq* _retn() { _Tseq* tmp = pd_seq; pd_seq = 0; return tmp; }

    friend class TaggedProfileList_out;

  private:
    _Tseq* pd_seq;
  };


  class TaggedProfileList_out {
  public:
    typedef TaggedProfileList _Tseq;
    typedef TaggedProfileList_var _T_var;

    inline TaggedProfileList_out(_Tseq*& s) : _data(s) { _data = 0; }
    inline TaggedProfileList_out(_T_var& sv)
      : _data(sv.pd_seq) { sv = (_Tseq*) 0; }
    inline TaggedProfileList_out(const TaggedProfileList_out& s) : _data(s._data) { }
    inline TaggedProfileList_out& operator=(const TaggedProfileList_out& s) { _data = s._data; return *this; }
    inline TaggedProfileList_out& operator=(_Tseq* s) { _data = s; return *this; }
    inline operator _Tseq*&() { return _data; }
    inline _Tseq*& ptr() { return _data; }
    inline _Tseq* operator->() { return _data; }
    inline TaggedProfile& operator [] (_CORBA_ULong i) {   return (*_data)[i]; }
    _Tseq*& _data;

  private:
    TaggedProfileList_out();
    TaggedProfileList_out operator=( const _T_var&);
  };

  struct IOR {
    _CORBA_String_member  type_id;
    TaggedProfileList     profiles;

    // the following are omniORB private functions
    void operator>>= (cdrStream &s);
    void operator<<= (cdrStream &s);
    static char* unmarshaltype_id(cdrStream&);
  };

  typedef _CORBA_ULong ComponentId;
  static _core_attr const ComponentId TAG_ORB_TYPE;
  static _core_attr const ComponentId TAG_CODE_SETS;
  static _core_attr const ComponentId TAG_POLICIES;
  static _core_attr const ComponentId TAG_ALTERNATE_IIOP_ADDRESS;
  static _core_attr const ComponentId TAG_ASSOCIATION_OPTIONS;
  static _core_attr const ComponentId TAG_SEC_NAME;
  static _core_attr const ComponentId TAG_SPKM_1_SEC_MECH;
  static _core_attr const ComponentId TAG_SPKM_2_SEC_MECH;
  static _core_attr const ComponentId TAG_KERBEROSV5_SEC_MECH;
  static _core_attr const ComponentId TAG_CSI_ECMA_SECRET_SEC_MECH;
  static _core_attr const ComponentId TAG_CSI_ECMA_HYBRID_SEC_MECH;
  static _core_attr const ComponentId TAG_SSL_SEC_TRANS;
  static _core_attr const ComponentId TAG_CSI_ECMA_PUBLIC_SEC_MECH;
  static _core_attr const ComponentId TAG_GENERIC_SEC_MECH;
  static _core_attr const ComponentId TAG_FIREWALL_TRANS;
  static _core_attr const ComponentId TAG_SCCP_CONTACT_INFO;
  static _core_attr const ComponentId TAG_JAVA_CODEBASE;
  static _core_attr const ComponentId TAG_CSI_SEC_MECH_LIST;
  static _core_attr const ComponentId TAG_NULL_TAG;
  static _core_attr const ComponentId TAG_TLS_SEC_TRANS;
  static _core_attr const ComponentId TAG_COMPLETE_OBJECT_KEY;
  static _core_attr const ComponentId TAG_ENDPOINT_ID_POSITION;
  static _core_attr const ComponentId TAG_LOCATION_POLICY;
  static _core_attr const ComponentId TAG_DCE_STRING_BINDING;
  static _core_attr const ComponentId TAG_DCE_BINDING_NAME;
  static _core_attr const ComponentId TAG_DCE_NO_PIPES;
  static _core_attr const ComponentId TAG_DCE_SEC_MECH;
  static _core_attr const ComponentId TAG_INET_SEC_TRANS;
  static _core_attr const ComponentId TAG_GROUP; // FT SPEC
  static _core_attr const ComponentId TAG_PRIMARY; // FT SPEC
  static _core_attr const ComponentId TAG_HEARTBEAT_ENABLED; // FT SPEC

  // omniORB specific:
  static _core_attr const ComponentId TAG_OMNIORB_BIDIR;
  static _core_attr const ComponentId TAG_OMNIORB_UNIX_TRANS;
  static _core_attr const ComponentId TAG_OMNIORB_PERSISTENT_ID;
  static _core_attr const ComponentId TAG_OMNIORB_RESTRICTED_CONNECTION;


  static const char* ComponentIDtoName(ComponentId);
  // omniORB private function.
  // Return the name given the ComponentId. Return nil if the ComponentId
  // is not recongised.

  struct TaggedComponent {
    ComponentId	            tag;
    _CORBA_Unbounded_Sequence_Octet component_data;

    // the following are omniORB private functions
    void operator>>= (cdrStream &s) const;
    void operator<<= (cdrStream &s);
  };
  
  static char* dumpComponent(const TaggedComponent&);
  // omniORB private function. Produce the textual dump of the component
  // content

  typedef _CORBA_Unbounded_Sequence<TaggedComponent> MultipleComponentProfile;

  class MultipleComponentProfile_out;

  class MultipleComponentProfile_var {
  public:
    typedef MultipleComponentProfile _Tseq;
    typedef MultipleComponentProfile_var _T_var;

    inline MultipleComponentProfile_var() : pd_seq(0) {}
    inline MultipleComponentProfile_var(_Tseq* s) : pd_seq(s) {}
    inline MultipleComponentProfile_var(const _T_var& sv) {
      if( sv.pd_seq ) {
	pd_seq = new _Tseq;
	*pd_seq = *sv.pd_seq;
      } else
	pd_seq = 0;
    }
    inline ~MultipleComponentProfile_var() { if( pd_seq ) delete pd_seq; }

    inline _T_var& operator = (_Tseq* s) {
      if( pd_seq )  delete pd_seq;
      pd_seq = s;
      return *this;
    }
    inline _T_var& operator = (const _T_var& sv) {
      if( sv.pd_seq ) {
	if( !pd_seq )  pd_seq = new _Tseq;
	*pd_seq = *sv.pd_seq;
      } else if( pd_seq ) {
	delete pd_seq;
	pd_seq = 0;
      }
      return *this;
    }

    inline TaggedComponent& operator [] (_CORBA_ULong i) {   return (*pd_seq)[i]; }
    inline _Tseq* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
    inline operator _Tseq& () const { return *pd_seq; }
#else
    inline operator const _Tseq& () const { return *pd_seq; }
    inline operator _Tseq& () { return *pd_seq; }
#endif

    inline const _Tseq& in() const { return *pd_seq; }
    inline _Tseq& inout() { return *pd_seq; }
    inline _Tseq*& out() { if (pd_seq) { delete pd_seq; pd_seq = 0; } return pd_seq; }
    inline _Tseq* _retn() { _Tseq* tmp = pd_seq; pd_seq = 0; return tmp; }

    friend class MultipleComponentProfile_out;

  private:
    _Tseq* pd_seq;
  };

  class MultipleComponentProfile_out {
  public:
    typedef MultipleComponentProfile _Tseq;
    typedef MultipleComponentProfile_var _T_var;

    inline MultipleComponentProfile_out(_Tseq*& s) : _data(s) { _data = 0; }
    inline MultipleComponentProfile_out(_T_var& sv)
      : _data(sv.pd_seq) { sv = (_Tseq*) 0; }
    inline MultipleComponentProfile_out(const MultipleComponentProfile_out& s) : _data(s._data) { }
    inline MultipleComponentProfile_out& operator=(const MultipleComponentProfile_out& s) { _data = s._data; return *this; }
    inline MultipleComponentProfile_out& operator=(_Tseq* s) { _data = s; return *this; }
    inline operator _Tseq*&() { return _data; }
    inline _Tseq*& ptr() { return _data; }
    inline _Tseq* operator->() { return _data; }
    inline TaggedComponent& operator [] (_CORBA_ULong i) {   return (*_data)[i]; }
    _Tseq*& _data;

  private:
    MultipleComponentProfile_out();
    MultipleComponentProfile_out operator=( const _T_var&);
  };


  typedef _CORBA_Unbounded_Sequence<MultipleComponentProfile> MultipleComponentProfileList;

  typedef _CORBA_ULong ServiceID;

  struct ServiceContext {
    ServiceID              context_id;
    _CORBA_Unbounded_Sequence_Octet context_data;

    // the following are omniORB private functions
    void operator>>= (cdrStream &s) const;
    void operator<<= (cdrStream &s);
  };

  typedef _CORBA_Unbounded_Sequence<ServiceContext> ServiceContextList;

  class ServiceContextList_out;

  class ServiceContextList_var {
  public:
    typedef ServiceContextList _Tseq;
    typedef ServiceContextList_var _T_var;

    inline ServiceContextList_var() : pd_seq(0) {}
    inline ServiceContextList_var(_Tseq* s) : pd_seq(s) {}
    inline ServiceContextList_var(const _T_var& sv) {
      if( sv.pd_seq ) {
	pd_seq = new _Tseq;
	*pd_seq = *sv.pd_seq;
      } else
	pd_seq = 0;
    }
    inline ~ServiceContextList_var() { if( pd_seq ) delete pd_seq; }

    inline _T_var& operator = (_Tseq* s) {
      if( pd_seq )  delete pd_seq;
      pd_seq = s;
      return *this;
    }
    inline _T_var& operator = (const _T_var& sv) {
      if( sv.pd_seq ) {
	if( !pd_seq )  pd_seq = new _Tseq;
	*pd_seq = *sv.pd_seq;
      } else if( pd_seq ) {
	delete pd_seq;
	pd_seq = 0;
      }
      return *this;
    }

    inline ServiceContext& operator [] (_CORBA_ULong i) {   return (*pd_seq)[i]; }
    inline _Tseq* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
    inline operator _Tseq& () const { return *pd_seq; }
#else
    inline operator const _Tseq& () const { return *pd_seq; }
    inline operator _Tseq& () { return *pd_seq; }
#endif

    inline const _Tseq& in() const { return *pd_seq; }
    inline _Tseq& inout() { return *pd_seq; }
    inline _Tseq*& out() { if (pd_seq) { delete pd_seq; pd_seq = 0; } return pd_seq; }
    inline _Tseq* _retn() { _Tseq* tmp = pd_seq; pd_seq = 0; return tmp; }

    friend class ServiceContextList_out;

  private:
    _Tseq* pd_seq;
  };

  class ServiceContextList_out {
  public:
    typedef ServiceContextList _Tseq;
    typedef ServiceContextList_var _T_var;

    inline ServiceContextList_out(_Tseq*& s) : _data(s) { _data = 0; }
    inline ServiceContextList_out(_T_var& sv)
      : _data(sv.pd_seq) { sv = (_Tseq*) 0; }
    inline ServiceContextList_out(const ServiceContextList_out& s) : _data(s._data) { }
    inline ServiceContextList_out& operator=(const ServiceContextList_out& s) { _data = s._data; return *this; }
    inline ServiceContextList_out& operator=(_Tseq* s) { _data = s; return *this; }
    inline operator _Tseq*&() { return _data; }
    inline _Tseq*& ptr() { return _data; }
    inline _Tseq* operator->() { return _data; }
    inline ServiceContext& operator [] (_CORBA_ULong i) {   return (*_data)[i]; }
    _Tseq*& _data;

  private:
    ServiceContextList_out();
    ServiceContextList_out operator=( const _T_var&);
  };

  static _core_attr const ServiceID TransactionService;
  static _core_attr const ServiceID CodeSets; 
  static _core_attr const ServiceID ChainBypassCheck;
  static _core_attr const ServiceID ChainBypassInfo;
  static _core_attr const ServiceID LogicalThreadId;
  static _core_attr const ServiceID BI_DIR_IIOP;
  static _core_attr const ServiceID SendingContextRunTime;
  static _core_attr const ServiceID INVOCATION_POLICIES;
  static _core_attr const ServiceID FORWARDED_IDENTITY;
  static _core_attr const ServiceID UnknownExceptionInfo;
  static _core_attr const ServiceID RTCorbaPriority;
  static _core_attr const ServiceID RTCorbaPriorityRange;
  static _core_attr const ServiceID GROUP_VERSION; // FT SPEC
  static _core_attr const ServiceID REQUEST;       // FT SPEC

  static _core_attr const ServiceID OMNIORB_RESTRICTED_CONNECTION;

  static const char* ServiceIDtoName(ServiceID);
  // omniORB private function.
  // Return the name given the ComponentId. Return nil if the ComponentId
  // is not recongised.

};

#endif // __OMNIORB_IOP_H__
