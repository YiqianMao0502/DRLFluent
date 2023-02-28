// -*- Mode: C++; -*-
//                            Package   : omniORB
// rmutex.cc                  Created on: 2001/06/27
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Research Cambridge
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
//    Recursive mutex, to be used by single-thread policy POAs
//

#include <omniORB4/CORBA.h>
#include <rmutex.h>


omni_rmutex::omni_rmutex()
  : pd_cond(&pd_lock),
    pd_holder(0),
    pd_depth(0),
    pd_dummy(0)
{
}

omni_rmutex::~omni_rmutex()
{
  OMNIORB_ASSERT(!pd_holder);
  OMNIORB_ASSERT(!pd_dummy);
}

void
omni_rmutex::lock()
{
  omni_thread* me = omni_thread::self();
  int dummy = 0;

  if (!me) {
    // Create a dummy thread
    omniORB::logs(15, "Create dummy omni_thread in rmutex lock.");
    me = omni_thread::create_dummy();
    dummy = 1;
  }

  omni_mutex_lock sync(pd_lock);

  if (pd_holder == me) {
    ++pd_depth;
    return;
  }

  while (pd_holder) pd_cond.wait();

  OMNIORB_ASSERT(pd_depth == 0);
  pd_holder = me;
  pd_dummy  = dummy;
  pd_depth  = 1;
}

void
omni_rmutex::unlock()
{
  omni_thread* me = omni_thread::self();
  OMNIORB_ASSERT(me); // Can only be used by threads with an omni_thread

  omni_mutex_lock sync(pd_lock);

  OMNIORB_ASSERT(pd_holder == me);

  if (--pd_depth == 0) {
    pd_holder = 0;
    pd_cond.signal();

    if (pd_dummy) {
      omniORB::logs(15, "Release dummy omni_thread in rmutex unlock.");
      omni_thread::release_dummy();
      pd_dummy = 0;
    }
  }
}
