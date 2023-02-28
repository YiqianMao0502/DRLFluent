// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniORB.h                  Created on: 6/2/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2014 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//
// Description:
//      This is the public API of omniORB's extension to CORBA.
//      The API is intended to be used in application code.

#ifndef __OMNIORB_H__
#define __OMNIORB_H__


struct omniOrbBoaKey {
  _CORBA_ULong hi;
  _CORBA_ULong med;
  _CORBA_ULong lo;
};

OMNI_NAMESPACE_BEGIN(omni)
class omniInterceptors;
OMNI_NAMESPACE_END(omni)

_CORBA_MODULE omniORB

_CORBA_MODULE_BEG

  ///////////////////////////////////////////////////////////////////////
  // omniORB version                                                   //
  //                                                                   //
  // versionString()                                                   //
  //   Return the omniORB version as a string, e.g. 4.0.6. The caller  //
  //   must not free the string.                                       //
  _CORBA_MODULE_FN const char* versionString();                        //
  //                                                                   //
  // versionHex()                                                      //
  //   Return the omniORB version as a hex integer. e.g. for omniORB   //
  //   4.0.6 final, the hex string is 0x040006f0.                      //
  //   The last byte represents the patch level -- a1 means alpha 1;   //
  //   b2 means beta 2; c3 means release candidate 3; f0 means final;  //
  //   f1 means final patch level 1, and so on.                        //
  //                                                                   //
  _CORBA_MODULE_FN _CORBA_ULong versionHex();                          //
  ///////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////
  // Tracing level                                                     //
  //     level 0 - critical errors only                                //
  //     level 1 - informational messages only                         //
  //     level 2 - configuration information and warnings              //
  //     level 5 - the above plus report server thread creation and    //
  //               communication socket shutdown                       //
  //     level 10 - the above plus execution trace messages            //
  //     level 25 - output trace message per send or receive giop message
  //     level 30 - dump up to 128 bytes of a giop message             //
  //     level 40 - dump the complete giop message                     //
  //                                                                   //
  //    Valid values = (n >= 0)                                        //
  _CORBA_MODULE_VAR _core_attr _CORBA_ULong   traceLevel;              //
  //                                                                   //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtraceLevel <n>, or by direct assignment to this   //
  //     variable.                                                     //
  //                                                                   //
  _CORBA_MODULE_VAR _core_attr _CORBA_Boolean traceExceptions;         //
  //                                                                   //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtraceExceptions, or by direct assignment to this  //
  //     variable.  If true, then system exceptions are logged when    //
  //     they are thrown.                                              //
  //                                                                   //
  _CORBA_MODULE_VAR _core_attr _CORBA_Boolean traceInvocations;        //
  //                                                                   //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtraceInvocations, or by direct assignment to this //
  //     variable.  If true, then each local and remote invocation     //
  //     will generate a trace message.                                //
  //                                                                   //
  _CORBA_MODULE_VAR _core_attr _CORBA_Boolean traceInvocationReturns;  //
  //                                                                   //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtraceInvocations, or by direct assignment to this //
  //     variable.  If true, then the return of each invocation will   //
  //     generate a trace message.                                     //
  //                                                                   //
  _CORBA_MODULE_VAR _core_attr _CORBA_Boolean traceThreadId;           //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtraceThreadId, or by direct assignment to this    //
  //     variable.  If true, then the logging messages emmited due to  //
  //     the two options above will contain the thread id of the       //
  //     logging thread.                                               //
  //                                                                   //
  _CORBA_MODULE_VAR _core_attr _CORBA_Boolean traceTime;               //
  //     If true, logging messages will include the current time.      //
  //                                                                   //
  _CORBA_MODULE_VAR _core_attr _CORBA_Boolean traceLocking;            //
  //     If true, all locking operations will be traced.               //
  ///////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // getInterceptors()                                                  //
  //   Only call this function after ORB_init().                        //
  //   The returned object contains all the ORB processing points where //
  //   interception functions can be added.                             //
  //   Calling this function before ORB_init() will result in a system  //
  //   exception.                                                       //
  _CORBA_MODULE_FN _OMNI_NS(omniInterceptors)* getInterceptors();       //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // setMainThread()                                                    //
  //                                                                    //
  // POAs with the MAIN_THREAD policy dispatch calls on the "main"      //
  // thread. By default, omniORB assumes that the thread which          //
  // initialised the omnithread library is the "main" thread. To choose //
  // a different thread, call this function from the desired "main"     //
  // thread. The calling thread must have an omni_thread associated     //
  // with it. If it does not, throws CORBA::INITIALIZE.                 //
  //                                                                    //
  // Note that calls are only actually dispatched to the "main" thread  //
  // if ORB::run() or ORB::perform_work() is called from that thread.   //
  //                                                                    //
  _CORBA_MODULE_FN void setMainThread();                                //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // setClientCallTimeout()                                             //
  // setClientThreadCallTimeout()                                       //
  // setClientThreadCallDeadline()                                      //
  // setClientConnectTimeout()                                          //
  //                                                                    //
  // Functions to set call timeouts at runtime.                         //
  //                                                                    //
  // setClientCallTimeout() either sets the global timeout, or the      //
  // timeout for the specfied object reference. The timeout is          //
  // specified in milliseconds.                                         //
  //                                                                    //
  // setClientThreadCallTimeout() sets the timeout for the calling      //
  // thread to the specified number of milliseconds. If the calling     //
  // thread is not an omni_thread, it throws CORBA::INITIALIZE.         //
  //                                                                    //
  // setClientThreadCallDeadline() sets an absolute deadline for all    //
  // calls in the calling thread. The seconds and nanoseconds should be //
  // acquired from omni_thread::get_time(). Again throws INITIALIZE if  //
  // the calling thread is not an omni_thread.                          //
  //                                                                    //
  // setClientConnectTimeout() sets the overriding timeout for calls    //
  // that involve opening a new network connection.                     //
  //                                                                    //
  _CORBA_MODULE_FN void setClientCallTimeout(CORBA::ULong millisecs);   //
  _CORBA_MODULE_FN void setClientCallTimeout(CORBA::Object_ptr obj,     //
    					     CORBA::ULong millisecs);   //
                                                                        //
  _CORBA_MODULE_FN void setClientThreadCallTimeout(CORBA::ULong millisecs);
                                                                        //
  _CORBA_MODULE_FN void setClientThreadCallDeadline(unsigned long secs, //
                                                    unsigned long ns);  //
                                                                        //
  _CORBA_MODULE_FN inline void                                          //
  setClientThreadCallDeadline(const omni_time_t& tt)                    //
  {                                                                     //
    setClientThreadCallDeadline(tt.s, tt.ns);                           //
  }                                                                     //
  _CORBA_MODULE_FN void setClientConnectTimeout(CORBA::ULong millisecs);//
                                                                        //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // giopMaxMsgSize()                                                   //
  //                                                                    //
  // Return the ORB-wide limit on the size of GIOP message (excluding   //
  // the header). This value is set by the ORB option giopMaxMsgSize.   //
  //                                                                    //
  _CORBA_MODULE_FN _CORBA_ULong giopMaxMsgSize();                       //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // setPersistentServerIdentifier()                                    //
  //                                                                    //
  // Sets an octet sequence used to persistently identify "this"        //
  // server. Stored object references matching this identifier are      //
  // re-written to use the current endpoint details.                    //
  //                                                                    //
  _CORBA_MODULE_FN void setPersistentServerIdentifier(                  //
                            const _CORBA_Unbounded_Sequence_Octet& id); //
  ////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////
  /////////////////////////// omniORB::logger //////////////////////////
  //////////////////////////////////////////////////////////////////////

  static inline int trace(_CORBA_ULong tl) { return traceLevel >= tl; }

  typedef void (*logFunction)(const char*);

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // setLogFunction()                                                   //
  //                                                                    //
  // Set a logging function. If set to a non-zero function pointer, the //
  // provided function is called for all omniORB log messages.          //
  //                                                                    //
  _CORBA_MODULE_FN void setLogFunction(logFunction f);                  //
  ////////////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // setLogFilename()                                                   //
  //                                                                    //
  // Set a logging file to use instead of stderr. The file is opened    //
  // for appending. If the file cannot be opened, CORBA::INITIALIZE is  //
  // thrown.                                                            //
  //                                                                    //
  _CORBA_MODULE_FN void setLogFilename(const char* n);                  //
  _CORBA_MODULE_FN const char* getLogFilename();                        //
  ////////////////////////////////////////////////////////////////////////


  class logger {
  public:
    logger(const char* prefix = 0);  // prefix defaults to "omniORB: "
    ~logger();
    // The destructor flushes the message.

    logger& operator<<(char c);
    logger& operator<<(unsigned char c) { return (*this) << (char)c; }
    logger& operator<<(signed char c) { return (*this) << (char)c; }
    logger& operator<<(const char *s);
    logger& operator<<(const unsigned char *s) {
      return (*this) << (const char*)s;
    }
    logger& operator<<(const signed char *s) {
      return (*this) << (const char*)s;
    }
    logger& operator<<(const void *p);
    logger& operator<<(int n);
    logger& operator<<(unsigned int n);
    logger& operator<<(long n);
    logger& operator<<(unsigned long n);
    logger& operator<<(short n) {return operator<<((int)n);}
    logger& operator<<(unsigned short n) {return operator<<((unsigned int)n);}
#ifdef HAS_Cplusplus_Bool
    logger& operator<<(bool b) { return operator<<((int)b); }
#endif
#ifndef NO_FLOAT
    logger& operator<<(double n);
    logger& operator<<(float n) { return operator<<((double)n); }
#endif
    logger& operator<<(const omniLocalIdentity*);
    logger& operator<<(const omniIdentity*);
    logger& operator<<(omniObjKey&);

    logger& operator<<(const CORBA::SystemException&);

    class exceptionStatus {
    public:
      exceptionStatus(CORBA::CompletionStatus s, CORBA::ULong m) :
	status(s), minor(m), minor_string(0) {}

      exceptionStatus(CORBA::CompletionStatus s, const char* description) :
	status(s), minor(0), minor_string(description) {}

      CORBA::CompletionStatus status;
      CORBA::ULong            minor;
      const char*             minor_string;
    private:
      exceptionStatus();
    };

    logger& operator<<(const exceptionStatus&);

    void flush();
    // Flushes the logger -- it can then be re-used for another
    // message.

  private:
    logger(const logger&);
    logger& operator=(const logger&);

    inline void reserve(int n) { if( pd_end - pd_p - 1 < n )  more(n); }
    void more(int n);

    const char* pd_prefix;
    char*       pd_buf;
    char*       pd_p;      // assert(*pd_p == '\0')
    char*       pd_end;    // assert(pd_p < pd_end)
  };


  _CORBA_MODULE_FN void do_logs(const char* msg);
  // internal

  _CORBA_MODULE_FN inline void logs(_CORBA_ULong tl, const char* msg) {
    if( traceLevel >= tl )  do_logs(msg);
  }
  // Writes log message with prefix, and appends '\n'.



  ////////////////////////////////////////////////////////////////////////
  // When an operation is invoked via an object reference, a            //
  // CORBA::TRANSIENT exception may be raised. 				//
  //									//
  // One circumstance in which a TRANSIENT exception can arise is as    //
  // follows:                                                           //
  //                                                                    //
  //     1. The client invokes on an object reference.                  //
  //     2. The object replies with a LOCATION_FORWARD message.         //
  //     3. The client caches the new location and retries to the new   //
  //        location.                                                   //
  //     4. Time passes...                                              //
  //     5. The client tries to invoke on the object again, using the   //
  //        cached, forwarded location.                                 //
  //     6. The attempt to contact the object fails.                    //
  //     7. The ORB runtime resets the location cache and throws a      //
  //        TRANSIENT exception with minor code                         //
  //        TRANSIENT_FailedOnForwarded.                                //
  //                                                                    //
  // In this situation, the default TRANSIENT exception handler retries //
  // the request.                                                       //
  //                                                                    //
  // In all other circumstances, TRANSIENT exceptions are propagated to //
  // the application.                                                   //
  //									//
  // This retry behaviour can be overridden by installing an exception	//
  // handler.  An exception handler is a function with the same 	//
  // signature as omniORB::transientExceptionHandler_t. The handler 	//
  // will be called when a CORBA::TRANSIENT exception is caught by the 	//
  // ORB. The handler is passed three arguments: a <cookie>, the 	//
  // no. of retries <n_retries> and the value of the exception caught 	//
  // <ex>. The handler is expected to do whatever is appropriate and 	//
  // return a boolean value. If the return value is TRUE (1), the ORB 	//
  // would retry the operation again. If the return value is FALSE (0),	//
  // the CORBA::TRANSIENT exception would be re-throw and is expected   //
  // to be caught by the application code.			      	//
  //									//
  // The overloaded functions omniORB::installTransientExceptionHandler //
  // can be used to install the exception handlers for CORBA::TRANSIENT.//
  // Two overloaded forms are available. The first form installs an 	//
  // exception handler for all object references except for those which //
  // have an exception handler installed by the second form, which takes//
  // an addition argument <obj> to identify the target object reference.//
  // The argument <cookie> is an opaque pointer which will be passed 	//
  // on by the ORB when it calls the exception handler.			//

  typedef CORBA::Boolean
  (*transientExceptionHandler_t)(void*                   cookie,
                                 CORBA::ULong            n_retries,
                                 const CORBA::TRANSIENT& ex);

  typedef CORBA::Boolean
  (*transientExceptionHandlerExt_t)(void*                   cookie,
                                    CORBA::ULong            n_retries,
                                    const CORBA::TRANSIENT& ex,
                                    CORBA::Object_ptr       obj,
                                    const char*             op);

  _CORBA_MODULE_FN void
  installTransientExceptionHandler(void* cookie,
                                   transientExceptionHandler_t fn);

  _CORBA_MODULE_FN void
  installTransientExceptionHandler(CORBA::Object_ptr           obj,
                                   void*                       cookie,
                                   transientExceptionHandler_t fn);

  _CORBA_MODULE_FN void
  installTransientExceptionHandlerExt(void* cookie,
                                      transientExceptionHandlerExt_t fn);

  _CORBA_MODULE_FN void
  installTransientExceptionHandlerExt(CORBA::Object_ptr              obj,
                                      void*                          cookie,
                                      transientExceptionHandlerExt_t fn);

  ////////////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////////////////
  // When an operation is invoked via an object reference, a		//
  // CORBA::TIMEOUT exception may be raised. 			        //
  //									//
  // By default, the ORB will pass this exception on to the application.//
  //									//
  // This behaviour can be overridden by installing an exception	//
  // handler. The function signature of the handler should be the same 	//
  // as omniORB::timeoutExceptionHandler_t. The handlers can be 	//
  // installed using the overloaded functions				//
  // omniORB::installTimeoutExceptionHandler. The use of these	        //
  // functions is similar to those for the TRANSIENT exception. See 	//
  // above for details.							//

  typedef CORBA::Boolean
  (*timeoutExceptionHandler_t)(void*                 cookie,
                               CORBA::ULong          n_retries,
                               const CORBA::TIMEOUT& ex);

  typedef CORBA::Boolean
  (*timeoutExceptionHandlerExt_t)(void*                 cookie,
                                  CORBA::ULong          n_retries,
                                  const CORBA::TIMEOUT& ex,
                                  CORBA::Object_ptr     obj,
                                  const char*           op);

  _CORBA_MODULE_FN void
  installTimeoutExceptionHandler(void* cookie,
				 timeoutExceptionHandler_t fn);

  _CORBA_MODULE_FN void
  installTimeoutExceptionHandler(CORBA::Object_ptr         obj,
                                 void*                     cookie,
                                 timeoutExceptionHandler_t fn);

  _CORBA_MODULE_FN void
  installTimeoutExceptionHandlerExt(void* cookie,
                                    timeoutExceptionHandlerExt_t fn);

  _CORBA_MODULE_FN void
  installTimeoutExceptionHandlerExt(CORBA::Object_ptr            obj,
                                    void*                        cookie,
                                    timeoutExceptionHandlerExt_t fn);

  ////////////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////////////////
  // When an operation is invoked via an object reference, a		//
  // CORBA::COMM_FAILURE exception may be raised. 			//
  //									//
  // By default, the ORB will pass this exception on to the application.//
  //									//
  // This behaviour can be overridden by installing an exception	//
  // handler. The function signature of the handler should be the same 	//
  // as omniORB::commFailureExceptionHandler_t. The handlers can be 	//
  // installed using the overloaded functions				//
  // omniORB::installCommFailureExceptionHandler. The use of these	//
  // functions is similar to those for the TRANSIENT exception. See 	//
  // above for details.							//

  typedef CORBA::Boolean
  (*commFailureExceptionHandler_t)(void*                      cookie,
                                   CORBA::ULong               n_retries,
                                   const CORBA::COMM_FAILURE& ex);

  typedef CORBA::Boolean
  (*commFailureExceptionHandlerExt_t)(void*                      cookie,
                                      CORBA::ULong               n_retries,
                                      const CORBA::COMM_FAILURE& ex,
                                      CORBA::Object_ptr          obj,
                                      const char*                op);

  _CORBA_MODULE_FN void
  installCommFailureExceptionHandler(void* cookie,
                                     commFailureExceptionHandler_t fn);

  _CORBA_MODULE_FN void
  installCommFailureExceptionHandler(CORBA::Object_ptr             obj,
                                     void*                         cookie,
                                     commFailureExceptionHandler_t fn);

  _CORBA_MODULE_FN void
  installCommFailureExceptionHandlerExt(void* cookie,
                                        commFailureExceptionHandlerExt_t fn);

  _CORBA_MODULE_FN void
  installCommFailureExceptionHandlerExt(CORBA::Object_ptr                obj,
                                        void*                            cookie,
                                        commFailureExceptionHandlerExt_t fn);

  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // When an operation is invoked via an object reference, a system	//
  // exception may be raised. If the exception is either                //
  // CORBA::TRANSIENT or CORBA::COMM_FAILURE, the handling of this      //
  // exception is as described above.			                //
  //									//
  // By default, if the exception is neither CORBA::TRANSIENT and	//
  // CORBA::COMM_FAILURE, the ORB will pass this exception on to the	//
  // application. 							//
  //									//
  // This behaviour can be overridden by installing an exception        //
  // handler.                                                           //
  // The function signature of the handler should be the same as	//
  // omniORB::systemExceptionHandler_t. The handlers can be installed   //
  // using the overloaded functions					//
  // omniORB::installSystemExceptionHandler.                            //
  // The use of these functions is similar to those for the TRANSIENT	//
  // exception. See above for details. Notice that the installed        //
  // exception handler will only be called when the system exception is //
  // neither CORBA::TRANSIENT nor CORBA::COMM_FAILURE.			//

  typedef CORBA::Boolean
  (*systemExceptionHandler_t)(void*                         cookie,
                              CORBA::ULong                  n_retries,
                              const CORBA::SystemException& ex);

  typedef CORBA::Boolean
  (*systemExceptionHandlerExt_t)(void*                         cookie,
                                 CORBA::ULong                  n_retries,
                                 const CORBA::SystemException& ex,
                                 CORBA::Object_ptr             obj,
                                 const char*                   op);

  _CORBA_MODULE_FN void
  installSystemExceptionHandler(void* cookie,
                                systemExceptionHandler_t fn);

  _CORBA_MODULE_FN void
  installSystemExceptionHandler(CORBA::Object_ptr        obj,
                                void*                    cookie,
                                systemExceptionHandler_t fn);

  _CORBA_MODULE_FN void
  installSystemExceptionHandlerExt(void* cookie,
                                   systemExceptionHandlerExt_t fn);

  _CORBA_MODULE_FN void
  installSystemExceptionHandlerExt(CORBA::Object_ptr           obj,
                                   void*                       cookie,
                                   systemExceptionHandlerExt_t fn);

  ////////////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////////////////
  // class fatalException                                               //
  //                                                                    //
  // This exception is thrown if a bug inside the omniORB runtime is    //
  // detected. The exact location in the source where the exception is  //
  // thrown is indicated by file() and line().                          //
  //                                                                    //
  class fatalException {                                                //
  public:                                                               //
    fatalException(const char *file,int line,const char *errmsg);       //
    inline ~fatalException() {}                                         //
    inline const char *file() const   { return pd_file;   }             //
    inline int         line() const   { return pd_line;   }             //
    inline const char *errmsg() const { return pd_errmsg; }             //
  private:                                                              //
    const char *pd_file;                                                //
    int         pd_line;                                                //
    const char *pd_errmsg;                                              //
                                                                        //
    fatalException();                                                   //
  };                                                                    //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // class LOCATION_FORWARD                                             //
  //  This may be thrown as an exception from object implementation     //
  // to pass a LOCATION_FORWARD message back to the client -- which     //
  // should retry the request on the object given by argument to the    //
  // constructor.  The reference is consumed.                           //
  // From GIOP 1.2 onwards, location forward can either be temporary    //
  // or permanent. This is indicated by the permanent flag.             //
  //  It is recommended that the facilities provided by the POA be used //
  // instead where possible.                                            //
  //                                                                    //
  class LOCATION_FORWARD {                                              //
  public:                                                               //
    inline LOCATION_FORWARD(CORBA::Object_ptr objref,                   //
			    CORBA::Boolean permanent=0)                 //
      : pd_objref(objref), pd_permanent(permanent) {}                   //
                                                                        //
    inline CORBA::Object_ptr get_obj() const { return pd_objref; }      //
    inline CORBA::Boolean is_permanent() const { return pd_permanent; } //
                                                                        //
    inline LOCATION_FORWARD(const LOCATION_FORWARD& l)                  //
      : pd_objref(l.pd_objref), pd_permanent(l.pd_permanent) {}         //
                                                                        //
  private:                                                              //
    CORBA::Object_ptr pd_objref;                                        //
    CORBA::Boolean    pd_permanent;                                     //
    LOCATION_FORWARD();                                                 //
    LOCATION_FORWARD& operator=(const LOCATION_FORWARD&);               //
  };                                                                    //
  ////////////////////////////////////////////////////////////////////////

#define _INCLUDE_OMNIORBCOMPAT_
#include <omniORB4/omniORBcompat.h>
#undef  _INCLUDE_OMNIORBCOMPAT_

#ifndef HAS_Cplusplus_Namespace
  friend class omni;
  friend class CORBA;
private:
#endif

#ifdef HAS_Cplusplus_catch_exception_by_base

#  define _OMNIORB_EX_ONLY_CD(x)

#else

#  define _OMNIORB_EX_ONLY_CD(x) x

  // Internal omniORB class.  Used in the stubs to pass
  // user-defined exceptions to a lower level.

  class StubUserException {
  public:
    // This exception is thrown in the stubs to pass a
    // CORBA::UserException down.  It is needed because
    // gcc 2.7 cannot catch exceptions by base class.

    inline StubUserException(CORBA::Exception* e) : pd_e(e) {}

    // inline StubUserException(const StubUserException& e);
    // inline ~StubUserException() {}
    // The defaults will do.


    inline CORBA::Exception* ex() { return pd_e; }

  private:
    StubUserException();
    StubUserException& operator=(const StubUserException&);

    CORBA::Exception* pd_e;
  };
#endif

_CORBA_MODULE_END

#ifndef OMNIORB_NO_EXCEPTION_LOGGING

OMNI_NAMESPACE_BEGIN(omni)

class omniExHelper {
public:

#define OMNIORB_EX(name) \
  static void name(const char*, int, CORBA::ULong, CORBA::CompletionStatus);

  OMNIORB_FOR_EACH_SYS_EXCEPTION(OMNIORB_EX)

#undef OMNIORB_EX

  static const char* strip(const char*);
};

OMNI_NAMESPACE_END(omni)

#define OMNIORB_THROW(name, minor, completion) \
  do {\
    _OMNI_NS(omniExHelper)::name(__FILE__, __LINE__, minor, completion); \
    throw ::CORBA::name(minor, completion); \
  } while(0)

#else

#define OMNIORB_THROW(name, minor, completion) \
  throw ::CORBA::name(minor, completion)

#endif


#endif // __OMNIORB_H__
