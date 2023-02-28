// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyThreadCache.cc           Created on: 2000/05/26
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2005-2012 Apasphere Ltd
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

#include <omnipy.h>
#include "pyThreadCache.h"

static int static_cleanup = 0;
// Set true when static data is being destroyed. Used to make sure
// Python things aren't used after they have gone away.

static omni_mutex the_guard;

omni_mutex*                    omnipyThreadCache::guard      = &the_guard;
const unsigned int             omnipyThreadCache::tableSize  = 67;
omnipyThreadCache::CacheNode** omnipyThreadCache::table      = 0;
unsigned int                   omnipyThreadCache::scanPeriod = 30;

class omnipyThreadScavenger : public omni_thread {
public:
  omnipyThreadScavenger() : dying_(0), cond_(omnipyThreadCache::guard) {
    start_undetached();
  }
  ~omnipyThreadScavenger() { }

  void kill() {
    {
      omni_mutex_lock l(*omnipyThreadCache::guard);
      dying_ = 1;
      cond_.signal();
    }
    join(0);
  }

  void* run_undetached(void*);
private:
  CORBA::Boolean   dying_;
  omni_condition   cond_;
  PyThreadState*   threadState_;
  PyObject*        workerThread_;

#if PY_VERSION_HEX >= 0x02030000
  PyGILState_STATE gilstate_;
#endif
};

class omnipyThreadData : public omni_thread::value_t {
public:
  omnipyThreadData(omnipyThreadCache::CacheNode* cn)
    : cn_(cn) {}

  ~omnipyThreadData() {
    omnipyThreadCache::threadExit(cn_);
  }

private:
  omnipyThreadCache::CacheNode* cn_;
};


static omnipyThreadScavenger* the_scavenger = 0;
static omni_thread::key_t     omnithread_key;


void
omnipyThreadCache::
init()
{
  omnithread_key = omni_thread::allocate_key();
  table          = new CacheNode*[tableSize];
  for (unsigned int i=0; i < tableSize; i++) table[i] = 0;

  the_scavenger  = new omnipyThreadScavenger();
}


void
omnipyThreadCache::
shutdown()
{
  if (the_scavenger) the_scavenger->kill();
  the_scavenger = 0;

  table = 0;
}


omnipyThreadCache::CacheNode*
omnipyThreadCache::
addNewNode(long id, unsigned int hash)
{
  CacheNode* cn = new CacheNode;
  cn->id = id;

  omni_thread* ot = omni_thread::self();

  if (ot) {
    if (omniORB::trace(20)) {
      omniORB::logger l;
      l << "Creating new Python state for thread id " << id << "\n";
    }

#if PY_VERSION_HEX >= 0x02030000
    cn->gilstate     = PyGILState_Ensure();
    cn->threadState  = PyThreadState_Get();
    cn->can_scavenge = 0;
#else
    PyEval_AcquireLock();
    cn->threadState  = PyThreadState_New(omniPy::pyInterpreter);
    cn->can_scavenge = 0;
    PyThreadState_Swap(cn->threadState);
#endif
    omni_thread::value_t* tv = ot->set_value(omnithread_key,
					     new omnipyThreadData(cn));
    OMNIORB_ASSERT(tv);
  }
  else {
    if (omniORB::trace(20)) {
      omniORB::logger l;
      l << "Creating new Python state for non-omni thread id " << id << "\n";
    }

#if PY_VERSION_HEX >= 0x02030000
    cn->gilstate     = PyGILState_Ensure();
    cn->threadState  = PyThreadState_Get();
    cn->can_scavenge = 1;
#else
    PyEval_AcquireLock();
    cn->threadState  = PyThreadState_New(omniPy::pyInterpreter);
    cn->can_scavenge = 1;
    PyThreadState_Swap(cn->threadState);
#endif
  }    

  cn->used         = 1;
  cn->active       = 1;
  cn->workerThread = 0;

  // Insert into hash table
  {
    omni_mutex_lock _l(*guard);
    CacheNode* he = table[hash];
    cn->next = he;
    cn->back = &(table[hash]);
    if (he) he->back = &(cn->next);
    table[hash] = cn;
  }

  // Create omniORB worker thread threading state.
  //
  // Note that this happens after inserting the CacheNode into the
  // hash table, and outside any locks. This is because there is a
  // remote possibility that while executing the Python code to
  // create the worker thread object, the thread will end up calling
  // back through the thread cache (either because of a registered
  // thread hook, or because the cyclic garbage collector runs). In
  // that case, the re-entry will get a valid Python thread state,
  // albeit without a threading.Thread object.

  cn->workerThread = PyEval_CallObject(omniPy::pyWorkerThreadClass,
				       omniPy::pyEmptyTuple);
  if (!cn->workerThread) {
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l << "Exception trying to create worker thread.\n";
      }
      PyErr_Print();
    }
    else {
      PyErr_Clear();
    }
  }
  PyEval_SaveThread();

  return cn;
}


void
omnipyThreadCache::
threadExit(CacheNode* cn)
{
  OMNIORB_ASSERT(!cn->active);

  if (static_cleanup) {
    // Too late to do anything now
    return;
  }

  if (omniORB::trace(20)) {
    omniORB::logger l;
    l << "Deleting Python state for thread id " << cn->id
      << " (thread exit)\n";
  }

  if (table) {
    // Remove the CacheNode from the table
    omni_mutex_lock _l(*guard);

    if (cn->back) {
      CacheNode* cnn = cn->next;
      *(cn->back) = cnn;
      if (cnn) cnn->back = cn->back;
    }
  }

  // Acquire Python thread lock and remove Python-world things
  PyEval_RestoreThread(cn->threadState);

  if (cn->workerThread) {
    PyObject* tmp = PyObject_CallMethod(cn->workerThread, (char*)"delete", 0);
    if (!tmp) {
      if (omniORB::trace(10)) {
	{
	  omniORB::logger l;
	  l << "Exception trying to delete worker thread.\n";
	}
	PyErr_Print();
      }
      else {
	PyErr_Clear();
      }
    }
    Py_XDECREF(tmp);
    Py_DECREF(cn->workerThread);
  }

#if PY_VERSION_HEX >= 0x02030000
  PyGILState_Release(cn->gilstate);
#else
  PyThreadState_Swap(0);
  PyThreadState_Clear(cn->threadState);
  PyThreadState_Delete(cn->threadState);
  PyEval_ReleaseLock();
#endif

  delete cn;
}


void*
omnipyThreadScavenger::
run_undetached(void*)
{
  unsigned long abs_sec, abs_nsec;
  unsigned int  i;
  omnipyThreadCache::CacheNode *cn, *cnn, *to_remove;

  omniORB::logs(15, "Python thread state scavenger start.");

  // Create a thread state for the scavenger thread itself
#if PY_VERSION_HEX >= 0x02030000
  gilstate_    = PyGILState_Ensure();
  threadState_ = PyThreadState_Get();
#else
  PyEval_AcquireLock();
  threadState_  = PyThreadState_New(omniPy::pyInterpreter);
  PyThreadState_Swap(threadState_);
#endif

  workerThread_ = PyEval_CallObject(omniPy::pyWorkerThreadClass,
				    omniPy::pyEmptyTuple);
  if (!workerThread_) {
    if (omniORB::trace(2)) {
      omniORB::logs(2, "Exception trying to create WorkerThread for thread "
                    "state scavenger.");
      PyErr_Print();
    }
    else {
      PyErr_Clear();
    }
  }

  PyEval_SaveThread();

  // Main loop
  while (!dying_) {

    to_remove = 0;

    {
      omni_mutex_lock _l(*omnipyThreadCache::guard);

      omni_thread::get_time(&abs_sec,&abs_nsec);
      abs_sec += omnipyThreadCache::scanPeriod;
      cond_.timedwait(abs_sec, abs_nsec);

      if (dying_) break;

      omniORB::logs(15, "Scanning Python thread states.");
    
      for (i=0; i < omnipyThreadCache::tableSize; i++) {
	cn = omnipyThreadCache::table[i];

	while (cn) {
	  cnn = cn->next;
	  if (cn->can_scavenge && !cn->active) {

	    if (cn->used) {
	      cn->used = 0;
	    }
	    else {
	      // Unlink from hash table
	      *(cn->back) = cnn;
	      if (cnn) cnn->back = cn->back;
	      
	      if (omniORB::trace(20)) {
		omniORB::logger l;
		l << "Will delete Python state for thread id "
		  << cn->id << " (scavenged)\n";
	      }
	      cn->next = to_remove;
	      to_remove = cn;
	    }
	  }
	  cn = cnn;
	}
      }
    }

    for (cn = to_remove; cn; cn=cnn) {
      cnn = cn->next;

      if (omniORB::trace(20)) {
	omniORB::logger l;
	l << "Delete Python state for thread id " 
	  << cn->id << " (scavenged)\n";
      }

      // Acquire Python thread lock and remove Python-world things
      PyEval_RestoreThread(threadState_);

      if (cn->workerThread) {
        PyObject* tmp = PyObject_CallMethod(cn->workerThread,
                                            (char*)"delete", 0);
	if (!tmp) {
	  if (omniORB::trace(1)) {
	    {
	      omniORB::logger l;
	      l << "Exception trying to delete worker thread.\n";
	    }
	    PyErr_Print();
	  }
	  else {
	    PyErr_Clear();
	  }
	}
	Py_XDECREF(tmp);
        Py_DECREF(cn->workerThread);
      }
      PyThreadState_Clear(cn->threadState);
      PyThreadState_Delete(cn->threadState);

      PyEval_SaveThread();

      delete cn;
    }
  }

  omnipyThreadCache::CacheNode** table;
  {
    omni_mutex_lock _l(*omnipyThreadCache::guard);
    table = omnipyThreadCache::table;
    omnipyThreadCache::table = 0;
  }

  // Delete all table entries
  PyEval_RestoreThread(threadState_);

  for (i=0; i < omnipyThreadCache::tableSize; i++) {
    cn = table[i];

    while (cn) {
      if (cn->can_scavenge) {
	if (omniORB::trace(20)) {
	  omniORB::logger l;
	  l << "Deleting Python state for thread id "
	    << cn->id << " (shutdown)\n";
	}

	if (cn->workerThread) {
          PyObject* tmp = PyObject_CallMethod(cn->workerThread,
                                              (char*)"delete", 0);
          if (!tmp) PyErr_Clear();
	  Py_XDECREF(tmp);
          Py_DECREF(cn->workerThread);
	}
	PyThreadState_Clear(cn->threadState);
	PyThreadState_Delete(cn->threadState);

	// Remove the CacheNode
	cnn = cn->next;
	delete cn;
	cn = cnn;
      }
      else {
	if (omniORB::trace(20)) {
	  omniORB::logger l;
	  l << "Remove Python state for thread id "
	    << cn->id << " from cache (shutdown)\n";
	}
	cnn = cn->next;
	cn->next = 0;
	cn->back = 0;
	cn = cnn;
      }
    }
  }

  delete [] table;

  // Remove this thread's Python state
  if (workerThread_) {
    PyObject* tmp = PyObject_CallMethod(workerThread_, (char*)"delete", 0);
    if (!tmp) PyErr_Clear();
    Py_XDECREF(tmp);
    Py_DECREF(workerThread_);
  }

#if PY_VERSION_HEX >= 0x02030000
  PyGILState_Release(gilstate_);
#else
  PyThreadState_Swap(0);
  PyThreadState_Clear(threadState_);
  PyThreadState_Delete(threadState_);
  PyEval_ReleaseLock();
#endif

  omniORB::logs(15, "Python thread state scavenger exit.");

  return 0;
}


class _omnipy_cleanup_detector {
public:
  inline ~_omnipy_cleanup_detector() { static_cleanup = 1; }
};

static _omnipy_cleanup_detector _the_omnipy_cleanup_detector;
