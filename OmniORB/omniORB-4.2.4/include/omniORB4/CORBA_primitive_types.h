// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_primitive_types.h    Created on: 2001/08/17
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
//    Primitive types
//

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////// Primitive types //////////////////////////
//////////////////////////////////////////////////////////////////////

typedef _CORBA_Boolean    Boolean;
typedef _CORBA_Char       Char;
typedef _CORBA_Octet      Octet;
typedef _CORBA_Short      Short;
typedef _CORBA_UShort     UShort;
typedef _CORBA_Long       Long;
typedef _CORBA_ULong      ULong;
# ifdef HAS_LongLong
typedef _CORBA_LongLong   LongLong;
typedef _CORBA_ULongLong  ULongLong;
# endif
typedef _CORBA_WChar      WChar;
# ifndef NO_FLOAT
typedef _CORBA_Float      Float;
typedef _CORBA_Double     Double;
# ifdef HAS_LongDouble
typedef _CORBA_LongDouble LongDouble;
# endif
# endif

typedef _CORBA_Boolean&    Boolean_out;
typedef _CORBA_Char&       Char_out;
typedef _CORBA_Octet&      Octet_out;
typedef _CORBA_Short&      Short_out;
typedef _CORBA_UShort&     UShort_out;
typedef _CORBA_Long&       Long_out;
typedef _CORBA_ULong&      ULong_out;
# ifdef HAS_LongLong
typedef _CORBA_LongLong&   LongLong_out;
typedef _CORBA_ULongLong&  ULongLong_out;
# endif
typedef _CORBA_WChar&      WChar_out;
# ifndef NO_FLOAT
typedef _CORBA_Float&      Float_out;
typedef _CORBA_Double&     Double_out;
# ifdef HAS_LongDouble
typedef _CORBA_LongDouble& LongDouble_out;
# endif
# endif
