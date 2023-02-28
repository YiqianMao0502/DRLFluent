// -*- Mode: C++; -*-
//                            Package   : omniORB
// corbaOrb.h                 Created on: 22/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
//    Implementation of the ORB interface.
//

#ifndef __CORBAORB_H__
#define __CORBAORB_H__


class omniOrbORB : public CORBA::ORB {
public:
  virtual ~omniOrbORB();
  omniOrbORB(int is_nil);

  virtual char* id();

  virtual char* object_to_string(CORBA::Object_ptr);
  virtual CORBA::Object_ptr string_to_object(const char*);

  virtual CORBA::ORB::ObjectIdList* list_initial_services();
  virtual CORBA::Object_ptr resolve_initial_references(const char* id);

  virtual CORBA::Boolean work_pending();
  virtual void perform_work();
  virtual void run();
  virtual void shutdown(CORBA::Boolean);
  virtual void destroy();

  virtual void register_initial_reference(const char* id,
					  CORBA::Object_ptr obj);

  virtual CORBA::Boolean _non_existent();
  virtual void* _ptrToObjRef(const char*);
  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Override CORBA::Object.

  void actual_shutdown();

  CORBA::Boolean run_timeout(unsigned long secs, unsigned long nanosecs);
  // Same as run(), but stopping when the absolute timeout is reached.
  // Returns true if shutdown, false if timed out. Note the potential
  // for race conditions if shutdown coincides with timeout: only
  // treat a timed-out indication as a hint.

  static CORBA::Boolean all_destroyed();
  // True if all ORBs have been destroyed (although there's only ever
  // one at present).
  //  NO CONCURRENCY CONTROL. This is intended to be used in the final
  //  clean-up by a static destructor.

private:
  void do_shutdown(CORBA::Boolean wait_for_completion);

  int pd_refCount;
  // Protected by <omni::poRcLock>.

  int pd_destroyed;
  int pd_shutdown;
  int pd_shutdown_in_progress;
  // All protected by <orb_lock>.
};


#endif  // __CORBAORB_H__
