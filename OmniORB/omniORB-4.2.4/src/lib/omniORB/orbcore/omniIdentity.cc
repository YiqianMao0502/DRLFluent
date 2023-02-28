// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniIdentity.cc            Created on: 2001/09/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2007 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniIdentity.h>

OMNI_USING_NAMESPACE(omni)

int omniIdentity::identity_count = 0;

static omni_tracedcondition* cond = 0;

void
omniIdentity::waitForLastIdentity()
{
  omni_tracedmutex_lock sync(*omni::internalLock);

  if (identity_count == 0)
    return;

  omniORB::logs(15, "Waiting for client invocations to complete");

  cond = new omni_tracedcondition(omni::internalLock, "omniIdentity::cond");

  while (identity_count) cond->wait();

  delete cond;
  cond = 0;
}

void
omniIdentity::lastIdentityHasBeenDeleted()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if (cond)
    cond->signal();
}

void
omniIdentity::disconnect()
{
  // Default implementation does nothing except release
  // omni::internalLock to satisfy the locking scheme.
  omni::internalLock->unlock();
}


void*
omniIdentity::ptrToClass(int* cptr)
{
  if (cptr == &omniIdentity::_classid) return (omniIdentity*)this;
  return 0;
}

int omniIdentity::_classid;
