//				Package : omnithread
// omnithread/nt.h		Created : 6/95 tjr
//
//    Copyright (C) 2002-2009 Apasphere Ltd
//    Copyright (C) 1995-1997 Olivetti & Oracle Research Laboratory
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
// OMNI thread implementation classes for NT threads.
//

#ifndef __omnithread_nt_h_
#define __omnithread_nt_h_

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#  define OMNI_DEFINED_WIN32_LEAN_AND_MEAN
#endif

// TryEnterCriticalSection was introduced in Windows NT 4.0, so we need to
// tell the Windows header files to target that version (or later)
#ifndef WINVER
#  define WINVER 0x0400
#endif
#ifndef _WIN32_WINNT
#  define _WIN32_WINNT WINVER
#endif

#include <windows.h>

#ifdef OMNI_DEFINED_WIN32_LEAN_AND_MEAN
#  undef WIN32_LEAN_AND_MEAN
#  undef OMNI_DEFINED_WIN32_LEAN_AND_MEAN
#endif


#ifndef __BCPLUSPLUS__
#define OMNI_THREAD_WRAPPER \
    unsigned __stdcall omni_thread_wrapper(LPVOID ptr)
#else
#define OMNI_THREAD_WRAPPER \
    void _USERENTRY omni_thread_wrapper(void *ptr)
#endif

extern "C" OMNI_THREAD_WRAPPER;

#define OMNI_MUTEX_IMPLEMENTATION			\
    CRITICAL_SECTION crit;

#define OMNI_MUTEX_LOCK_IMPLEMENTATION                  \
    EnterCriticalSection(&crit);

#define OMNI_MUTEX_TRYLOCK_IMPLEMENTATION               \
    return TryEnterCriticalSection(&crit);

#define OMNI_MUTEX_UNLOCK_IMPLEMENTATION                \
    LeaveCriticalSection(&crit);

#define OMNI_CONDITION_IMPLEMENTATION			\
    CRITICAL_SECTION crit;				\
    omni_thread* waiting_head;				\
    omni_thread* waiting_tail;

#define OMNI_SEMAPHORE_IMPLEMENTATION			\
    HANDLE nt_sem;

#define OMNI_THREAD_IMPLEMENTATION			\
    HANDLE handle;					\
    DWORD nt_id;					\
    void* return_val;					\
    HANDLE cond_semaphore;				\
    omni_thread* cond_next;				\
    omni_thread* cond_prev;				\
    BOOL cond_waiting;					\
    static int nt_priority(priority_t);			\
    friend class omni_condition;			\
    friend OMNI_THREAD_WRAPPER;

#endif
