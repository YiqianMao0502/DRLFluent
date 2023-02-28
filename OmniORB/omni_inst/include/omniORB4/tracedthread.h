// -*- Mode: C++; -*-
//                            Package   : omniORB2
// tracedthread.h             Created on: 15/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2010-2017 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//    omni_thread style mutex and condition variables with checks.
//

#ifndef __OMNI_TRACEDTHREAD_H__
#define __OMNI_TRACEDTHREAD_H__


// Lock tracing is controlled by autoconf, or in the dummy
// omniconfig.h.  You can override it here if you wish.

//#define OMNIORB_ENABLE_LOCK_TRACES


//////////////////////////////////////////////////////////////////////
// omniORB locks and partial lock order
//
//
// BiDirClientRope::pd_lock
//   Protects bidir state in rope
//   Before omniTransportLock.
//   Before giopServer::pd_lock.
//
// corbaBoa boa_lock
//   Protects BOA refcount, state.
//   Before omni::internalLock.
//   Before omniObjAdapter oa_lock.
//
// corbaOrb orb_lock
//   Protects CORBA::ORB existence and state.
//   Synchronises ORB::run().
//   Held while calling module attach() and detach().
//   Before omniTransportLock.
//   Before omni::poRcLock.
//
// giopServer::pd_lock
//   Protects server state
//   Before omniTransportLock.
//   Before SocketCollection::pd_collection_lock.
//
// omni::internalLock
//   Protects many parts of omniORB internal state including object table.
//   Before omniServantActivatorTaskQueue::pd_queue_lock.
//   Before omni::objref_rc_lock.
//   Before omniIOR::lock
//
// omniServantActivatorTaskQueue::pd_task_lock
//   Serialises calls to ServantActivator methods.
//   Before omni::internalLock.
//   Before omniObjAdapter::sd_detachedObjectLock.
//   Calls into application-provided incarnate / etherealize.
//
// omniTransportLock
//   Protects connection state, ropes, strands.
//   Before SocketCollection::pd_collection_lock.
//
// omniObjAdapter oa_lock
//   Protects object adapters, incoming endpoints.
//   Before giopServer::pd_lock.
//
// omniObjAdapter::sd_detachedObjectLock
//   Handles count of detached objects in object adapter.
//   After all other locks, but caller should not hold omni::internalLock.
//
// omniOrbPOA::pd_lock
//   Protects POA's state.
//   Before omni::internalLock.
//   Before omni::poRcLock.
//
// poa poa_lock
//   Protects collection of POAs.
//   Before omniOrbPOA::pd_lock.
//   Before omniObjAdapter oa_lock.
//   Before poamanager pm_lock.
//   Before omni::poRcLock.
//
// poamanager pm_lock
//   Protects POAManager state.
//   Before omni::poRcLock.
//
// sslActiveCollection::pd_lock
//   Protects socket count in sslActiveCollection.
//   Before SocketCollection::pd_collection_lock
//
// tcpActiveCollection::pd_lock
//   Protects socket count in tcpActiveCollection.
//   Before SocketCollection::pd_collection_lock
//
// unixActiveCollection::pd_lock
//   Protects socket count in unixActiveCollection.
//   Before SocketCollection::pd_collection_lock
//
//   
//////////////////////////////////////////////////////////////////////
//
// Locks that are after all others in the partial order. No locks are
// acquired while holding these; lock ordering above does not
// necessarily list these.
//
// anyLock
//   Protects pointers inside Any.
//
// ContextImpl::pd_lock
//   Protects Context.
//
// DynAnyImplBase::refCountLock
//   Protects DynAny implementation ref count and existence.
//
// giopStream::dumpbuf::lock
//   Ensures only one thread is dumping a buffer at a time.
//
// initRefs ba_lock
//   Protects bootstrap agent.
//
// initRefs sl_lock
//   Protects lists of initial reference services.
//
// libcWrapper rand_lock
//   Serialises call to rand() on platforms without rand_r().
//
// omni::objref_rc_lock
//   Protects normal object reference refcounts.
//
// omni::poRcLock
//   Pseudo object reference count lock.
//
// omniAsyncCallDescriptor::sd_lock
//   Protects state of async calls.
//
// omniAsyncInvoker::pd_lock
//   Protects invoker state.
//
// omniCompressionManager::pd_lock
//   Protects omniCompressionManager refcount.
//
// omniExHandlers::TableLock
//   Protects omniExHandlers::Table.
//
// omniInternal nil_ref_lock
//   Protects creation of nil objref singletons.
//
// omniIOR::lock
//   Protects omniIOR refcount and IORInfo pointer.
//
// omniOrbPOA::pd_main_thread_sync.mu
//   Used in Main Thread dispatch.
//
// omniServantActivatorTaskQueue::pd_queue_lock
//   Protects ServantActivator queue.
//
// orbMultiRequest::q_lock
//   Protects queue for multiple request handling.
//
// poa generateUniqueId lock
//   Protects id generation.
// 
// poa RemoveRefTask::pd_mu
//   Used to remove servant reference from separate thread in main thread POAs.
//
// proxyObjectFactory::ofl_mutex
//   Protects proxyObjectFactory table.
//
// Scavenger::mutex
//   Protects lifetime of the connection scavenger.
//
// SocketCollection::pd_collection_lock
//   Protects sets of sockets.
//
// TypeCode aliasExpandedTc_lock
//   Protects TypeCode's alias expanded version.
//
// TypeCode::refcount_lock
//   Protects reference count and internal structure of TypeCodes.
//
// valueFactoryTableTracker::vf_lock
//   Protects valuefactory table.
//
// zlibCompressorFactory::pd_lock
//   Protects zlibCompressorFactory refcount.
//
// zlibCompressor::pd_lock
//   Protects zlibCompressor refcount.
//


//////////////////////////////////////////////////////////////////////
////////////////////////// omni_tracedmutex //////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef OMNIORB_ENABLE_LOCK_TRACES

#define ASSERT_OMNI_TRACEDMUTEX_HELD(m, yes)

class omni_tracedmutex : public omni_mutex {
public:
  inline omni_tracedmutex(const char* /*name*/=0) : omni_mutex() {}
};

typedef omni_mutex_lock omni_tracedmutex_lock;

class omni_tracedcondition : public omni_condition {
public:
  inline omni_tracedcondition(omni_tracedmutex* m, const char* /*name*/=0)
    : omni_condition(m) {}
};

#else

class omni_tracedcondition;


class omni_tracedmutex {
public:
  omni_tracedmutex(const char* name=0);
  ~omni_tracedmutex();

  void lock();
  void unlock();
  inline void acquire(void) { lock();   }
  inline void release(void) { unlock(); }

  void assert_held(const char* file, int line, int yes);

private:
  friend class omni_tracedcondition;

  omni_tracedmutex(const omni_tracedmutex&);
  omni_tracedmutex& operator=(const omni_tracedmutex&);

  omni_mutex     pd_lock;    // protects other members
  omni_condition pd_cond;    // so can wait for mutex to unlock
  omni_thread*   pd_holder;  // the thread holding pd_m, or 0
  int            pd_n_conds; // number of dependent condition vars
  int            pd_deleted; // set true on deletion, may catch later use
  char*          pd_logname; // name to use for logging
};

//////////////////////////////////////////////////////////////////////
//////////////////////// omni_tracedcondition ////////////////////////
//////////////////////////////////////////////////////////////////////

class omni_tracedcondition {
public:
  omni_tracedcondition(omni_tracedmutex* m, const char* name = 0);
  ~omni_tracedcondition();

  void wait();
  int timedwait(unsigned long secs, unsigned long nanosecs = 0);
  inline int timedwait(const omni_time_t& t) { return timedwait(t.s, t.ns); }
  void signal();
  void broadcast();

private:
  omni_tracedcondition(const omni_tracedcondition&);
  omni_tracedcondition& operator=(const omni_tracedcondition&);

  omni_tracedmutex& pd_mutex;
  omni_condition    pd_cond;
  int               pd_n_waiters;
  int               pd_deleted;
  char*             pd_logname;
};

//////////////////////////////////////////////////////////////////////
//////////////////////// omni_tracedmutex_lock ///////////////////////
//////////////////////////////////////////////////////////////////////

class omni_tracedmutex_lock {
public:
  inline omni_tracedmutex_lock(omni_tracedmutex& m) :pd_m(m) { m.lock(); }
  inline ~omni_tracedmutex_lock() { pd_m.unlock(); }

private:
  omni_tracedmutex_lock(const omni_tracedmutex_lock&);
  omni_tracedmutex_lock& operator = (const omni_tracedmutex_lock&);

  omni_tracedmutex& pd_m;
};


#define ASSERT_OMNI_TRACEDMUTEX_HELD(m, yes)  \
  (m).assert_held(__FILE__, __LINE__, (yes))

// #ifndef OMNIORB_ENABLE_LOCK_TRACES
#endif


//////////////////////////////////////////////////////////////////////
///////////////////////// omni_optional_lock /////////////////////////
//////////////////////////////////////////////////////////////////////

class omni_optional_lock {
public:
  inline omni_optional_lock(omni_tracedmutex& m, int locked,
			    int locked_on_exit)
    : pd_locked(locked_on_exit), pd_m(m)
    { if( !locked ) pd_m.lock(); }

  inline ~omni_optional_lock() { if( !pd_locked )  pd_m.unlock(); }

private:
  omni_optional_lock(const omni_optional_lock&);
  omni_optional_lock& operator = (const omni_optional_lock&);

  int               pd_locked;
  omni_tracedmutex& pd_m;
};


//////////////////////////////////////////////////////////////////////
//////////////////////// omni_tracedmutex_unlock /////////////////////
//////////////////////////////////////////////////////////////////////

class omni_tracedmutex_unlock {
public:
  inline omni_tracedmutex_unlock(omni_tracedmutex& m) : pd_m(m) { m.unlock(); }
  inline ~omni_tracedmutex_unlock() { pd_m.lock(); }

private:
  omni_tracedmutex_unlock(const omni_tracedmutex_unlock&);
  omni_tracedmutex_unlock& operator=(const omni_tracedmutex_unlock&);

  omni_tracedmutex& pd_m;
};


#endif  // __OMNITRACEDTHREAD_H__
