// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslAddress.h               Created on: 29 May 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2013 Apasphere Ltd
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

#ifndef __SSLADDRESS_H__
#define __SSLADDRESS_H__

OMNI_NAMESPACE_BEGIN(omni)

class sslAddress : public giopAddress {
 public:

  sslAddress(const IIOP::Address& address, sslContext* ctx);

  const char*  type()      const;
  const char*  address()   const;
  const char*  host()      const;
  giopAddress* duplicate() const;
  giopAddress* duplicate(const char* host) const;

  giopActiveConnection* Connect(const omni_time_t& deadline,
				CORBA::ULong  	   strand_flags,
				CORBA::Boolean&    timed_out) const;
  CORBA::Boolean Poke() const;
  ~sslAddress() {}

 private:
  IIOP::Address      pd_address;
  CORBA::String_var  pd_address_string;
  sslContext*        pd_ctx;

  sslAddress();
  sslAddress(const sslAddress&);
};

OMNI_NAMESPACE_END(omni)

#endif // __SSLADDRESS_H__
