// -*- Mode: C++; -*-
//                            Package   : omniORB
// IIOP.h                     Created on: 8/2/96
//                            Author    : Sai Lai Lo (sll)
//
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
//       C++ mapping of the OMG IIOP module
//       Reference: CORBA V2.0 12.8.2
//	
//	

#ifndef __OMNIORB_IIOP_H__
#define __OMNIORB_IIOP_H__

class IIOP {
public:

  typedef GIOP::Version Version;

  struct Address {
    _CORBA_String_member    host;
    _CORBA_UShort           port;

    void operator>>=(cdrStream&) const;
    void operator<<=(cdrStream&);
  };

  struct ProfileBody {
    Version                 version;
    Address                 address;
    _CORBA_Unbounded_Sequence_Octet  object_key;
    IOP::MultipleComponentProfile    components;

    ProfileBody() : components(2) {}
  };

  typedef _CORBA_Unbounded_Sequence<ProfileBody> ProfileBodyList;

  static _core_attr const _CORBA_UShort DEFAULT_CORBALOC_PORT;

  static void encodeProfile(const ProfileBody&,IOP::TaggedProfile&);
  static void encodeMultiComponentProfile(const IOP::MultipleComponentProfile&,
					  IOP::TaggedProfile&);

  static void unmarshalProfile(const IOP::TaggedProfile&, ProfileBody&);
  static void unmarshalMultiComponentProfile(const IOP::TaggedProfile&,
					     IOP::MultipleComponentProfile&);

  static void unmarshalObjectKey(const IOP::TaggedProfile& p,
				 _CORBA_Unbounded_Sequence_Octet& key);
  // The input profile must be TAG_INTERNET_IOP
  // Extract the object key into <key>. The octet buffer inside <key> is
  // still own by <p>. So p must not be deallocated before key is.
};


#endif // __OMNIORB_IIOP_H__
