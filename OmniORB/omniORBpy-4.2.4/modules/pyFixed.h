// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyFixed.h                  Created on: 2001/03/30
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2005-2014 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//    This file is part of the omniORBpy library
//
//    The omniORBpy library is free software; you can redistribute it
//    and/or modify it under the terms of the GNU Lesser General
//    Public License as published by the Free Software Foundation;
//    either version 2.1 of the License, or (at your option) any later
//    version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library. If not, see http://www.gnu.org/licenses/
//
//
// Description:
//    Implementation of Fixed type for Python

#ifndef _pyFixed_h_
#define _pyFixed_h_


#if defined(__cygwin__) && defined(DL_IMPORT)
# undef DL_IMPORT
# define DL_IMPORT(RTYPE) RTYPE
#endif

extern "C" {

  struct omnipyFixedObject {
    PyObject_HEAD
    CORBA::Fixed* ob_fixed;
  };

  extern PyTypeObject omnipyFixed_Type;

#define omnipyFixed_Check(op) PyObject_TypeCheck(op, &omnipyFixed_Type)

}

#endif // _pyFixed_h_
