// -*- Mode: C++; -*-
//                            Package   : omniORB
// constants.cc               Created on: 26/9/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2003-2004 Apasphere Ltd
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
//	

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif


OMNI_NAMESPACE_BEGIN(omni)

// See the description of this variable in omniInternal.h
const char* omniORB_4_2_dyn = "omniORB dynamic library version 4.2.x";

OMNI_NAMESPACE_END(omni)


//////////////////////////////////////////////////////////////////////
// Each pseudo object type must be assigned a magic number.
// This magic number is written into the member pd_magic of each instance
// The static function PR_is_valid() in each of the pseudo object class 
// can be used to test if the instance is indeed valid. If a random pointer 
// is passed to isvalid(), it is unlikely that the magic number would match.

const CORBA::ULong CORBA::Context::PR_magic         = 0x43545854U; // CTXT
const CORBA::ULong CORBA::ContextList::PR_magic     = 0x4354584CU; // CTXL
const CORBA::ULong CORBA::Environment::PR_magic     = 0x454E564CU; // ENVI
const CORBA::ULong CORBA::ExceptionList::PR_magic   = 0x4558434CU; // EXCL
const CORBA::ULong CORBA::NamedValue::PR_magic      = 0x4E56414CU; // NVAL
const CORBA::ULong CORBA::NVList::PR_magic          = 0x4E564C54U; // NVLT
const CORBA::ULong CORBA::Request::PR_magic         = 0x52455154U; // REQT
const CORBA::ULong CORBA::TypeCode::PR_magic        = 0x54594F4CU; // TCOL
const CORBA::ULong DynamicAny::DynAny::PR_magic     = 0x44594E54U; // DYNT
const CORBA::ULong CORBA::ValueBase::_PR_magic      = 0x56414C42U; // VALB
