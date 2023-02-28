// -*- Mode: C++; -*-
//                            Package   : omniORB
// corbaObject.cc             Created on: 13/5/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
// Description:
//    Implementation of the CORBA::Object interface.
//      
 
#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/minorCode.h>
#include <omniORB4/omniObjRef.h>
#include <omniORB4/objTracker.h>
#include <objectAdapter.h>
#include <anonObject.h>
#include <exceptiondefs.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////// CORBA::Object ///////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Object::~Object()
{
  pd_magic = 0;
}


CORBA::Boolean
CORBA::Object::_is_a(const char* repoId)
{
  if( !repoId )  return 0;

  if( _NP_is_pseudo() )  return _ptrToObjRef(repoId) ? 1 : 0;

  if( _NP_is_nil() ) {
    if( omni::strMatch(repoId, "") )  return 1;
    else                              return 0;
  }
  else {
    return pd_obj->_real_is_a(repoId);
  }
}


CORBA::Boolean
CORBA::Object::_non_existent()
{
  if ( !_PR_is_valid(this) )  OMNIORB_THROW(BAD_PARAM,
					    BAD_PARAM_InvalidObjectRef,
					    CORBA::COMPLETED_NO);

  if( _NP_is_nil()    )  return 1;
  if( _NP_is_pseudo() )  return 0;

  try {
    return pd_obj->_remote_non_existent();
  }
  catch(CORBA::OBJECT_NOT_EXIST&) {
    return 1;
  }
}


CORBA::Boolean
CORBA::Object::_is_equivalent(CORBA::Object_ptr other_object)
{
  if ( !_PR_is_valid(this) )  OMNIORB_THROW(BAD_PARAM,
					    BAD_PARAM_InvalidObjectRef,
					    CORBA::COMPLETED_NO);


  if ( !_PR_is_valid(other_object) ) 
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidObjectRef,
		  CORBA::COMPLETED_NO);


  if( other_object == this )  return 1;

  // Pseudo objects are equivalent only if pointers are equal.
  // (So the above test should have gotten it).
  if( _NP_is_pseudo() )
    return 0;

  if( other_object->_NP_is_nil() ) {
    return _NP_is_nil();
  }
  else {
    if( _NP_is_nil() )  return 0;

    omniObjRef* objptr = _PR_getobj();
    omniObjRef* other_objptr = other_object->_PR_getobj();

    return objptr->__is_equivalent(other_objptr);
  }
}


CORBA::ULong
CORBA::Object::_hash(CORBA::ULong maximum)
{
  if( _NP_is_nil() || maximum == 0 )  return 0;

  if( _NP_is_pseudo() )
    return CORBA::ULong((omni::ptr_arith_t) this) % maximum;

  return _PR_getobj()->__hash(maximum);
}


CORBA::Object_ptr 
CORBA::Object::_duplicate(CORBA::Object_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


CORBA::Object_ptr
CORBA::Object::_nil()
{
  static CORBA::Object* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new CORBA::Object;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


void
CORBA::Object::_NP_incrRefCount()
{
  OMNIORB_ASSERT(pd_obj);  OMNIORB_ASSERT(!_NP_is_pseudo());

  omni::duplicateObjRef(pd_obj);
}


void
CORBA::Object::_NP_decrRefCount()
{
  OMNIORB_ASSERT(pd_obj);  OMNIORB_ASSERT(!_NP_is_pseudo());

  omni::releaseObjRef(pd_obj);
}


void*
CORBA::Object::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


void
CORBA::
Object::_marshalObjRef(CORBA::Object_ptr obj, cdrStream& s)
{
  if (obj->_NP_is_pseudo()) OMNIORB_THROW(MARSHAL,MARSHAL_LocalObject,
					  (CORBA::CompletionStatus)s.completion());
  omniObjRef::_marshal(obj->_PR_getobj(),s);
}


CORBA::Object_ptr
CORBA::Object::_unmarshalObjRef(cdrStream& s)
{
  omniObjRef* o = omniObjRef::_unMarshal(_PD_repoId,s);
  if (o)
    return (CORBA::Object_ptr)o->_ptrToObjRef(_PD_repoId);
  else
    return _nil();
}



const char*
CORBA::Object::_PD_repoId = "IDL:omg.org/CORBA/Object:1.0";


// We put this here rather than in anonObject.cc to ensure that
// it is always linked into the application.
static const omniAnonObjRef_pof _theomniAnonObjRef_pof;

//////////////////////////////////////////////////////////////////////
//////////////////////// CORBA::Object_Helper ////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Object_ptr
CORBA::
Object_Helper::_nil() 
{
  return Object::_nil();
}


CORBA::Boolean
CORBA::
Object_Helper::is_nil(CORBA::Object_ptr obj)
{
  return CORBA::is_nil(obj);
}


void
CORBA::
Object_Helper::release(CORBA::Object_ptr obj)
{
  CORBA::release(obj);
}


void
CORBA::
Object_Helper::duplicate(CORBA::Object_ptr obj)
{
  CORBA::Object::_duplicate(obj);
}


void
CORBA::
Object_Helper::marshalObjRef(CORBA::Object_ptr obj, cdrStream& s)
{
  CORBA::Object::_marshalObjRef(obj,s);
}


CORBA::Object_ptr
CORBA::
Object_Helper::unmarshalObjRef(cdrStream& s)
{
  return CORBA::Object::_unmarshalObjRef(s);
}



