// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// omnipy.h                   Created on: 2000/02/24
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2014 Apasphere Ltd
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
//    Master header file for omnipy internals

#ifndef _omnipy_h_
#define _omnipy_h_

#define PY_SSIZE_T_CLEAN

#if defined(__VMS)
#include <Python.h>
#else
#include PYTHON_INCLUDE
#endif

#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/minorCode.h>
#include <exceptiondefs.h>
#include <objectTable.h>
#include <orbParameters.h>
#include <omniORBpy.h>
#include "omnipy_sysdep.h"
#include "pyThreadCache.h"

#undef minor


OMNI_USING_NAMESPACE(omni)

////////////////////////////////////////////////////////////////////////////
// Python types                                                           //
////////////////////////////////////////////////////////////////////////////

extern "C" {

  // Object reference
  struct PyObjRefObject {
    PyObject_HEAD
    CORBA::Object_ptr obj;
  };

  // ORB
  struct PyORBObject {
    PyObjRefObject base;
    CORBA::ORB_ptr orb;
  };

  // POA
  struct PyPOAObject {
    PyObjRefObject base;
    PortableServer::POA_ptr poa;
  };

  // POAManager
  struct PyPOAManagerObject {
    PyObjRefObject base;
    PortableServer::POAManager_ptr pm;
  };

  // POACurrent
  struct PyPOACurrentObject {
    PyObjRefObject base;
    PortableServer::Current_ptr pc;
  };
}


////////////////////////////////////////////////////////////////////////////
// Exception handling                                                     //
////////////////////////////////////////////////////////////////////////////

class Py_BAD_PARAM : public CORBA::BAD_PARAM {
public:
  inline Py_BAD_PARAM(CORBA::ULong minor, CORBA::CompletionStatus completed,
		      PyObject* message)
    : CORBA::BAD_PARAM(minor, completed)
  {
    info_ = PyList_New(1);
    PyList_SetItem(info_, 0, message);
  }

  inline ~Py_BAD_PARAM()
  {
    // We cannot guarantee that the interpreter lock is held when the
    // destructor is run, so ownership of the info list should have
    // been handed over before now.
    if (info_) {
      omniORB::logs(1, "ERROR: Py_BAD_PARAM info not freed.");
    }
  }

  // Log the exception if necessary, then throw it.
  static void raise(const char* file, int line,
		    CORBA::ULong minor, CORBA::CompletionStatus completed,
		    PyObject* message);


  // Add a message to the info.
  inline void add(PyObject* message)
  {
    PyList_Append(info_, message);
  }

  // Return the stack of messages. Caller takes ownership of the list.
  inline PyObject* getInfo()
  {
    PyObject* r = info_;
    info_ = 0;
    return r;
  }

  // Log the stack of messages then re-throw a base BAD_PARAM.
  // Releases the list.
  inline void logInfoAndThrow()
  {
    PyObject* info = getInfo();
    if (omniORB::traceExceptions) {
      PyObject* info_repr = PyObject_Repr(info);
      omniORB::logger log;
      log << "BAD_PARAM info: " << String_AsString(info_repr) << "\n";
      Py_DECREF(info_repr);
    }
    Py_DECREF(info);
    throw CORBA::BAD_PARAM(minor(), completed());
  }

  // Log the stack of messages and release them.
  inline void logInfoAndDrop(const char* msg=0)
  {
    PyObject* info = getInfo();

    if (omniORB::traceExceptions) {
      PyObject* info_repr = PyObject_Repr(info);
      omniORB::logger log;

      if (msg)
        log << msg << ": ";

      log << "BAD_PARAM info: " << String_AsString(info_repr) << "\n";
      Py_DECREF(info_repr);
    }
    Py_DECREF(info);
  }


  // Virtual functions inherited from CORBA::SystemException
  CORBA::Exception* _NP_duplicate() const;
  void              _raise()        const;
  const char*       _NP_typeId()    const;

  // Copy constructor that transfers ownership of info_
  inline Py_BAD_PARAM(const Py_BAD_PARAM& e)
    : CORBA::BAD_PARAM(e.minor(), e.completed())
  {
    Py_BAD_PARAM* ne = OMNI_CONST_CAST(Py_BAD_PARAM*, &e);
    info_ = ne->getInfo();
  }

  static Py_BAD_PARAM* _downcast(CORBA::Exception* e);

private:
  PyObject* info_; // Stack of messages.
};


#define THROW_PY_BAD_PARAM(minor, completion, message) \
  Py_BAD_PARAM::raise(__FILE__, __LINE__, minor, completion, message);


// Useful macro
#define RAISE_PY_BAD_PARAM_IF(x,minor) \
  if (x) { \
    CORBA::BAD_PARAM _ex(minor, CORBA::COMPLETED_NO); \
    return omniPy::handleSystemException(_ex); \
  }

class omniPy {
public:

  ////////////////////////////////////////////////////////////////////////////
  // The global Python interpreter state                                    //
  ////////////////////////////////////////////////////////////////////////////

  static PyInterpreterState* pyInterpreter;


  ////////////////////////////////////////////////////////////////////////////
  // Global pointers to Python objects                                      //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject* py_omnipymodule;    	// _omnipy module
  static PyObject* py_pseudoFns;        //  pseudoFns
  static PyObject* py_policyFns;        //  policyFns
  static PyObject* pyCORBAmodule;      	// CORBA module
  static PyObject* pyCORBAsysExcMap;   	//  The system exception map
  static PyObject* pyCORBAORBClass;    	//  ORB class
  static PyObject* pyCORBAAnyClass;    	//  Any class
  static PyObject* pyCORBATypeCodeClass;//  TypeCode class
  static PyObject* pyCORBAContextClass;	//  Context class
  static PyObject* pyCORBAValueBase;    //  ValueBase class
  static PyObject* pyCORBAValueBaseDesc;//  Descriptor for ValueBase
  static PyObject* pyomniORBmodule;    	// The omniORB module
  static PyObject* pyomniORBobjrefMap; 	//  The objref class map
  static PyObject* pyomniORBskeletonMap;//  The skeleton class map
  static PyObject* pyomniORBtypeMap;   	//  Type map
  static PyObject* pyomniORBvalueMap;  	//  Value factory map
  static PyObject* pyomniORBwordMap;   	//  Reserved word map
  static PyObject* pyomniORBUnknownValueBase;
                                        //  Base class for unknown valuetypes
  static PyObject* pyPortableServerModule;
                                        // Portable server module
  static PyObject* pyPOAClass;          //  POA class
  static PyObject* pyPOAManagerClass;   //  POAManager class
  static PyObject* pyPOACurrentClass;   //  Current class
  static PyObject* pyServantClass;     	//  Servant class
  static PyObject* pyCreateTypeCode;   	// Function to create a TypeCode object
  static PyObject* pyWorkerThreadClass;	// Worker thread class
  static PyObject* pyEmptyTuple;       	// Zero element tuple


  ////////////////////////////////////////////////////////////////////////////
  // 'Static' strings                                                       //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject* pyservantAttr;
  static PyObject* pyobjAttr;
  static PyObject* pyNP_RepositoryId;


  ////////////////////////////////////////////////////////////////////////////
  // Constant strings to facilitate comparison by pointer                   //
  ////////////////////////////////////////////////////////////////////////////

  static const char* string_Py_omniObjRef;
  static const char* string_Py_omniServant;
  static const char* string_Py_ServantActivator;
  static const char* string_Py_ServantLocator;
  static const char* string_Py_AdapterActivator;


  ////////////////////////////////////////////////////////////////////////////
  // Pointer to the ORB                                                     //
  ////////////////////////////////////////////////////////////////////////////

  static CORBA::ORB_ptr orb;


  ////////////////////////////////////////////////////////////////////////////
  // Code sets                                                              //
  ////////////////////////////////////////////////////////////////////////////

  static omniCodeSet::NCS_C* ncs_c_utf_8;


  ////////////////////////////////////////////////////////////////////////////
  // C++ API object                                                         //
  ////////////////////////////////////////////////////////////////////////////

  static omniORBpyAPI cxxAPI;


  ////////////////////////////////////////////////////////////////////////////
  // Module initialisation functions                                        //
  ////////////////////////////////////////////////////////////////////////////

  static void initObjRefFunc     (PyObject* d);
  static void initORBFunc        (PyObject* d);
  static void initPOAFunc        (PyObject* d);
  static void initPOAManagerFunc (PyObject* d);
  static void initPOACurrentFunc (PyObject* d);
  static void initInterceptorFunc(PyObject* d);
  static void initomniFunc       (PyObject* d);
  static void initFixed          (PyObject* d);
  static void initCallDescriptor (PyObject* d);
  static void initServant        (PyObject* d);
  static void initTypeCode       (PyObject* d);


  ////////////////////////////////////////////////////////////////////////////
  // PyRefHolder holds a references to a Python object                      //
  ////////////////////////////////////////////////////////////////////////////

  class PyRefHolder {
  public:
    inline PyRefHolder(PyObject* obj=0)    : obj_(obj) {}
    inline PyRefHolder(PyObject* obj, int) : obj_(obj) { Py_XINCREF(obj); }

    inline ~PyRefHolder() { Py_XDECREF(obj_); }

    inline PyObject* retn() {
      PyObject* r = obj_;
      obj_ = 0;
      return r;
    }

    inline PyObject* dup() {
      Py_XINCREF(obj_);
      return obj_;
    }

    inline PyRefHolder& operator=(PyObject* obj)
    {
      if (obj != obj_) {
        Py_XDECREF(obj_);
        obj_ = obj;
      }
      return *this;
    }

    inline PyObject* change(PyObject* obj) {
      if (obj != obj_) {
        Py_XDECREF(obj_);
        obj_ = obj;
      }
      return obj;
    }

    inline PyObject* obj() {
      return obj_;
    }

    inline CORBA::Boolean valid() {
      return obj_ != 0;
    }

    // Cast operators for various concrete Python types, to allow
    // PyObjectHolder to be passed in Python API functions.
    inline operator PyObject*()        { return obj_; }
    inline operator PyVarObject*()     { return (PyVarObject*)obj_; }
    inline operator PyLongObject*()    { return (PyLongObject*)obj_; }
    inline operator PyListObject*()    { return (PyListObject*)obj_; }
    inline operator PyTupleObject*()   { return (PyTupleObject*)obj_; }

#if (PY_VERSION_HEX < 0x03000000)
    inline operator PyIntObject*()     { return (PyIntObject*)obj_; }
    inline operator PyStringObject*()  { return (PyStringObject*)obj_; }
#else
    inline operator PyBytesObject*()   { return (PyBytesObject*)obj_; }
#endif
    inline operator PyUnicodeObject*() { return (PyUnicodeObject*)obj_; }

#if (PY_VERSION_HEX >= 0x03030000)
    inline operator PyASCIIObject*()   { return (PyASCIIObject*)obj_; }
    inline operator PyCompactUnicodeObject*()
                                       { return (PyCompactUnicodeObject*)obj_; }
#endif

    // Operators for our own types
    inline operator PyObjRefObject*()  { return (PyObjRefObject*)obj_; }
    inline operator PyPOAObject*()     { return (PyPOAObject*)obj_; }

    // Pointer operator used in some Python macros like PyInt_Check.
    inline PyObject* operator->()      { return obj_; }

  private:
    PyObject* obj_;

    // Not implemented
    PyRefHolder(const PyRefHolder&);
    PyRefHolder& operator=(const PyRefHolder&);
  };


  ////////////////////////////////////////////////////////////////////////////
  // InterpreterUnlocker releases the Python interpreter lock               //
  ////////////////////////////////////////////////////////////////////////////

  class InterpreterUnlocker {
  public:
    inline InterpreterUnlocker() {
      tstate_ = PyEval_SaveThread();
    }
    inline ~InterpreterUnlocker() {
      PyEval_RestoreThread(tstate_);
    }
    inline void lock() {
      PyEval_RestoreThread(tstate_);
    }
    inline void unlock() {
      tstate_ = PyEval_SaveThread();
    }
  private:
    PyThreadState* tstate_;
  };


  ////////////////////////////////////////////////////////////////////////////
  // Utility functions                                                      //
  ////////////////////////////////////////////////////////////////////////////

  // Set the Python execution state to handle a system exception.
  // Returns a NULL PyObject so you can say
  //   return handleSystemException(ex).
  static
  PyObject* handleSystemException(const CORBA::SystemException& ex,
				  PyObject* info = 0);

  // Create a new Python object for the given system exception
  static
  PyObject* createPySystemException(const CORBA::SystemException& ex);

  // Throw a C++ system exception equivalent to the given Python exception
  static
  void produceSystemException(PyObject* eobj, PyObject* erepoId,
			      PyObject* etype, PyObject* etraceback);

  // Handle the current Python exception. An exception must have
  // occurred. Deals with system exceptions and
  // omniORB.LocationForward; all other exceptions print a traceback
  // and raise UNKNOWN.
  static
  void handlePythonException();

  // Raise an exception with no arguments
  static
  PyObject* raiseScopedException(PyObject* module, const char* scope,
                                 const char* exc_class);


  // Handle the omniORB.LocationForward exception in the argument.
  static
  void handleLocationForward(PyObject* evalue);
  
  // Ensure there is an omni_thread associated with the calling thread.
  static
  omni_thread* ensureOmniThread();

  // String formatting function. Equivalent to Python fmt % (args)
  static
  PyObject* formatString(const char* fmt, const char* pyfmt, ...);

  // Get ULong from integer
  static inline
  CORBA::ULong
  getULongVal(PyObject* obj,
              CORBA::CompletionStatus completion = CORBA::COMPLETED_NO)
  {
#if (PY_VERSION_HEX < 0x03000000)

    if (PyInt_Check(obj)) {
      long v = PyInt_AS_LONG(obj);

      if (v < 0
#if SIZEOF_LONG > 4
          || v > 0xffffffff
#endif
          ) {
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, completion,
                           formatString("Value %s out of range for ULong",
                                        "O", obj))
      }
      return (CORBA::ULong)v;
    }
#endif
    if (!PyLong_Check(obj))
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, completion,
                         formatString("Expecting int, got %r", "O",
                                      obj->ob_type));

    unsigned long v = PyLong_AsUnsignedLong(obj);

    if (PyErr_Occurred() 
#if SIZEOF_LONG > 4
        || v > 0xffffffff
#endif
        ) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, completion,
                         formatString("Value %s out of range for ULong",
                                      "O", obj));
    }
    return v;
  }

  // Get value from an enum
  static inline
  CORBA::ULong getEnumVal(PyObject* pyenum)
  {
    PyRefHolder ev(PyObject_GetAttrString(pyenum, (char*)"_v"));
    if (!ev.valid())
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO,
                         omniPy::formatString("Expecting enum item, got %r",
                                              "O", pyenum->ob_type));
    return getULongVal(ev);
  }


  ////////////////////////////////////////////////////////////////////////////
  // Fixed point                                                            //
  ////////////////////////////////////////////////////////////////////////////

  // Create a new omnipyFixedObject.
  static
  PyObject* newFixedObject(const CORBA::Fixed& f);

  // Version for CORBA.fixed() function
  static
  PyObject* newFixedObject(PyObject* self, PyObject* args);


  ////////////////////////////////////////////////////////////////////////////
  // Python object creation functions. Take ownership of passed in objects. //
  ////////////////////////////////////////////////////////////////////////////

  static
  PyObject* createPyObjRefObject(CORBA::Object_ptr obj);

  static
  PyObject* createPyORBObject(CORBA::ORB_ptr orb);

  static
  PyObject* createPyPOAObject(PortableServer::POA_ptr poa);

  static
  PyObject* createPyPOAManagerObject(PortableServer::POAManager_ptr pm);

  static
  PyObject* createPyPOACurrentObject(PortableServer::Current_ptr pc);


  ////////////////////////////////////////////////////////////////////////////
  // Python type checking                                                   //
  ////////////////////////////////////////////////////////////////////////////

  static PyTypeObject*  PyObjRefType;

  static CORBA::Boolean pyObjRefCheck(PyObject* pyobj);
  static CORBA::Boolean pyORBCheck(PyObject* pyobj);
  static CORBA::Boolean pyPOACheck(PyObject* pyobj);
  static CORBA::Boolean pyPOAManagerCheck(PyObject* pyobj);
  static CORBA::Boolean pyPOACurrentCheck(PyObject* pyobj);


  ////////////////////////////////////////////////////////////////////////////
  // Object reference functions                                             //
  ////////////////////////////////////////////////////////////////////////////

  // Get CORBA::Object_ptr from a Python object reference. Returns
  // null if not a valid reference.
  static inline CORBA::Object_ptr
  getObjRef(PyObject* pyobj)
  {
    PyRefHolder pyobjref(PyObject_GetAttr(pyobj, pyobjAttr));

    if (pyobjref.valid() && pyObjRefCheck(pyobjref)) {
      return ((PyObjRefObject*)pyobjref)->obj;
    }
    else {
      PyErr_Clear();
      return 0;
    }
  }


  //
  // Caller must hold the Python interpreter lock.
  static
  PyObject* createPyCorbaObjRef(const char* targetRepoId,
				const CORBA::Object_ptr objref);

  static
  PyObject* createPyPseudoObjRef(const CORBA::Object_ptr objref);


  // Functions which mirror omni::createObjRef(). These versions don't
  // look for C++ proxy factories, and spot local Python servants.
  //
  // Caller must NOT hold the Python interpreter lock.
  static
  omniObjRef* createObjRef(const char*        targetRepoId,
			   omniIOR*           ior,
			   CORBA::Boolean     locked,
			   omniIdentity*      id = 0,
			   CORBA::Boolean     type_verified = 0,
			   CORBA::Boolean     is_forwarded = 0);

  static
  omniObjRef* createLocalObjRef(const char*         mostDerivedRepoId,
				const char*         targetRepoId,
				omniObjTableEntry*  entry,
				omniObjRef*         orig_ref,
				CORBA::Boolean      type_verified = 0);

  static
  omniObjRef* createLocalObjRef(const char* 	    mostDerivedRepoId,
				const char* 	    targetRepoId,
				const _CORBA_Octet* key,
				int                 keysize,
				omniObjRef*         orig_ref,
				CORBA::Boolean      type_verified = 0);

  // When a POA creates a reference to a Python servant, it does not
  // have a proxy object factory for it, so it creates an
  // omniAnonObjRef. This function converts one of them into a
  // Py_omniObjRef with a reference to the local servant.
  //
  // Caller must NOT hold the Python interpreter lock.
  static
  CORBA::Object_ptr makeLocalObjRef(const char* targetRepoId,
				    const CORBA::Object_ptr objref);

  // Copy a Python object reference in an argument or return value.
  // Compares the type of the objref with the target type, and creates
  // a new objref of the target type if they are not compatible.
  // Throws BAD_PARAM if the Python object is not an object reference.
  //
  // Caller must hold the Python interpreter lock.
  static
  PyObject* copyObjRefArgument(PyObject*               pytargetRepoId,
			       PyObject*               pyobjref,
			       CORBA::CompletionStatus compstatus);

  // Mirror of omniURI::stringToObject(). Caller must hold the Python
  // interpreter lock.
  static
  CORBA::Object_ptr stringToObject(const char* uri);

  // Mirror of CORBA::UnMarshalObjRef(). Caller must hold the Python
  // interpreter lock.
  static
  CORBA::Object_ptr UnMarshalObjRef(const char* repoId, cdrStream& s);



  ////////////////////////////////////////////////////////////////////////////
  // Recursive marshalling functions                                        //
  ////////////////////////////////////////////////////////////////////////////
  
  // Helper function to return the TypeCode kind of a descriptor
  static inline
  CORBA::ULong descriptorToTK(PyObject* d_o)
  {
#if (PY_VERSION_HEX < 0x03000000)
    if (PyInt_Check(d_o))
      return PyInt_AS_LONG(d_o);
    else
      return PyInt_AS_LONG(PyTuple_GET_ITEM(d_o, 0));
#else
    if (PyLong_Check(d_o))
      return PyLong_AsLong(d_o);
    else
      return PyLong_AsLong(PyTuple_GET_ITEM(d_o, 0));
#endif
  }

  // Validate that the argument has the type specified by the
  // descriptor. If it has not, throw CORBA::BAD_PARAM with the given
  // completion status.
  //
  typedef void (*ValidateTypeFn)(PyObject*, PyObject*,
				 CORBA::CompletionStatus,
				 PyObject*);

  static const ValidateTypeFn validateTypeFns[];

  static void validateTypeIndirect(PyObject* d_o, PyObject* a_o,
				   CORBA::CompletionStatus compstatus,
				   PyObject* track);

  static inline
  void validateType(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus,
		    PyObject* track = 0)
  {
    CORBA::ULong tk = descriptorToTK(d_o);

    if (tk <= 33) { // tk_local_interface
      validateTypeFns[tk](d_o, a_o, compstatus, track);
    }
    else if (tk == 0xffffffff) { // Indirection
      validateTypeIndirect(d_o, a_o, compstatus, track);
    }
    else OMNIORB_THROW(BAD_TYPECODE, BAD_TYPECODE_UnknownKind, compstatus);
  }

  // Marshal the given argument object a_o, which has the type
  // specified by d_o. This function MUST NOT be called without having
  // first called validateType() with the same arguments, since it
  // performs no argument type checking.
  //
  typedef void (*MarshalPyObjectFn)(cdrStream& stream, PyObject*, PyObject*);

  static const MarshalPyObjectFn marshalPyObjectFns[];

  static void marshalPyObjectIndirect(cdrStream& stream,
				      PyObject* d_o, PyObject* a_o);

  static inline
  void marshalPyObject(cdrStream& stream, PyObject* d_o, PyObject* a_o)
  {
    CORBA::ULong tk = descriptorToTK(d_o);

    if (tk <= 33) { // tk_local_interface
      marshalPyObjectFns[tk](stream, d_o, a_o);
    }
    else if (tk == 0xffffffff) { // Indirection
      marshalPyObjectIndirect(stream, d_o, a_o);
    }
    else OMNIORB_ASSERT(0);
  }

  // Unmarshal a PyObject, which has the type specified by d_o.
  //
  typedef PyObject* (*UnmarshalPyObjectFn)(cdrStream& stream, PyObject*);

  static const UnmarshalPyObjectFn unmarshalPyObjectFns[];

  static PyObject* unmarshalPyObjectIndirect(cdrStream& stream, PyObject* d_o);

  static inline
  PyObject* unmarshalPyObject(cdrStream& stream, PyObject* d_o)
  {
    CORBA::ULong tk = descriptorToTK(d_o);

    if (tk <= 33) { // tk_local_interface
      PyObject* r = unmarshalPyObjectFns[tk](stream, d_o);
      if (!r) handlePythonException();
      return r;
    }
    else if (tk == 0xffffffff) { // Indirection
      return unmarshalPyObjectIndirect(stream, d_o);
    }
    else OMNIORB_THROW(BAD_TYPECODE,
		       BAD_TYPECODE_UnknownKind,
		       (CORBA::CompletionStatus)stream.completion());
    return 0;
  }

  // Take a descriptor and an argument object, and return a "copy" of
  // the argument. Immutable types need not be copied. If the argument
  // does not match the descriptor, throws BAD_PARAM.
  //
  typedef PyObject* (*CopyArgumentFn)(PyObject*, PyObject*,
				      CORBA::CompletionStatus);

  static const CopyArgumentFn copyArgumentFns[];

  static PyObject* copyArgumentIndirect(PyObject* d_o, PyObject* a_o,
					CORBA::CompletionStatus compstatus);

  static inline
  PyObject* copyArgument(PyObject* d_o, PyObject* a_o,
			 CORBA::CompletionStatus compstatus)
  {
    CORBA::ULong tk = descriptorToTK(d_o);

    if (tk <= 33) { // tk_local_interface
      PyObject* r = copyArgumentFns[tk](d_o, a_o, compstatus);
      if (!r) handlePythonException();
      return r;
    }
    else if (tk == 0xffffffff) { // Indirection
      return copyArgumentIndirect(d_o, a_o, compstatus);
    }
    else OMNIORB_THROW(BAD_TYPECODE, BAD_TYPECODE_UnknownKind, compstatus);
    return 0; // For dumb compilers
  }

  static inline
  void marshalRawPyString(cdrStream& stream, PyObject* pystring)
  {
    CORBA::ULong slen;
    const char*  str = String_AS_STRING_AND_SIZE(pystring, slen);

    ++slen;
    slen >>= stream;

    stream.put_small_octet_array((const CORBA::Octet*)str, slen);
  }


  static inline PyObject*
  unmarshalRawPyString(cdrStream& stream, CORBA::ULong len)
  {
    if (!stream.checkInputOverrun(1, len))
      OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		    (CORBA::CompletionStatus)stream.completion());

    PyObject* pystring;

#if (PY_VERSION_HEX < 0x03000000)
    pystring = PyString_FromStringAndSize(0, len - 1);
    stream.get_octet_array((_CORBA_Octet*)PyString_AS_STRING(pystring), len);

#else
    const char* data = (const char*)stream.inData(len);
    if (data) {
      // Read data directly from stream's buffer
      pystring = PyUnicode_FromStringAndSize((const char*)data, len-1);
    }
    else {
      char* buf = new char[len];
      stream.get_octet_array((_CORBA_Octet*)buf, len);
      pystring = PyUnicode_FromStringAndSize((const char*)buf, len-1);
      delete [] buf;
    }
#endif
    return pystring;
  }

  static inline PyObject*
  unmarshalRawPyString(cdrStream& stream)
  {
    CORBA::ULong len; len <<= stream;
    return unmarshalRawPyString(stream, len);
  }


  ////////////////////////////////////////////////////////////////////////////
  // Valuetype / abstract interface marshalling functions                   //
  ////////////////////////////////////////////////////////////////////////////

  static void
  validateTypeValue(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus,
		    PyObject* track);

  static void
  validateTypeValueBox(PyObject* d_o, PyObject* a_o,
		       CORBA::CompletionStatus compstatus,
		       PyObject* track);

  static void
  validateTypeAbstractInterface(PyObject* d_o, PyObject* a_o,
				CORBA::CompletionStatus compstatus,
				PyObject* track);

  static void
  marshalPyObjectValue(cdrStream& stream, PyObject* d_o, PyObject* a_o);

  static void
  marshalPyObjectValueBox(cdrStream& stream, PyObject* d_o, PyObject* a_o);

  static void
  marshalPyObjectAbstractInterface(cdrStream& stream,
				   PyObject* d_o, PyObject* a_o);

  static PyObject*
  unmarshalPyObjectValue(cdrStream& stream, PyObject* d_o);
  // Shared by Value and ValueBox

  static PyObject*
  unmarshalPyObjectAbstractInterface(cdrStream& stream, PyObject* d_o);

  static PyObject*
  copyArgumentValue(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus);

  static PyObject*
  copyArgumentValueBox(PyObject* d_o, PyObject* a_o,
		       CORBA::CompletionStatus compstatus);

  static PyObject*
  copyArgumentAbstractInterface(PyObject* d_o, PyObject* a_o,
				CORBA::CompletionStatus compstatus);


  ////////////////////////////////////////////////////////////////////////////
  // TypeCode and Any support functions                                     //
  ////////////////////////////////////////////////////////////////////////////

  // Marshal a type descriptor as a TypeCode:
  static
  void marshalTypeCode(cdrStream& stream, PyObject* d_o);

  // Unmarshal a TypeCode, returning a descriptor:
  static
  PyObject* unmarshalTypeCode(cdrStream& stream);


  ////////////////////////////////////////////////////////////////////////////
  // Context support functions                                              //
  ////////////////////////////////////////////////////////////////////////////

  // Validate a Context object.
  static
  void validateContext(PyObject* c_o, CORBA::CompletionStatus compstatus);

  // Marshal context c_o, filtered according to pattern list p_o.
  static
  void marshalContext(cdrStream& stream, PyObject* p_o, PyObject* c_o);

  // Unmarshal context. Trust the sender to correctly filter.
  static
  PyObject* unmarshalContext(cdrStream& stream);

  // Filter context c_o according to pattern list p_o. Returns a new Context.
  static
  PyObject* filterContext(PyObject* p_o, PyObject* c_o);


  ////////////////////////////////////////////////////////////////////////////
  // Interceptor functions                                                  //
  ////////////////////////////////////////////////////////////////////////////

  // Register ORB interceptors if need be
  static
  void registerInterceptors();


  ////////////////////////////////////////////////////////////////////////////
  // Proxy call descriptor object                                           //
  ////////////////////////////////////////////////////////////////////////////

  static
  void Py_localCallBackFunction(omniCallDescriptor* cd, omniServant* svnt);


  class Py_omniCallDescriptor : public omniAsyncCallDescriptor {
  public:

    struct InvokeArgs {
      const char*    op;
      int            op_len;
      CORBA::Boolean oneway;
      PyObject*      in_d;
      PyObject*      out_d;
      PyObject*      exc_d;
      PyObject*      ctxt_d;
      PyObject*      args;
      PyObject*      excep_name;
      PyObject*      callback;
      CORBA::Boolean contains_values;
      omniObjRef*    oobjref;

      inline CORBA::Boolean error() { return args == 0; }

      inline InvokeArgs(CORBA::Object_ptr cxxobjref, PyObject* pyargs)
      {
        PyObject*    op_str;
        PyObject*    desc;
        CORBA::ULong len;

        op_str = PyTuple_GET_ITEM(pyargs, 0);
        op     = String_AS_STRING_AND_SIZE(op_str, len);

        op_len = len + 1;

        desc   = PyTuple_GET_ITEM(pyargs, 1);
        in_d   = PyTuple_GET_ITEM(desc, 0);
        out_d  = PyTuple_GET_ITEM(desc, 1);
        exc_d  = PyTuple_GET_ITEM(desc, 2);
        oneway = (out_d == Py_None);

        OMNIORB_ASSERT(PyTuple_Check(in_d));
        OMNIORB_ASSERT(out_d == Py_None || PyTuple_Check(out_d));
        OMNIORB_ASSERT(exc_d == Py_None || PyDict_Check(exc_d));

        int desclen = PyTuple_GET_SIZE(desc);

        if (desclen >= 4) {
          ctxt_d = PyTuple_GET_ITEM(desc, 3);
          if (ctxt_d == Py_None) {
            ctxt_d = 0;
          }
          else {
            OMNIORB_ASSERT(PyList_Check(ctxt_d));
          }
        }
        else
          ctxt_d = 0;

        contains_values = 0;

        if (desclen == 5) {
          PyObject* v = PyTuple_GET_ITEM(desc, 4);
          if (v != Py_None)
            contains_values = 1;
        }

        args = PyTuple_GET_ITEM(pyargs, 2);

        OMNIORB_ASSERT(PyTuple_Check(args));

        int arg_len = PyTuple_GET_SIZE(in_d) + (ctxt_d ? 1:0);

        if (PyTuple_GET_SIZE(args) != arg_len) {
          char* err = new char[80];
          sprintf(err, "Operation requires %d argument%s; %d given",
                  arg_len, (arg_len == 1) ? "" : "s",
                  (int)PyTuple_GET_SIZE(args));

          PyErr_SetString(PyExc_TypeError, err);
          delete [] err;
          args = 0;
          return;
        }

        // AMI callback excep method name
        if (PyTuple_GET_SIZE(pyargs) > 3)
          excep_name = PyTuple_GET_ITEM(pyargs, 3);
        else
          excep_name = 0;

        // AMI callback object
        if (PyTuple_GET_SIZE(pyargs) > 4)
          callback = PyTuple_GET_ITEM(pyargs, 4);
        else
          callback = 0;

        oobjref = cxxobjref->_PR_getobj();
      }
    };

    // Synchronous call
    inline Py_omniCallDescriptor(InvokeArgs& a)
      : omniAsyncCallDescriptor(Py_localCallBackFunction,
                                a.op, a.op_len, a.oneway, 0, 0, 0),
        in_d_      (a.in_d,   1),
        out_d_     (a.out_d,  1),
        exc_d_     (a.exc_d,  1),
        ctxt_d_    (a.ctxt_d, 1),
        args_      (a.args,   1),
        result_    (0),
        excep_name_(0),
        callback_  (0),
        poller_    (0),
        unlocker_  (0),
        in_marshal_(0)
    {
      init();
    }

    // Asynchronous call
    inline Py_omniCallDescriptor(InvokeArgs& a, CORBA::Boolean need_poller)
      : omniAsyncCallDescriptor(Py_localCallBackFunction,
                                a.op, a.op_len, a.oneway, 0, 0),
        in_d_      (a.in_d,   1),
        out_d_     (a.out_d,  1),
        exc_d_     (a.exc_d,  1),
        ctxt_d_    (a.ctxt_d, 1),
        args_      (a.args,   1),
        result_    (0),
        excep_name_(a.excep_name, 1),
        callback_  (a.callback,   1),
        poller_    (need_poller ? makePoller() : 0),
        unlocker_  (0),
        in_marshal_(0)
    {
      init();
    }

    // Upcall
    inline Py_omniCallDescriptor(const char*    op, int op_len,
				 CORBA::Boolean oneway,
				 PyObject*      in_d,
                                 PyObject*      out_d,
				 PyObject*      exc_d,
                                 PyObject*      ctxt_d)

      : omniAsyncCallDescriptor(Py_localCallBackFunction, op, op_len,
                                oneway, 0, 0, 1),
        in_d_      (in_d,   1),
        out_d_     (out_d,  1),
        exc_d_     (exc_d,  1),
        ctxt_d_    (ctxt_d, 1),
        args_      (0),
        result_    (0),
        excep_name_(0),
        callback_  (0),
        poller_    (0),
        unlocker_  (0),
        in_marshal_(0)
    {
      init();
    }

    virtual ~Py_omniCallDescriptor();

    inline void unlocker(InterpreterUnlocker* ul) { unlocker_ = ul; }
    inline InterpreterUnlocker* unlocker()        { return unlocker_; }

    inline PyObject* args()   { return args_.obj(); }
    inline PyObject* in_d()   { return in_d_.obj(); }
    inline PyObject* out_d()  { return out_d_.obj(); }
    inline PyObject* exc_d()  { return exc_d_.obj(); }

    inline PyObject* result() { return result_.retn(); }
    // Extract and take ownership of stored results

    inline void setDescriptors(PyObject*& in_d,  int& in_l,
                               PyObject*& out_d, int& out_l,
                               PyObject*& exc_d,
                               PyObject*& ctxt_d)
    {
      in_d   = in_d_.obj();
      in_l   = in_l_;
      out_d  = out_d_.obj();
      out_l  = out_l_;
      exc_d  = exc_d_.obj();
      ctxt_d = ctxt_d_.obj();
    }


    //
    // Client side methods

    virtual void initialiseCall(cdrStream&);
    virtual void marshalArguments(cdrStream& stream);
    virtual void unmarshalReturnedValues(cdrStream& stream);
    virtual void userException(cdrStream& stream, _OMNI_NS(IOP_C)* iop_client,
			       const char* repoId);


    //
    // AMI

    virtual void completeCallback();

    PyObject* raisePyException();
    // Raise a Python exception corresponding to the exception held in
    // pd_exception.

    inline PyObject* callback()
    {
      if (callback_.valid()) {
        return callback_.dup();
      }
      else {
        Py_INCREF(Py_None);
        return Py_None;
      }
    }

    inline void callback(PyObject* cb)
    {
      if (cb != Py_None) {
        Py_INCREF(cb);
        callback_ = cb;
      }
      else
        callback_ = 0;
    }

    inline PyObject* poller()
    {
      if (poller_.valid()) {
        return poller_.dup();
      }
      else {
        Py_INCREF(Py_None);
        return Py_None;
      }
    }


    //
    // Server side methods

    virtual void unmarshalArguments(cdrStream& stream);

    // Throws BAD_PARAM if result is bad. _Always_ consumes result.
    void setAndValidateReturnedValues(PyObject* result);

    // Simply set the returned values
    void setReturnedValues(PyObject* result) { result_ = result; }

    // Marshal the returned values, and release the stored result
    virtual void marshalReturnedValues(cdrStream& stream);

  private:
    PyRefHolder          in_d_;
    int                  in_l_;
    PyRefHolder          out_d_;
    int                  out_l_;
    PyRefHolder          exc_d_;
    PyRefHolder          ctxt_d_;

    PyRefHolder          args_;
    PyRefHolder          result_;
    PyRefHolder          excep_name_;
    PyRefHolder          callback_;
    PyRefHolder          poller_;

    InterpreterUnlocker* unlocker_;
    CORBA::Boolean       in_marshal_;

    inline void init()
    {
      in_l_  = PyTuple_GET_SIZE(in_d_);
      out_l_ = is_oneway() ? -1 : PyTuple_GET_SIZE(out_d_);
    }

    PyObject* makePoller();

    // Not implemented
    Py_omniCallDescriptor(const Py_omniCallDescriptor&);
    Py_omniCallDescriptor& operator=(const Py_omniCallDescriptor&);
  };

  class CDInterpreterUnlocker : public InterpreterUnlocker {
  public:
    inline CDInterpreterUnlocker(Py_omniCallDescriptor& cd)
      : InterpreterUnlocker(), cd_(&cd)
    {
      cd_->unlocker(this);
    }
    inline ~CDInterpreterUnlocker()
    {
      cd_->unlocker(0);
    }
  private:
    Py_omniCallDescriptor* cd_;
  };


  ////////////////////////////////////////////////////////////////////////////
  // Servant object                                                         //
  ////////////////////////////////////////////////////////////////////////////

  class Py_omniServant : public virtual PortableServer::ServantBase {

  public:

    Py_omniServant(PyObject* pyservant, PyObject* opdict, const char* repoId);

    virtual ~Py_omniServant();

    virtual CORBA::Boolean _dispatch(omniCallHandle& handle);

    void remote_dispatch(Py_omniCallDescriptor* pycd);
    void local_dispatch (Py_omniCallDescriptor* pycd);

    PyObject* py_this();

    virtual void*                   _ptrToInterface(const char* repoId);
    virtual const char*             _mostDerivedRepoId();
    virtual CORBA::Boolean          _is_a(const char* logical_type_id);
    virtual PortableServer::POA_ptr _default_POA();
    virtual CORBA::Boolean          _non_existent();

    inline PyObject* pyServant() { Py_INCREF(pyservant_); return pyservant_; }

    // _add_ref and _remove_ref lock the Python interpreter lock
    // _locked versions assume the interpreter lock is already locked
    virtual void                    _add_ref();
    virtual void                    _remove_ref();
    void                            _locked_add_ref();
    void                            _locked_remove_ref();

  private:
    PyObject* pyservant_;	// Python servant object
    PyObject* opdict_;		// Operation descriptor dictionary
    PyObject* pyskeleton_;	// Skeleton class object
    char*     repoId_;
    int       refcount_;

    // Not implemented:
    Py_omniServant(const Py_omniServant&);
    Py_omniServant& operator=(const Py_omniServant&);
  };

  class PYOSReleaseHelper {
  public:
    PYOSReleaseHelper(Py_omniServant* pyos) : pyos_(pyos) {}
    ~PYOSReleaseHelper() {
      pyos_->_locked_remove_ref();
    }
  private:
    Py_omniServant* pyos_;
  };

  // Function to find or create a Py_omniServant object for a Python
  // servant object. If the Python object is not an instance of a
  // class derived from PortableServer.Servant, returns 0.
  //
  // Caller must hold the Python interpreter lock.
  static Py_omniServant* getServantForPyObject(PyObject* pyservant);


  ////////////////////////////////////////////////////////////////////////////
  // ServantManager / AdapterActivator implementations                      //
  ////////////////////////////////////////////////////////////////////////////

  class Py_ServantActivator
  {
  public:
    Py_ServantActivator(PyObject* pysa) : pysa_(pysa) { Py_INCREF(pysa_); }
    ~Py_ServantActivator() { Py_DECREF(pysa_); }

    PortableServer::Servant incarnate(const PortableServer::ObjectId& oid,
				      PortableServer::POA_ptr         poa);

    void etherealize(const PortableServer::ObjectId& oid,
		     PortableServer::POA_ptr         poa,
		     PortableServer::Servant         serv,
		     CORBA::Boolean                  cleanup_in_progress,
		     CORBA::Boolean                  remaining_activations);

    inline PyObject* pyobj() { return pysa_; }

  private:
    PyObject* pysa_;

    // Not implemented
    Py_ServantActivator(const Py_ServantActivator&);
    Py_ServantActivator& operator=(const Py_ServantActivator&);
  };

  class Py_ServantLocator
  {
  public:
    Py_ServantLocator(PyObject* pysl) : pysl_(pysl) { Py_INCREF(pysl_); }
    ~Py_ServantLocator() { Py_DECREF(pysl_); }

    PortableServer::Servant preinvoke(const PortableServer::ObjectId& oid,
				      PortableServer::POA_ptr poa,
				      const char*             operation,
				      void*&                  cookie);

    void postinvoke(const PortableServer::ObjectId& oid,
		    PortableServer::POA_ptr         poa,
		    const char*                     operation,
		    void*                           cookie,
		    PortableServer::Servant         serv);

    inline PyObject* pyobj() { return pysl_; }

  private:
    PyObject* pysl_;

    // Not implemented
    Py_ServantLocator(const Py_ServantLocator&);
    Py_ServantLocator& operator=(const Py_ServantLocator&);
  };

  class Py_AdapterActivator
  {
  public:
    Py_AdapterActivator(PyObject* pyaa) : pyaa_(pyaa) { Py_INCREF(pyaa_); }
    ~Py_AdapterActivator() { Py_DECREF(pyaa_); }

    CORBA::Boolean unknown_adapter(PortableServer::POA_ptr parent,
				   const char*             name);

    inline PyObject* pyobj() { return pyaa_; }

  private:
    PyObject* pyaa_;

    // Not implemented
    Py_AdapterActivator(const Py_AdapterActivator&);
    Py_AdapterActivator& operator=(const Py_AdapterActivator&);
  };

  // Function to create a C++ local object for a Python object. If the
  // Python object is not an instance of a mapped local object,
  // returns 0.
  //
  // Caller must hold the Python interpreter lock.
  static CORBA::LocalObject_ptr getLocalObjectForPyObject(PyObject* pyobj);

  // Convert a LocalObject to the underlying Python object. If the
  // object is not a suitable Python LocalObject, throw INV_OBJREF.
  //
  // Caller must hold the Python interpreter lock.
  static PyObject* getPyObjectForLocalObject(CORBA::LocalObject_ptr lobj);


  ////////////////////////////////////////////////////////////////////////////
  // PyUserException is a special CORBA::UserException                      //
  ////////////////////////////////////////////////////////////////////////////

  class PyUserException : public CORBA::UserException {
  public:
    // Constructor used during unmarshalling
    PyUserException(PyObject* desc);

    // Constructor used during marshalling. Throws BAD_PARAM with the
    // given completion status if the exception object doesn't match
    // the descriptor.
    // Always consumes reference to exc.
    PyUserException(PyObject* desc, PyObject* exc,
		    CORBA::CompletionStatus comp_status);

    // Copy constructor
    PyUserException(const PyUserException& e);

    virtual ~PyUserException();

    // Set the Python exception state to the contents of this exception.
    // Caller must hold the Python interpreter lock.
    // Returns 0 so callers can do "return ex.setPyExceptionState()".
    PyObject* setPyExceptionState();

    // DECREF the contained Python exception object. Caller must hold
    // the Python interpreter lock.
    void decrefPyException();

    // Marshalling operators for exception body, not including
    // repository id:

    // Caller must not hold interpreter lock
    void operator>>=(cdrStream& stream) const;

    // Caller must hold interpreter lock
    void operator<<=(cdrStream& stream);

    // Inherited virtual functions
    virtual void              _raise() const;
    virtual const char*       _NP_repoId(int* size)          const;
    virtual void              _NP_marshal(cdrStream& stream) const;
    virtual CORBA::Exception* _NP_duplicate()         	     const;
    virtual const char*       _NP_typeId()            	     const;

    static const char* _PD_typeId;

    static PyUserException* _downcast(CORBA::Exception* ex);
    static const PyUserException* _downcast(const CORBA::Exception* ex);

  private:
    PyObject* 	   desc_;          // Descriptor tuple
    PyObject* 	   exc_;           // The exception object
    CORBA::Boolean decref_on_del_; // True if exc_ should be DECREF'd when
				   // this object is deleted.
  };


  ////////////////////////////////////////////////////////////////////////////
  // ValueTrackerClearer safely clears a ValueTracker                       //
  ////////////////////////////////////////////////////////////////////////////

  class ValueTrackerClearer {
  public:
    inline ValueTrackerClearer(cdrStream& s) : s_(s) {}
    inline ~ValueTrackerClearer() {
      if (s_.valueTracker()) {
        InterpreterUnlocker u;
        s_.clearValueTracker();
      }
    };
  private:
    cdrStream& s_;
  };


  ////////////////////////////////////////////////////////////////////////////
  // PyUnlockingCdrStream unlocks the interpreter lock around blocking calls//
  ////////////////////////////////////////////////////////////////////////////

  class PyUnlockingCdrStream : public cdrStreamAdapter {
  public:
    PyUnlockingCdrStream(cdrStream& stream)
      : cdrStreamAdapter(stream)
    {
    }

    ~PyUnlockingCdrStream() { }

    // Override virtual functions in cdrStreamAdapter
    void put_octet_array(const _CORBA_Octet* b, int size,
			 omni::alignment_t align=omni::ALIGN_1);
    void get_octet_array(_CORBA_Octet* b,int size,
			 omni::alignment_t align=omni::ALIGN_1);
    void skipInput(_CORBA_ULong size);

    void copy_to(cdrStream&, int size, omni::alignment_t align=omni::ALIGN_1);

    void fetchInputData(omni::alignment_t,size_t);
    _CORBA_Boolean reserveOutputSpaceForPrimitiveType(omni::alignment_t align,
						      size_t required);
    _CORBA_Boolean maybeReserveOutputSpace(omni::alignment_t align,
					   size_t required);
  };

};

#ifdef HAS_Cplusplus_catch_exception_by_base

#define OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS \
catch (Py_BAD_PARAM& ex) { \
  return omniPy::handleSystemException(ex, ex.getInfo()); \
} \
catch (const CORBA::SystemException& ex) { \
  return omniPy::handleSystemException(ex); \
}
#else

#define OMNIPY_CATCH_AND_HANDLE_SPECIFIED_EXCEPTION(exc) \
catch (const CORBA::exc& ex) { \
  return omniPy::handleSystemException(ex); \
}
#define OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS \
catch (Py_BAD_PARAM& ex) { \
  return omniPy::handleSystemException(ex, ex.getInfo()); \
} \
OMNIORB_FOR_EACH_SYS_EXCEPTION(OMNIPY_CATCH_AND_HANDLE_SPECIFIED_EXCEPTION)

#endif


#endif // _omnipy_h_
