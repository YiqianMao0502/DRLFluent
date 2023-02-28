// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpEndpoint.h              Created on: 19 Mar 2001
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

#ifndef __TCPENDPOINT_H__
#define __TCPENDPOINT_H__

#include <omniORB4/omniServer.h>

OMNI_NAMESPACE_BEGIN(omni)

class tcpConnection;

class tcpEndpoint : public giopEndpoint,
		    public SocketCollection,
		    public SocketHolder {
public:

  tcpEndpoint(const char* param);

  // The following implement giopEndpoint abstract functions
  const char* type() const;
  const char* address() const;
  const orbServer::EndpointList* addresses() const;
  CORBA::Boolean publish(const orbServer::PublishSpecs& publish_specs,
			 CORBA::Boolean 	  	all_specs,
			 CORBA::Boolean 	  	all_eps,
			 orbServer::EndpointList& 	published_eps);
  CORBA::Boolean Bind();
  giopConnection* AcceptAndMonitor(giopConnection::notifyReadable_t,void*);
  void Poke();
  void Shutdown();

  ~tcpEndpoint();

protected:
  CORBA::Boolean notifyReadable(SocketHolder*);
  // implement SocketCollection::notifyReadable
  

private:
  const char*          		   pd_address_param;
  IIOP::Address                    pd_address;
  orbServer::EndpointList          pd_addresses;

  SocketHandle_t                   pd_new_conn_socket;
  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;
  CORBA::Boolean                   pd_poked;

  tcpEndpoint();
  tcpEndpoint(const tcpEndpoint&);
  tcpEndpoint& operator=(const tcpEndpoint&);
};


class tcpActiveConnection;

class tcpActiveCollection : public giopActiveCollection, 
			    public SocketCollection {
public:
  const char* type() const;
  // implement giopActiveCollection::type

  void Monitor(giopConnection::notifyReadable_t func, void* cookie);
  // implement giopActiveCollection::Monitor

  CORBA::Boolean isEmpty() const;
  // implement giopActiveCollection::isEmpty

  void deactivate();
  // implement giopActiveCollection::deactivate

  tcpActiveCollection();
  ~tcpActiveCollection();

  friend class tcpActiveConnection;

protected:
  CORBA::Boolean notifyReadable(SocketHolder*);
  // implement SocketCollection::notifyReadable

  void addMonitor(SocketHandle_t);
  void removeMonitor(SocketHandle_t);

private:
  CORBA::ULong      pd_n_sockets;
  CORBA::Boolean    pd_shutdown;
  omni_tracedmutex  pd_lock;

  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;

  tcpActiveCollection(const tcpActiveCollection&);
  tcpActiveCollection& operator=(const tcpActiveCollection&);
};

OMNI_NAMESPACE_END(omni)

#endif // __TCPENDPOINT_H__
