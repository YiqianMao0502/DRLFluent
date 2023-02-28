// -*- Mode: C++; -*-
//                            Package   : omniORB
// objectStub.h               Created on: 04/09/2000
//                            Author    : Sai-Lai Lo (sll)
//
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

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
class omni_is_a_CallDesc : public omniCallDescriptor {
public:
  inline omni_is_a_CallDesc(const char* op,
			    int oplen, const char* i_1,
			    CORBA::Boolean upcall=0)
    : omniCallDescriptor(lcfn, op, oplen, 0, 0, 0, upcall),
      a_1((char*)i_1)
    {}

  inline ~omni_is_a_CallDesc() {
    if (is_upcall()) { _CORBA_String_helper::dealloc(a_1); }
  }

  void marshalArguments(cdrStream&);
  void unmarshalReturnedValues(cdrStream&);
  void unmarshalArguments(cdrStream&);
  void marshalReturnedValues(cdrStream&);

  static void lcfn(omniCallDescriptor* cd, omniServant* servant);

  char*    a_1;
  CORBA::Boolean result;
};

//////////////////////////////////////////////////////////////////////
class omni_non_existent_CallDesc : public omniCallDescriptor {
public:
  inline omni_non_existent_CallDesc(const char* op, int oplen,
				    CORBA::Boolean upcall=0)
    : omniCallDescriptor(lcfn, op, oplen, 0, 0, 0, upcall)
    {}

  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);

  static void lcfn(omniCallDescriptor* cd, omniServant* servant);

  CORBA::Boolean result;
};


//////////////////////////////////////////////////////////////////////
class omni_interface_CallDesc : public omniCallDescriptor {
public:
  inline omni_interface_CallDesc(const char* op, int oplen,
				 CORBA::Boolean upcall=0)
    : omniCallDescriptor(lcfn, op, oplen, 0, 0, 0, upcall)
    {}

  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);

  inline CORBA::Object_ptr result() { return pd_result._retn(); }

  static void lcfn(omniCallDescriptor* cd, omniServant* servant);

  CORBA::Object_var pd_result;
};

OMNI_NAMESPACE_END(omni)
