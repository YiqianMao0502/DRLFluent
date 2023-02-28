// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniObjRef.h               Created on: 22/2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2002-2011 Apasphere Ltd
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
// Description:
//    Base class for object references.
//

#ifndef __OMNIOBJREF_H__
#define __OMNIOBJREF_H__

OMNI_NAMESPACE_BEGIN(omni)

class omniInternal;

OMNI_NAMESPACE_END(omni)

class omniIOR;
class omniCallDescriptor;
class omniAsyncCallDescriptor;
class omniIdentity;
class omniLocalIdentity;
class omniObjTableEntry;
class omniRemoteIdentity;
class omniServant;

class omniObjRef {
public:
  virtual void* _ptrToObjRef(const char* repoId) = 0;
  // The most derived class which overrides this virtual function will be
  // called to return a pointer to the object reference class identified by
  // <repoId>.  So for IDL type Echo, the returned pointer will be an
  // Echo_ptr.  All object reference types should support widening to
  // CORBA::Object_ptr.
  //  If this reference object cannot be widened to the class specified, a
  // null pointer will be returned.  No attempt is made to contact the object
  // or create a new object refence of a more derived type.  See _realNarrow.
  //  <repoId> must not be NULL.
  //  This function does not throw any exceptions.
  //  This function is thread-safe.

  virtual const char* _localServantTarget();
  // Return a string which servant->_ptrToInterface() must accept for
  // the servant to be contacted with the local case optimisation.
  // This funtion is thread safe.

  inline const char* _mostDerivedRepoId() const {
    return pd_mostDerivedRepoId;
  }
  // The interface repository id of the most derived interface
  // supported by the object.  May be the empty string '\0'.
  //  This function is thread-safe.

  _CORBA_Boolean _real_is_a(const char* repoId);
  // Returns 1 if the object is really an instance of the type identified
  // by the IR repository ID <repoId>.  If the stub for the object type is
  // linked into the executable, the ORB can fully determine the _is_a
  // relation without the need to contact the object remotely.  Otherwise,
  // contact the object implementation using _remote_is_a().
  //  This function is thread-safe.

  void* _realNarrow(const char* repoId);
  // If the actual type of the object can be narrowed to the requested
  // interface type identified by the IR repository ID <repoId>, return
  // a valid object reference.  Otherwise, return 0.  The return value is
  // of type void* and can be casted to the T_ptr type of the interface
  // T directly.  If necessary we contact the object itself to check the
  // inheritance relation.  <repoId> must be a type for which there exists
  // a proxy object factory.
  //  This function is thread-safe.
  //  Does not throw any exceptions.

  void* _uncheckedNarrow(const char* repoId);
  // Return a valid object reference to the interface type identified
  // by <repoId>. We do not contact the object itself to check the
  // inheritance relation, so subsequent operations on this object may
  // fail because it actually does not implement the interface. This
  // function _always_ succeeds, no matter what interface we ask for.
  // <repoId> must be a type for which there exists a proxy object
  // factory.
  //  This function is thread-safe.
  //  Does not throw any exceptions.

  void _assertExistsAndTypeVerified();
  // If pd_flags.object_exists is zero, contact the object and verify
  // that it exists.
  //  If pd_flags.type_verified is zero, ask the object if it supports
  // the interface we are exporting.
  //  This function is thread-safe.

  _CORBA_Boolean _remote_is_a(const char* repoId);
  // Contacts the object implementation to determine whether it is an
  // instance of the type identified by <repoId>.  It is possible that
  // the implementation is a local servant of course.
  //  May throw system exceptions.
  //  This function is thread-safe.

  _CORBA_Boolean _remote_non_existent();
  // Contacts to object implementation to determine whether it exists.
  // It is possible that the implementation is a local servant of course.
  // Returns 0 if the object exists, or 1 or throws OBJECT_NOT_EXIST if
  // not.  May throw other system exceptions.
  //  This function is thread-safe.

  void* _transientExceptionHandler(void*& cookie, _CORBA_Boolean& ext);
  // If a transientExceptionHandler_t has been installed for this object
  // by _transientExceptionHandler(void*,void*), returns this handler and its
  // associated opaque argument in cookie.
  // Otherwise return 0.
  //  This function is thread-safe.

  void _transientExceptionHandler(void* new_handler, void* cookie,
                                  _CORBA_Boolean ext);
  // Set the transientExceptionHandler_t of this object.  By default,
  // i.e. when this function is not called for an object, the global
  // transientExceptionHandler_t will be invoked when a
  // CORBA::TRANSIENT exception is caught in a remote call from a
  // proxy object.  The argument <cookie> is an opaque argument that
  // will be passed to the exception handler. ext is true if the
  // handler has extended parameters.
  //  This function is thread-safe.

  void* _timeoutExceptionHandler(void*& cookie, _CORBA_Boolean& ext);
  // If a timeoutExceptionHandler_t has been installed for this object
  // by _timeoutExceptionHandler(void*,void*), returns this handler and its
  // associated opaque argument in cookie.
  // Otherwise return 0.
  //  This function is thread-safe.

  void _timeoutExceptionHandler(void* new_handler, void* cookie,
                                _CORBA_Boolean ext);
  // Set the timeoutExceptionHandler_t of this object.  By default,
  // i.e. when this function is not called for an object, the global
  // timeoutExceptionHandler_t will be invoked when a CORBA::TIMEOUT
  // exception is caught in a remote call from a proxy object.  The
  // argument <cookie> is an opaque argument that will be passed to
  // the exception handler. ext is true if the handler has extended
  // parameters.
  //  This function is thread-safe.

  void* _commFailureExceptionHandler(void*& cookie, _CORBA_Boolean& ext);
  // If a commFailureExceptionHandler_t has been installed for this object
  // by _commFailureExceptionHandler(void*,void*), returns this handler and its
  // associated opaque argument in cookie.
  // Otherwise return 0.
  //  This function is thread-safe.

  void _commFailureExceptionHandler(void* new_handler, void* cookie,
                                    _CORBA_Boolean ext);
  // Set the commFailureExceptionHandler_t of this object.  By
  // default, i.e. when this function is not called for an object, the
  // global commFailureExceptionHandler_t will be invoked when a
  // CORBA::COMM_FAILURE exception is caught in a remote call from a
  // proxy object.  The argument <cookie> is an opaque argument that
  // will be passed to the exception handler. ext is true if the
  // handler has extended parameters.
  //  This function is thread-safe.

  void* _systemExceptionHandler(void*& cookie, _CORBA_Boolean& ext);
  // If a systemExceptionHandler_t has been installed for this object
  // by _systemExceptionHandler(void*,void*), returns this handler and its
  // associated opaque argument in cookie.
  // Otherwise return 0.
  //  This function is thread-safe.

  void _systemExceptionHandler(void* new_handler, void* cookie,
                               _CORBA_Boolean ext);
  // Set the systemExceptionHandler_t of this object.  By default,
  // i.e. when this function is not called for an object, the global
  // systemExceptionHandler_t will be invoked when a
  // CORBA::SystemException exception, other than CORBA::TRANSIENT,
  // CORBA::COMM_FAILURE or CORBA::TIMEOUT is caught in a remote call
  // from a proxy object.  The handlers for CORBA::TRANSIENT,
  // CORBA::COMM_FAILURE and CORBA::TIMEOUT are installed their own
  // install functions.  The argument <cookie> is an opaque argument
  // that will be passed to the exception handler. ext is true if the
  // handler has extended parameters.
  // This function is thread-safe.

  inline omniIdentity* _identity() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_id;
  }
  // Must hold <omni::internalLock>.

  inline int _isForwardLocation() const {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_flags.forward_location;
  }
  //  Must hold <omni::internalLock>.

  inline _CORBA_Boolean _is_nil() { return !pd_ior; }
  // This function is thread-safe.

  inline void _noExistentCheck() {
    pd_flags.type_verified = 1;
    pd_flags.object_exists = 1;
  }
  // This function instructs the ORB to skip the existence test
  // performed in _assertExistsAndTypeVerified().
  //  Really ought to only call this if you know no-one else
  // is accessing this reference, or holding <omni::internalLock>.

  _CORBA_Boolean __is_equivalent(omniObjRef* other_obj);
  // Returns true if this and the other_obj is equivalent. In other
  // words, they both have the same object key and in the same address space.
  // Caller must not hold <omni::internalLock>
  // other_obj must not be nil.
  // This function is thread-safe.

  _CORBA_ULong __hash(_CORBA_ULong maximum);
  // Returns the result of passing the object key through the ORB's hash
  // function. The return value is not larger than maximum.
  // Caller must not hold <omni::internalLock>
  // This function is thread-safe.

  void _invoke(omniCallDescriptor&, _CORBA_Boolean do_assert=1);
  // Does the client-side work of making a request.  Dispaches
  // the call to the object identity, and deals with exception
  // handlers, retries and location forwarding.

  void _invoke_async(omniAsyncCallDescriptor*);
  // Equivalent to _invoke, but the call is performed asynchronously.
  // The call descriptor is informed when the call completes.

  omniIOR* _getIOR();
  // Returns an omniIOR. The object contains all the fields in
  // the IOR of the object reference. The object is read-only
  // and shouldn't be modified. The returned object should be
  // released by calling its release() method. This
  // function is atomic and thread-safe.
  
  static void _marshal(omniObjRef*, cdrStream&);
  // Marshal this object reference to the cdrStream.

  static omniObjRef* _unMarshal(const char* repoId, cdrStream& s);
  // Unmarshal from the cdrStream an object reference. The
  // object reference is expected to implement the interface identified
  // by the repository ID <repoId>.

  static char* _toString(omniObjRef*);
  static omniObjRef* _fromString(const char*);

  void _setIdentity(omniIdentity* id);
  // Set or change the identity to dispatch to.
  //  Must hold <omni::internalLock>.

  static void _shutdown();
  // Called during ORB shutdown. Disables all object references.
  //  Must not hold <omni::internalLock>.

  virtual void _enableShortcut(omniServant* svt,
			       const _CORBA_Boolean* invalid);
  // If <svt> is non-zero, it is a local activated servant. This
  // function is called by an object adapter to indicate that it is
  // acceptable to make future calls directly to the servant, avoiding
  // the object adapter. If the boolean pointed to by <invalid>
  // becomes true, the servant is no longer valid and a normal
  // dispatch through _invoke() should be made. A zero <svt> pointer
  // indicates that the shortcut can no longer be made. This mechanism
  // has obvious race conditions, but that's the price you pay for
  // speed.
  //  No concurrency control!

  inline void _setTimeout(unsigned long secs, unsigned long ns)
  {
    pd_timeout.assign(secs, ns);
  }

  void _NP_disconnect();
  // Forcibly disconnect all connections associated with this object
  // reference.

protected:
  virtual ~omniObjRef();
  // Must not hold <omni::internalLock>.

  omniObjRef();
  // Constructor for nil object references.  No operations should
  // be invoked on nil object references.  This should be tested
  // in the public interface.

  omniObjRef(const char* intfRepoId, omniIOR* ior,
	     omniIdentity* id, _CORBA_Boolean static_repoId = 0);
  // Constructed with an initial ref count of 1.  If <static_repoId>
  // is true, assumes that the <intfRepoId> string will remain valid
  // for the entire life time of the objref, meaning that there is no
  // need to copy it.

  void _locateRequest();
  // Issues a GIOP LocateRequest.  Throws OBJECT_NOT_EXIST, or
  // returns normally if the object does exist.
  // This function is thread-safe.

private:
  omniObjRef(const omniObjRef&);
  omniObjRef& operator = (const omniObjRef&);
  // Not implemented.

  void _disable();
  // Disable the object reference so all future invocations throw BAD_INV_ORDER
  //  Must hold <omni::internalLock>.


  ///////////////////////////////////////////////
  // Some friends need to access the internals //
  ///////////////////////////////////////////////
  friend class _OMNI_NS(omniInternal);
  friend class omniPy;
  friend void omni::duplicateObjRef(omniObjRef*);
  friend void omni::releaseObjRef(omniObjRef*);

  friend omniObjRef* omni::createObjRef(const char*,omniIOR*,
					_CORBA_Boolean,omniIdentity*);

  friend omniObjRef* omni::createLocalObjRef(const char*, const char*,
					     omniObjTableEntry*,
					     const omniIORHints&);

  friend void omni::revertToOriginalProfile(omniObjRef*);
  friend void omni::locationForward(omniObjRef*,
				    omniObjRef*,
				    _CORBA_Boolean);

  //////////////////
  // Data members //
  //////////////////

  int pd_refCount;
  // Protected by <omniInternal.cc:objref_rc_lock>.

  char* pd_mostDerivedRepoId;
  // The interface repository id that we have been told is the
  // most derived interface of the object implementation.  This
  // may be the same as the static type of this reference, or
  // it may be wrong (if the implementation has been replaced
  // by one of a more derived type), or it may be '\0' (if the
  // IOR contained no repoId).
  //  Immutable.

  char* pd_intfRepoId;
  // The repository id of the interface that this reference is
  // presenting.  That is, if this is a c++ <foo_ptr>, then this
  // will be the repository id for <foo>.  Thus for an anonymous
  // object reference, this is the CORBA::Object repoId.  Clearly
  // _ptrToObjRef(pd_intfRepoId) must be non-zero!
  //  This may or may not be equal to <pd_mostDerivedRepoId>,
  // but unlike <pd_mostDerivedRepoId> it may *not* be the empty
  // string.
  //  Immutable.

  omniIOR* pd_ior;
  // The decoded IOR of this object reference.
  // Mutable. Protected by <omniIOR::lock>.

  omniIdentity* pd_id;
  // An encapsulation of the current implementation of this
  // object.  Either a local object, or the location and
  // key of a remote object.
  //  Mutable.  Protected by <omni::internalLock>.

  omniObjRef*  pd_next;
  omniObjRef** pd_prev;
  // Doubly linked list of all non-nil object references, so they may
  // be disabled at ORB shutdown.
  //  Protected by <omni::objref_rc_lock>.

  struct {
    unsigned forward_location            : 1;
    // True if we have had a LOCATION_FORWARD.  In this case the
    // identity (pd_id) is not the same as the one referred to in the
    // iop profiles.

    unsigned type_verified               : 1;
    // True if we believe that the object implementation supports
    // the interface <pd_intfRepoId>.  If this is zero then we
    // must contact the object to confirm that it supports this
    // interface before making any invocations.

    unsigned object_exists               : 1;
    // True if we have asked the object if it exists, and it says
    // yes.  This is set to zero in the constructor, so that we
    // will contact the object and check that it exists before the
    // very first invocation.

    unsigned transient_exception_handler : 1;
    unsigned timeout_exception_handler   : 1;
    unsigned commfail_exception_handler  : 1;
    unsigned system_exception_handler    : 1;
    // True if a per-object exception handler has been installed
    // for this reference.

    unsigned static_repoId               : 1;
    // True if pd_intfRepoId is static and should not be deleted when
    // this objref is destroyed.

    unsigned orb_shutdown                : 1;
    // True if the ORB that created this objref has been shut down.

  } pd_flags;
  // Mutable.  Protected by <omni::internalLock>.

  omni_time_t pd_timeout;
};


#endif  // __OMNIOBJREF_H__
