// -*- Mode: C++; -*-
//                            Package   : omniORB
// ir.cc                      Created on: 12/1998
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
//   Client side of CORBA::Object::_get_interface(). Only available if
//   the Interface Repository stubs have been compiled in.
//

#define ENABLE_CLIENT_IR_SUPPORT
#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <initRefs.h>
#include <exceptiondefs.h>
#include <objectStub.h>


OMNI_USING_NAMESPACE(omni)

CORBA::InterfaceDef_ptr
CORBA::
Object::_get_interface()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_objref();

  if( !_PR_is_valid(this) )  OMNIORB_THROW(BAD_PARAM,
					   BAD_PARAM_InvalidObjectRef,
					   CORBA::COMPLETED_NO);

  if( _NP_is_pseudo() ) OMNIORB_THROW(NO_IMPLEMENT,
				      NO_IMPLEMENT_DIIOnLocalObject,
				      CORBA::COMPLETED_NO);

  // Try asking the object itself...
  omni_interface_CallDesc call_desc("_interface", sizeof("_interface"));
  try {
    pd_obj->_invoke(call_desc, 0);
    CORBA::Object_var obj = call_desc.result();
    return CORBA::InterfaceDef::_narrow(obj);
  }
  catch (CORBA::Exception& ex) {
  }

  // Failed to contact the object directly. Try the interface repository...
  CORBA::Object_var o(omniInitialReferences::resolve("InterfaceRepository"));
  CORBA::Repository_ptr repository = CORBA::Repository::_narrow(o);

  if( CORBA::is_nil(repository) )
    OMNIORB_THROW(INTF_REPOS, INTF_REPOS_NotAvailable, CORBA::COMPLETED_NO);

  CORBA::Contained_ptr interf =
    repository->lookup_id(pd_obj->_mostDerivedRepoId());
  return CORBA::InterfaceDef::_narrow(interf);
}
