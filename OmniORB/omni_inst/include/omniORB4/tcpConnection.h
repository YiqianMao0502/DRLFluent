// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpConnection.h            Created on: 19 Mar 2001
//                            Author    : Sai Lai Lo (sll)
//
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

#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__

#include <SocketCollection.h>

OMNI_NAMESPACE_BEGIN(omni)

class tcpEndpoint;

class tcpConnection : public giopConnection, public SocketHolder {
public:

  int Send(void* buf, size_t sz,
	   const omni_time_t& deadline);

  int Recv(void* buf, size_t sz,
	   const omni_time_t& deadline);

  void Shutdown();

  const char* myaddress();

  const char* peeraddress();

  void setSelectable(int now = 0,CORBA::Boolean data_in_buffer = 0);

  void clearSelectable();

  CORBA::Boolean isSelectable();

  CORBA::Boolean Peek();

  SocketHandle_t handle() const { return pd_socket; }

  tcpConnection(SocketHandle_t,SocketCollection*);

  ~tcpConnection();

  friend class tcpEndpoint;

private:
  CORBA::String_var pd_myaddress;
  CORBA::String_var pd_peeraddress;
};


class tcpActiveConnection : public giopActiveConnection, public tcpConnection {
public:
  giopActiveCollection* registerMonitor();
  giopConnection& getConnection();

  tcpActiveConnection(SocketHandle_t);
  ~tcpActiveConnection();

private:
  CORBA::Boolean pd_registered;

  tcpActiveConnection(const tcpActiveConnection&);
  tcpActiveConnection& operator=(const tcpActiveConnection&);
};


OMNI_NAMESPACE_END(omni)

#endif //__TCPCONNECTION_H__
