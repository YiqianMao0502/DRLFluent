// -*- Mode: C++; -*-
//                            Package   : omniORB
// callDescriptor.h           Created on: 12/1998
//                            Author    : David Riddoch (djr)
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2003-2012 Apasphere Ltd
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
//    Object that encapsulates a call to be performed
//

#ifndef __OMNIORB_CALLDESCRIPTOR_H__
#define __OMNIORB_CALLDESCRIPTOR_H__

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif


class omniObjRef;
class omniServant;
class omniCurrent;

OMNI_NAMESPACE_BEGIN(omni)
class omniOrbPOA;
class giopAddress;
OMNI_NAMESPACE_END(omni)

//////////////////////////////////////////////////////////////////////
///////////////////////// omniCallDescriptor /////////////////////////
//////////////////////////////////////////////////////////////////////

class omniCallDescriptor {
public:
  typedef void (*LocalCallFn)(omniCallDescriptor*, omniServant*);

  struct InterceptorFn {
    LocalCallFn    fn;
    InterceptorFn* next;
  };

  // Call descriptor constructor.
  //
  // lcfn         -- function used to perform a local call on a servant
  // op           -- operation name. Null represents a LocateRequest
  //                 rather than a normal call
  // op_len       -- length of operation name
  // oneway       -- true if a oneway call
  // user_excns   -- set of valid user exceptions or null if not known
  // n_user_excns -- number of user exceptions
  // is_upcall    -- true if this is an upcall to a servant

  inline omniCallDescriptor(LocalCallFn	      lcfn,
			    const char*	      op_,
			    size_t	      op_len_,
			    _CORBA_Boolean    oneway,
			    const char*const* user_excns_,
			    int		      n_user_excns_,
                            _CORBA_Boolean    is_upcall_)
    : pd_do_call(sd_interceptor_call ? sd_interceptor_call : lcfn),
      pd_local_call(lcfn),
      pd_op(op_), pd_oplen(op_len_),
      pd_user_excns(user_excns_),
      pd_n_user_excns(n_user_excns_),
      pd_is_oneway(oneway),
      pd_is_upcall(is_upcall_),
      pd_contains_values(0),
      pd_first_address_used(0),
      pd_current_address(0),
      pd_objref(0),
      pd_poa(0),
      pd_localId(0)
  {}

  virtual ~omniCallDescriptor() {}

  //////////////////////////////////////////////////
  // Methods to implement call on the client side //
  //////////////////////////////////////////////////

  virtual void initialiseCall(cdrStream&);

  virtual void marshalArguments(cdrStream&);
  // Defaults to no arguments.

  virtual void unmarshalReturnedValues(cdrStream&);
  // Defaults to no arguments and returns void.

  virtual void userException(cdrStream& stream, _OMNI_NS(IOP_C)* iop_client,
			     const char*);
  // Called on the client side to handle a user exception.
  // Defaults to no user exceptions, and thus throws CORBA::UNKNOWN.
  // Most versions of this throw either a user exception or
  // CORBA::UNKNOWN, but it is permitted to return successfully. This
  // is done by DII, for example.
  // If iop_client is non-zero, must call iop_client->RequestCompleted().

  void validateUserException(const CORBA::UserException& ex);
  // Called to validate a user exception in a local call. If the call
  // descriptor knows the exception is not valid for the call, throws
  // CORBA::UNKNOWN. If the exception is valid, or the call descriptor
  // does not know, returns.

  //////////////////////////////////////////////////
  // Methods to implement call on the server side //
  //////////////////////////////////////////////////

  virtual void unmarshalArguments(cdrStream&);
  // Defaults to no arguments.

  virtual void marshalReturnedValues(cdrStream&);
  // Defaults to no arguments and returns void.

  ////////////////
  // Invocation //
  ////////////////

  // Called by stub code to perform a local call.
  inline void doLocalCall(omniServant* servant) {
    pd_do_call(this, servant);
  }

  // Interceptor functions *must* call this to continue the call.
  inline void interceptedCall(omniServant* servant) {
    LocalCallFn lcfn;
    if (pd_interceptor_stack) {
      lcfn = pd_interceptor_stack->fn;
      pd_interceptor_stack = pd_interceptor_stack->next;
    }
    else
      lcfn = pd_local_call;

    lcfn(this, servant);
  }

  ///////////////
  // Accessors //
  ///////////////

  inline const char* op() const            { return pd_op;           }
  inline size_t op_len() const             { return pd_oplen;        }
  inline _CORBA_Boolean is_oneway() const  { return pd_is_oneway;    }
  inline void set_oneway(_CORBA_Boolean o) { pd_is_oneway = o;       }
  inline const char* const* user_excns()   { return pd_user_excns;   }
  inline int n_user_excns()                { return pd_n_user_excns; }
  inline _CORBA_Boolean is_upcall() const  { return pd_is_upcall;    }

  inline _CORBA_Boolean haslocalCallFn() const {
    return (pd_local_call) ? 1 : 0;
  }

  inline const _OMNI_NS(giopAddress)* firstAddressUsed() { 
    return pd_first_address_used;
  }

  inline void firstAddressUsed(const _OMNI_NS(giopAddress)* a) { 
    pd_first_address_used = a;
  }

  inline const _OMNI_NS(giopAddress)* currentAddress() { 
    return pd_current_address;
  }

  inline void currentAddress(const _OMNI_NS(giopAddress)* a) { 
    pd_current_address = a;
  }

  inline const omni_time_t& getDeadline() const {
    return pd_deadline;
  }

  inline void setDeadline(const omni_time_t& deadline) {
    pd_deadline = deadline;
  }

  inline void containsValues(_CORBA_Boolean v) {
    pd_contains_values = v;
  }
  inline _CORBA_Boolean containsValues() {
    return pd_contains_values;
  }
  // If pd_contains_values is true, the arguments / return values
  // contain valuetypes. The spec requires that values are always
  // passed by value, so local calls must make copies. Since values
  // can be shared by separate arguments, the whole argument list must
  // be copied in a single operation. We do this by marshalling via a
  // temprary memory buffer. This is slow, so it can be turned off by
  // setting the copyValuesInLocalCalls parameter to false.


  /////////////////////
  // Current support //
  /////////////////////

  inline void objref(omniObjRef* o)           { pd_objref = o; }
  inline omniObjRef* objref()                 { return pd_objref; }
  inline void poa(_OMNI_NS(omniOrbPOA*) poa_) { pd_poa = poa_; }
  inline _OMNI_NS(omniOrbPOA*) poa()          { return pd_poa; }
  inline void localId(omniLocalIdentity* lid) { pd_localId = lid; }
  inline omniLocalIdentity* localId()         { return pd_localId; }


  //////////////////////////////
  // Interceptor registration //
  //////////////////////////////

  static void addInterceptor(LocalCallFn func);
  static void removeInterceptor(LocalCallFn func);


private:
  LocalCallFn                  pd_do_call;
  LocalCallFn                  pd_local_call;
  const char*                  pd_op;
  size_t                       pd_oplen;
  const char*const*            pd_user_excns;
  int                          pd_n_user_excns;
  _CORBA_Boolean               pd_is_oneway;
  _CORBA_Boolean               pd_is_upcall;
  _CORBA_Boolean               pd_contains_values;

  const _OMNI_NS(giopAddress)* pd_first_address_used;
  const _OMNI_NS(giopAddress)* pd_current_address;
  // state holder for the giop transport in relation to this call. Not
  // manipulated by this class other than the access functions.
  // Initialised to 0 in ctor.

  /////////////////////
  // Current support //
  /////////////////////

  omniCurrent*                 pd_current;
  omniCallDescriptor*          pd_current_next;
  // The omniCurrent object maintains a stack of call descriptors,
  // representing nested colocated calls. These pointers are
  // maintained by functions in omniCurrent.

  omniObjRef*                  pd_objref;
  // This is set on the client side to indicate the object reference
  // invoked on. It is used for two things: 1. to check if a servant
  // found by the in process identity can be used in a direct local
  // call; 2. as the return value from PortableServer::Current::
  // get_reference(). In the case of a remote upcall, this is zero,
  // and an objref is constructed from information in the local id on
  // request.

  _OMNI_NS(omniOrbPOA*)        pd_poa;
  omniLocalIdentity*           pd_localId;
  // Both always set on the way through the POA during an upcall.

  ////////////////////////////
  // Deadline for this call //
  ////////////////////////////

  // This is a state holder for the call. Not manipulated by this class
  // other than the access functions. Initialised to 0 in ctor.
  omni_time_t                  pd_deadline;


  ///////////////////////
  // Call interceptors //
  ///////////////////////

  static _core_attr LocalCallFn sd_interceptor_call;
  // Function used in place of the normal local call function if
  // interceptors are registered. Initialises the interceptor stack
  // and calls the first interceptor in the stack.

  static void setupInterception(omniCallDescriptor* cd, omniServant* servant);
  // Function assigned to sd_interceptor_call when interceptors are registered.

  static _core_attr InterceptorFn* sd_interceptor_stack;
  InterceptorFn*                   pd_interceptor_stack;
  // Global interceptor stack and this call descriptor's pointer into it.


  /////////////////////
  // Not implemented //
  /////////////////////

  omniCallDescriptor(const omniCallDescriptor&);
  omniCallDescriptor& operator = (const omniCallDescriptor&);
  // Not implemented.
};


//////////////////////////////////////////////////////////////////////
///////////////////////// omniAsyncCallDescriptor ////////////////////
//////////////////////////////////////////////////////////////////////

class omniAsyncCallDescriptor : public omniCallDescriptor
{
public:
  // Constructor for normal synchronous calls. Created on the stack.
  inline omniAsyncCallDescriptor(LocalCallFn	   lcfn,
				 const char*	   op_,
				 size_t		   op_len_,
				 _CORBA_Boolean	   oneway,
				 const char*const* user_excns_,
				 int		   n_user_excns_,
				 _CORBA_Boolean	   is_upcall_)
    : omniCallDescriptor(lcfn, op_, op_len_, oneway,
			 user_excns_, n_user_excns_,
			 is_upcall_),
      pd_exception(0),
      pd_cond(0),
      pd_set_cond(0)
  {}

  // Constructor for asynchronous calls. Created on the heap.
  // 
  // In callback AMI, completeCallback() makes the callback to the
  // ReplyHandler, then deletes this; in polling AMI, the poller
  // valuetype owns the call descriptor and deletes it when the poller
  // is deleted.
  inline omniAsyncCallDescriptor(LocalCallFn	   lcfn,
				 const char*	   op_,
				 size_t		   op_len_,
				 _CORBA_Boolean	   oneway,
				 const char*const* user_excns_,
				 int		   n_user_excns_)
    : omniCallDescriptor(lcfn, op_, op_len_, oneway,
			 user_excns_, n_user_excns_, 0),
      pd_exception(0),
      pd_cond(0),
      pd_set_cond(0),
      pd_complete(0),
      pd_do_callback(1)
  {}

  virtual ~omniAsyncCallDescriptor();

  virtual void completeCallback();

  // Handler set / get. Object reference is a Messaging::ReplyHandler
  // subclass.
  virtual void 	      setHandler(omniObjRef* objref);
  virtual omniObjRef* getHandler();


  inline void setComplete()
  {
    _CORBA_Boolean do_callback;
    {
      omni_tracedmutex_lock l(sd_lock);
      do_callback = pd_do_callback;
      pd_complete = 1;

      if (pd_cond)
	pd_cond->broadcast();

      if (pd_set_cond)
        pd_set_cond->signal();
    }
    if (do_callback)
      completeCallback();
  }

  inline _CORBA_Boolean isComplete()
  {
    omni_tracedmutex_lock l(sd_lock);
    return pd_complete;
  }

  inline _CORBA_Boolean lockedIsComplete()
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(sd_lock, 1);
    return pd_complete;
  }

  inline void wait()
  {
    omni_tracedmutex_lock l(sd_lock);
    
    if (pd_complete)
      return;

    if (!pd_cond)
      pd_cond = new omni_tracedcondition(&sd_lock,
					 "omniAsyncCallDescriptor::pd_cond");

    while (!pd_complete)
      pd_cond->wait();
  }

  inline _CORBA_Boolean wait(const omni_time_t& t)
  {
    omni_tracedmutex_lock l(sd_lock);

    if (pd_complete)
      return 1;

    if (!pd_cond)
      pd_cond = new omni_tracedcondition(&sd_lock,
					 "omniAsyncCallDescriptor::pd_cond");

    pd_cond->timedwait(t);
    return pd_complete;
  }

  inline _CORBA_Boolean isReady(CORBA::ULong timeout)
  {
    if (timeout == 0)
      return isComplete();

    if (timeout == 0xffffffff) {
      wait();
      return 1;
    }

    omni_time_t timeout_tt(timeout / 1000, (timeout % 1000) * 1000000);
    omni_time_t deadline;
    omni_thread::get_time(deadline, timeout_tt);

    return wait(deadline);
  }

    
  inline void storeException(const CORBA::Exception& ex)
  {
    pd_exception = CORBA::Exception::_duplicate(&ex);
  }

  inline _CORBA_Boolean exceptionOccurred()
  {
    return pd_exception ? 1 : 0;
  }

  inline void raiseException()
  {
    if (pd_exception)
      pd_exception->_raise();
  }

  inline CORBA::Exception* getException()
  {
    CORBA::Exception* ex = pd_exception;
    pd_exception = 0;
    return ex;
  }
  
  inline _CORBA_Boolean addToSet(omni_tracedcondition* set_cond)
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(sd_lock, 1);

    if (pd_set_cond)
      return 0;
    
    pd_set_cond = set_cond;
    return 1;
  }

  inline void remFromSet(omni_tracedcondition* set_cond)
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(sd_lock, 1);
    OMNIORB_ASSERT(pd_set_cond == set_cond);

    pd_set_cond = 0;
  }

  static _core_attr omni_tracedmutex sd_lock;
  
protected:
  CORBA::Exception*       pd_exception;
  omni_tracedcondition*   pd_cond;
  omni_tracedcondition*   pd_set_cond;
  _CORBA_Boolean          pd_complete;
  _CORBA_Boolean          pd_do_callback;

  // Not implemented
  omniAsyncCallDescriptor(const omniAsyncCallDescriptor&);
  omniAsyncCallDescriptor& operator=(const omniAsyncCallDescriptor&);
};


//////////////////////////////////////////////////////////////////////
/////////////////////////// omniStdCallDesc //////////////////////////
//////////////////////////////////////////////////////////////////////

//  This just provides a namespace for pre-defined call descriptors.

class omniStdCallDesc {
public:

  // Mangled signature: void
  typedef omniCallDescriptor void_call;

  // Mangled signature: _cCORBA_mObject_i_cstring
  class _cCORBA_mObject_i_cstring : public omniCallDescriptor {
  public:
    inline _cCORBA_mObject_i_cstring(LocalCallFn lcfn, const char* op_,
		     size_t oplen, const char* a_0,
		     _CORBA_Boolean upcall=0) :
      omniCallDescriptor(lcfn, op_, oplen, 0, 0, 0, upcall),
      arg_0( OMNI_CONST_CAST(char*, a_0) ) {}
 
    inline ~_cCORBA_mObject_i_cstring() {
      if (is_upcall()) { 
	_CORBA_String_helper::dealloc(arg_0);
      }
    }

    void marshalArguments(cdrStream&);
    void unmarshalReturnedValues(cdrStream&);
    void unmarshalArguments(cdrStream&);
    void marshalReturnedValues(cdrStream&);
 
    inline CORBA::Object_ptr result() { return pd_result._retn(); }
 
    char* arg_0;
    CORBA::Object_var pd_result;
  };

};


//////////////////////////////////////////////////////////////////////
///////////////////// omniLocalOnlyCallDescriptor ////////////////////
//////////////////////////////////////////////////////////////////////

// This class is needed to support calls to objects which may only
// reside in the local address space.  eg. ServantLocator,
// ServantActivator, AdapterActivator.

class omniLocalOnlyCallDescriptor : public omniCallDescriptor {
public:
  omniLocalOnlyCallDescriptor(LocalCallFn lcfn, const char* op_,
			      size_t op_len_, _CORBA_Boolean is_oneway_ = 0)
    : omniCallDescriptor(lcfn, op_, op_len_, is_oneway_, 0, 0, 0) {}

  // Only useful as client side descriptor. No set up for server side upcall.

  // We only need to override this one -- as it will throw an
  // exception, so the other members won't get called.
  void marshalArguments(cdrStream&);
};

#undef _core_attr

#endif  // __OMNIORB_CALLDESCRIPTOR_H__
