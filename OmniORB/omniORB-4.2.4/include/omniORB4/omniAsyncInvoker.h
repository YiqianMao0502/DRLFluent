// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniAsyncInvoker.h         Created on: 20 Dec 2000
//                            Authors   : Duncan Grisby
//                                        Sai Lai Lo
//
//    Copyright (C) 2006-2013 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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

#ifndef __OMNIASYNCINVOKER_H__
#define __OMNIASYNCINVOKER_H__

// Usage:
//
//    An omniAsyncInvoker, or an invoker in short, is an object that
//    executes commands asynchronously using a pool of threads it manages
//    internally.
//
//    The command to be executed is an omniTask instance. The omniTask
//    class is based on the Command Pattern (Design Patterns, chapter 5).
//
//    The client creates an instance of the omniTask and registers it with
//    the omniAsyncInvoker using the insert() method. The invoker will
//    dispatch a thread to call the execute() method in the omniTask object.
//    Notice that the call to the execute() method is always done by
//    another thread, hence giving the invoker its asynchronous nature.
//
//    Depending on the category of an omniTask, the invoker will
//    choose a thread to execute the task in one of the following
//    ways:
//
//       AnyTime: the task will be executed by one of the threads in the
//                pool. If no thread is available, the task may be queued
//                indefinitely until a thread is available
//
//       DedicatedThread: the task is executed by a particular thread. This
//                        is particularly useful if the task involves calling
//                        into a non-thread safe library and it is desirable
//                        to always use the same thread to do so.
//
//                        The thread that will execute the command is always
//                        the thread that calls the invoker's perform()
//                        method.
//
//                        Notice that the task may never be dispatched if
//                        no external thread calls perform().
//
//       ImmediateDispatch: A thread will be dispatched to execute the task
//                          immediately.
//
//                          To do so the invoker may have to spawn a new
//                          thread to do the work. If the invoker fails to
//                          spawn a thread for various reasons, the insert()
//                          call fails.
//
//   Once the execute() method is called on a task, the invoker will forget
//   about it. If the task object is heap allocated, it has to be garbage
//   collected by some external means. The simplist approach is to delete
//   the task object before the execute() method returns.


class omniAsyncPool;
class omniAsyncDedicated;
class omniAsyncWorker;


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

class omniTaskLink {
public:
  omniTaskLink* next;
  omniTaskLink* prev;

  inline omniTaskLink()
  {
    next = prev = this;
  }

  inline void enq(omniTaskLink& head)
  {
    next = head.prev->next;
    head.prev->next = this;
    prev = head.prev;
    head.prev = this;
  }

  inline void deq()
  {
    prev->next = next;
    next->prev = prev;
  }

  static inline unsigned int is_empty(omniTaskLink& head)
  {
    return (head.next == &head);
  }

private:
  omniTaskLink(const omniTaskLink&);
  omniTaskLink& operator=(const omniTaskLink&);
};


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

class omniTask : public omniTaskLink {
public:

  // What sort of task dispatch?
  enum Category { AnyTime,           // Run whenever a thread is free
		  ImmediateDispatch, // Run immediately
		  DedicatedThread    // Use the dedicated thread
  };

  // What is this task doing?
  enum Purpose { General,            // General task
                 ServerUpcall,       // Server-side upcall into application code
                 ClientInvocation    // Client-side operation invocation
  };

  virtual void execute() = 0;

  inline omniTask(Category cat = AnyTime, Purpose pur = General)
    : pd_category(cat), pd_purpose(pur), pd_self(0) {}

  virtual ~omniTask() {}

  inline Category     category()           { return pd_category; }
  inline void         category(Category c) { pd_category = c; }
  inline Purpose      purpose()            { return pd_purpose; }
  inline void         purpose(Purpose p)   { pd_purpose = p; }

  inline omni_thread* selfThread()         { return pd_self; }
  // The worker thread assigned to handle the task. Set by the worker.

private:
  Category     pd_category;
  Purpose      pd_purpose;
  omni_thread* pd_self;

  omniTask(const omniTask&);
  omniTask& operator=(const omniTask&);

  friend class omniAsyncWorker;
};

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

class omniAsyncInvoker {
 public:
  omniAsyncInvoker();
  // Constructor.

  virtual ~omniAsyncInvoker();
  // Returns only when all the threads doing Anytime tasks have exited.
  // Notice that any tasks still sitting on the pending queues will be
  // discarded quietly.

  CORBA::Boolean insert(omniTask*);
  // insert the task into the correct pending queue. The task will be
  // dispatched according to its category.
  //
  // returns 1 if the task has been inserted successfully.
  // returns 0 if the task cannot be inserted.

  CORBA::Boolean work_pending();
  // Return 1 if there are DedicatedThread tasks pending, 0 if none.

  void perform(unsigned long secs = 0, unsigned long nanosecs = 0);
  // Loop performing dedicated thread tasks. If a timeout is
  // specified, returns when the absolute time passes; otherwise,
  // blocks until shut down.

  void shutdown();
  // Release threads blocked in perform.


  static _core_attr unsigned int idle_timeout;
  // No. of seconds an idle thread waits before it exits. default is
  // 10 seconds.

private:

  omniAsyncWorker* getWorker(omniAsyncPool* pool);

  inline unsigned int workerStart()
  {
    omni_tracedmutex_lock l(pd_lock);
    return ++pd_total_threads;
  }

  inline unsigned int workerStartLocked()
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);
    return ++pd_total_threads;
  }

  inline unsigned int workerStop()
  {
    omni_tracedmutex_lock l(pd_lock);
    
    if (--pd_total_threads == 0)
      pd_idle_cond.broadcast();
    
    return pd_total_threads;
  }

  omni_tracedmutex     pd_lock;
  omni_tracedcondition pd_idle_cond;
  unsigned int         pd_total_threads;   // Total number of threads.
  omniAsyncWorker*     pd_idle_threads;

  omniAsyncPool*       pd_general;         // General threads/tasks
  omniAsyncPool*       pd_server;          // Server threads/tasks
  omniAsyncPool*       pd_client;          // Client threads/tasks
  omniAsyncDedicated*  pd_dedicated;       // Dedicated tasks

  CORBA::Boolean       pd_keep_working;


  friend class omniAsyncWorker;
  friend class omniAsyncPool;
  friend class omniAsyncDedicated;
};

#endif // __OMNIASYNCINVOKER_H__
