// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopStrand.h               Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2012 Apasphere Ltd
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

#ifndef __GIOPSTRAND_H__
#define __GIOPSTRAND_H__

#include <omniORB4/omniTransport.h>
#include "giopStrandFlags.h"

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif


OMNI_NAMESPACE_BEGIN(omni)

class giopStream;
class giopStreamImpl;
class giopWorker;
class giopServer;
class giopCompressor;
class GIOP_S;
struct giopStream_Buffer;


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class giopStreamList {
public:
  giopStreamList* next;
  giopStreamList* prev;

  inline giopStreamList() {
    next = this;
    prev = this;
  }

  inline void insert(giopStreamList& head) {
    next = head.prev->next;
    head.prev->next = this;
    prev = head.prev;
    head.prev = this;
  }

  inline void remove() {
    prev->next = next;
    next->prev = prev;
    next = prev = this;
  }

  static inline CORBA::Boolean is_empty(giopStreamList& head) {
    return (head.next == &head);
  }

private:
  giopStreamList(const giopStreamList&);
  giopStreamList& operator=(const giopStreamList&);
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class giopStrand : public Strand {
public:

  giopStrand(const giopAddress*);
  // Ctor for an active strand. I.e. those that are used to connect to
  // a remote address space.
  // When a connection is establshed, the reference count goes to 1.
  //
  // No thread safety precondition


  giopStrand(giopConnection*,giopServer*);
  // Ctor for a passive strand. I.e. those that are created because a
  // client has connected to this address space.
  // Increment the reference count on the connection.
  //
  // No thread safety precondition


  CORBA::Boolean safeDelete(CORBA::Boolean forced = 0);
  // This should be the *ONLY* method to call to delete a strand.
  // Return TRUE if the strand can be considered deleted.
  //
  // The function checks if this connection is satisfied before it returns
  // true:
  //
  //    giopStreamList::is_empty(clients) &&
  //    giopStreamList::is_empty(servers) &&
  //    giopStream::noLockWaiting(this)
  //
  // If the function returns true, the above condition becomes an invariant
  // and should not be violated until the dtor of the strand is called.
  //
  // The <forced> flag, if set explicitly to 1, causes the function to
  // skip checking for the above condition. Instead it just go ahead as if
  // the condition is met. This flag is used for internal implementation
  // and should not be used by any client of this class.
  //
  // Internally, the strand may stays on a bit longer until the connection's
  // reference count goes to 0 as well.
  //
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock unless forced == 1.

private:
  CORBA::Boolean pd_safelyDeleted;

public:

  CORBA::Boolean deletePending();
  // Return true, if safeDelete() has been called and it returns true.
  //
  // If this method returns true, the following invariant is always true
  // until the strand is deleted: 
  //
  //    giopStreamList::is_empty(clients) &&
  //    giopStreamList::is_empty(servers) &&
  //    giopStream::noLockWaiting(this)
  //
  // The caller must not do anything that would cause the strand to violate
  // this invariant. For example, attempt or wait to acquire a lock on the
  // strand, or to queue any GIOP_S or GIOP_C object to the strand.
  //
  // 
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock.


  GIOP_S* acquireServer(giopWorker*);
  // Acquire a GIOP_S from the strand. Normally this is only  done on
  // passive strands. However, it can also be used for active strands when
  // they become birectional, i.e. BiDir == 1.
  //
  // Return 0 if a GIOP_S cannot be acquired.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  void   releaseServer(IOP_S*);
  // Release the GIOP_S to the strand. The GIOP_S must have been acquired
  // previously through acquireServer from this strand. Passing in a GIOP_S
  // from a different strand would result in undefined behaviour.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.


  enum State { ACTIVE,  // The strand is in active use

	       DYING,   // Something terminally wrong has happened to the
	                // strand, it should be removed at the earliest
	                // convenient time.

	       TIMEDOUT // This strand can still be used when required but
       	                // it can also be removed if resources are scarce.
  };

  State state() const { return pd_state; }
  // No thread safety precondition, use with extreme care

  void state(State s) { pd_state = s; }
  // No thread safety precondition, use with extreme care


  /////////////////////////////////////////////////////////////////////////
  CORBA::Boolean startIdleCounter();
  // returns 1 if the idle counter has been successfully started.
  // returns 0 if the idle counter is already active or has already expired.
  //
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock.

  CORBA::Boolean stopIdleCounter();
  // returns 1 if the idle counter has been successfully stopped.
  // returns 0 if the idle counter has already expired and cannot be reset.
  // In the latter case, the caller should assume that the strand is about
  // to be shutdown and hence should cleanup its usage accordingly.
  //
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock.


  ////////////////////////////////////////////////////////////////////////
  // When idlebeats go to 0, the strand has been idle for a sufficently
  // long time and should be deleted.
  // This variable SHOULD NOT be manipulated outside the implementation of
  // giopStrand.
  CORBA::Long         idlebeats;


  giopStreamList      servers;
  giopStreamList      clients;
  // a strand may have more than one giopStream instance associated with
  // it. Mostly this is because from GIOP 1.2 onwards, requests can be
  // interleaved on associated connection. Each of these request is
  // represented by a giopStream instance. They are linked together by
  // servers and clients.
  //   servers - all the GIOP_S that is serving calls for this strand
  //   clients - all the GIOP_C that is doing invocation using this strand
  //
  // Except when a strand is used to support bidirectional GIOP, only one of
  // the list will be populated (because plain GIOP is asymetric and one
  // end is either a client or a server but not both). With bidirectional GIOP,
  // both lists may be populated.

  inline CORBA::Boolean isClient() { return (address != 0); }
  // Return TRUE if this is an active strand on the client side. Unless
  // biDir is TRUE, only those messages expected by a GIOP client can be
  // received from this connection.

  inline CORBA::Boolean isBiDir() { return (flags & GIOPSTRAND_BIDIR) ? 1 : 0; }
  // Return TRUE if this is a bidirectional strand.

  const giopAddress*  address;
  // address is provided as ctor arg if this is a active strand, otherwise
  // it is 0.

  giopConnection*     connection;
  // connection is provided as ctor arg if this is a passive strand
  // otherwise it is obtained by address->connect().

  giopServer*         server;
  // server is provided as ctor arg if this is a passive strand
  // otherwise it is 0.

  CORBA::ULong        flags;
  // Flags for use by interceptors. See giopStrandFlags.h for
  // allocated flags values.
  // Initialised to 0 in the constructor.

  CORBA::Boolean      gatekeeper_checked;
  // only applies to passive strand. TRUE(1) means that the serverTransportRule
  // has been checked. This flag is set by giopWorker and is
  // not manipulated by the strand class.

  CORBA::Boolean      first_use;
  // only applies to active strand. TRUE(1) means this connection has
  // not been used for any purpose before.
  // This flag is set to 1 by ctor and reset to 0 by GIOP_C.

  CORBA::Boolean      first_call;
  // only applies to active strand. TRUE(1) means this connection has
  // not yet been used to start a normal invocation. It may have been
  // used for other purposes, e.g. a locate request.
  // This flag is set to 1 by ctor and reset to 0 by GIOP_C.

  CORBA::Boolean      orderly_closed;
  // only applies to active strand. TRUE(1) means a GIOP CloseConnection
  // was received and cause the strand state to change to DYING.
  // This flag is set to 0 by ctor and set to 1 by the giopImpl?? classes.


  CORBA::Boolean      biDir_initiated;
  // only applies to active strand. TRUE(1) means biDir service context
  // has been sent for this connection.
  // This flag is initialised to 0 by ctor and set to 1 by 
  // setBiDirServiceContext.

  CORBA::Boolean      biDir_has_callbacks;
  // only applies to active bidirectional strand. TRUE(1) means call back
  // objects have been sent through this connection. In other words, the
  // connection may receive invocations on these objects from the other
  // end.
  // This flag is initialised to 0 by ctor and set to 1 by 
  // omniObjRef::_marshal.


  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  // The following are data structures used by the giopStream instances
  // associated with this strand AND SHOULD NOT BE manipulated by the Strand
  // class!!!

  CORBA::Boolean      tcs_selected;
  omniCodeSet::TCS_C* tcs_c;
  omniCodeSet::TCS_W* tcs_w;
  GIOP::Version       version;
  giopStreamImpl*     giopImpl;
  // The transmission codesets for char, wchar, string and wstring are
  // selected by the client based on the codeset info in the IOR. The
  // client informs the server of its selection using a codeset service
  // context. This is done only once per lifetime of a connection (strand).
  // If <tcs_selected> == 1,
  //   <tcs_c>, <tcs_w> and <version> records the chosen code set convertors
  //   and the GIOP version for which the convertors apply.

  giopCompressor*     compressor;
  // Compressor used for ZIOP.

  // Condition variables and counters to implement giopStream locking
  // functions.
  omni_tracedcondition rdcond;
  int                  rd_nwaiting;
  int                  rd_n_justwaiting;
  omni_tracedcondition wrcond;
  int                  wr_nwaiting;


  CORBA::ULong newSeqNumber();
  // Return a number suitable for use as the GIOP request id.
  //
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock.

private:
  CORBA::ULong         seqNumber;
  // monotonically increasing number to be used as the GIOP request id.


public:
  giopStream_Buffer*   head;
  giopStream_Buffer*   spare;

public:
  static _core_attr StrandList  active;
  static _core_attr StrandList  active_timedout;
  static _core_attr StrandList  passive;
  // Throughout the lifetime of a strand, it is a member of one and only one
  // of the lists:
  //   active           - the ORB uses this connection in the role of a client
  //                      it is 'active' in the sense that the connection was
  //                      initiated by this ORB
  //   active_timedout  - the connection was previously active and has been
  //                      idled for some time. It will be deleted soon.
  //   passive          - the ORB uses this connection in the role of a server
  //                      it is 'passive' because the connection was initiated
  //                      by the remote party.
  //


  static _core_attr CORBA::ULong idleOutgoingBeats;
  // Number to instantiate idlebeats when the active strand becomes idle.

  static _core_attr CORBA::ULong idleIncomingBeats;
  // idleIncomingBeats * scanPeriod == no. of sec. a passive strand should
  // be allowed to stay idle.

public:
  void deleteStrandAndConnection(CORBA::Boolean forced=0);
  // Decrement connection's reference count. If it goes to 0, delete
  // this strand as well. This call ensures that both the strand and
  // connection die at the same time.
  //
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock unless forced == 1.

#ifdef __GNUG__
  friend class keep_gcc_happy;
#endif

private:
  virtual ~giopStrand();

  State pd_state;

  giopStrand(const giopStrand&);
  giopStrand& operator=(const giopStrand&);

};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __GIOPSTRAND_H__
