// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixAddress.h              Created on: 19 Mar 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2011 Apasphere Ltd.
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

#ifndef __UNIXADDRESS_H__
#define __UNIXADDRESS_H__

OMNI_NAMESPACE_BEGIN(omni)

class unixAddress : public giopAddress {
 public:

  unixAddress(const char* filename);
  const char* type() const;
  const char* address() const;
  giopAddress* duplicate() const;
  giopActiveConnection* Connect(const omni_time_t& deadline,
				CORBA::ULong  	   strand_flags,
				CORBA::Boolean&    timed_out) const;
  CORBA::Boolean Poke() const;
  ~unixAddress() {}

 private:
  CORBA::String_var  pd_address_string;
  CORBA::String_var  pd_filename;

  unixAddress();
  unixAddress(const unixAddress&);
};

OMNI_NAMESPACE_END(omni)

#endif // __UNIXADDRESS_H__
