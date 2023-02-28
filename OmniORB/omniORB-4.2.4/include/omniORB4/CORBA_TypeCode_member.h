// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_TypeCode_member.h    Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2006 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//    CORBA::TypeCode_member
//

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_member //////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_var;

class TypeCode_member {
public:
  TypeCode_member();
  inline TypeCode_member(TypeCode_ptr p) : _ptr(p) {}
  TypeCode_member(const TypeCode_member& p);
  ~TypeCode_member();

  TypeCode_member& operator=(TypeCode_ptr p);
  TypeCode_member& operator=(const TypeCode_member& p);
  TypeCode_member& operator=(const TypeCode_var& p);

  inline TypeCode_ptr operator->() const { return _ptr; }
  inline operator TypeCode_ptr() const   { return _ptr; }

  inline TypeCode_ptr  in() const { return _ptr; }
  inline TypeCode_ptr& inout()    { return _ptr; }
  TypeCode_ptr& out();
  TypeCode_ptr _retn();

  TypeCode_ptr _ptr;

  inline void NP_swap(TypeCode_member& other) {
    TypeCode_ptr tmp = _ptr;
    _ptr = other._ptr;
    other._ptr = tmp;
  }

  void operator>>=(cdrStream&) const;
  void operator<<=(cdrStream&);
};


class InterfaceDef;
class _objref_InterfaceDef;

class ImplementationDef {}; // Not used.
typedef ImplementationDef* ImplementationDef_ptr;
typedef ImplementationDef_ptr ImplementationDefRef;

class OperationDef;
class _objref_OperationDef;

class                     ServerRequest;
typedef ServerRequest*    ServerRequest_ptr;
typedef ServerRequest_ptr ServerRequestRef;

class                                                Request;
typedef Request*                                     Request_ptr;
typedef Request_ptr                                  RequestRef;
typedef _CORBA_PseudoObj_Var<Request>                Request_var;
typedef _CORBA_PseudoObj_Out<Request,Request_var>    Request_out;
typedef _CORBA_PseudoObj_Member<Request,Request_var> Request_member;
