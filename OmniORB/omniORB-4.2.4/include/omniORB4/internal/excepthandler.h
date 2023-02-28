// -*- Mode: C++; -*-
//                            Package   : omniORB
// excepthandler.h            Created on: 13/5/97
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2011-2014 Apasphere Ltd
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
//

#ifndef __EXCEPTHANDLER_H__
#define __EXCEPTHANDLER_H__


class omniExHandlers_iterator;

class omniExHandlers {
public:
  void*                   transient_hdr;
  void*                   transient_cookie;
  void*                   timeout_hdr;
  void*                   timeout_cookie;
  void*                   commfail_hdr;
  void*                   commfail_cookie;
  void*                   sysexcpt_hdr;
  void*                   sysexcpt_cookie;
  CORBA::Boolean          transient_ext;
  CORBA::Boolean          timeout_ext;
  CORBA::Boolean          commfail_ext;
  CORBA::Boolean          sysexcpt_ext;

  static omniExHandlers** Table;
  static omni_tracedmutex TableLock;
  friend class omniExHandlers_iterator;

private:
  omniExHandlers();
  omniObjRef*             objptr;
  omniExHandlers*         next;
};


class omniExHandlers_iterator {
public:
  omniExHandlers_iterator();
  // Concurrency control:
  //       MUTEX = omniExHandlers::TableLock
  // Pre-condition:
  //       Does not hold <MUTEX> on entry
  // Post-condition:
  //       Hold <MUTEX> on exit

  ~omniExHandlers_iterator();
  // Concurrency control:
  //       MUTEX = omniExHandlers::TableLock
  // Pre-condition:
  //       Hold <MUTEX> on entry
  // Post-condition:
  //       Does not hold <MUTEX> on exit

  omniExHandlers* find_or_create(omniObjRef* p);
  // Locate the omniExHandlers for this object
  // If found just returns
  // else create a new record, initialise all the handler fields to null and
  // returns the new record.

  omniExHandlers* find(omniObjRef* p);
  // Locate the omniExHandlers for this object
  // Return 0 if not found.

  void remove(omniObjRef* p);
  // Locate the omniExHandlers for this object, if found, delete the record

};


#endif // __EXCEPTHANDLER_H__
