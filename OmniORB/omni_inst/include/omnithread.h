// -*- Mode: C++; -*-
//				Package : omnithread
// omnithread.h			Created : 7/94 tjr
//
//    Copyright (C) 2002-2011 Apasphere Ltd
//    Copyright (C) 1994-1997 Olivetti & Oracle Research Laboratory
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
// Interface to OMNI thread abstraction.
//
// This file declares classes for threads and synchronisation objects
// (mutexes, condition variables and counting semaphores).
//
// Wherever a seemingly arbitrary choice has had to be made as to the interface
// provided, the intention here has been to be as POSIX-like as possible.  This
// is why there is no semaphore timed wait, for example.
//

#ifndef __omnithread_h_
#define __omnithread_h_

class omni_mutex;
class omni_condition;
class omni_semaphore;
class omni_thread;

//
// OMNI_THREAD_EXPOSE can be defined as public or protected to expose the
// implementation class - this may be useful for debugging.  Hopefully this
// won't change the underlying structure which the compiler generates so that
// this can work without recompiling the library.
//

#ifndef OMNI_THREAD_EXPOSE
#  define OMNI_THREAD_EXPOSE private
#endif

//
// Include implementation-specific header file.
//
// This must define 7 CPP macros of the form OMNI_x_IMPLEMENTATION for mutex,
// condition variable, semaphore and thread.  Each should define any
// implementation-specific members of the corresponding classes.
//

//
// Windows

#if defined(__WIN32__)

#  if defined(__POSIX_NT__)
#    include <omnithread/posix.h>
#  else
#    include <omnithread/nt.h>
#  endif

#  if defined(_MSC_VER) || defined(__BCPLUSPLUS__)

// Using MSVC++ or Borland C++ to compile. If compiling library as a
// DLL, define _OMNITHREAD_DLL. If compiling as a static library,
// define _WINSTATIC. If compiling an application that is to be
// statically linked to omnithread, define _WINSTATIC (if the
// application is to be dynamically linked, there is no need to define
// any of these macros).

#    if defined (_OMNITHREAD_DLL) && defined(_WINSTATIC)
#      error "Both _OMNITHREAD_DLL and _WINSTATIC are defined."
#    elif defined(_OMNITHREAD_DLL)
#      define _OMNITHREAD_NTDLL_ __declspec(dllexport)
#    elif !defined(_WINSTATIC)
#      define _OMNITHREAD_NTDLL_ __declspec(dllimport)
#    elif defined(_WINSTATIC)
#      define _OMNITHREAD_NTDLL_
#    endif
 // _OMNITHREAD_DLL && _WINSTATIC

#  else

// Not using MSVC++ to compile
#    define _OMNITHREAD_NTDLL_

#  endif

//
// vxWorks

#elif defined(__vxWorks__)
#  include <omnithread/VxThread.h>

//
// Everything else uses pthreads

#else
#  include <omnithread/posix.h>

#endif

//
// Additional platform dependencies

#if defined(__macos__)
#  include <sched.h>
#endif


#if !defined(__WIN32__)
#  define _OMNITHREAD_NTDLL_
#endif

#if (!defined(OMNI_MUTEX_IMPLEMENTATION)        || \
     !defined(OMNI_MUTEX_LOCK_IMPLEMENTATION)   || \
     !defined(OMNI_MUTEX_TRYLOCK_IMPLEMENTATION)|| \
     !defined(OMNI_MUTEX_UNLOCK_IMPLEMENTATION) || \
     !defined(OMNI_CONDITION_IMPLEMENTATION)    || \
     !defined(OMNI_SEMAPHORE_IMPLEMENTATION)    || \
     !defined(OMNI_THREAD_IMPLEMENTATION))
#  error "Implementation header file incomplete"
#endif


//
// This exception is thrown in the event of a fatal error.
//

class _OMNITHREAD_NTDLL_ omni_thread_fatal {
public:
  int error;
  omni_thread_fatal(int e = 0) : error(e) {}
};


//
// This exception is thrown when an operation is invoked with invalid
// arguments.
//

class _OMNITHREAD_NTDLL_ omni_thread_invalid {};


///////////////////////////////////////////////////////////////////////////
//
// Time
//
///////////////////////////////////////////////////////////////////////////

struct omni_time_t {
  unsigned long s;  // seconds
  unsigned long ns; // nanoseconds

  inline omni_time_t(unsigned long _s=0, unsigned long _ns=0)
    : s(_s), ns(_ns) {}

  inline void assign(unsigned long _s=0, unsigned long _ns=0)
  {
    s  = _s;
    ns = _ns;
  }

  // void* operator used to test for non-zero time. We cant use bool
  // in case we're using an ancient compiler with no bool.
  inline operator void*() const
  {
    return (void*)(s || ns);
  }

  inline int operator==(const omni_time_t& o) const
  {
    return o.s == s && o.ns == ns;
  }

  inline int operator<(const omni_time_t& o) const
  {
    return s < o.s || (s == o.s && ns < o.ns);
  }

  inline int operator<=(const omni_time_t& o) const
  {
    return s < o.s || (s == o.s && ns <= o.ns);
  }

  inline int operator>(const omni_time_t& o) const
  {
    return s > o.s || (s == o.s && ns > o.ns);
  }

  inline int operator>=(const omni_time_t& o) const
  {
    return s > o.s || (s == o.s && ns >= o.ns);
  }

  inline omni_time_t& operator+=(const omni_time_t& o)
  {
    unsigned long os  = o.s + o.ns / 1000000000;
    unsigned long ons = o.ns % 1000000000;
    unsigned long nns = ns + ons;
    s += os + nns / 1000000000;
    ns = nns % 1000000000;
    return *this;
  }

  inline omni_time_t operator+(const omni_time_t& o) const
  {
    omni_time_t n(*this);
    n += o;
    return n;
  }

  inline omni_time_t& operator-=(const omni_time_t& o)
  {
    unsigned long os  = o.s + o.ns / 1000000000;
    unsigned long ons = o.ns % 1000000000;
    unsigned long nns = ns + 1000000000 - ons;

    s -= os + (1 - nns / 1000000000);
    ns = nns % 1000000000;
    return *this;
  }

  inline omni_time_t operator-(const omni_time_t& o) const
  {
    omni_time_t n(*this);
    n -= o;
    return n;
  }
};


///////////////////////////////////////////////////////////////////////////
//
// Mutex
//
///////////////////////////////////////////////////////////////////////////

class _OMNITHREAD_NTDLL_ omni_mutex {
public:
  omni_mutex();
  ~omni_mutex();

  inline void lock()    { OMNI_MUTEX_LOCK_IMPLEMENTATION   }
  inline void unlock()  { OMNI_MUTEX_UNLOCK_IMPLEMENTATION }

  inline int trylock()  { OMNI_MUTEX_TRYLOCK_IMPLEMENTATION }
  // if mutex is unlocked then lock it and return 1 (true).
  // If it is already locked then return 0 (false).

  inline void acquire() { lock(); }
  inline void release() { unlock(); }
  // the names lock and unlock are preferred over acquire and release
  // since we are attempting to be as POSIX-like as possible.

  friend class omni_condition;

private:
  // dummy copy constructor and operator= to prevent copying
  omni_mutex(const omni_mutex&);
  omni_mutex& operator=(const omni_mutex&);

OMNI_THREAD_EXPOSE:
  OMNI_MUTEX_IMPLEMENTATION
};

//
// As an alternative to:
// {
//   mutex.lock();
//   .....
//   mutex.unlock();
// }
//
// you can use a single instance of the omni_mutex_lock class:
//
// {
//   omni_mutex_lock l(mutex);
//   ....
// }
//
// This has the advantage that mutex.unlock() will be called automatically
// when an exception is thrown.
//

class _OMNITHREAD_NTDLL_ omni_mutex_lock {
public:
  inline omni_mutex_lock(omni_mutex& m) : mutex(m) { mutex.lock(); }
  inline ~omni_mutex_lock() { mutex.unlock(); }

private:
  omni_mutex& mutex;

  // dummy copy constructor and operator= to prevent copying
  omni_mutex_lock(const omni_mutex_lock&);
  omni_mutex_lock& operator=(const omni_mutex_lock&);
};

class _OMNITHREAD_NTDLL_ omni_mutex_trylock {
public:
  omni_mutex_trylock(omni_mutex& m) : mutex(m), locked(mutex.trylock()) {}
  ~omni_mutex_trylock() { if (locked) mutex.unlock(); }
  operator int() const { return locked; }

private:
  omni_mutex& mutex;
  const int locked;

  // dummy copy constructor and operator= to prevent copying
  omni_mutex_trylock(const omni_mutex_trylock&);
  omni_mutex_trylock& operator=(const omni_mutex_trylock&);
};



///////////////////////////////////////////////////////////////////////////
//
// Condition variable
//
///////////////////////////////////////////////////////////////////////////

class _OMNITHREAD_NTDLL_ omni_condition {

private:
  omni_mutex* mutex;

public:
  omni_condition(omni_mutex* m);
  // constructor must be given a pointer to an existing mutex. The
  // condition variable is then linked to the mutex, so that there is an
  // implicit unlock and lock around wait() and timed_wait().

  ~omni_condition();

  void wait();
  // wait for the condition variable to be signalled.  The mutex is
  // implicitly released before waiting and locked again after waking up.
  // If wait() is called by multiple threads, a signal may wake up more
  // than one thread.  See POSIX threads documentation for details.

  int timedwait(unsigned long secs, unsigned long nanosecs = 0);
  // timedwait() is given an absolute time to wait until.  To wait for a
  // relative time from now, use omni_thread::get_time. See POSIX threads
  // documentation for why absolute times are better than relative.
  // Returns 1 (true) if successfully signalled, 0 (false) if time
  // expired.

  inline int timedwait(const omni_time_t& t)
  {
    return timedwait(t.s, t.ns);
  }

  void signal();
  // if one or more threads have called wait(), signal wakes up at least
  // one of them, possibly more.  See POSIX threads documentation for
  // details.

  void broadcast();
  // broadcast is like signal but wakes all threads which have called
  // wait().

private:
  // dummy copy constructor and operator= to prevent copying
  omni_condition(const omni_condition&);
  omni_condition& operator=(const omni_condition&);

OMNI_THREAD_EXPOSE:
  OMNI_CONDITION_IMPLEMENTATION
};


///////////////////////////////////////////////////////////////////////////
//
// Counting semaphore
//
///////////////////////////////////////////////////////////////////////////

class _OMNITHREAD_NTDLL_ omni_semaphore {

public:
  omni_semaphore(unsigned int initial = 1);
  ~omni_semaphore();

  void wait();
  // if semaphore value is > 0 then decrement it and carry on. If it's
  // already 0 then block.

  int trywait();
  // if semaphore value is > 0 then decrement it and return 1 (true).
  // If it's already 0 then return 0 (false).

  void post();
  // if any threads are blocked in wait(), wake one of them up. Otherwise
  // increment the value of the semaphore.

private:
  // dummy copy constructor and operator= to prevent copying
  omni_semaphore(const omni_semaphore&);
  omni_semaphore& operator=(const omni_semaphore&);

OMNI_THREAD_EXPOSE:
  OMNI_SEMAPHORE_IMPLEMENTATION
};


//
// A helper class for semaphores, similar to omni_mutex_lock above.
//

class _OMNITHREAD_NTDLL_ omni_semaphore_lock {
public:
  omni_semaphore_lock(omni_semaphore& s) : sem(s) { sem.wait(); }
  ~omni_semaphore_lock() { sem.post(); }

private:
  omni_semaphore& sem;

  // dummy copy constructor and operator= to prevent copying
  omni_semaphore_lock(const omni_semaphore_lock&);
  omni_semaphore_lock& operator=(const omni_semaphore_lock&);
};


///////////////////////////////////////////////////////////////////////////
//
// Thread
//
///////////////////////////////////////////////////////////////////////////

class _OMNITHREAD_NTDLL_ omni_thread {
public:

  enum priority_t {
    PRIORITY_LOW,
    PRIORITY_NORMAL,
    PRIORITY_HIGH
  };

  enum state_t {
    STATE_NEW,		// thread object exists but thread hasn't
			// started yet.
    STATE_RUNNING,	// thread is running.
    STATE_TERMINATED	// thread has terminated but storage has not
                        // been reclaimed (i.e. waiting to be joined).
  };

  //
  // Constructors set up the thread object but the thread won't start until
  // start() is called. The create method can be used to construct and start
  // a thread in a single call.
  //

  omni_thread(void (*fn)(void*), void* arg = 0,
	      priority_t pri = PRIORITY_NORMAL);

  omni_thread(void* (*fn)(void*), void* arg = 0,
	      priority_t pri = PRIORITY_NORMAL);

  // these constructors create a thread which will run the given function
  // when start() is called.  The thread will be detached if given a
  // function with void return type, undetached if given a function
  // returning void*. If a thread is detached, storage for the thread is
  // reclaimed automatically on termination. Only an undetached thread
  // can be joined.

  void start();
  // start() causes a thread created with one of the constructors to
  // start executing the appropriate function.

protected:

  omni_thread(void* arg = 0, priority_t pri = PRIORITY_NORMAL);
  // this constructor is used in a derived class.  The thread will
  // execute the run() or run_undetached() member functions depending on
  // whether start() or start_undetached() is called respectively.

  void start_undetached();
  // can be used with the above constructor in a derived class to cause
  // the thread to be undetached.  In this case the thread executes the
  // run_undetached member function.

  virtual ~omni_thread();
  // destructor cannot be called by user (except via a derived class).
  // Use exit() instead. This also means a thread object must be
  // allocated with new - it cannot be statically or automatically
  // allocated. The destructor of a class that inherits from omni_thread
  // shouldn't be public either (otherwise the thread object can be
  // destroyed while the underlying thread is still running).

public:

  void join(void**);
  // join causes the calling thread to wait for another's completion,
  // putting the return value in the variable of type void* whose address
  // is given (unless passed a null pointer). Only undetached threads
  // may be joined. Storage for the thread will be reclaimed.

  void set_priority(priority_t);
  // set the priority of the thread.

  static omni_thread* create(void (*fn)(void*), void* arg = 0,
			     priority_t pri = PRIORITY_NORMAL);

  static omni_thread* create(void* (*fn)(void*), void* arg = 0,
			     priority_t pri = PRIORITY_NORMAL);
  // create spawns a new thread executing the given function with the
  // given argument at the given priority. Returns a pointer to the
  // thread object. It simply constructs a new thread object then calls
  // start.

  static void exit(void* return_value = 0);
  // causes the calling thread to terminate.

  static omni_thread* self();
  // returns the calling thread's omni_thread object.  If the calling
  // thread is not the main thread and is not created using this
  // library, returns 0. (But see create_dummy() below.)

  static void yield();
  // allows another thread to run.

  static void sleep(unsigned long secs, unsigned long nanosecs = 0);
  // sleeps for the given time.

  static inline void sleep(const omni_time_t& t)
  {
    sleep(t.s, t.ns);
  }

  static void get_time(unsigned long* abs_sec, unsigned long* abs_nsec,
		       unsigned long rel_sec = 0, unsigned long rel_nsec = 0);
  // calculates an absolute time in seconds and nanoseconds, suitable for
  // use in timed_waits on condition variables, which is the current time
  // plus the given relative offset.

  static inline void get_time(omni_time_t& t)
  {
    get_time(&t.s, &t.ns);
  }

  static inline void get_time(omni_time_t& t, const omni_time_t& r)
  {
    get_time(&t.s, &t.ns, r.s, r.ns);
  }

  static inline void get_time(omni_time_t& t, unsigned long rel_sec)
  {
    get_time(&t.s, &t.ns, rel_sec, 0);
  }

  static void stacksize(unsigned long sz);
  static unsigned long stacksize();
  // Use this value as the stack size when spawning a new thread.
  // The default value (0) means that the thread library default is
  // to be used.


  // Per-thread data
  //
  // These functions allow you to attach additional data to an
  // omni_thread. First allocate a key for yourself with
  // allocate_key(). Then you can store any object whose class is
  // derived from value_t. Any values still stored in the omni_thread
  // when the thread exits are deleted.
  //
  // These functions are NOT thread safe, so you should be very
  // careful about setting/getting data in a different thread to the
  // current thread.

  typedef unsigned int key_t;
  static key_t allocate_key();

  class value_t {
  public:
    virtual ~value_t() {}
  };

  value_t* set_value(key_t k, value_t* v);
  // Sets a value associated with the given key. The key must have
  // been allocated with allocate_key(). If a value has already been
  // set with the specified key, the old value_t object is deleted and
  // replaced. Returns the value which was set, or zero if the key is
  // invalid.

  value_t* get_value(key_t k);
  // Returns the value associated with the key. If the key is invalid,
  // or there is no value for the key, returns zero.

  value_t* remove_value(key_t k);
  // Removes the value associated with the key and returns it.  If the
  // key is invalid, or there is no value for the key, returns zero.


  // Dummy omni_thread
  //
  // Sometimes, an application finds itself with threads created
  // outside of omnithread which must interact with omnithread
  // features such as the per-thread data. In this situation,
  // omni_thread::self() would normally return 0. These functions
  // allow the application to create a suitable dummy omni_thread
  // object.

  static omni_thread* create_dummy();
  // creates a dummy omni_thread for the calling thread. Future calls
  // to self() will return the dummy omni_thread. Throws
  // omni_thread_invalid if this thread already has an associated
  // omni_thread (real or dummy).

  static void release_dummy();
  // release the dummy omni_thread for this thread. This function MUST
  // be called before the thread exits. Throws omni_thread_invalid if
  // the calling thread does not have a dummy omni_thread.

  // class ensure_self should be created on the stack. If created in a
  // thread without an associated omni_thread, it creates a dummy
  // thread which is released when the ensure_self object is deleted.

  class ensure_self {
  public:
    inline ensure_self() : _dummy(0)
    {
      _self = omni_thread::self();
      if (!_self) {
	_dummy = 1;
	_self  = omni_thread::create_dummy();
      }
    }
    inline ~ensure_self()
    {
      if (_dummy)
	omni_thread::release_dummy();
    }
    inline omni_thread* self() { return _self; }
  private:
    omni_thread* _self;
    int          _dummy;
  };

private:

  virtual void run(void*) {}
  virtual void* run_undetached(void*) { return 0; }
  // can be overridden in a derived class.  When constructed using the
  // the constructor omni_thread(void*, priority_t), these functions are
  // called by start() and start_undetached() respectively.

  void common_constructor(void* arg, priority_t pri, int det);
  // implements the common parts of the constructors.

  omni_mutex mutex;
  // used to protect any members which can change after construction,
  // i.e. the following 2 members.

  state_t _state;
  priority_t _priority;

  static omni_mutex* next_id_mutex;
  static int next_id;
  int _id;

  void (*fn_void)(void*);
  void* (*fn_ret)(void*);
  void* thread_arg;
  int detached;
  int _dummy;
  value_t**     _values;
  unsigned long _value_alloc;

  omni_thread(const omni_thread&);
  omni_thread& operator=(const omni_thread&);
  // Not implemented

public:
  priority_t priority() {

    // return this thread's priority.

    omni_mutex_lock l(mutex);
    return _priority;
  }

  state_t state() {

    // return thread state (invalid, new, running or terminated).

    omni_mutex_lock l(mutex);
    return _state;
  }

  int id() { return _id; }
  // return unique thread id within the current process.


  // This class plus the instance of it declared below allows us to execute
  // some initialisation code before main() is called.

  class _OMNITHREAD_NTDLL_ init_t {
  public:
    init_t();
    ~init_t();
  };

  friend class init_t;
  friend class omni_thread_dummy;

OMNI_THREAD_EXPOSE:
  OMNI_THREAD_IMPLEMENTATION
};



///////////////////////////////////////////////////////////////////////////
//
// Atomic reference count
//
///////////////////////////////////////////////////////////////////////////

#ifndef OMNI_DISABLE_ATOMIC_OPS
#  include <omnithread/atomic.h>
#endif

#ifndef OMNI_REFCOUNT_DEFINED

#  define OMNI_REFCOUNT_DEFAULT

class _OMNITHREAD_NTDLL_ omni_refcount {
public:
  inline omni_refcount(int start) : count(start) {}
  inline ~omni_refcount() {}

  // Atomically increment reference count and return new value
  inline int inc() {
    omni_mutex_lock l(lock);
    return ++count;
  }

  // Atomically decrement reference count and return new value
  inline int dec() {
    omni_mutex_lock l(lock);
    return --count;
  }

  // Return snapshot of current value. Real count may have changed by
  // the time the value is looked at!
  inline int value() {
    return count;
  }

private:
  static omni_mutex lock;
  int count;
};

#endif // OMNI_REFCOUNT_DEFINED



#ifndef __rtems__
static omni_thread::init_t omni_thread_init;
#else
// RTEMS calls global Ctor/Dtor in a context that is not
// a posix thread. Calls to functions to pthread_self() in
// that context returns 0. 
// So, for RTEMS we will make the thread initialization at the
// beginning of the Init task that has a posix context.
#endif

#endif
