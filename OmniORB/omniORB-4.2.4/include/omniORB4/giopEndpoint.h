// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopEndpoint.h             Created on: 20 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
//    Copyright (C) 2000      AT&T Laboratories Cambridge
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

#ifndef __GIOPENDPOINT_H__
#define __GIOPENDPOINT_H__

#include <omniORB4/omniutilities.h>

OMNI_NAMESPACE_BEGIN(omni)

class giopActiveConnection;   // Active in the sense that it is created with 
                              // Connect()
class giopConnection;
class giopEndpoint;
class giopActiveCollection;   // Singleton to act on a bunch of 
                              // giopActiveConnection.
class giopServer;
class giopStrand;

class IORPublish;             // Information to publish in IORs

class giopConnection {
public:
  typedef void (*notifyReadable_t)(void* cookie,giopConnection* conn);

  // None of the members raise an exception.

  virtual int Send(void* buf, size_t sz,
		   const omni_time_t& deadline) = 0;

  virtual int Recv(void* buf, size_t sz,
		   const omni_time_t& deadline) = 0;

  virtual void Shutdown() = 0;

  virtual const char* myaddress() = 0;
  virtual const char* peeraddress() = 0;

  virtual const char* peeridentity();
  // Return a string identifying the peer, if appropriate for the
  // connection type. By default returns zero to indicate no peer
  // identification is possible.

  virtual void* peerdetails();
  // Return connection-specific details about the peer. For SSL
  // connections, the void* may be cast to an X509*. Returns zero to
  // indicate no peer details are available. The connection retains
  // ownership of the returned data.


  _CORBA_Boolean gatekeeperCheck(giopStrand* strand);
  // For a passive connection, check if the peer is permitted to
  // connect. Returns true if permitted; false if not.

  virtual _CORBA_Boolean gatekeeperCheckSpecific(giopStrand* strand);
  // Transport-specific additional gatekeeper check. Called by
  // gatekeeperCheck().


  virtual void setSelectable(int now = 0,
			     _CORBA_Boolean data_in_buffer = 0) = 0;
  // Indicates that this connection should be watched by a select()
  // so that any new data arriving on the connection will be noted.
  // If now == 1, immediately make this connection part of the select
  // set (if the platforms allows it).
  // If now == 2, immediately make this connection part of the select
  // set, but only if it is already marked selectable.
  // If data_in_buffer == 1, treat this connection as if there are
  // data available from the connection already.

  virtual void clearSelectable() = 0;
  // Indicates that this connection need not be watched any more.

  virtual _CORBA_Boolean isSelectable() = 0;
  // Returns true if this connection is selectable, false if not. It
  // may not be if the server is very heavily loaded and there are
  // more file descriptors in use than available in a select() fd_set.


  virtual _CORBA_Boolean Peek() = 0;
  // Do nothing and return immediately if the socket has not been set
  // to be watched by a previous setSelectable().  Otherwise, monitor
  // the connection's status for a short time. Return true if it
  // becomes readable, otherwise returns false.

  giopConnection() : pd_refcount(1), pd_dying(0), 
		     pd_has_dedicated_thread(0), 
		     pd_dedicated_thread_in_upcall(0),
                     pd_n_workers(0), pd_max_workers(0),
                     pd_has_hit_n_workers_limit(0) {}

  int decrRefCount(_CORBA_Boolean forced=0);
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock unless forced == 1.

  void incrRefCount();
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock.

  inline int  max_workers()       { return pd_max_workers; }
  inline void max_workers(int mw) { pd_max_workers = mw; }
  // Functions to get/set the maximum number of worker threads that
  // will service this connection. Interceptors can change the value.
  //
  // Thread Safety preconditions:
  //    None. In a serverReceiveRequest interceptor, the thread has
  //    exclusive access to the connection, so modifications are safe.
  //    In other contexts, access is not thread safe.


  friend class giopServer;

protected:
  virtual ~giopConnection() {}

private:

  int            pd_refcount;

  _CORBA_Boolean pd_dying;
  // Initialised to 0. Read and write by giopServer exclusively.

  _CORBA_Boolean pd_has_dedicated_thread;
  // Initialised to 0. Read and write by giopServer exclusively.

  _CORBA_Boolean pd_dedicated_thread_in_upcall;
  // Initialised to 0. Read and write by giopServer exclusively.

  int            pd_n_workers;
  // Initialised to 0. Read and write by giopServer exclusively.

  int            pd_max_workers;
  // Initialised to 0. Read and write by giopServer and interceptors.

  _CORBA_Boolean pd_has_hit_n_workers_limit;
  // Initialised to 0. Read and write by giopServer exclusively.

  giopConnection(const giopConnection&);
  giopConnection& operator=(const giopConnection&);
};


class giopAddress {
public:
  // Each giopAddress must register via decodeIOR interceptor if it
  // wants to decode its own IOR component.

  // None of the members raise an exception.

  static giopAddress* str2Address(const char* address);
  // Given a string, returns an instance that can be used to connect to
  // the address.
  // The format of the address string is as follows:
  //     giop:<transport name>:[<transport specific fields]+
  //     ^^^^^^^^^^^^^^^^^^^^^
  //        transport identifier
  //
  // The format of the following transports are defined (but may not be
  // implemented yet):
  //
  //   giop:tcp:<hostname>:<port no.>
  //   giop:ssl:<hostname>:<port no.>
  //   giop:unix:<filename>
  //   giop:fd:<file no.>
  //
  // Returns 0 if no suitable endpoint can be created.

  virtual const char* type() const = 0;
  // Return the transport identifier, e.g. "giop:tcp","giop:ssl", etc.

  virtual const char* address() const = 0;
  // Return the string that describes this remote address.
  // The string format is described in str2Address().

  virtual const char* host() const;
  // Return a string containing the host name or IP address for the
  // address, if that is meaningful for the address type. Returns 0 if
  // not relevant.

  virtual giopActiveConnection*
  Connect(const omni_time_t& deadline,
	  _CORBA_ULong       strand_flags,
	  _CORBA_Boolean&    timed_out) const = 0;
  // Connect to the remote address.
  // Returns 0 if no connection can be established. Sets timed_out to
  // true if the connection attempt failed due to timeout.

  virtual giopAddress* duplicate() const = 0;
  // Return an identical instance.

  virtual giopAddress* duplicate(const char* host) const;
  // Return an instance that is identical except with a different
  // host. Used when resolving a host name into an address.

  giopAddress() {}
  virtual ~giopAddress() {}

  static giopAddress* fromTcpAddress(const IIOP::Address& addr);
  static giopAddress* fromSslAddress(const IIOP::Address& addr);

private:
  giopAddress(const giopAddress&);
  giopAddress& operator= (const giopAddress&);

};

typedef omnivector<giopAddress*>  giopAddressList;


class giopEndpoint {
public:
  // None of the members raise an exception.

  static giopEndpoint* str2Endpoint(const char* endpoint);
  // Given a string, returns an instance that represent the endpoint
  // The format of an endpoint string is as follows:
  //     giop:<transport name>:[<transport specific fields]+
  //     ^^^^^^^^^^^^^^^^^^^^^
  //        transport identifier
  //
  // The format of the following transports are defined (but may not be
  // implemented yet):
  //
  //   giop:tcp:<hostname>:<port no.>      note 1
  //   giop:ssl:<hostname>:<port no.>      note 1
  //   giop:unix:<filename>
  //   giop:fd:<file no.>
  //
  // Note 1: if <hostname> is empty, the IP address of one of the host
  //         network interfaces will be used.
  //         if <port no.> is not present, a port number is chosen by
  //         the operation system.
  //
  // Returns 0 if no suitable endpoint can be created.

  static _CORBA_Boolean strIsValidEndpoint(const char* endpoint);
  // Return true if endpoint is syntactically correct as described
  // in str2Endpoint(). None of the fields are optional.

  static _CORBA_Boolean addToIOR(const char* endpoint,
                                 IORPublish* eps = 0);
  // Return true if the endpoint has been sucessfully registered so that
  // all IORs generated by the ORB will include this endpoint.

  virtual const char* type() const = 0;
  // return the transport identifier

  virtual const char* address() const = 0;
  // return the string that describes this endpoint.
  // The string format is described in str2Endpoint().

  virtual const orbServer::EndpointList* addresses() const = 0;
  // return all the addresses that can be used to contact this
  // endpoint. There can be more than one in the case of multiple IP
  // addresses, for example.

  virtual _CORBA_Boolean
          publish(const orbServer::PublishSpecs& publish_specs,
                  _CORBA_Boolean                 all_specs,
                  _CORBA_Boolean                 all_eps,
                  orbServer::EndpointList&       published_eps) = 0;
  // Publish endpoints according to the publish_specs. Returns true if
  // the publish specs were understood and handled, false otherwise.

  virtual _CORBA_Boolean Bind() = 0;
  // Establish a binding to the this address.
  // Return TRUE(1) if the binding has been established successfully,
  // otherwise returns FALSE(0).

  virtual giopConnection* 
          AcceptAndMonitor(giopConnection::notifyReadable_t func,
			   void* cookie) = 0;
  // Accept a new connection. Returns 0 if no connection can be accepted.
  // In addition, for all the connections of this endpoint that has been
  // marked, monitors their status.  If data have arrived at a connection,
  // call the callback function <func> with the <cookie> and the pointer to
  // the connection as the arguments.

  virtual void Poke() = 0;
  // Call to unblock any thread blocking in accept().

  virtual void Shutdown() = 0;
  // Remove the binding.

  void           set_no_publish() { pd_no_publish = 1; }
  _CORBA_Boolean no_publish()     { return pd_no_publish; }
  
  giopEndpoint() : pd_no_publish(0) {}
  virtual ~giopEndpoint() {}

private:
  giopEndpoint(const giopEndpoint&);
  giopEndpoint& operator=(const giopEndpoint&);
  _CORBA_Boolean pd_no_publish;
};

typedef omnivector<giopEndpoint*>  giopEndpointList;


class giopActiveConnection {
public:
  // 'Active' in the sense that this is created with Connect().
  virtual giopActiveCollection* registerMonitor() = 0;

  virtual giopConnection& getConnection() = 0;

  // This class could have been written to inherit from giopConnection and
  // with just one abstract function added. The trouble is if we do so,
  // giopConnection must be a public virtual base class. If that is
  // the case, then we cannot simply cast a giopConnection* back to its
  // implementation class. Instead dynamic_cast has to be used. This is not
  // what we want to do.

  giopActiveConnection() {}
  virtual ~giopActiveConnection() {}

private:
  giopActiveConnection(const giopActiveConnection&);
  giopActiveConnection& operator=(const giopActiveConnection&);
};


class giopActiveCollection {
public:
  // A singleton to act on a bunch of giopActiveConnection of the same
  // transport type.

  virtual const char* type() const = 0;

  virtual void Monitor(giopConnection::notifyReadable_t func,void* cookie) = 0;
  // For all the connections that are associated with this singleton and
  // have previously been registered via registerMonitor(), watches their
  // status. If data have arrived at a connection, call the callback
  // function <func> with the <cookie> and the pointer to the connection as
  // the arguments.  This function will only returns when there is no
  // connection to monitor, i.e. all the connections that were registered
  // have been deleted, or when deactivate() is called.

  virtual _CORBA_Boolean isEmpty() const = 0;
  // Returns TRUE(1) if no connections have been added via registerMonitor().

  virtual void deactivate() = 0;
  // Stop monitoring connections.

  giopActiveCollection() {}
  virtual ~giopActiveCollection() {}

private:
  giopActiveCollection(const giopActiveCollection&);
  giopActiveCollection& operator=(const giopActiveCollection&);
};


class giopTransportImpl {
public:

  virtual giopEndpoint* toEndpoint(const char* param) = 0;
  // Returns the endpoint object for this endpoint if it is recognised by
  // this transport.

  virtual giopAddress* toAddress(const char* param) = 0;
  // Returns the address object for this address if it is recognised by
  // this transport.

  virtual _CORBA_Boolean isValid(const char* param) = 0;
  // Returns 1 if the address/endpoint is recognised by this transport

  virtual _CORBA_Boolean addToIOR(const char* param, IORPublish* eps) = 0;
  // Make this endpoint part of the IORs created by this ORB.

  virtual const omnivector<const char*>* getInterfaceAddress() = 0;
  // Get the addresses of all the interfaces that can be used to talk to
  // this host using this transport.

  virtual void initialise();
  // Initialise the transport implementation. Called once the 1st time
  // ORB_init() is called.

  static const omnivector<const char*>* getInterfaceAddress(const char* type);
  // Get the addresses of all the interfaces that belongs to the transport
  // type <type>. These addresses can be used to talk to this host.
  // e.g. type == "giop:tcp" causes the tcp transport implementation to
  // returns the IP address of all the network interfaces of this host.
  // If <type> does not match returns 0.

  static giopTransportImpl* str2Transport(const char* endpoint);
  // Return the giopTransportImpl that matches the given endpoint URI.

  const char*        type;
  giopTransportImpl* next;

  giopTransportImpl(const char* t);
  virtual ~giopTransportImpl();

private:
  giopTransportImpl();
  giopTransportImpl(const giopTransportImpl&);
  giopTransportImpl& operator=(const giopTransportImpl&);
};


OMNI_NAMESPACE_END(omni)

#endif // __GIOPENDPOINT_H__
