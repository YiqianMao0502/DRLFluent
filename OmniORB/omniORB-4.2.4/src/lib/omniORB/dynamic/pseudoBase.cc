// -*- Mode: C++; -*-
//                            Package   : omniORB
// pseudoBase.cc              Created on: 9/1998
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2012 Apasphere Ltd
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
//   PseudoObjBase provides reference counting for the pseudo object
//   types.
//

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <pseudo.h>

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////// PseudoObjBase ///////////////////////////
//////////////////////////////////////////////////////////////////////

PseudoObjBase::~PseudoObjBase() {}


void
PseudoObjBase::decrRefCount()
{
  if (!pd_refCount.value()) {
    omniORB::logs(1, "Warning: CORBA::release() has been called too many times "
                  "for a pseudo object. The object has already "
                  "been destroyed.");
    return;
  }
  
  if (pd_refCount.dec() == 0)
    delete this;
}

OMNI_NAMESPACE_END(omni)
