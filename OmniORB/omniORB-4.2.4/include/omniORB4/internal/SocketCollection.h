// -*- Mode: C++; -*-
//                            Package   : omniORB
// SocketCollection.h         Created on: 23 Jul 2001
//                            Author    : Sai Lai Lo (sll)
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2005-2013 Apasphere Ltd.
//    Copyright (C) 2001      AT&T Laboratories Cambridge
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

#ifndef __SOCKETCOLLECTION_H__
#define __SOCKETCOLLECTION_H__

#include <tcpSocket.h>


OMNI_NAMESPACE_BEGIN(omni)

class SocketCollection;


//
// Class SocketHolder holds a socket inside a collection. It contains
// flags to indicate whether the socket is selectable, and so on.
// Connection classes (e.g. tcpConnection) derive from this class.

class SocketHolder {

public:
  SocketHolder(SocketHandle_t s)
    : pd_socket(s),
      pd_belong_to(0),
      pd_shutdown(0),
      pd_selectable(0),
      pd_data_in_buffer(0),
      pd_peeking(0),
      pd_peek_go(0),
      pd_nonblocking(0),
      pd_peek_cond(0),
      pd_fd_index(-1),
      pd_next(0),
      pd_prev(0) { }

  virtual ~SocketHolder();

  void setSelectable(int now,
		     CORBA::Boolean data_in_buffer,
		     CORBA::Boolean deprecated_hold_lock=0);
  // Indicate that this socket should be watched for readability.
  //
  // If now is 1, immediately make the socket selectable (if the
  // platform permits it), rather than waiting until the select loop
  // rescans.
  //
  // If now is 2, immediately make the socket selectable (if the
  // platform permits it), but only if it is already marked
  // selectable.
  //
  // If data_in_buffer is true, the socket is considered to already
  // have data available to read.
  //
  // deprecated_hold_lock used to be used to indicate that the caller
  // already held the associated SocketCollection's lock during a
  // notifyReadable callback. The lock is no longer held in callbacks,
  // but the parameter is retained for backwards compatibility.

  void clearSelectable();
  // Indicate that this socket should not be watched any more.

  CORBA::Boolean Peek();
  // Watch the socket for a while to see if any data arrives. If the
  // socket is not already selectable, wait for a bit in case it
  // becomes selectable. Mark the socket as no longer selectable and
  // return true if the socket becomes readable, otherwise return
  // false.

  inline void
  setBlocking()
  {
    if (pd_nonblocking) {
      tcpSocket::setBlocking(pd_socket);
      pd_nonblocking = 0;
    }
  }

  inline void
  setNonBlocking()
  {
    if (!pd_nonblocking) {
      tcpSocket::setNonBlocking(pd_socket);
      pd_nonblocking = 1;
    }
  }


  friend class SocketCollection;

protected:
  SocketHandle_t       	pd_socket;
  SocketCollection*    	pd_belong_to;
  CORBA::Boolean       	pd_shutdown;

private:
  CORBA::Boolean       	pd_selectable;     // True if socket is selectable
  CORBA::Boolean       	pd_data_in_buffer; // True if data already available
  CORBA::Boolean       	pd_peeking;        // True if a thread is currently
					   // peeking
  CORBA::Boolean        pd_peek_go;        // True if the peeking thread
					   // should return true, even if it
					   // did not see data to read
  CORBA::Boolean        pd_nonblocking;    // True if the socket is nonblocking
  omni_tracedcondition* pd_peek_cond;      // Condition to signal a waiting
					   // peeker
  int                  	pd_fd_index;       // -1 if select thread is not
					   // watching; otherwise, index of
					   // the fd within the poll / select
					   // list.
  SocketHolder*        	pd_next;
  SocketHolder**       	pd_prev;
};

typedef omnivector<SocketHolder*> SocketHolderVec;


//
// SocketCollection manages a collection of sockets.

class SocketCollection {
public:

  SocketCollection();

  virtual ~SocketCollection();

  virtual CORBA::Boolean notifyReadable(SocketHolder*) = 0;
  // Callback used by Select(). If it returns false, something has
  // gone very wrong with the collection and exits the Select loop.
  // No locks are held during the call.

  CORBA::Boolean isSelectable(SocketHandle_t sock);
  // Indicates whether the given socket can be selected upon.

  CORBA::Boolean Select();
  // Returns true if the Select() has successfully done a scan.
  // otherwise returns false to indicate that an error has been
  // detected and this function should not be called again.
  //
  // For each of the sockets that has been marked watchable and indeed
  // has become readable, call notifyReadable() with the socket as the
  // argument.

  void wakeUp();
  // On platforms where is is possible, immediately wake up a thread
  // blocked in Select().

  void incrRefCount();
  void decrRefCount();

  void addSocket(SocketHolder* sock);
  // Add this socket to the collection. Increments this collection's
  // refcount.

  void removeSocket(SocketHolder* sock);
  // Remove the socket from this collection. Returns the socket which
  // has been removed. Decrements this collection's refcount.

  static omni_time_t   scan_interval;
  static unsigned      idle_scans;

private:
  int                  pd_refcount;
  omni_tracedmutex     pd_collection_lock;

  // Absolute time at which we scan through the socket list choosing
  // the selectable ones.
  omni_time_t          pd_abs_time;

  // On platforms that support it, we use a pipe to wake up the select.
  int                  pd_pipe_read;
  int                  pd_pipe_write;
  CORBA::Boolean       pd_pipe_full;

  // On platforms that support pipes, after scanning a while with no
  // activity, we poll / select with an infinite timeout to prevent
  // unnecessary processing.
  unsigned             pd_idle_count;

#if defined(USE_POLL)
  // On platforms where we use poll(), we maintain a pre-allocated
  // array of pollfd structures and a parallel array of pointers to
  // SocketHolders. pd_pollfd_n is the number of populated pollfds.
  // pd_pollfd_len is the current length of both arrays.
  struct pollfd*       pd_pollfds;
  SocketHolder**       pd_pollsockets;
  unsigned             pd_pollfd_n;
  unsigned             pd_pollfd_len;

  void growPollLists();
  // Expand the pd_pollfds and pd_pollsockets to fit more values.

#elif defined(__WIN32__)
  // Windows has select() but its fd_sets are more like pollfds, just
  // less convenient...
  omni_tracedcondition pd_select_cond; // timedwait on if nothing to select
  fd_set               pd_fd_set;
  SocketHolder*        pd_fd_sockets[FD_SETSIZE];
  
#else
  // On other platforms we use select(). We maintain an fd_set and the
  // highest socket number set in it plus 1.
  fd_set               pd_fd_set;
  int                  pd_fd_set_n;
#endif

  // Linked list of registered sockets.
  SocketHolder*        pd_collection;
  CORBA::Boolean       pd_changed;

  inline void sendNotifications(SocketHolderVec& to_notify)
  {
    SocketHolderVec::iterator it  = to_notify.begin();
    SocketHolderVec::iterator end = to_notify.end();
  
    for (; it != end; ++it)
      notifyReadable(*it);

    to_notify.clear();
  }
  
  friend class SocketHolder;
};

OMNI_NAMESPACE_END(omni)

#endif // __SOCKETCOLLECTION_H__
