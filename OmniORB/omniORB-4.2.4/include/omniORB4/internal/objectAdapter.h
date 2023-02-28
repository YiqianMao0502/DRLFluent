// -*- Mode: C++; -*-
//                            Package   : omniORB2
// objectAdapter.h            Created on: 5/3/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
//    Copyright (C) 1996,1999 AT&T Research Cambridge
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
//

#ifndef __OMNI_OBJECTADAPTER_H__
#define __OMNI_OBJECTADAPTER_H__

#include <omniORB4/omniutilities.h>
#include <omniORB4/omniServer.h>

#ifndef OMNIORB_USEHOSTNAME_VAR
#define OMNIORB_USEHOSTNAME_VAR "OMNIORB_USEHOSTNAME"
#endif

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

class omniCallDescriptor;
class omniCallHandle;
class omniLocalIdentity;

OMNI_NAMESPACE_BEGIN(omni)

class Rope;

class omniObjAdapter {
public:
  virtual ~omniObjAdapter();

  static omniObjAdapter* getAdapter(const _CORBA_Octet* key, int keysize);
  // Partially decode the key and locate the required object adapter,
  // instantiating it if necassary and possible.  If the required adapter
  // does not exist, returns 0.
  //  Must be called without any ORB locks held.

  static _CORBA_Boolean isInitialised();
  static _CORBA_Boolean isDeactivating();

  static void initialise();
  // Initialises incoming rope factories.  May be called multiple
  // times - subsequent calls are ignored (unless shutdown() is
  // called).
  //  This function is thread safe.
  //  May throw exceptions if incoming rope factories fail to
  // initialise.

  static void shutdown();
  // Destroys incoming rope factories.  Stops the incoming scavenger.
  // All object adapters should have already been deactivated before
  // this is called.  However, does nothing if we have not already
  // been initialised.
  //  This function is thread safe.

  static _CORBA_Boolean endpointInList(const char* ep,
				       const orbServer::EndpointList& eps);
  // Returns true if the endpoint is in the list.

  void adapterActive();
  void adapterInactive();
  // These two are used to ensure that connections are only accepted
  // once an adapter is interested, and will not be accepted as soon
  // as the last adapter has finished being interested.
  //  These methods may throw exceptions if any of the rope factories
  // has problems.

  void waitForActiveRequestsToComplete(int locked);
  //  <locked> =>> hold <omni::internalLock>.

  void waitForAllRequestsToComplete(int locked);
  //  <locked> =>> hold <omni::internalLock>.

  virtual void incrRefCount() = 0;
  virtual void decrRefCount() = 0;
  // Locking rules are object adapter specific.

  virtual void dispatch(omniCallHandle&, omniLocalIdentity*) = 0;
  // Dispatch request to given object.

  virtual void dispatch(omniCallHandle&,
			const _CORBA_Octet* key, int keysize) = 0;
  // Dispatch request to object with given key (which is not
  // in the active object map).

  virtual void dispatch(omniCallDescriptor&, omniLocalIdentity*) = 0;
  // Dispatch a local request.

  virtual int objectExists(const _CORBA_Octet* key, int keysize) = 0;
  // This is only called for objects which are not in the active
  // object map.  Should return true if the adapter is able to
  // activate such an object on demand, otherwise it returns false.
  //  Alternatively it may throw an omniORB::LOCATION_FORWARD
  // exception, giving an alternative location for the object.

  inline void enterAdapter() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    OMNIORB_ASSERT(pd_nReqInThis >= 0);
    pd_nReqInThis++;
  }
  // Adapter calls this when a request enters the adapter.

  inline void startRequest() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    OMNIORB_ASSERT(pd_nReqActive >= 0);
    pd_nReqActive++;
  }
  // Adapter calls this when it is committed to making an
  // upcall into the implementation, or is about to exit
  // from the adapter (either normally or by throwing
  // an exception).

  inline void leaveAdapter() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    OMNIORB_ASSERT(pd_nReqInThis > 0);  OMNIORB_ASSERT(pd_nReqActive > 0);
    --pd_nReqInThis;
    --pd_nReqActive;
    if( !pd_nReqActive && pd_signalOnZeroInvocations )
      pd_signal->broadcast();
  }
  // This is called by the localIdentity when the request
  // leaves the adapter.

  inline void exitAdapter(int loentry=0, int loexit=0) {
    if( !loentry )  omni::internalLock->lock();
    OMNIORB_ASSERT(pd_nReqInThis > 0);
    --pd_nReqInThis;
    int do_signal = !pd_nReqInThis && pd_signalOnZeroInvocations;
    if( !loexit )  omni::internalLock->unlock();
    if( do_signal )  pd_signal->broadcast();
  }

  inline void detached_object() {
    sd_detachedObjectLock.lock();
    pd_nDetachedObjects++;
    sd_detachedObjectLock.unlock();
  }

  void met_detached_object();
  // Decrements <pd_nDetachedObjects>.  Another thread can
  // rendezvous with this going to zero using
  // wait_for_detached_objects() below.
  //  Must not hold <omni::internalLock>.

  void wait_for_detached_objects();
  // Waits until all detached objects have been 'met'.
  //  Must not hold <omni::internalLock>.

  virtual void lastInvocationHasCompleted(omniLocalIdentity* id) = 0;
  // Called when an object which has been deactivated is idle.
  // This may happen when the object is deactivated (in which
  // case the object adapter calls this), or when any outstanding
  // method invocations complete (in which case the
  // omniLocalIdentity calls this).
  //  The caller must hold <omni::internalLock> on entry. It is
  //  released on exit.

  virtual void* _ptrToClass(int* cptr);
  static inline omniObjAdapter* _downcast(omniObjAdapter* a) {
    return a ? (omniObjAdapter*)a->_ptrToClass(&_classid) : 0;
  }
  static _core_attr int _classid;


  ////////////////////
  // Incoming       //
  ////////////////////

  static _CORBA_Boolean matchMyEndpoints(const char*);
  // Returns true(1) if the argument is the address of one of my endpoints
  // returns false(0) otherwise.

  static const orbServer::EndpointList& listMyEndpoints();

  struct Options {
    inline Options() : publish_all(0) {}
    ~Options();

    struct EndpointURI {
      _CORBA_String_var uri;
      _CORBA_Boolean    no_publish;
    };
    typedef omnivector<EndpointURI*> EndpointURIList;
    EndpointURIList   endpoints;
    _CORBA_String_var publish;

    // Backwards-compatibility:
    EndpointURIList   no_listen;
    CORBA::Boolean    publish_all;
  };

  static _core_attr Options options;


protected:
  omniObjAdapter(int is_nil=0);

  void adapterDestroyed();
  // Called by derived adapter classes when the adapter has been destroyed.

  int                  pd_nReqInThis;
  // The number of requests active in this adapter.
  //  Protected by <omni::internalLock>.

  int                  pd_nReqActive;
  // The number of requests actually in progress in this
  // adapter.
  //  Protected by <omni::internalLock>.

  int                  pd_signalOnZeroInvocations;
  // If true then we broadcast on <pd_signal> when there are
  // no more invocations in progress.  Each party waiting for
  // this event increments this value, and decrements it
  // when done.
  //  Protected by <omni::internalLock>.

  omni_tracedcondition* pd_signal;
  // Uses <omni::internalLock> as mutex.
  //  Used to signal changes to:
  //     pd_nReqInThis,
  //     pd_nReqActive,
  //  Used in POA for changes to:
  //     pd_rq_state

private:
  static omni_tracedmutex     sd_detachedObjectLock;
  static omni_tracedcondition sd_detachedObjectSignal;

  int pd_nDetachedObjects;
  // The number of objects which have been placed deactivated,
  // but have not yet been etherealised.
  //  Protected by <sd_detachedObjectLock>.

  int pd_signalOnZeroDetachedObjects;
  // This should be incremented to indicate that someone is
  // interested in the completion of etherealisation of all
  // objects which have been deactivated.  If this is non zero
  // when that event occurs, <pd_signal> will be broadcast to.
  //  Protected by <sd_detachedObjectLock>.

  int pd_isActive;
  // True if this adapter is interested in accepting incoming
  // connections.
  //  Protected by <oa_lock>.
};


class omniObjAdapter_var {
public:
  typedef omniObjAdapter_var T_var;
  typedef omniObjAdapter*    T_ptr;

  inline omniObjAdapter_var() : pd_val(0) {}
  inline omniObjAdapter_var(T_ptr p) : pd_val(p) {}
  inline ~omniObjAdapter_var() { if( pd_val )  pd_val->decrRefCount(); }

  inline T_var& operator = (T_ptr p) {
    OMNIORB_ASSERT(!pd_val);
    pd_val = p;
    return *this;
  }

  inline T_ptr operator -> () { return pd_val; }
  inline operator T_ptr () { return pd_val; }

private:
  omniObjAdapter_var(const T_var&);
  T_var& operator = (const T_var&);

  T_ptr pd_val;
};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __OMNI_OBJECTADAPTER_H__
