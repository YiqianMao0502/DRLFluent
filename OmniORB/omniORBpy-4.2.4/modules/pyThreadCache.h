// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyThreadCache.h            Created on: 2000/05/26
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2005-2011 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
//
//    This file is part of the omniORBpy library
//
//    The omniORBpy library is free software; you can redistribute it
//    and/or modify it under the terms of the GNU Lesser General
//    Public License as published by the Free Software Foundation;
//    either version 2.1 of the License, or (at your option) any later
//    version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library. If not, see http://www.gnu.org/licenses/
//
//
// Description:
//    Cached mapping from threads to PyThreadState and
//    threading.Thread objects

#ifndef _pyThreadCache_h_
#define _pyThreadCache_h_


#if defined(__VMS)
#  include <pythread.h>
#else
#  include PYTHON_THREAD_INC
#endif

// Python 2.3 introduced functions to get and release the Python
// interpreter lock, and create thread state as necessary.
// Unfortunately, they are too inefficient since we would end up
// creating and destroying thread states (and Python threading.Thread
// objects) on every call. Even more unfortunately, we can't ignore
// the new functions and use our own scheme, because there are (debug)
// assertions in Python to check that the thread state is what is
// expected. So, we have to jump through all sorts of hoops to play
// nice, and it's still slower than the equivalent code in Python <=
// 2.2...


class omnipyThreadCache {
public:

  static omni_mutex* guard;

  static void init();
  static void shutdown();

  struct CacheNode {
    long             id;
    PyThreadState*   threadState;
    PyObject*        workerThread;

    CORBA::Boolean   used;
    CORBA::Boolean   can_scavenge;
    int              active;

#if PY_VERSION_HEX >= 0x02030000
    PyGILState_STATE gilstate;
#endif

    CacheNode*       next;
    CacheNode**      back;
  };

  // Fixed-size open hash table of cacheNodes
  static const unsigned int tableSize;
  static CacheNode**        table;

  // Time in seconds between runs of the node scavenger
  static unsigned int       scanPeriod;

  // Acquire the global interpreter lock
  static inline CacheNode* acquire()
  {
    CacheNode* cn;
#if PY_VERSION_HEX >= 0x02030000
    PyThreadState* tstate = PyGILState_GetThisThreadState();
    if (tstate) {
      cn = 0;
      PyEval_RestoreThread(tstate);
    }
    else
#endif
    {
      long id = PyThread_get_thread_ident();
      cn      = acquireNode(id);
      PyEval_RestoreThread(cn->threadState);
    }
    return cn;
  }

  // Release the global interpreter lock
  static inline void release(CacheNode* cn)
  {
    PyEval_SaveThread();
    if (cn)
      releaseNode(cn);
  }


  // Class lock acquires the Python interpreter lock when it is
  // created, and releases it again when it is deleted.
  class lock {
  public:
    inline lock() {
      cacheNode_ = acquire();
    }
    inline ~lock() {
      release(cacheNode_);
    }
  private:
    CacheNode* cacheNode_;
  };


  static inline CacheNode* acquireNode(long id) {
    unsigned int hash = id % tableSize; 
    CacheNode* cn;
    {
      omni_mutex_lock _l(*guard);
      OMNIORB_ASSERT(table);

      cn = table[hash];
      while (cn && cn->id != id) cn = cn->next;
      if (cn) {
	cn->used = 1;
	cn->active++;
	return cn;
      }
    }
    return addNewNode(id, hash);
  }

  static inline void releaseNode(CacheNode* cn) {
    omni_mutex_lock _l(*guard);
    cn->used = 1;
    cn->active--;
  }

  static CacheNode* addNewNode(long id, unsigned int hash);

  static void threadExit(CacheNode* cn);
};

#endif // _pyThreadCache_h_
