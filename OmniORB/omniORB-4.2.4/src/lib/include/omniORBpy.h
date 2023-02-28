// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// omniORBpy.h                Created on: 2002/05/25
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2002-2018 Duncan Grisby
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
// Description:
//    Header for the C++ API to omniORBpy

#ifndef _omniORBpy_h_
#define _omniORBpy_h_

// The file including this file must include the correct Python.h
// header. This file does not include it, to avoid difficulties with
// its name.

#include <omniORB4/CORBA.h>

// The omniORBpy C++ API consists of a singleton structure containing
// function pointers.
//
// In Python 3.x, a pointer to the API struct is stored in a PyCapsule
// named "_omnipy.API". Access it with code like:
//
//      omniORBpyAPI* api = (omniORBpyAPI*)PyCapsule_Import("_omnipy.API", 0);
// 
//
// In Python 2.x, a pointer to the API struct is stored as a PyCObject
// in the _omnipy module with the name API. Access it with code like:
//
//      PyObject*     omnipy = PyImport_ImportModule((char*)"_omnipy");
//      PyObject*     pyapi  = PyObject_GetAttrString(omnipy, (char*)"API");
//      omniORBpyAPI* api    = (omniORBpyAPI*)PyCObject_AsVoidPtr(pyapi);
//      Py_DECREF(pyapi);
//
// Obviously, you MUST NOT modify the function pointers!
//
// This arrangement of things means you do not have to link to the
// _omnipymodule library to be able to use the API.


struct omniORBpyAPI {

  PyObject* (*cxxObjRefToPyObjRef)(const CORBA::Object_ptr cxx_obj,
				   CORBA::Boolean hold_lock);
  // Convert a C++ object reference to a Python object reference.
  // If <hold_lock> is true, caller holds the Python interpreter lock.

  CORBA::Object_ptr (*pyObjRefToCxxObjRef)(PyObject* py_obj,
					   CORBA::Boolean hold_lock);
  // Convert a Python object reference to a C++ object reference.
  // Raises BAD_PARAM if the Python object is not an object reference.
  // If <hold_lock> is true, caller holds the Python interpreter lock.

  PyObject* (*handleCxxSystemException)(const CORBA::SystemException& ex);
  // Sets the Python exception state to reflect the given C++ system
  // exception. Always returns NULL. The caller must hold the Python
  // interpreter lock.

  void (*handlePythonSystemException)();
  // Handles the current Python exception. An exception must have
  // occurred. Handles all system exceptions and omniORB.
  // LocationForward; all other exceptions print a traceback and raise
  // CORBA::UNKNOWN. The caller must hold the Python interpreter lock.

  void (*marshalPyObject)(cdrStream& stream,
			  PyObject* desc, PyObject* obj,
			  CORBA::Boolean hold_lock);
  // Marshal the Python object into the stream, based on the type
  // descriptor desc.

  PyObject* (*unmarshalPyObject)(cdrStream& stream,
				 PyObject* desc, CORBA::Boolean hold_lock);
  // Unmarshal a Python object from the stream, based on type
  // descriptor desc.

  void (*marshalTypeDesc)(cdrStream& stream, PyObject* desc,
			  CORBA::Boolean hold_lock);
  // Marshal the type descriptor into the stream as a TypeCode.

  PyObject* (*unmarshalTypeDesc)(cdrStream& stream, CORBA::Boolean hold_lock);
  // Unmarshal a TypeCode from the stream, giving a type descriptor.

  void* (*acquireGIL)();
  // Acquire the Python Global Interpreter Lock in a way consistent
  // with omniORB's threads. Returns an opaque pointer that must be
  // given to releaseGIL to release the lock.

  void (*releaseGIL)(void* ptr);
  // Release the Python Global Interpreter Lock acquired with
  // acquireGIL.

  omniORBpyAPI();
  // Constructor for the singleton. Sets up the function pointers.
};


// Python GIL acquisition class

class omniORBpyLock {
public:
  inline omniORBpyLock(omniORBpyAPI* api, CORBA::Boolean do_it=1)
    : api_(api), ptr_(0), do_it_(do_it)
  {
    if (do_it)
      ptr_ = api->acquireGIL();
  }

  inline ~omniORBpyLock()
  {
    if (do_it_)
      api_->releaseGIL(ptr_);
  }
private:
  omniORBpyAPI*  api_;
  void*          ptr_;
  CORBA::Boolean do_it_;
};


// Macros to catch all C++ system exceptions and convert to Python
// exceptions. Use like
//
// try {
//   ...
// }
// OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
//
// The macros assume that api is a pointer to the omniORBpyAPI
// structure above.

#ifdef HAS_Cplusplus_catch_exception_by_base

#define OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS \
catch (const CORBA::SystemException& ex) { \
  return api->handleCxxSystemException(ex); \
}
#else

#define OMNIORBPY_CATCH_AND_HANDLE_SPECIFIED_EXCEPTION(exc) \
catch (const CORBA::exc& ex) { \
  return api->handleCxxSystemException(ex); \
}
#define OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS \
  OMNIORB_FOR_EACH_SYS_EXCEPTION(OMNIORBPY_CATCH_AND_HANDLE_SPECIFIED_EXCEPTION)

#endif


// Extensions to omniORB / omniORBpy may create their own pseudo
// object reference types. To provide a Python mapping for these, a
// function must be provided that takes a CORBA::Object_ptr and
// returns a suitable PyObject. Functions are registered by appending
// PyCObjects to the list _omnipy.pseudoFns. The CObjects must contain
// pointers to functions with this signature:

typedef PyObject* (*omniORBpyPseudoFn)(const CORBA::Object_ptr);


// Extensions may register functions to translate Python Policy
// objects to C++ CORBA::Policy objects. _omnipy.policyFns is a
// dictionary mapping CORBA::PolicyType to PyCObjects containing
// functions pointers. Functions take a policy value (i.e. the value
// inside the Python Policy object, not the Policy object itself), and
// must return a valid CORBA::Policy object, CORBA::Policy::_nil, or
// throw a CORBA::SystemException.

typedef CORBA::Policy_ptr (*omniORBpyPolicyFn)(PyObject*);



#endif // _omniORBpy_h_
