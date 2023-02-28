// -*- Mode: C++; -*-
//                            Package   : omniORB
// invoker.cc                 Created on: 11 Apr 2001
//                            Authors   : Duncan Grisby
//                                        Sai Lai Lo
//
//    Copyright (C) 2002-2013 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
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
// Description:
//	*** PROPRIETARY INTERFACE ***
//

#include <omniORB4/CORBA.h>
#include <omniORB4/omniInterceptors.h>
#include <omniORB4/omniAsyncInvoker.h>
#include <interceptors.h>
#include <orbParameters.h>
#include <orbOptions.h>
#include <initialiser.h>
#include <stdlib.h>

OMNI_USING_NAMESPACE(omni)

unsigned int omniAsyncInvoker::idle_timeout = 10;



////////////////////////////////////////////////////////////////////////////
// Configuration

CORBA::ULong   orbParameters::maxServerThreadPoolSize        = 100;
//   The max. no. of threads the server will allocate to do various
//   ORB tasks. This number does not include the dedicated thread
//   per connection when the threadPerConnectionPolicy is in effect
//
//   Valid values = (n >= 1) 

CORBA::ULong   orbParameters::maxClientThreadPoolSize        = 100;
//   The max. no. of threads a client will allocate to do asynchronous
//   calls.
//
//   Valid values = (n >= 1) 



////////////////////////////////////////////////////////////////////////////
// Helpers

static inline const char* plural(CORBA::ULong val)
{
  return val == 1 ? "" : "s";
}


///////////////////////////////////////////////////////////////////////////
// Worker interceptor

class omniAsyncWorkerInfo
  : public omniInterceptors::createThread_T::info_T {
public:
  omniAsyncWorkerInfo(omniAsyncWorker* worker) :
    pd_worker(worker), pd_elmt(omniInterceptorP::createThread) {}

  void run();
  omni_thread* self();

private:
  omniAsyncWorker*        pd_worker;
  omniInterceptorP::elmT* pd_elmt;
};


///////////////////////////////////////////////////////////////////////////
// Worker

class omniAsyncWorker : public omni_thread {
public:

  inline omniAsyncWorker(omniAsyncInvoker* invoker)
    : pd_invoker(invoker),
      pd_lock(invoker->pd_lock),
      pd_cond(&invoker->pd_lock, "omniAsyncWorker::pd_cond"),
      pd_id(id()),
      pd_pool(0),
      pd_task(0),
      pd_next_idle(0), pd_prev_idle(0)
  {
    pd_total_at_start = pd_invoker->workerStartLocked();
    start();
  }

  ~omniAsyncWorker()
  {
    unsigned int total = pd_invoker->workerStop();

    if (omniORB::trace(10)) {
      omniORB::logger log;
      log << "AsyncInvoker: thread id " << pd_id
          << " has exited. Total threads = " << total << ".\n";
    }
  }

  void run(void*) {
    if (omniORB::trace(10)) {
      omniORB::logger log;
      log << "AsyncInvoker: thread id " << pd_id
          << " has started. Total threads = " << pd_total_at_start << ".\n";
    }
    omniAsyncWorkerInfo info(this);
    info.run();
  }

  void mid_run();
  void real_run();

  inline void assign(omniAsyncPool* pool)
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);
    OMNIORB_ASSERT(!pd_pool);

    pd_pool = pool;
    pd_cond.signal();
  }

  inline void handle(omniTask* task)
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);
    OMNIORB_ASSERT(pd_pool);
    OMNIORB_ASSERT(!pd_task);

    pd_task = task;
    pd_cond.signal();
  }

  inline void stop()
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);
    pd_cond.signal();
  }

  inline void addIdle();
  inline void remIdle();

private:
  omniAsyncInvoker*    pd_invoker;
  omni_tracedmutex&    pd_lock;
  omni_tracedcondition pd_cond;
  int                  pd_id;
  unsigned int         pd_total_at_start;

  omniAsyncPool*       pd_pool;
  omniTask*            pd_task;
  omniAsyncWorker*     pd_next_idle;
  omniAsyncWorker**    pd_prev_idle;

  omniAsyncWorker(const omniAsyncWorker&);
  omniAsyncWorker& operator=(const omniAsyncWorker&);

  friend class omniAsyncPool;
  friend class omniAsyncInvoker;
};


///////////////////////////////////////////////////////////////////////////
// Pool

class omniAsyncPool {
public:
  inline omniAsyncPool(omniAsyncInvoker* invoker,
                       const char*       purpose,
                       unsigned int      max)
    : pd_invoker(invoker), pd_lock(invoker->pd_lock), pd_purpose(purpose),
      pd_total_threads(0), pd_pool_threads(0), pd_max(max),
      pd_idle_threads(0)
  {}

  virtual ~omniAsyncPool() { }

  inline const char* purpose() { return pd_purpose; }

  CORBA::Boolean insert(omniTask* task);

  void stop();
  
  virtual void workerRun(omniAsyncWorker* worker) = 0;

  inline unsigned int workerStart(omniAsyncWorker* worker)
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);
    return ++pd_total_threads;
  }

  inline unsigned int workerStop(omniAsyncWorker* worker)
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);
    return --pd_total_threads;
  }

private:

  omniAsyncInvoker* pd_invoker;
  omni_tracedmutex& pd_lock;

  const char*       pd_purpose;
  unsigned int      pd_total_threads;
  unsigned int      pd_pool_threads;
  unsigned int      pd_max;
  omniTaskLink      pd_tasks;
  omniAsyncWorker*  pd_idle_threads;

  friend class omniAsyncWorker;
};


CORBA::Boolean
omniAsyncPool::insert(omniTask* task)
{
  omni_tracedmutex_lock l(pd_lock);

  omniAsyncWorker* worker;

  if (pd_idle_threads) {
    // Use idle worker
    worker = pd_idle_threads;
    worker->remIdle();
    worker->handle(task);
  }
  else if (task->category() == omniTask::ImmediateDispatch ||
           pd_pool_threads < pd_max) {

    // Get a new worker from the invoker
    worker = pd_invoker->getWorker(this);

    if (!worker) {
      if (task->category() == omniTask::ImmediateDispatch) {
        omniORB::logs(2, "Unable to start new thread. Operation failed.");
        return 0;
      }
      else {
        omniORB::logs(2, "Unable to start new thread. Task queued.");
        task->enq(pd_tasks);
        return 1;
      }
    }
    ++pd_pool_threads;
    worker->handle(task);
  }
  else {
    // Add to queue
    task->enq(pd_tasks);
  }

  return 1;
}


void
omniAsyncPool::stop()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);

  omniAsyncWorker* worker = pd_idle_threads;
  while (worker) {
    worker->stop();
    worker = worker->pd_next_idle;
  }
}


///////////////////////////////////////////////////////////////////////////
// General pool

class omniAsyncPoolGeneral : public omniAsyncPool {
public:
  inline omniAsyncPoolGeneral(omniAsyncInvoker* invoker)
    : omniAsyncPool(invoker, "general", 100000)
  {}

  void workerRun(omniAsyncWorker* worker)
  {
    worker->real_run();
  }
};


///////////////////////////////////////////////////////////////////////////
// Server-side pool

class omniServerWorkerInfo
  : public omniInterceptors::assignUpcallThread_T::info_T {
public:
  inline omniServerWorkerInfo(omniAsyncWorker* worker) :
    pd_worker(worker), pd_elmt(omniInterceptorP::assignUpcallThread) {}

  void run();
  omni_thread* self();
  
private:
  omniAsyncWorker*        pd_worker;
  omniInterceptorP::elmT* pd_elmt;
};

class omniAsyncPoolServer : public omniAsyncPool {
public:
  inline omniAsyncPoolServer(omniAsyncInvoker* invoker)
    : omniAsyncPool(invoker, "server", orbParameters::maxServerThreadPoolSize)
  {}

  void workerRun(omniAsyncWorker* worker)
  {
    omniServerWorkerInfo info(worker);
    info.run();
  }
};


///////////////////////////////////////////////////////////////////////////
// Client-side pool

class omniClientWorkerInfo
  : public omniInterceptors::assignAMIThread_T::info_T {
public:
  inline omniClientWorkerInfo(omniAsyncWorker* worker) :
    pd_worker(worker), pd_elmt(omniInterceptorP::assignAMIThread) {}

  void run();
  omni_thread* self();
  
private:
  omniAsyncWorker*        pd_worker;
  omniInterceptorP::elmT* pd_elmt;
};

class omniAsyncPoolClient : public omniAsyncPool {
public:
  inline omniAsyncPoolClient(omniAsyncInvoker* invoker)
    : omniAsyncPool(invoker, "client", orbParameters::maxClientThreadPoolSize)
  {}

  void workerRun(omniAsyncWorker* worker)
  {
    omniClientWorkerInfo info(worker);
    info.run();
  }
};


///////////////////////////////////////////////////////////////////////////
// Dedicated thread queue

class omniAsyncDedicated {
public:
  inline omniAsyncDedicated(omniAsyncInvoker* invoker)
    : pd_invoker(invoker),
      pd_lock(invoker->pd_lock),
      pd_cond(&pd_lock, "omniAsyncDedicated::pd_cond")
  {}

  inline ~omniAsyncDedicated() {}

  CORBA::Boolean insert(omniTask* task);
  CORBA::Boolean work_pending();
  void perform(unsigned long secs = 0, unsigned long nanosecs = 0);
  void shutdown();

private:
  omniAsyncInvoker*    pd_invoker;
  omni_tracedmutex&    pd_lock;
  omni_tracedcondition pd_cond;

  omniTaskLink         pd_tasks;
};


CORBA::Boolean
omniAsyncDedicated::insert(omniTask* task)
{
  omni_tracedmutex_lock l(pd_lock);
  task->enq(pd_tasks);
  pd_cond.signal();
  return 1;
}

CORBA::Boolean
omniAsyncDedicated::work_pending()
{
  omni_tracedmutex_lock l(pd_lock);
  return !omniTaskLink::is_empty(pd_tasks);
}

void
omniAsyncDedicated::perform(unsigned long secs, unsigned long nanosecs)
{
  pd_invoker->workerStart();

  omniTask*      task = 0;
  CORBA::Boolean go   = 1;

  while (go) {
    {
      omni_tracedmutex_lock l(pd_lock);

      while (omniTaskLink::is_empty(pd_tasks) &&
             pd_invoker->pd_keep_working) {
        
        // Wait for a task
        if (secs || nanosecs) {
          if (pd_cond.timedwait(secs, nanosecs) == 0) {
            // Timed out
            go = 0;
            break;
          }
        }
        else {
          pd_cond.wait();
        }
      }
    
      if (!omniTaskLink::is_empty(pd_tasks)) {
        // Queued task available
        task = (omniTask*)pd_tasks.next;
        task->deq();
      }
      else if (!pd_invoker->pd_keep_working) {
        go = 0;
      }
    }
    if (task) {
      try {
        task->execute();
      }
      catch (...) {
        omniORB::logs(1, "AsyncInvoker: Warning: unexpected exception "
                      "caught while executing a task on the main thread.");
      }
      task = 0;
    }
  }
  pd_invoker->workerStop();
}


void
omniAsyncDedicated::shutdown()
{
  omniORB::logs(25, "Shut down dedicated thread queue.");
  omni_tracedmutex_lock l(pd_lock);
  pd_cond.broadcast();
}



///////////////////////////////////////////////////////////////////////////
// Worker

void
omniAsyncWorker::mid_run()
{
  unsigned int total;

  while (1) {
    {
      omni_tracedmutex_lock l(pd_lock);

      if (!pd_pool) {
        addIdle();

        omni_time_t deadline;
        omni_thread::get_time(deadline, omniAsyncInvoker::idle_timeout);
        
        while (pd_invoker->pd_keep_working && !pd_pool) {

          int signalled = pd_cond.timedwait(deadline);
          if (!signalled) // timed out
            break;
        }

        if (!pd_pool) {
          remIdle();
          return;
        }
      }
      total = pd_pool->workerStart(this);
    }

    const char* purpose = pd_pool->purpose();

    if (omniORB::trace(10)) {
      omniORB::logger log;
      log << "AsyncInvoker: thread id " << pd_id << " assigned to "
          << purpose << " tasks. Total " << purpose << " threads = "
          << total << ".\n";
    }

    pd_pool->workerRun(this);

    {
      omni_tracedmutex_lock l(pd_lock);
      total   = pd_pool->workerStop(this);
      pd_pool = 0;
    }

    if (omniORB::trace(10)) {
      omniORB::logger log;
      log << "AsyncInvoker: thread id " << pd_id << " released from "
          << purpose << " tasks. Total " << purpose << " threads = "
          << total << ".\n";
    }
  }
}


void omniAsyncWorker::real_run()
{
  OMNIORB_ASSERT(pd_pool);

  omni_thread*   self_thread = omni_thread::self();
  CORBA::Boolean immediate   = 0;

  while (1) {
    {
      omni_tracedmutex_lock l(pd_lock);

      if (!pd_task) {
        if (!omniTaskLink::is_empty(pd_pool->pd_tasks)) {
          // Queued task available
          pd_task = (omniTask*)pd_pool->pd_tasks.next;
          pd_task->deq();
        }
        else {
          addIdle();

          omni_time_t deadline;
          omni_thread::get_time(deadline, omniAsyncInvoker::idle_timeout);
        
          while (pd_invoker->pd_keep_working && !pd_task) {
            int signalled = pd_cond.timedwait(deadline);
            if (!signalled) // timed out
              break;
          }

          if (!pd_task) {
            remIdle();
            break;
          }
        }
      }
      if ((immediate = pd_task->category() == omniTask::ImmediateDispatch)) {
        // This thread is no longer counted towards the pool total
        --pd_pool->pd_pool_threads;
        
        if (omniORB::trace(25)) {
          omniORB::logger log;
          log << "AsyncInvoker: thread id " << pd_id
              << " performing immediate " << pd_pool->purpose() << " task.\n";
        }
      }
    }

    try {
      pd_task->pd_self = self_thread;
      pd_task->execute();
    }
    catch(...) {
      omniORB::logs(1, "AsyncInvoker: Warning: unexpected exception "
                    "caught while executing a task.");
    }

    {
      omni_tracedmutex_lock l(pd_lock);
      pd_task = 0;

      if (immediate) {
        // Thread is back in the pool
        ++pd_pool->pd_pool_threads;

        if (omniORB::trace(25)) {
          omniORB::logger log;
          log << "AsyncInvoker: thread id " << pd_id
              << " finished immediate " << pd_pool->purpose() << " task.\n";
        }
      }

      if (pd_pool->pd_pool_threads > pd_pool->pd_max)
        break;
    }
  }
  
  {
    omni_tracedmutex_lock l(pd_lock);
    --pd_pool->pd_pool_threads;
  }
}


void
omniAsyncWorker::addIdle()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);
  OMNIORB_ASSERT(!pd_prev_idle);

  if (pd_pool) {
    pd_next_idle = pd_pool->pd_idle_threads;
    pd_prev_idle = &pd_pool->pd_idle_threads;
    pd_pool->pd_idle_threads = this;
  }
  else {
    pd_next_idle = pd_invoker->pd_idle_threads;
    pd_prev_idle = &pd_invoker->pd_idle_threads;
    pd_invoker->pd_idle_threads = this;      
  }
  if (pd_next_idle)
    pd_next_idle->pd_prev_idle = &pd_next_idle;
}


void
omniAsyncWorker::remIdle()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);
  OMNIORB_ASSERT(pd_prev_idle);

  if (pd_next_idle)
    pd_next_idle->pd_prev_idle = pd_prev_idle;
    
  *pd_prev_idle = pd_next_idle;

  pd_next_idle = 0;
  pd_prev_idle = 0;
}


///////////////////////////////////////////////////////////////////////////
// Worker interceptors

void
omniAsyncWorkerInfo::run()
{
  if (pd_elmt) {
    omniInterceptors::createThread_T::interceptFunc f =
      (omniInterceptors::createThread_T::interceptFunc)pd_elmt->func;
    pd_elmt = pd_elmt->next;
    f(*this);
  }
  else
    pd_worker->mid_run();
}

omni_thread*
omniAsyncWorkerInfo::self()
{
  return pd_worker;
}


void
omniServerWorkerInfo::run()
{
  if (pd_elmt) {
    omniInterceptors::assignUpcallThread_T::interceptFunc f =
      (omniInterceptors::assignUpcallThread_T::interceptFunc)pd_elmt->func;
    pd_elmt = pd_elmt->next;
    f(*this);
  }
  else
    pd_worker->real_run();
}

omni_thread*
omniServerWorkerInfo::self()
{
  return pd_worker;
}


void
omniClientWorkerInfo::run()
{
  if (pd_elmt) {
    omniInterceptors::assignAMIThread_T::interceptFunc f =
      (omniInterceptors::assignAMIThread_T::interceptFunc)pd_elmt->func;
    pd_elmt = pd_elmt->next;
    f(*this);
  }
  else
    pd_worker->real_run();
}

omni_thread*
omniClientWorkerInfo::self()
{
  return pd_worker;
}


///////////////////////////////////////////////////////////////////////////
// Invoker

omniAsyncInvoker::omniAsyncInvoker()
  : pd_lock("omniAsyncInvoker::pd_lock"),
    pd_idle_cond(&pd_lock, "omniAsyncInvoker::pd_idle_cond"),
    pd_total_threads(0),
    pd_idle_threads(0),
    pd_general  (new omniAsyncPoolGeneral(this)),
    pd_server   (new omniAsyncPoolServer(this)),
    pd_client   (new omniAsyncPoolClient(this)),
    pd_dedicated(new omniAsyncDedicated(this)),
    pd_keep_working(1)
{
}


omniAsyncInvoker::~omniAsyncInvoker()
{
  CORBA::Boolean ok = 1;
  {
    omni_tracedmutex_lock l(pd_lock);

    pd_keep_working = 0;

    pd_client->stop();
    pd_server->stop();
    pd_general->stop();

    omniAsyncWorker* worker = pd_idle_threads;
    while (worker) {
      worker->stop();
      worker = worker->pd_next_idle;
    }

    // Wait for threads to exit
    if (pd_total_threads) {

      omni_time_t  deadline;
      unsigned int timeout;

      if (orbParameters::scanGranularity)
        timeout = orbParameters::scanGranularity;
      else
        timeout = 5;
    
      omni_thread::get_time(deadline, timeout);

      if (omniORB::trace(25)) {
        omniORB::logger l;
        l << "Wait for " << pd_total_threads << " invoker thread"
          << plural(pd_total_threads) << " to finish.\n";
      }

      int go = 1;
      while (go && pd_total_threads) {
        go = pd_idle_cond.timedwait(deadline);
      }

      if (go) {
        omniORB::logs(25, "Invoker threads finished.");
      }
      else {
        ok = 0;
        if (omniORB::trace(25)) {
          omniORB::logger log;
          log << "Timed out. " << pd_total_threads
              << " invoker threads remaining.\n";
        }
      }
    }
  }
  if (ok) {
    delete pd_dedicated;
    delete pd_client;
    delete pd_server;
    delete pd_general;
  }
  omniORB::logs(10, "AsyncInvoker: deleted.");
}


CORBA::Boolean
omniAsyncInvoker::insert(omniTask* t)
{
  if (t->category() == omniTask::DedicatedThread) {
    return pd_dedicated->insert(t);
  }
  else {
    switch (t->purpose()) {

    case omniTask::General:          return pd_general->insert(t);
    case omniTask::ServerUpcall:     return pd_server->insert(t);
    case omniTask::ClientInvocation: return pd_client->insert(t);
    }
  }
  return 0;
}

CORBA::Boolean
omniAsyncInvoker::work_pending()
{
  return pd_dedicated->work_pending();
}

void
omniAsyncInvoker::perform(unsigned long secs, unsigned long nanosecs)
{
  return pd_dedicated->perform(secs, nanosecs);
}

void
omniAsyncInvoker::shutdown()
{
  omniORB::logs(15, "Shut down AsyncInvoker...");
  {
    omni_tracedmutex_lock l(pd_lock);
    pd_keep_working = 0;
  }
  pd_dedicated->shutdown();
  omniORB::logs(15, "AsyncInvoker shut down.");
}

omniAsyncWorker*
omniAsyncInvoker::getWorker(omniAsyncPool* pool)
{ 
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);

  omniAsyncWorker* worker;

  if (pd_idle_threads) {
    worker = pd_idle_threads;
    worker->remIdle();
  }
  else {
    try {
      worker = new omniAsyncWorker(this);
    }
    catch (omni_thread_fatal&) {
      omniORB::logs(5, "AsyncInvoker: failed to create new thread.");
      return 0;
    }
  }

  worker->assign(pool);
  return worker;
}



OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class idleThreadTimeoutHandler : public orbOptions::Handler {
public:

  idleThreadTimeoutHandler() : 
    orbOptions::Handler("idleThreadTimeout",
			"idleThreadTimeout = n > 0 sec",
			1,
			"-ORBidleThreadTimeout < n > 0 sec >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v == 0) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_greater_than_zero_ulong_msg);
    }
    omniAsyncInvoker::idle_timeout = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),omniAsyncInvoker::idle_timeout,
			   result);
  }
};

static idleThreadTimeoutHandler idleThreadTimeoutHandler_;


/////////////////////////////////////////////////////////////////////////////
class maxServerThreadPoolSizeHandler : public orbOptions::Handler {
public:

  maxServerThreadPoolSizeHandler() : 
    orbOptions::Handler("maxServerThreadPoolSize",
			"maxServerThreadPoolSize = n >= 1",
			1,
			"-ORBmaxServerThreadPoolSize < n >= 1 >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 1) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_greater_than_zero_ulong_msg);
    }
    orbParameters::maxServerThreadPoolSize = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::maxServerThreadPoolSize,
			   result);
  }
};

static maxServerThreadPoolSizeHandler maxServerThreadPoolSizeHandler_;


/////////////////////////////////////////////////////////////////////////////
class maxClientThreadPoolSizeHandler : public orbOptions::Handler {
public:

  maxClientThreadPoolSizeHandler() : 
    orbOptions::Handler("maxClientThreadPoolSize",
			"maxClientThreadPoolSize = n >= 1",
			1,
			"-ORBmaxClientThreadPoolSize < n >= 1 >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 1) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_greater_than_zero_ulong_msg);
    }
    orbParameters::maxClientThreadPoolSize = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::maxClientThreadPoolSize,
			   result);
  }
};

static maxClientThreadPoolSizeHandler maxClientThreadPoolSizeHandler_;


////////////////////////////////////////////////////////////////////////
// Module initialiser
////////////////////////////////////////////////////////////////////////

class omni_invoker_initialiser : public omniInitialiser {
public:
  omni_invoker_initialiser() {
    orbOptions::singleton().registerHandler(idleThreadTimeoutHandler_);
    orbOptions::singleton().registerHandler(maxServerThreadPoolSizeHandler_);
    orbOptions::singleton().registerHandler(maxClientThreadPoolSizeHandler_);
  }

  void attach() { }
  void detach() { }
};

static omni_invoker_initialiser initialiser;

omniInitialiser& omni_invoker_initialiser_ = initialiser;


OMNI_NAMESPACE_END(omni)
