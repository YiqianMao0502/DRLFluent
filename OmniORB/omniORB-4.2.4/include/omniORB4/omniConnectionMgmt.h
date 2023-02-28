// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniConnectionMgmt.h       Created on: 2006/07/10
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2006 Apasphere Ltd.
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
// Description:
//    Proprietary omniORB connection management API


#ifndef _OMNICONNECTIONMGMT_H_
#define _OMNICONNECTIONMGMT_H_

#include <omniORB4/CORBA.h>

_CORBA_MODULE omniConnectionMgmt

_CORBA_MODULE_BEG

  _CORBA_MODULE_FN void init();
  // Initialise the omniConnectionMgmt extension library. Must be
  // called before CORBA::ORB_init().

  _CORBA_MODULE_FN CORBA::Object_ptr
  makeRestrictedReference(CORBA::Object_ptr obj,
			  CORBA::ULong      connection_id,
			  CORBA::ULong      max_connections,
			  CORBA::ULong      max_threads,
			  CORBA::Boolean    data_batch,
			  CORBA::Boolean    permit_interleaved,
			  CORBA::Boolean    server_hold_open);
  // Given an object reference, construct a new reference that uses
  // connections unique to references with <connection_id>. The client
  // will open at most <max_connections> network connections to the
  // server; the server will use at most <max_threads> concurrent
  // threads to service each of those connections. If <data_batch> is
  // true, the client will enable data batching on the connection if
  // relevant (e.g. Nagle's algorithm). If <permit_interleaved> is
  // true, multiple concurrent calls can be interleaved on a single
  // connection. If <server_hold_open> is true, the server will keep
  // the connection open until the client closes it.

_CORBA_MODULE_END


#endif // _OMNICONNECTIONMGMT_H_
