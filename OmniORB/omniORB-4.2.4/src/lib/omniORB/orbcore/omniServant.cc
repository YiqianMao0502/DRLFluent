// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniServant.cc             Created on: 26/2/99
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
//    Base class for object implementations (servants).
//

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/omniServant.h>
#include <objectTable.h>
#include <exceptiondefs.h>
#include <omniORB4/IOP_S.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <objectStub.h>

OMNI_USING_NAMESPACE(omni)

omniServant::~omniServant()
{
  if( !pd_activations.empty() ) {
    if (omniORB::trace(1)) {
      omniORB::logger l;
      l << "Error: a servant has been deleted that is still activated.\n";
      omni::internalLock->lock();

      omnivector<omniObjTableEntry*>::iterator i    = pd_activations.begin();
      omnivector<omniObjTableEntry*>::iterator last = pd_activations.end();

      for (; i != last; i++) {
	l << "      id: " << *i << '\n';
      }
      omni::internalLock->unlock();
    }
  }
}


void*
omniServant::_ptrToInterface(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (void*) 1;

  return 0;
}


void*
omniServant::_downcast()
{
  return 0;
}


const char*
omniServant::_mostDerivedRepoId()
{
  return "";
}


CORBA::Boolean
omniServant::_is_a(const char* repoId)
{
  return _ptrToInterface(repoId) ? 1 : 0;
}


CORBA::Boolean
omniServant::_non_existent()
{
  return 0;
}

_CORBA_Boolean
omniServant::_dispatch(omniCallHandle& handle)
{
  const char* op = handle.operation_name();

  if( omni::strMatch(op, "_is_a") ) {
    omni_is_a_CallDesc call_desc("_is_a",sizeof("_is_a"),0,1);
    handle.upcall(this,call_desc);
    return 1;
  }

  if( omni::strMatch(op, "_non_existent") ) {
    omni_non_existent_CallDesc call_desc("_non_existent",
					 sizeof("_non_existent"),1);
    handle.upcall(this,call_desc);
    return 1;
  }

  if( omni::strMatch(op, "_interface") ) {
    omni_interface_CallDesc call_desc("_interface",
				      sizeof("_interface"),1);
    handle.upcall(this,call_desc);
    return 1;
  }

  if( omni::strMatch(op, "_implementation") ) {
    omniORB::logs(2,
     "WARNING -- received GIOP request \'_implementation\'.\n"
     " This operation is not supported.  CORBA::NO_IMPLEMENT was raised.");
    OMNIORB_THROW(NO_IMPLEMENT,NO_IMPLEMENT_Unsupported, CORBA::COMPLETED_NO);
  }
  return 0;
}

void
omniServant::_addActivation(omniObjTableEntry* entry)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(entry);  OMNIORB_ASSERT(entry->servant() == this);

  pd_activations.push_back(entry);
}


void
omniServant::_removeActivation(omniObjTableEntry* entry)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(entry);

  omnivector<omniObjTableEntry*>::iterator i    = pd_activations.begin();
  omnivector<omniObjTableEntry*>::iterator last = pd_activations.end();

  CORBA::Boolean activation_found = 0;

  for (; i != last; i++) {
    if (*i == entry) {
      pd_activations.erase(i);
      activation_found = 1;
      break;
    }
  }
  OMNIORB_ASSERT(activation_found);
}


void omniServant::_add_ref()    {}
void omniServant::_remove_ref() {}
