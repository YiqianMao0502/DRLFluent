// -*- Mode: C++; -*-
//                            Package   : omniORB
// namedValue.cc              Created on: 9/1998
//                            Author    : David Riddoch (djr)
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
//
// Description:
//   Implementation of CORBA::NamedValue.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/objTracker.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <pseudo.h>
#include <exceptiondefs.h>

OMNI_NAMESPACE_BEGIN(omni)

NamedValueImpl::NamedValueImpl(CORBA::Flags flags)
{
  pd_flags = flags;
  pd_name = CORBA::string_dup("");
  pd_value = new CORBA::Any;
  if( !pd_value.operator->() )  _CORBA_new_operator_return_null();
}


NamedValueImpl::NamedValueImpl(const char* name, CORBA::Flags flags)
{
  if( !name )  OMNIORB_THROW(BAD_PARAM,
			     BAD_PARAM_NullStringUnexpected,
			     CORBA::COMPLETED_NO);
  pd_flags = flags;
  pd_name = CORBA::string_dup(name);
  pd_value = new CORBA::Any;
  if( !pd_value.operator->() )  _CORBA_new_operator_return_null();
}


NamedValueImpl::NamedValueImpl(const char* name, const CORBA::Any& value,
			       CORBA::Flags flags)
{
  if( !name )  OMNIORB_THROW(BAD_PARAM,
			     BAD_PARAM_NullStringUnexpected,
			     CORBA::COMPLETED_NO);
  pd_flags = flags;
  pd_name = CORBA::string_dup(name);
  pd_value = new CORBA::Any(value);
  if( !pd_value.operator->() )  _CORBA_new_operator_return_null();
}


NamedValueImpl::NamedValueImpl(char* name, CORBA::Flags flags)
{
  if( !name )  OMNIORB_THROW(BAD_PARAM,
			     BAD_PARAM_NullStringUnexpected,
			     CORBA::COMPLETED_NO);
  pd_flags = flags;
  pd_name = name;
  pd_value = new CORBA::Any;
  if( !pd_value.operator->() )  _CORBA_new_operator_return_null();
}


NamedValueImpl::NamedValueImpl(char* name, CORBA::Any* value,
			       CORBA::Flags flags)
{
  if( !name )   OMNIORB_THROW(BAD_PARAM,
			      BAD_PARAM_NullStringUnexpected,
			      CORBA::COMPLETED_NO);

  if( !value )  OMNIORB_THROW(BAD_PARAM,
			      BAD_PARAM_InvalidAny,
			      CORBA::COMPLETED_NO);
  pd_flags = flags;
  pd_name = name;
  pd_value = value;
}


NamedValueImpl::~NamedValueImpl() {}


const char*
NamedValueImpl::name() const
{
  return pd_name;
}


CORBA::Any*
NamedValueImpl::value() const
{
  return pd_value.operator->();
}


CORBA::Flags
NamedValueImpl::flags() const
{
  return pd_flags;
}


CORBA::Boolean
NamedValueImpl::NP_is_nil() const
{
  return 0;
}


CORBA::NamedValue_ptr
NamedValueImpl::NP_duplicate()
{
  incrRefCount();
  return this;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilNV : public CORBA::NamedValue, public omniTrackedObject {
public:
  virtual const char* name() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Any* value() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Flags flags() const {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::Flags(0);
  }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual CORBA::NamedValue_ptr NP_duplicate() {
    return _nil();
  }
};

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::NamedValue::~NamedValue() { pd_magic = 0; }


CORBA::NamedValue_ptr
CORBA::
NamedValue::_duplicate(NamedValue_ptr p)
{
  if (!PR_is_valid(p))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidNamedValue, CORBA::COMPLETED_NO);
  if( !CORBA::is_nil(p) )  return p->NP_duplicate();
  else     return _nil();
}


CORBA::NamedValue_ptr
CORBA::
NamedValue::_nil()
{
  static omniNilNV* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilNV;
      registerTrackedObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::release(NamedValue_ptr p)
{
  if( CORBA::NamedValue::PR_is_valid(p) && !CORBA::is_nil(p) )
    ((NamedValueImpl*)p)->decrRefCount();
}


void
CORBA::ORB::create_named_value(NamedValue_out nmval)
{
  nmval = new NamedValueImpl((CORBA::Flags)0);
}
