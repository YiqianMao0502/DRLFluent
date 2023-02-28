// -*- Mode: C++; -*-
//                            Package   : omniORB
// nvList.cc                  Created on: 9/1998
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
//   Implementation of CORBA::NVList.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/objTracker.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <pseudo.h>
#include <exceptiondefs.h>


OMNI_NAMESPACE_BEGIN(omni)

#define INIT_MAX_SEQ_LENGTH  6


NVListImpl::NVListImpl()
{
  pd_list.length(INIT_MAX_SEQ_LENGTH);
  pd_list.length(0);
}


NVListImpl::~NVListImpl()
{
  for( CORBA::ULong i=0; i < pd_list.length(); i++ )
    CORBA::release(pd_list[i]);
}


CORBA::ULong
NVListImpl::count() const
{
  return pd_list.length();
}


CORBA::NamedValue_ptr
NVListImpl::add(CORBA::Flags flags)
{
  CORBA::ULong len = pd_list.length();

  if( len == pd_list.maximum() )
    // allocate new space in decent chunks
    pd_list.length(len * 6 / 5 + 1);

  pd_list.length(len + 1);
  pd_list[len] = new NamedValueImpl(flags);
  return pd_list[len];
}


CORBA::NamedValue_ptr
NVListImpl::add_item(const char* name, CORBA::Flags flags)
{
  CORBA::ULong len = pd_list.length();

  if( len == pd_list.maximum() )
    pd_list.length(len * 6 / 5 + 1);

  pd_list.length(len + 1);
  pd_list[len] = new NamedValueImpl(name, flags);
  return pd_list[len];
}


CORBA::NamedValue_ptr
NVListImpl::add_value(const char* name, const CORBA::Any& value,
		      CORBA::Flags flags)
{
  CORBA::ULong len = pd_list.length();

  if( len == pd_list.maximum() )
    pd_list.length(len * 6 / 5 + 1);

  pd_list.length(len + 1);
  pd_list[len] = new NamedValueImpl(name, value, flags);
  return pd_list[len];
}


CORBA::NamedValue_ptr
NVListImpl::add_item_consume(char* name, CORBA::Flags flags)
{
  CORBA::ULong len = pd_list.length();

  if( len == pd_list.maximum() )
    pd_list.length(len * 6 / 5 + 1);

  pd_list.length(len + 1);
  pd_list[len] = new NamedValueImpl(name, flags);
  return pd_list[len];
}


CORBA::NamedValue_ptr
NVListImpl::add_value_consume(char* name, CORBA::Any* value,
			      CORBA::Flags flags)
{
  CORBA::ULong len = pd_list.length();

  if( len == pd_list.maximum() )
    pd_list.length(len * 6 / 5 + 1);

  pd_list.length(len + 1);
  pd_list[len] = new NamedValueImpl(name, value, flags);
  return pd_list[len];
}


CORBA::NamedValue_ptr
NVListImpl::item(CORBA::ULong index)
{
  if (index >= pd_list.length())
    throw CORBA::NVList::Bounds();

  return pd_list[index];
}


void
NVListImpl::remove(CORBA::ULong index)
{
  if (index >= pd_list.length())
    throw CORBA::NVList::Bounds();

  // operator[] on the sequence will do the bounds check for us here
  CORBA::release(pd_list[index]);

  for( CORBA::ULong i = index; i < pd_list.length() - 1; i++ )
    pd_list[i] = pd_list[i + 1];

  pd_list.length(pd_list.length() - 1);
}


CORBA::Boolean
NVListImpl::NP_is_nil() const
{
  return 0;
}


CORBA::NVList_ptr
NVListImpl::NP_duplicate()
{
  incrRefCount();
  return this;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilNVList : public CORBA::NVList, public omniTrackedObject {
public:
  virtual CORBA::ULong count() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::NamedValue_ptr add(CORBA::Flags) {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::NamedValue::_nil();
  }
  virtual CORBA::NamedValue_ptr add_item(const char*, CORBA::Flags) {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::NamedValue::_nil();
  }
  virtual CORBA::NamedValue_ptr add_value(const char*, const CORBA::Any&,
					  CORBA::Flags) {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::NamedValue::_nil();
  }
  virtual CORBA::NamedValue_ptr add_item_consume(char*, CORBA::Flags) {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::NamedValue::_nil();
  }
  virtual CORBA::NamedValue_ptr add_value_consume(char*, CORBA::Any*,
						  CORBA::Flags) {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::NamedValue::_nil();
  }
  virtual CORBA::NamedValue_ptr item(CORBA::ULong) {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::NamedValue::_nil();
  }
  virtual void remove(CORBA::ULong) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual CORBA::NVList_ptr NP_duplicate() {
    return _nil();
  }
};

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::NVList::~NVList() { pd_magic = 0; }


CORBA::NVList_ptr
CORBA::
NVList::_duplicate(NVList_ptr p)
{
  if (!PR_is_valid(p))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidNVList, CORBA::COMPLETED_NO);
  if( !CORBA::is_nil(p) )  return p->NP_duplicate();
  else     return _nil();
}


CORBA::NVList_ptr
CORBA::
NVList::_nil()
{
  static omniNilNVList* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilNVList;
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
CORBA::release(NVList_ptr p)
{
  if( CORBA::NVList::PR_is_valid(p) && !CORBA::is_nil(p) )
    ((NVListImpl*)p)->decrRefCount();
}


void
CORBA::ORB::create_list(Long count, NVList_out new_list)
{
  if (count < 0)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidInitialSize,
		  CORBA::COMPLETED_NO);

  // <count> is a hint about how long the list will become. We choose
  // to ignore it.

  new_list = new NVListImpl();
}


void
CORBA::ORB::create_operation_list(_objref_OperationDef* p, NVList_out new_list)
{
  new_list = CORBA::NVList::_nil();

  throw NO_IMPLEMENT(0, CORBA::COMPLETED_NO);
}
