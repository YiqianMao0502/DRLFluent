// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Current.h            Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
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
//    CORBA::Current
//

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////// Current         ////////////////////////
//////////////////////////////////////////////////////////////////////

class Current;
typedef class Current* Current_ptr;
typedef Current_ptr CurrentRef;

class Current : public Object {
public:

  typedef Current*                      _ptr_type;
  typedef _CORBA_PseudoObj_Var<Current> _var_type;

  static Current_ptr _duplicate(Current_ptr);
  static Current_ptr _narrow(Object_ptr);
  static Current_ptr _nil();

  // omniORB internal.
  static _core_attr const char* _PD_repoId;

protected:
  Current(int is_nil);
  virtual ~Current();

private:
  Current(const Current&);
  Current& operator=(const Current&);
};

typedef Current::_var_type                            Current_var;
typedef _CORBA_PseudoObj_Member<Current, Current_var> Current_member;
