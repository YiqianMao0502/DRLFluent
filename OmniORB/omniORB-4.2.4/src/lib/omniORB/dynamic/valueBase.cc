// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueBase.cc               Created on: 2003/08/20
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003-2012 Apasphere Ltd.
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
//    ValueBase implementation
//

#include <omniORB4/CORBA.h>
#include <omniORB4/valueType.h>
#include <omniORB4/objTracker.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////// ValueBase ///////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ValueBase*
CORBA::ValueBase::_downcast(CORBA::ValueBase* v)
{
  return (CORBA::ValueBase*)v;
}

void*
CORBA::ValueBase::_ptrToValue(const char* repoId)
{
  if (repoId == _PD_repoId)
    return (void*)this;
  if (omni::strMatch(repoId, _PD_repoId))
    return (void*)this;
  return 0;
}

const char* CORBA::ValueBase::_PD_repoId = "IDL:omg.org/CORBA/ValueBase:1.0";

void
CORBA::ValueBase::_NP_marshal(CORBA::ValueBase* v, cdrStream& s)
{
  omniValueType::marshal(v, _PD_repoId, s);
}

CORBA::ValueBase*
CORBA::ValueBase::_NP_unmarshal(cdrStream& s)
{
  return omniValueType::unmarshal(_PD_repoId, 0, 0, s);
}

CORBA::Boolean
CORBA::ValueBase::_NP_box() const
{
  return 0;
}

CORBA::ValueBase::ValueBase() : _pd_magic(_PR_magic) {}
CORBA::ValueBase::ValueBase(const ValueBase&) : _pd_magic(_PR_magic) {}
CORBA::ValueBase::~ValueBase() {}


//////////////////////////////////////////////////////////////////////
///////////////////// DefaultValueRefCountBase ///////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::DefaultValueRefCountBase::_add_ref()
{
  _pd__refCount.inc();
}

void
CORBA::DefaultValueRefCountBase::_remove_ref()
{
  if (_pd__refCount.dec() > 0)
    return;

  delete this;
}

CORBA::ULong
CORBA::DefaultValueRefCountBase::_refcount_value()
{
  return _pd__refCount.value();
}

CORBA::DefaultValueRefCountBase::~DefaultValueRefCountBase() {
  OMNIORB_ASSERT(_pd__refCount.value() == 0);
}


//////////////////////////////////////////////////////////////////////
///////////////// PortableServer::ValueRefCountBase //////////////////
//////////////////////////////////////////////////////////////////////


void
PortableServer::ValueRefCountBase::_add_ref()
{
  OMNIORB_BASE_CTOR(PortableServer::)ServantBase::_add_ref();
}

void
PortableServer::ValueRefCountBase::_remove_ref()
{
  OMNIORB_BASE_CTOR(PortableServer::)ServantBase::_remove_ref();
}

CORBA::ULong
PortableServer::ValueRefCountBase::_refcount_value()
{
  return OMNIORB_BASE_CTOR(PortableServer::)ServantBase::_refcount_value();
}

PortableServer::ValueRefCountBase::~ValueRefCountBase() {
}


//////////////////////////////////////////////////////////////////////
///////////////////// ValueFactoryBase ///////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::ValueFactoryBase::_add_ref()
{
  _pd_refCount.inc();
}

void
CORBA::ValueFactoryBase::_remove_ref()
{
  if (_pd_refCount.dec() > 0)
    return;

  delete this;
}

CORBA::ValueFactoryBase::ValueFactoryBase() : _pd_refCount(1) {}

CORBA::ValueFactoryBase::~ValueFactoryBase() {
  OMNIORB_ASSERT(_pd_refCount.value() == 0);
}

CORBA::ValueFactory
CORBA::ValueFactoryBase::_downcast(CORBA::ValueFactory vf)
{
  return (ValueFactory)vf;
}

void*
CORBA::ValueFactoryBase::_ptrToFactory(const char* repoId)
{
  if (omni::ptrStrMatch(repoId, CORBA::ValueBase::_PD_repoId))
    return (CORBA::ValueBase*)this;

  return 0;
}
