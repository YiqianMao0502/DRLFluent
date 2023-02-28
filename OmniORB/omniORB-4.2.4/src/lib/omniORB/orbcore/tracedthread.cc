// -*- Mode: C++; -*-
//                            Package   : omniORB
// tracedthread.cc            Created on: 15/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2002-2010 Apasphere Ltd
//    Copyright (C) 1996,1999 AT&T Research Cambridge
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
//    omni_thread style mutex and condition variables with checks.
//

#include <omniORB4/CORBA.h>
#include <stdio.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/tracedthread.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
////////////////////////// omni_tracedmutex //////////////////////////
//////////////////////////////////////////////////////////////////////

#define BOMB_OUT()  throw *(int*)0

#ifdef OMNIORB_ENABLE_LOCK_TRACES

static const char* bug_msg =
  " This is probably a bug in omniORB. Please submit a report\n"
  " (with stack trace if possible) to <bugs@omniorb-support.com>.\n";


omni_tracedmutex::omni_tracedmutex(const char* name)
  : pd_cond(&pd_lock),
    pd_holder(0),
    pd_n_conds(0),
    pd_deleted(0)
{
  if (name) {
    pd_logname = new char[strlen(name) + 30];
    sprintf(pd_logname, "%s (%p)", name, (void*)this);
  }
  else {
    pd_logname = new char[30];
    sprintf(pd_logname, "(%p)", (void*)this);
  }
  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": mutex constructed.\n";
  }
}


omni_tracedmutex::~omni_tracedmutex()
{
  if (pd_deleted) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- mutex (" << (void*)this
	  << ") deleted more than once.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": mutex deleted.\n";
  }
  if (pd_holder) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- mutex " << pd_logname
	  << " destroyed whilst held.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  if (pd_n_conds != 0) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- mutex " << pd_logname
	  << " destroyed whilst still being used "
	  << " by " << pd_n_conds << " condition variable(s).\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  pd_deleted = 1;
  delete [] pd_logname;
}


void
omni_tracedmutex::lock()
{
  if (pd_deleted) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- attempt to lock deleted mutex ("
	  << (void*)this << ").\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  omni_thread* me = omni_thread::self();

  omni_mutex_lock sync(pd_lock);

  if (me && pd_holder == me) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- attempt to lock mutex " << pd_logname
	  << " when already held.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  if (pd_holder && omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": block in lock.\n";
  }

  while (pd_holder)  pd_cond.wait();

  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": locked.\n";
  }

  pd_holder = me ? me : (omni_thread*) 1;
}


void
omni_tracedmutex::unlock()
{
  if (pd_deleted) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- attempt to unlock deleted mutex ("
	  << (void*)this << ").\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  omni_thread* me = omni_thread::self();

  {
    omni_mutex_lock sync(pd_lock);

    if (!pd_holder ||
	(me && pd_holder != me)) {

      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Assertion failed -- attempt to unlock mutex " << pd_logname
	    << " not held.\n"
	    << bug_msg;
      }
      BOMB_OUT();
    }

    pd_holder = 0;
  }
  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": unlocked.\n";
  }
  pd_cond.signal();
}


void
omni_tracedmutex::assert_held(const char* file, int line, int yes)
{
  {
    omni_mutex_lock sync(pd_lock);

    omni_thread* me = omni_thread::self();

    if (( yes && pd_holder == me) ||
	(!yes && pd_holder != me) ||
	(!me))
      
      return;
  }

  if (omniORB::trace(1)) {
    omniORB::logger log;
    log << "Assertion failed -- mutex " << pd_logname
	<< (yes ? " is not held.\n" : " should not be held.\n")
	<< bug_msg
	<< "   file: " << file << "\n"
	<< "   line: " << line << "\n";
  }
  BOMB_OUT();
}


//////////////////////////////////////////////////////////////////////
//////////////////////// omni_tracedcondition ////////////////////////
//////////////////////////////////////////////////////////////////////

omni_tracedcondition::omni_tracedcondition(omni_tracedmutex* m,
					   const char* name)
  : pd_mutex(*m), pd_cond(&m->pd_lock), pd_n_waiters(0),
    pd_deleted(0)
{
  if (!m) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- omni_tracedcondition "
	  << (name ? name : "<unknown>")
	  << " initialised with a nil mutex argument.\n" << bug_msg;
    }
    BOMB_OUT();
  }

  if (name) {
    pd_logname = new char[strlen(name) + 30];
    sprintf(pd_logname, "%s (%p)", name, (void*)this);
  }
  else {
    pd_logname = new char[30];
    sprintf(pd_logname, "(%p)", (void*)this);
  }

  pd_mutex.pd_lock.lock();
  pd_mutex.pd_n_conds++;
  pd_mutex.pd_lock.unlock();

  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": condition constructed.\n";
  }
}


omni_tracedcondition::~omni_tracedcondition()
{
  if (pd_deleted) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- condition (" << (void*)this
	  << ") deleted more than once.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": condition deleted.\n";
  }
  if (pd_n_waiters) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Warning: omni_tracedcondition " << pd_logname
	  << " was deleted, but there are still threads waiting on it.\n";
    }
  }
  pd_mutex.pd_lock.lock();
  pd_mutex.pd_n_conds--;
  pd_mutex.pd_lock.unlock();
  pd_deleted = 1;
  delete [] pd_logname;
}


void
omni_tracedcondition::wait()
{
  if (pd_deleted) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- attempt to wait on deleted condition ("
	  << pd_logname << ").\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  omni_thread* me = omni_thread::self();

  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": wait...\n";
  }

  omni_mutex_lock sync(pd_mutex.pd_lock);

  if (me && pd_mutex.pd_holder != me) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- attempt to wait on condition variable "
	  << pd_logname
	  << " but the calling thread does not hold the associated mutex "
	  << pd_mutex.pd_logname << "\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  pd_mutex.pd_holder = 0;
  pd_mutex.pd_cond.signal();
  pd_n_waiters++;
  pd_cond.wait();
  pd_n_waiters--;
  while( pd_mutex.pd_holder )  pd_mutex.pd_cond.wait();
  pd_mutex.pd_holder = me ? me : (omni_thread*) 1;

  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": wait completed.\n";
  }
}


int
omni_tracedcondition::timedwait(unsigned long secs, unsigned long nanosecs)
{
  if (pd_deleted) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- attempt to wait on deleted condition ("
	  << pd_logname << ").\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  omni_thread* me = omni_thread::self();

  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": timedwait...\n";
  }

  omni_mutex_lock sync(pd_mutex.pd_lock);

  if (me && pd_mutex.pd_holder != me) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- attempt to timedwait on condition variable "
	  << pd_logname
	  << " but the calling thread does not hold the associated mutex "
	  << pd_mutex.pd_logname << "\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  pd_mutex.pd_holder = 0;
  pd_mutex.pd_cond.signal();
  pd_n_waiters++;
  int ret = pd_cond.timedwait(secs, nanosecs);
  pd_n_waiters--;
  while( pd_mutex.pd_holder )  pd_mutex.pd_cond.wait();
  pd_mutex.pd_holder = me ? me : (omni_thread*) 1;

  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": timedwait completed (" << ret << ")\n";
  }
  return ret;
}


void
omni_tracedcondition::signal()
{
  if (pd_deleted) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- attempt to signal on deleted condition ("
	  << (void*)this << ")\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": signal.\n";
  }
  pd_cond.signal();
}


void
omni_tracedcondition::broadcast()
{
  if (pd_deleted) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Assertion failed -- attempt to broadcast on deleted condition ("
	  << (void*)this << ")\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  if (omniORB::traceLocking) {
    omniORB::logger log;
    log << "- " << pd_logname << ": broadcast.\n";
  }
  pd_cond.broadcast();
}


#endif  // ifdef OMNIORB_ENABLE_LOCK_TRACES

#if defined __vxWorks__
#  ifndef OMNIORB_ENABLE_LOCK_TRACES
void _dummy_TRACEDTHREAD_workaround_for_bug_in_munch_2_cdtor_c_ () {}
#  endif  // ifndef OMNIORB_ENABLE_LOCK_TRACES
#endif // __vxWorks__
