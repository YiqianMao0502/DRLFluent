// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopBiDir.h                Created on: 17/7/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2005-2013 Apasphere Ltd
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

#ifndef __GIOPBIDIR_H__
#define __GIOPBIDIR_H__

#include <giopRope.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class BiDirInfo : public omniIOR::IORExtraInfo {
public:
  BiDirInfo(char* s) :
    OMNIORB_BASE_CTOR(omniIOR::)IORExtraInfo(IOP::TAG_OMNIORB_BIDIR),
    sendfrom(s) {}

  CORBA::String_var sendfrom;

  ~BiDirInfo() {}

private:
  BiDirInfo();
  BiDirInfo(const BiDirInfo&);
  BiDirInfo& operator=(const BiDirInfo&);
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class BiDirServerRope : public giopRope {
public:

  static int selectRope(const giopAddressList&, omniIOR::IORInfo*, Rope*&);
  // Look into the IORInfo, if bidirectional giop is suitable for this ior
  // (i.e. the IOP component TAG_OMNIORB_BIDIR exists and its GIOP version
  // is >= 1.2), search the list and return the BiDirServerRope whose
  // redirection addresses matches one of the addresses in the
  // giopAddressList. The ref count of the returned rope is incremented by
  // 1. If a rope is found, returns 1. Otherwise returns 0.
  //
  // Thread Safety preconditions:
  //    Caller *must* hold omniTransportLock.

  static BiDirServerRope* addRope(giopStrand*, const giopAddressList&);
  // Add a BiDirServerRope for the bidirectional strand if one has not
  // been created already. Add the list of redirection addresses to 
  // this rope. Return the rope instance. Notice that the reference
  // count of the rope is unchanged.
  //
  // Thread Safety preconditions:
  //    Caller *must* hold omniTransportLock.


  BiDirServerRope(giopStrand*,giopAddress*);

  IOP_C* acquireClient(const omniIOR*,
		       const CORBA::Octet* key,
		       CORBA::ULong keysize,
		       omniCallDescriptor*);  // override giopRope

  static _core_attr RopeLink ropes;
  // All ropes created by addRope are linked together by this list.

  virtual void decrRefCount();

  virtual ~BiDirServerRope();

 protected:
  virtual void realIncrRefCount();

private:

  CORBA::String_var pd_sendfrom;
  giopAddressList   pd_redirect_addresses;  

  CORBA::Boolean match(const char* sendfrom,
		       const giopAddressList& addrlist) const;

  BiDirServerRope();
  BiDirServerRope(const BiDirServerRope&);
  BiDirServerRope& operator=(const BiDirServerRope&);
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class BiDirClientRope : public giopRope {
public:

  BiDirClientRope(const giopAddressList& addrlist, omniIOR::IORInfo* info);

  IOP_C* acquireClient(const omniIOR*      ior,
		       const CORBA::Octet* key,
		       CORBA::ULong        keysize,
		       omniCallDescriptor* cd); // override giopRope
protected:
  void filterAndSortAddressList();

private:
  omni_tracedmutex pd_lock;

  BiDirClientRope();
  BiDirClientRope(const BiDirClientRope&);
  BiDirClientRope& operator=(const BiDirClientRope&);
};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __GIOPBIDIR_H__
