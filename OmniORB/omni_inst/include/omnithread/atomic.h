// -*- Mode: C++; -*-
//				Package : omnithread
// omnithread/atomic.h		Created : 2011/04/14 dgrisby
//
//    Copyright (C) 2011 Apasphere Ltd
//
//    This file is part of the omnithread library
//
//    The omnithread library is free software; you can redistribute it and/or
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
// Atomic operations for platforms that support them.

#ifndef __omnithread_atomic_h_
#define __omnithread_atomic_h_


#ifdef OMNI_HAVE_SYNC_ADD_AND_FETCH

#  define OMNI_REFCOUNT_DEFINED

class _OMNITHREAD_NTDLL_ omni_refcount {
public:
  inline omni_refcount(int start) : count(start) {}
  inline ~omni_refcount() {}

  // Atomically increment reference count and return new value
  inline int inc() {
    return __sync_add_and_fetch(&count, 1);
  }

  // Atomically decrement reference count and return new value
  inline int dec() {
    return __sync_sub_and_fetch(&count, 1);
  }

  // Return snapshot of current value. Real count may have changed by
  // the time the value is looked at!
  inline int value() {
    return count;
  }

private:
  volatile int count;
};

#endif // OMNI_ATOMIC_GCC_SYNC


#if defined(_MSC_VER) && !defined(OMNI_REFCOUNT_DEFINED)

#  define OMNI_REFCOUNT_DEFINED

class _OMNITHREAD_NTDLL_ omni_refcount {
public:
  inline omni_refcount(int start) : count(start) {}
  inline ~omni_refcount() {}

  // Atomically increment reference count and return new value
  inline int inc() {
    return InterlockedIncrement(&count);
  }

  // Atomically decrement reference count and return new value
  inline int dec() {
    return InterlockedDecrement(&count);
  }

  // Return snapshot of current value. Real count may have changed by
  // the time the value is looked at!
  inline int value() {
    return count;
  }

private:
  volatile LONG count;
};

#endif // _MSC_VER


#endif // __omnithread_atomic_h_
