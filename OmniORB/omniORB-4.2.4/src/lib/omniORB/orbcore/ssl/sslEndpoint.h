// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslEndpoint.h              Created on: 29 May 2001
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

#ifndef __SSLENDPOINT_H__
#define __SSLENDPOINT_H__

#include <omniORB4/omniServer.h>

OMNI_NAMESPACE_BEGIN(omni)

class sslConnection;

class sslEndpoint : public giopEndpoint,
		    public SocketCollection,
		    public SocketHolder {
public:

  sslEndpoint(const char* param, sslContext* ctx);
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

  ~sslEndpoint();

protected:
  CORBA::Boolean notifyReadable(SocketHolder*);
  // implement SocketCollection::notifyReadable

 private:
  const char*          		   pd_address_param;
  IIOP::Address                    pd_address;
  orbServer::EndpointList          pd_addresses;
  sslContext*                      pd_ctx;

  SocketHandle_t                   pd_new_conn_socket;
  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;
  int                              pd_go;

  sslEndpoint();
  sslEndpoint(const sslEndpoint&);
  sslEndpoint& operator=(const sslEndpoint&);
};


class sslActiveConnection;

class sslActiveCollection : public giopActiveCollection, 
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

  sslActiveCollection();
  ~sslActiveCollection();

  friend class sslActiveConnection;

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

  sslActiveCollection(const sslActiveCollection&);
  sslActiveCollection& operator=(const sslActiveCollection&);
};

OMNI_NAMESPACE_END(omni)

#endif // __SSLENDPOINT_H__
