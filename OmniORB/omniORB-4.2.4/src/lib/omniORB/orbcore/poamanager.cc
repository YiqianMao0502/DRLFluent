// -*- Mode: C++; -*-
//                            Package   : omniORB
// poamanager.cc              Created on: 12/5/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2005 Apasphere Ltd
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
//    Internal implementation of the PortableServer::POAManager.
//

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <poamanager.h>
#include <poaimpl.h>
#include <poacurrentimpl.h>
#include <exceptiondefs.h>
#include <omniCurrent.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/objTracker.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
///////////////////// PortableServer::POAManager /////////////////////
//////////////////////////////////////////////////////////////////////

OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POAManager,
				       AdapterInactive,
       "IDL:omg.org/PortableServer/POAManager/AdapterInactive" PS_VERSION)


PortableServer::POAManager::~POAManager() {}


PortableServer::POAManager_ptr
PortableServer::POAManager::_duplicate(PortableServer::POAManager_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


PortableServer::POAManager_ptr
PortableServer::POAManager::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) || !obj->_NP_is_pseudo() )  return _nil();

  POAManager_ptr p = (POAManager_ptr) obj->_ptrToObjRef(_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}


PortableServer::POAManager_ptr
PortableServer::POAManager::_nil()
{
  static omniOrbPOAManager* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniOrbPOAManager(1 /* is nil */);
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


const char* PortableServer::POAManager::_PD_repoId =
  "IDL:omg.org/PortableServer/POAManager" PS_VERSION;

//////////////////////////////////////////////////////////////////////
////////////////////////// omniOrbPOAManager /////////////////////////
//////////////////////////////////////////////////////////////////////

#define CHECK_NOT_NIL()  \
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref()


static omni_tracedmutex     pm_lock("pm_lock");
static omni_tracedcondition pm_cond(&pm_lock, "pm_cond");
// Condition variable used to signal deactivations

omniOrbPOAManager::~omniOrbPOAManager() {}


void
omniOrbPOAManager::activate()
{
  CHECK_NOT_NIL();

  omni_tracedmutex_lock sync(pm_lock);

  if( pd_state == INACTIVE )  throw AdapterInactive();
  if( pd_state == ACTIVE   )  return;

  pd_state = ACTIVE;

  for( CORBA::ULong i = 0; i < pd_poas.length(); i++ )
    pd_poas[i]->pm_change_state(pd_state);
}


void
omniOrbPOAManager::hold_requests(CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();

  if( wait_for_completion ) {
    // Complain if in the context of an operation invocation.
    // Fortunately, the spec says we don't have to worry about whether
    // we're in the context of an operation invocation in a POA
    // managed by this POAManager. An invocation on any POA in the
    // same ORB will do.
    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }
  }
  POASeq poas;
  {
    omni_tracedmutex_lock sync(pm_lock);

    if( pd_state == INACTIVE )  throw AdapterInactive();
    if( pd_state == HOLDING  )  return;

    pd_state = HOLDING;
    poas.length(pd_poas.length());

    for( CORBA::ULong i = 0; i < pd_poas.length(); i++ ) {
      pd_poas[i]->pm_change_state(pd_state);
      if( wait_for_completion ) {
	poas[i] = pd_poas[i];
	poas[i]->incrRefCount();
      }
    }
  }

  if( wait_for_completion )
    for( CORBA::ULong i = 0; i < poas.length(); i++ ) {
      poas[i]->pm_waitForReqCmpltnOrSttChnge(HOLDING);
      poas[i]->decrRefCount();
    }
}


void
omniOrbPOAManager::discard_requests(CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();

  if( wait_for_completion ) {
    // Complain if in the context of an operation invocation.
    // Fortunately, the spec says we don't have to worry about whether
    // we're in the context of an operation invocation in a POA
    // managed by this POAManager. An invocation on any POA in the
    // same ORB will do.
    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }
  }
  POASeq poas;
  {
    omni_tracedmutex_lock sync(pm_lock);

    if( pd_state == INACTIVE   )  throw AdapterInactive();
    if( pd_state == DISCARDING )  return;

    pd_state = DISCARDING;
    poas.length(pd_poas.length());

    for( CORBA::ULong i = 0; i < pd_poas.length(); i++ ) {
      pd_poas[i]->pm_change_state(pd_state);
      if( wait_for_completion ) {
	poas[i] = pd_poas[i];
	poas[i]->incrRefCount();
      }
    }
  }

  if( wait_for_completion )
    for( CORBA::ULong i = 0; i < poas.length(); i++ ) {
      poas[i]->pm_waitForReqCmpltnOrSttChnge(DISCARDING);
      poas[i]->decrRefCount();
    }
}


static void
deactivate_thread_fn(void* args)
{
  OMNIORB_ASSERT(args);
  void** targs = (void**) args;

  omniOrbPOAManager::POASeq* ppoas = (omniOrbPOAManager::POASeq*) targs[0];
  omniOrbPOAManager::POASeq& poas = *ppoas;
  CORBA::Boolean etherealise = (CORBA::Boolean) (omni::ptr_arith_t) targs[1];
  int* deactivated = (int*)targs[2];
  delete[] targs;

  for( CORBA::ULong i = 0; i < poas.length(); i++ ) {
    poas[i]->pm_deactivate(etherealise);
    poas[i]->decrRefCount();
  }
  delete ppoas;

  *deactivated = 1;
  pm_cond.broadcast();
}


void
omniOrbPOAManager::deactivate(CORBA::Boolean etherealize_objects,
			      CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();

  if( wait_for_completion ) {
    // Complain if in the context of an operation invocation.
    // Fortunately, the spec says we don't have to worry about whether
    // we're in the context of an operation invocation in a POA
    // managed by this POAManager. An invocation on any POA in the
    // same ORB will do.
    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }
  }
  POASeq* ppoas = new POASeq;
  POASeq& poas = *ppoas;
  {
    omni_tracedmutex_lock sync(pm_lock);

    if( pd_state == INACTIVE ) {
      while( !pd_deactivated )	pm_cond.wait();
      return;
    }

    pd_state = INACTIVE;
    poas.length(pd_poas.length());

    for( CORBA::ULong i = 0; i < pd_poas.length(); i++ ) {
      pd_poas[i]->pm_change_state(pd_state);
      poas[i] = pd_poas[i];
      poas[i]->incrRefCount();
    }
  }

  void** args = new void* [3];
  args[0] = ppoas;

  if (etherealize_objects)
    args[1] = (void*)1;
  else
    args[1] = (void*)0;

  args[2] = &pd_deactivated;

  if( wait_for_completion )
    deactivate_thread_fn(args);
  else
    (new omni_thread(deactivate_thread_fn, args))->start();
}


PortableServer::POAManager::State
omniOrbPOAManager::get_state()
{
  omni_tracedmutex_lock sync(pm_lock);
  return pd_state;
}

////////////////////////////
// Override CORBA::Object //
////////////////////////////

void*
omniOrbPOAManager::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, PortableServer::POAManager::_PD_repoId) )
    return (PortableServer::POAManager_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


void
omniOrbPOAManager::_NP_incrRefCount()
{
  omni::poRcLock->lock();
  pd_refCount++;
  omni::poRcLock->unlock();
}


void
omniOrbPOAManager::_NP_decrRefCount()
{
  omni::poRcLock->lock();
  int done = --pd_refCount > 0;
  omni::poRcLock->unlock();
  if( done )  return;

  OMNIORB_USER_CHECK(pd_poas.length() == 0);
  OMNIORB_USER_CHECK(pd_refCount == 0);
  // If either of these fails then the application has released a
  // POAManager reference too many times.

  delete this;
}

//////////////
// Internal //
//////////////

void
omniOrbPOAManager::gain_poa(omniOrbPOA* poa)
{
  omni_tracedmutex_lock sync(pm_lock);

  pd_poas.length(pd_poas.length() + 1);
  pd_poas[pd_poas.length() - 1] = poa;

  switch( pd_state ) {
  case HOLDING:
    break;
  default:
    poa->pm_change_state(pd_state);
    break;
  }
}


void
omniOrbPOAManager::lose_poa(omniOrbPOA* poa)
{
  omni_tracedmutex_lock sync(pm_lock);

  CORBA::ULong i, len = pd_poas.length();

  for( i = 0; i < len; i++ )
    if( pd_poas[i] == poa )
      break;

  if( i == len )
    throw omniORB::fatalException(__FILE__, __LINE__,
				  "lose_poa(...) for POA I didn't own!");

  for( ; i < len - 1; i++ )
    pd_poas[i] = pd_poas[i + 1];

  pd_poas.length(len - 1);
}
