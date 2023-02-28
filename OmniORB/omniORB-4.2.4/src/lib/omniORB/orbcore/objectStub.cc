// -*- Mode: C++; -*-
//                            Package   : omniORB
// objectStub.cc              Created on: 04/09/2000
//                            Author    : Sai-Lai Lo (sll)
//
//    Copyright (C) 2005 Apasphere Ltd
//    Copyright (C) 2000 AT&T Research Cambridge
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
//

#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <objectStub.h>

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void
omni_is_a_CallDesc::marshalArguments(cdrStream& s)
{
  s.marshalRawString(a_1);
}

void
omni_is_a_CallDesc::unmarshalReturnedValues(cdrStream& s)
{
  result = s.unmarshalBoolean();
}

void
omni_is_a_CallDesc::unmarshalArguments(cdrStream& s)
{
  a_1 = s.unmarshalRawString();
}

void
omni_is_a_CallDesc::marshalReturnedValues(cdrStream& s)
{
  s.marshalBoolean(result);
}

void
omni_is_a_CallDesc::lcfn(omniCallDescriptor* cd, omniServant* servant)
{
  omni_is_a_CallDesc* tcd = (omni_is_a_CallDesc*) cd;

  tcd->result = servant->_is_a(tcd->a_1);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void
omni_non_existent_CallDesc::unmarshalReturnedValues(cdrStream& s)
{
  result = s.unmarshalBoolean();
}

void
omni_non_existent_CallDesc::marshalReturnedValues(cdrStream& s)
{
  s.marshalBoolean(result);
}


void
omni_non_existent_CallDesc::lcfn(omniCallDescriptor* cd, omniServant* servant)
{
  ((omni_non_existent_CallDesc*) cd)->result = servant->_non_existent();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void
omni_interface_CallDesc::unmarshalReturnedValues(cdrStream& s)
{
  pd_result = CORBA::Object::_unmarshalObjRef(s);
}

void
omni_interface_CallDesc::marshalReturnedValues(cdrStream& s)
{
  CORBA::Object::_marshalObjRef(pd_result,s);
}

void
omni_interface_CallDesc::lcfn(omniCallDescriptor* cd, omniServant* servant)
{
  omniObjRef* intf = servant->_do_get_interface();
  omni_interface_CallDesc* icd = (omni_interface_CallDesc*)cd;

  if (intf) {
    icd->pd_result = (CORBA::Object_ptr)
                     intf->_ptrToObjRef(CORBA::Object::_PD_repoId);
  }
  else {
    icd->pd_result = CORBA::Object::_nil();
  }
}

OMNI_NAMESPACE_END(omni)
