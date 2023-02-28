// -*- Mode: C++; -*-
//                            Package   : omniORB
// policy.cc                  Created on: 11/5/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2013 Apasphere Ltd
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
//    Implementation of CORBA::Policy.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/objTracker.h>
#include <exceptiondefs.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////// CORBA::Policy ///////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Policy::~Policy() {}


CORBA::PolicyType
CORBA::Policy::policy_type()
{
  return pd_type;
}


CORBA::Policy_ptr
CORBA::Policy::copy()
{
  OMNIORB_ASSERT(_NP_is_nil());
  _CORBA_invoked_nil_pseudo_ref();
  return 0;
}


void
CORBA::Policy::destroy()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();
}


CORBA::Policy_ptr
CORBA::Policy::_duplicate(CORBA::Policy_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


CORBA::Policy_ptr
CORBA::Policy::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) )  return _nil();

  Policy_ptr p = (Policy_ptr) obj->_ptrToObjRef(Policy::_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}


CORBA::Policy_ptr
CORBA::Policy::_nil()
{
  static Policy* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new Policy;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


CORBA::Policy::Policy(CORBA::PolicyType type)
  : pd_refCount(1), pd_type(type)
{
  _PR_setobj((omniObjRef*) 1);
}


CORBA::Policy::Policy()
  : pd_refCount(0), pd_type(0)
{
  _PR_setobj(0);
}


void*
CORBA::Policy::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, CORBA::Policy::_PD_repoId) )
    return (CORBA::Policy_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


void
CORBA::Policy::_NP_incrRefCount()
{
  OMNIORB_ASSERT(!_NP_is_nil());

  omni::poRcLock->lock();
  pd_refCount++;
  omni::poRcLock->unlock();
}


void
CORBA::Policy::_NP_decrRefCount()
{
  omni::poRcLock->lock();
  int done = --pd_refCount > 0;
  omni::poRcLock->unlock();
  if( done )  return;

  OMNIORB_USER_CHECK(pd_refCount == 0);
  // If this fails then the application has released a Policy
  // reference too many times.

  delete this;
}


const char*
CORBA::Policy::_PD_repoId = "IDL:omg.org/CORBA/Policy:1.0";


//////////////////////////////////////////////////////////////////////
///////////////// CORBA::PolicyError user exception //////////////////
//////////////////////////////////////////////////////////////////////

#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the variables external linkage otherwise. Its a bug.
namespace CORBA {

_init_in_def_( const PolicyErrorCode BAD_POLICY               = 0; )
_init_in_def_( const PolicyErrorCode UNSUPPORTED_POLICY       = 1; )
_init_in_def_( const PolicyErrorCode BAD_POLICY_TYPE          = 2; )
_init_in_def_( const PolicyErrorCode BAD_POLICY_VALUE         = 3; )
_init_in_def_( const PolicyErrorCode UNSUPPORTED_POLICY_VALUE = 4; )

}
#else
_init_in_def_( const PolicyErrorCode CORBA::BAD_POLICY               = 0; )
_init_in_def_( const PolicyErrorCode CORBA::UNSUPPORTED_POLICY       = 1; )
_init_in_def_( const PolicyErrorCode CORBA::BAD_POLICY_TYPE          = 2; )
_init_in_def_( const PolicyErrorCode CORBA::BAD_POLICY_VALUE         = 3; )
_init_in_def_( const PolicyErrorCode CORBA::UNSUPPORTED_POLICY_VALUE = 4; )
#endif


OMNIORB_DEFINE_USER_EX_COMMON_FNS(CORBA, PolicyError,
				  "IDL:omg.org/CORBA/PolicyError:1.0")


CORBA::PolicyError::PolicyError(const CORBA::PolicyError& _s) : CORBA::UserException(_s)
{
  reason = _s.reason;

}

CORBA::PolicyError::PolicyError(PolicyErrorCode _reason)
{
  pd_insertToAnyFn    = CORBA::PolicyError::insertToAnyFn;
  pd_insertToAnyFnNCP = CORBA::PolicyError::insertToAnyFnNCP;
  reason = _reason;

}

CORBA::PolicyError& CORBA::PolicyError::operator=(const CORBA::PolicyError& _s)
{
  ((CORBA::UserException*) this)->operator=(_s);
  reason = _s.reason;

  return *this;
}

void
CORBA::
PolicyError::operator>>=(cdrStream& _n) const {
  reason >>= _n;
}

void
CORBA::
PolicyError::operator<<=(cdrStream& _n) {
  (PolicyErrorCode&)reason <<= _n;
}
