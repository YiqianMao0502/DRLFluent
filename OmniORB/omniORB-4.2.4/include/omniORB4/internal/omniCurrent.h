// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniCurrent.h              Created on: 2001/06/01
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2011 Apasphere Ltd
//    Copyright (C) 2001      AT&T Research Cambridge
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
//    Generic support for the kinds of Current interface.
//

#ifndef __OMNICURRENT_H__
#define __OMNICURRENT_H__

#include <omnithread.h>
#include <omniORB4/omniORB.h>
#include <omniORB4/omniInternal.h>
#include <omniORB4/callDescriptor.h>
#include <orbParameters.h>

#if !defined(_core_attr)
# define _core_attr_defined
# if defined(_OMNIORB_LIBRARY)
#   define _core_attr
# else
#   define _core_attr _OMNIORB_NTDLL_IMPORT
# endif
#endif


class omniCurrent : public omni_thread::value_t {
public:
  omniCurrent(omni_thread* thread);
  // Should really be private but some compilers warn about a class
  // with only a private constructor and no friends.

  virtual ~omniCurrent();

  static void init();

  static inline omniCurrent* get()
  {
    if (!_OMNI_NS(orbParameters)::supportCurrent) return 0;

    omni_thread* thr = omni_thread::self();
    if (!thr || !thread_key) return 0;

    omniCurrent* current = (omniCurrent*)(thr->get_value(thread_key));
    if (!current)
      current = new omniCurrent(thr);
    return current;
  }

  static inline omniCurrent* get(omni_thread* thr)
  {
    if (!thread_key) return 0;
    omniCurrent* current = (omniCurrent*)(thr->get_value(thread_key));
    if (!current)
      current = new omniCurrent(thr);
    return current;
  }

  // Accessors
  inline omniCallDescriptor* callDescriptor()   { return pd_callDescriptor;   }
  inline const omni_time_t&  timeout()          { return pd_timeout;          }
  inline CORBA::Boolean      timeout_absolute() { return pd_timeout_absolute; }

  // Stack manipulation
  inline void setCallDescriptor(omniCallDescriptor* desc)
  {
    pd_callDescriptor = desc;
  }

  // Timeout setting
  inline void setTimeout(unsigned long secs, unsigned long ns)
  {
    pd_timeout.assign(secs, ns);
    pd_timeout_absolute = 0;
  }
  inline void setTimeout(const omni_time_t& t)
  {
    pd_timeout = t;
    pd_timeout_absolute = 0;
  }

  inline void setDeadline(unsigned long secs, unsigned long ns)
  {
    pd_timeout.assign(secs, ns);
    pd_timeout_absolute = 1;
  }
  inline void setDeadline(const omni_time_t& t)
  {
    pd_timeout = t;
    pd_timeout_absolute = 1;
  }

private:
  static _core_attr omni_thread::key_t thread_key;

  omniCallDescriptor* pd_callDescriptor;
  // Stack of call descriptors

  omni_time_t         pd_timeout;
  // Per-thread timeout

  CORBA::Boolean      pd_timeout_absolute;
  // If true, timeout is an absolute completion time, otherwise it's a
  // time per call.

  // Not implemented:
  omniCurrent(const omniCurrent&);
  omniCurrent& operator=(const omniCurrent&);
};

#ifdef _core_attr_defined
# undef _core_attr
# undef _core_attr_defined
#endif

#endif // __OMNICURRENT_H__
