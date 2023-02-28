// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// omnipy.cc                  Created on: 1999/06/01
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2014 Apasphere Ltd
//    Copyright (C) 1999 AT&T Laboratories Cambridge
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
//    Main entry points for _omnipy Python module

#ifdef __WIN32__
#define DLL_EXPORT _declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#include "pydistdate.hh"
#include <omnipy.h>
#include <initialiser.h>

OMNI_USING_NAMESPACE(omni)

////////////////////////////////////////////////////////////////////////////
// Global pointers to Python objects                                      //
////////////////////////////////////////////////////////////////////////////

PyInterpreterState* omniPy::pyInterpreter;

PyObject* omniPy::py_omnipymodule;	// The _omnipy extension
PyObject* omniPy::py_pseudoFns;         //  pseudoFns
PyObject* omniPy::py_policyFns;         //  policyFns
PyObject* omniPy::pyCORBAmodule;	// The CORBA module
PyObject* omniPy::pyCORBAsysExcMap;	//  The system exception map
PyObject* omniPy::pyCORBAORBClass;	//  ORB class
PyObject* omniPy::pyCORBAAnyClass;	//  Any class
PyObject* omniPy::pyCORBATypeCodeClass;	//  TypeCode class
PyObject* omniPy::pyCORBAContextClass;	//  Context class
PyObject* omniPy::pyCORBAValueBase;    	//  ValueBase class
PyObject* omniPy::pyCORBAValueBaseDesc;	//  ValueBase descriptor
PyObject* omniPy::pyomniORBmodule;	// The omniORB module
PyObject* omniPy::pyomniORBobjrefMap;	//  The objref class map
PyObject* omniPy::pyomniORBskeletonMap;	//  The skeleton class map
PyObject* omniPy::pyomniORBtypeMap;     //  The repoId to descriptor mapping
PyObject* omniPy::pyomniORBvalueMap;    //  The repoId to value factory mapping
PyObject* omniPy::pyomniORBwordMap;     //  Reserved word map
PyObject* omniPy::pyomniORBUnknownValueBase;
                                        //  Base class for unknown valuetypes
PyObject* omniPy::pyPortableServerModule;
                                        // Portable server module
PyObject* omniPy::pyPOAClass;           //  POA class
PyObject* omniPy::pyPOAManagerClass;    //  POAManager class
PyObject* omniPy::pyPOACurrentClass;    //  Current class
PyObject* omniPy::pyServantClass;       //  Servant class
PyObject* omniPy::pyCreateTypeCode;	// Function to create a TypeCode object
PyObject* omniPy::pyWorkerThreadClass;  // Worker thread class
PyObject* omniPy::pyEmptyTuple;         // Zero element tuple.

PyObject* omniPy::pyservantAttr;
PyObject* omniPy::pyobjAttr;
PyObject* omniPy::pyNP_RepositoryId;


////////////////////////////////////////////////////////////////////////////
// Pointer to the ORB                                                     //
////////////////////////////////////////////////////////////////////////////

CORBA::ORB_ptr omniPy::orb;


////////////////////////////////////////////////////////////////////////////
// Code sets                                                              //
////////////////////////////////////////////////////////////////////////////

omniCodeSet::NCS_C* omniPy::ncs_c_utf_8;


////////////////////////////////////////////////////////////////////////////
// Constant strings to facilitate comparison by pointer                   //
////////////////////////////////////////////////////////////////////////////

const char* omniPy::string_Py_omniObjRef       = "Py_omniObjRef";
const char* omniPy::string_Py_omniServant      = "Py_omniServant";
const char* omniPy::string_Py_ServantActivator = "Py_ServantActivator";
const char* omniPy::string_Py_ServantLocator   = "Py_ServantLocator";
const char* omniPy::string_Py_AdapterActivator = "Py_AdapterActivator";


////////////////////////////////////////////////////////////////////////////
// Module inititaliser to hook orb creation and destruction               //
////////////////////////////////////////////////////////////////////////////

class omni_python_initialiser : public omniInitialiser {
public:
  void attach() {
    omniPy::registerInterceptors();
  }
  void detach() {
    omnipyThreadCache::shutdown();
    if (omniPy::orb) omniPy::orb = 0;
  }
};

static omni_python_initialiser the_omni_python_initialiser;


// Function to generate a list of system exception names
static PyObject*
generateExceptionList()
{
  int i = 0;
# define INCREMENT_COUNT(exc) i++;
  OMNIORB_FOR_EACH_SYS_EXCEPTION(INCREMENT_COUNT)
# undef INCREMENT_COUNT

  PyObject* excs = PyList_New(i);
  i = 0;

# define ADD_EXCEPTION_NAME(exc) \
  PyList_SetItem(excs, i++, String_FromString(#exc));
  OMNIORB_FOR_EACH_SYS_EXCEPTION(ADD_EXCEPTION_NAME)
# undef ADD_EXCEPTION_NAME

  return excs;
}


////////////////////////////////////////////////////////////////////////////
// Convenience functions                                                  //
////////////////////////////////////////////////////////////////////////////

PyObject*
omniPy::formatString(const char* fmt, const char* pyfmt, ...)
{
  PyObject* fmt_string = String_FromString(fmt);

  va_list va;
  va_start(va, pyfmt);
  PyObject* args = Py_VaBuildValue(pyfmt, va);
  va_end(va);

  PyObject* ret = String_Format(fmt_string, args);

  Py_DECREF(fmt_string);
  Py_DECREF(args);
  return ret;
}


// Things visible to Python:

extern "C" {

  ////////////////////////////////////////////////////////////////////////////
  // omnipy private functions                                               //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject*
  omnipy_checkVersion(PyObject* self, PyObject* args)
  {
    int   maj, min;
    char* mod;
    int   rev=0;

    if (!PyArg_ParseTuple(args, (char*)"iis|i", &maj, &min, &mod, &rev))
      return 0;

    if (!(maj == 4 && min == 2 && rev == 1)) {
      if (omniORB::trace(1)) {
	omniORB::logger l;
        l << "ERROR! omniORBpy version "
	  << OMNIPY_MAJOR << "." << OMNIPY_MINOR
          << " expects stubs version 4.2. "
	  << "Stubs in " << mod << " are version "
	  << maj << "." << min << " (rev " << rev << ").\n";
      }
      PyErr_SetString(PyExc_ImportError,
                      "Stubs not compatible with omniORBpy version 4.2.");
      return 0;
    }
    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  omnipy_coreVersion(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)"")) return 0;
    return Py_BuildValue((char*)"s", omniORB::versionString());
  }

#define OMNIPY_ATTR(x) \
  PyObject_GetAttrString(omniPy::pyomniORBmodule, (char*)x);

  static PyObject*
  omnipy_registerPyObjects(PyObject* self, PyObject* args)
  {
    PyObject* temp;

    // Get a pointer to the interpreter state
    PyThreadState* tstate = PyThreadState_Get();
    omniPy::pyInterpreter = tstate->interp;

    if (!PyArg_ParseTuple(args, (char*)"O", &omniPy::pyomniORBmodule))
      return 0;

    OMNIORB_ASSERT(PyModule_Check(omniPy::pyomniORBmodule));

    omniPy::pyCORBAmodule = OMNIPY_ATTR("CORBA");

    OMNIORB_ASSERT(omniPy::pyCORBAmodule &&
		   PyModule_Check(omniPy::pyCORBAmodule));

    omniPy::pyCORBAsysExcMap = OMNIPY_ATTR("sysExceptionMapping");

    omniPy::pyCORBAORBClass =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"ORB");

    omniPy::pyCORBAAnyClass =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"Any");

    omniPy::pyCORBATypeCodeClass =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"TypeCode");

    omniPy::pyCORBAContextClass =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"Context");

    omniPy::pyCORBAValueBase =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"ValueBase");

    omniPy::pyCORBAValueBaseDesc =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"_d_ValueBase");

    omniPy::pyomniORBobjrefMap        = OMNIPY_ATTR("objrefMapping");
    omniPy::pyomniORBtypeMap          = OMNIPY_ATTR("typeMapping");
    omniPy::pyomniORBwordMap          = OMNIPY_ATTR("keywordMapping");
    omniPy::pyPortableServerModule    = OMNIPY_ATTR("PortableServer");
    omniPy::pyomniORBskeletonMap      = OMNIPY_ATTR("skeletonMapping");
    omniPy::pyomniORBvalueMap         = OMNIPY_ATTR("valueFactoryMapping");
    omniPy::pyomniORBUnknownValueBase = OMNIPY_ATTR("UnknownValueBase");

    OMNIORB_ASSERT(omniPy::pyPortableServerModule);
    OMNIORB_ASSERT(PyModule_Check(omniPy::pyPortableServerModule));

    omniPy::pyPOAClass =
      PyObject_GetAttrString(omniPy::pyPortableServerModule, (char*)"POA");

    omniPy::pyPOAManagerClass =
      PyObject_GetAttrString(omniPy::pyPortableServerModule, (char*)"POAManager");

    omniPy::pyPOACurrentClass =
      PyObject_GetAttrString(omniPy::pyPortableServerModule, (char*)"Current");

    omniPy::pyServantClass =
      PyObject_GetAttrString(omniPy::pyPortableServerModule, (char*)"Servant");

    temp = OMNIPY_ATTR("tcInternal");

    omniPy::pyCreateTypeCode = PyObject_GetAttrString(temp,
						      (char*)"createTypeCode");

    omniPy::pyWorkerThreadClass = OMNIPY_ATTR("WorkerThread");

    omniPy::pyEmptyTuple = OMNIPY_ATTR("_emptyTuple");

    OMNIORB_ASSERT(omniPy::pyCORBAsysExcMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyCORBAsysExcMap));
    OMNIORB_ASSERT(omniPy::pyCORBAORBClass);
    OMNIORB_ASSERT(omniPy::pyCORBAAnyClass);
    OMNIORB_ASSERT(omniPy::pyCORBATypeCodeClass);
    OMNIORB_ASSERT(omniPy::pyCORBAContextClass);
    OMNIORB_ASSERT(omniPy::pyCORBAValueBaseDesc);
    OMNIORB_ASSERT(PyTuple_Check(omniPy::pyCORBAValueBaseDesc));
    OMNIORB_ASSERT(omniPy::pyCORBAValueBase);
    OMNIORB_ASSERT(omniPy::pyomniORBobjrefMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBobjrefMap));
    OMNIORB_ASSERT(omniPy::pyomniORBskeletonMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBskeletonMap));
    OMNIORB_ASSERT(omniPy::pyomniORBtypeMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBtypeMap));
    OMNIORB_ASSERT(omniPy::pyomniORBvalueMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBvalueMap));
    OMNIORB_ASSERT(omniPy::pyomniORBwordMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBwordMap));
    OMNIORB_ASSERT(omniPy::pyomniORBUnknownValueBase);
    OMNIORB_ASSERT(omniPy::pyPOAClass);
    OMNIORB_ASSERT(omniPy::pyPOACurrentClass);
    OMNIORB_ASSERT(omniPy::pyPOAManagerClass);
    OMNIORB_ASSERT(omniPy::pyServantClass);
    OMNIORB_ASSERT(omniPy::pyCreateTypeCode);
    OMNIORB_ASSERT(PyFunction_Check(omniPy::pyCreateTypeCode));
    OMNIORB_ASSERT(omniPy::pyWorkerThreadClass);
    OMNIORB_ASSERT(omniPy::pyEmptyTuple);
    OMNIORB_ASSERT(PyTuple_Check(omniPy::pyEmptyTuple));

    omniPy::pyservantAttr     = OMNIPY_ATTR("_servantAttr");
    omniPy::pyobjAttr         = OMNIPY_ATTR("_objAttr");
    omniPy::pyNP_RepositoryId = OMNIPY_ATTR("_NP_RepositoryId");

    OMNIORB_ASSERT(omniPy::pyservantAttr);
    OMNIORB_ASSERT(String_Check(omniPy::pyservantAttr));

    OMNIORB_ASSERT(omniPy::pyobjAttr);
    OMNIORB_ASSERT(String_Check(omniPy::pyobjAttr));

    OMNIORB_ASSERT(omniPy::pyNP_RepositoryId);
    OMNIORB_ASSERT(String_Check(omniPy::pyNP_RepositoryId));

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  omnipy_need_ORB_init(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)""))
      return 0;

    if (omniPy::orb)
      Py_RETURN_FALSE;
    else
      Py_RETURN_TRUE;
  }

  ////////////////////////////////////////////////////////////////////////////
  // CORBA:: functions                                                      //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject*
  omnipy_ORB_init(PyObject* self, PyObject* args)
  {
    PyObject* pyargv;
    char*     orbid;
    int       argc;
    char**    argv;

    OMNIORB_ASSERT(omniPy::orb == 0);

    if (!PyArg_ParseTuple(args, (char*)"Os", &pyargv, &orbid))
      return 0;

    if (!PyList_Check(pyargv)) {
      PyErr_SetString(PyExc_TypeError,
		      "argument 1: parameter must be an argument list");
      return 0;
    }

    argc = PyList_GET_SIZE(pyargv);
    argv = new char*[argc];

    PyObject* o;
    int i;
    for (i=0; i<argc; i++) {
      o = PyList_GET_ITEM(pyargv, i);
      if (!String_Check(o)) {
	PyErr_SetString(PyExc_TypeError,
			"argument 1: parameter must be a list of strings.");
	delete[] argv;
	return 0;
      }
      argv[i] = (char*)String_AsString(o);
    }

    int orig_argc = argc;

    CORBA::ORB_ptr orb;
    try {
      orb = CORBA::ORB_init(argc, argv, orbid);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    if (omniORB::trace(2)) {
      omniORB::logger l;
      l << "omniORBpy distribution date: " OMNIORBPY_DIST_DATE "\n";
    }

    omniPy::orb = orb;

    // Remove eaten arguments from Python argv list
    if (argc < orig_argc) {
      int r;
      char *s, *t;
      for (i=0; i<argc; ++i) {
	s = argv[i];

	while (1) {
	  o = PyList_GetItem(pyargv, i); OMNIORB_ASSERT(o != 0);
	  t = (char*)String_AsString(o);
	  if (s == t) break;
	  r = PySequence_DelItem(pyargv, i);
	  OMNIORB_ASSERT(r != -1);
	}
      }
      while (PyList_Size(pyargv) > argc) {
	// Delete -ORB arguments at end
	r = PySequence_DelItem(pyargv, i);
	OMNIORB_ASSERT(r != -1);
      }
    }
    delete [] argv;

    // Initialise the thread state cache
    omnipyThreadCache::init();

    return omniPy::createPyORBObject(orb);
  }

  ////////////////////////////////////////////////////////////////////////////
  // CDR stream marshalling/unmarshalling                                   //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject*
  omnipy_cdrMarshal(PyObject* self, PyObject* args)
  {
    PyObject *desc, *data;
    int endian = -1;

    if (!PyArg_ParseTuple(args, (char*)"OO|i", &desc, &data, &endian))
      return 0;

    if (endian > 1 || endian < -1) {
      PyErr_SetString(PyExc_ValueError,
		      "argument 3: endian must be 0 or 1");
      return 0;
    }

    try {
      omniPy::validateType(desc, data, CORBA::COMPLETED_NO);

      if (endian == -1) {
	// Marshal into an encapsulation
	cdrEncapsulationStream stream;
        omniPy::ValueTrackerClearer vtc(stream);

	omniPy::marshalPyObject(stream, desc, data);

	return RawString_FromStringAndSize((char*)stream.bufPtr(),
                                           stream.bufSize());
      }
      else {
	// Marshal into a raw buffer
	cdrMemoryStream stream;
        omniPy::ValueTrackerClearer vtc(stream);

	if (endian != omni::myByteOrder)
	  stream.setByteSwapFlag(endian);

	omniPy::marshalPyObject(stream, desc, data);

	return RawString_FromStringAndSize((char*)stream.bufPtr(),
                                           stream.bufSize());
      }
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static inline PyObject* do_cdrUnmarshal(cdrStream& stream, PyObject* desc)
  {
    PyObject* r = omniPy::unmarshalPyObject(stream, desc);
    if (r && stream.checkInputOverrun(1, 1)) {
      // More data in stream -- must have used the wrong TypeCode
      Py_DECREF(r);
      OMNIORB_THROW(MARSHAL, MARSHAL_MessageTooLong, CORBA::COMPLETED_NO);
    }
    return r;
  }

  static PyObject*
  omnipy_cdrUnmarshal(PyObject* self, PyObject* args)
  {
    PyObject*  desc;
    char*      encap;
    Py_ssize_t size;
    int        endian = -1;
    
#if (PY_VERSION_HEX < 0x03000000)

    if (!PyArg_ParseTuple(args, (char*)"Os#|i",
			  &desc, &encap, &size, &endian))
      return 0;

#else

    if (!PyArg_ParseTuple(args, (char*)"Oy#|i",
			  &desc, &encap, &size, &endian))
      return 0;

#endif

    if (endian > 1 || endian < -1) {
      PyErr_SetString(PyExc_ValueError,
		      "argument 3: endian must be 0 or 1");
      return 0;
    }

    try {
      if (endian == -1) {
	// Encapsulation
	cdrEncapsulationStream stream((CORBA::Octet*)encap, size);
        omniPy::ValueTrackerClearer vtc(stream);
	return do_cdrUnmarshal(stream, desc);
      }
      else {
	// Simple buffer. Is it aligned ok?
	if ((omni::ptr_arith_t)encap ==
	    omni::align_to((omni::ptr_arith_t)encap, omni::ALIGN_8)) {

	  cdrMemoryStream stream((CORBA::Octet*)encap, size);
          omniPy::ValueTrackerClearer vtc(stream);

	  if (endian != omni::myByteOrder)
	    stream.setByteSwapFlag(endian);

	  return do_cdrUnmarshal(stream, desc);
	}
	else {
	  // Unfortunately, this is a common case, due to the way
	  // Python string objects are laid out.
	  cdrMemoryStream stream;
          omniPy::ValueTrackerClearer vtc(stream);

	  if (endian != omni::myByteOrder)
	    stream.setByteSwapFlag(endian);

	  stream.put_octet_array((CORBA::Octet*)encap, size);
	  return do_cdrUnmarshal(stream, desc);
	}
      }
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }


  static PyObject*
  omnipy_ensureInit(PyObject* self, PyObject* args)
  {
    PyObject* m = PyImport_ImportModule((char*)"_omnipy");
    PyObject* o = PyObject_GetAttrString(m, (char*)"omni_func");
    PyObject* f = 0;
    
    if (o && PyModule_Check(o))
      f = PyObject_GetAttrString(o, (char*)"log");

    if (!o || !PyModule_Check(o) || !f || f == Py_None) {
      PyErr_Clear();

      omniORB::logs(5, "Reinitialise omniORBpy sub-modules.");
      PyObject* d = PyModule_GetDict(m);
      omniPy::initORBFunc(d);
      omniPy::initPOAFunc(d);
      omniPy::initPOAManagerFunc(d);
      omniPy::initPOACurrentFunc(d);
      omniPy::initInterceptorFunc(d);
      omniPy::initomniFunc(d);
    }
    Py_XDECREF(o);
    Py_XDECREF(f);

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  omnipy_servantThis(PyObject* self, PyObject* args)
  {
    PyObject* pyservant;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyservant)) return 0;

    omniPy::Py_omniServant* pyos = omniPy::getServantForPyObject(pyservant);
    RAISE_PY_BAD_PARAM_IF(!pyos, BAD_PARAM_WrongPythonType);

    omniPy::PYOSReleaseHelper _r(pyos);

    try {
      return pyos->py_this();
    }
    catch (PortableServer::POA::WrongPolicy& ex) {
      return omniPy::raiseScopedException(omniPy::pyPortableServerModule,
                                          "POA", "WrongPolicy");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }


  ////////////////////////////////////////////////////////////////////////////
  // Python method table                                                    //
  ////////////////////////////////////////////////////////////////////////////

  static PyMethodDef omnipy_methods[] = {

    // omnipy specific things:
    {(char*)"checkVersion",      omnipy_checkVersion,            METH_VARARGS},
    {(char*)"coreVersion",       omnipy_coreVersion,             METH_VARARGS},
    {(char*)"registerPyObjects", omnipy_registerPyObjects,       METH_VARARGS},
    {(char*)"cdrMarshal",        omnipy_cdrMarshal,              METH_VARARGS},
    {(char*)"cdrUnmarshal",      omnipy_cdrUnmarshal,            METH_VARARGS},
    {(char*)"need_ORB_init",     omnipy_need_ORB_init,           METH_VARARGS},
    {(char*)"ensureInit",        omnipy_ensureInit,              METH_VARARGS},

    // Wrappers for functions in CORBA::
    {(char*)"ORB_init",          omnipy_ORB_init,                METH_VARARGS},

    // Functions for servants
    {(char*)"servantThis",       omnipy_servantThis,             METH_VARARGS},

    {0,0}
  };

  static void initModule(PyObject* m)
  {
    PyObject* d = PyModule_GetDict(m);

    PyObject* ver = String_FromString(OMNIPY_VERSION_STRING);
    PyDict_SetItemString(d, (char*)"__version__", ver);
    Py_DECREF(ver);

    PyObject* excs = generateExceptionList();
    PyDict_SetItemString(d, (char*)"system_exceptions", excs);
    Py_DECREF(excs);

    omniPy::py_omnipymodule = m;
    omniPy::initObjRefFunc(d);
    omniPy::initORBFunc(d);
    omniPy::initPOAFunc(d);
    omniPy::initPOAManagerFunc(d);
    omniPy::initPOACurrentFunc(d);
    omniPy::initInterceptorFunc(d);
    omniPy::initomniFunc(d);
    omniPy::initFixed(d);
    omniPy::initCallDescriptor(d);
    omniPy::initServant(d);

    // Set up the C++ API singleton
#if (PY_VERSION_HEX < 0x03000000)
    PyObject* api = PyCObject_FromVoidPtr((void*)&omniPy::cxxAPI, 0);
#else
    PyObject* api = PyCapsule_New((void*)&omniPy::cxxAPI, "_omnipy.API", 0);
#endif
    PyDict_SetItemString(d, (char*)"API", api);
    Py_DECREF(api);

    // Empty list for external modules to register additional pseudo
    // object creation functions.
    omniPy::py_pseudoFns = PyList_New(0);
    PyDict_SetItemString(d, (char*)"pseudoFns", omniPy::py_pseudoFns);
    Py_DECREF(omniPy::py_pseudoFns);

    // Empty dict for external modules to register additional policy
    // object creation functions.
    omniPy::py_policyFns = PyDict_New();
    PyDict_SetItemString(d, (char*)"policyFns", omniPy::py_policyFns);
    Py_DECREF(omniPy::py_policyFns);

    // Codesets
    omniPy::ncs_c_utf_8 = omniCodeSet::getNCS_C("UTF-8");

    // omniORB module initialiser
    omniInitialiser::install(&the_omni_python_initialiser);
  }

#if (PY_VERSION_HEX < 0x03000000)

  void DLL_EXPORT init_omnipy()
  {
    // Make sure Python is running multi-threaded
    PyEval_InitThreads();

    PyObject* m = Py_InitModule((char*)"_omnipy", omnipy_methods);
    initModule(m);
  }

#else

  static struct PyModuleDef omnipymodule = {
    PyModuleDef_HEAD_INIT,
    "_omnipy",
    "omniORBpy",
    -1,
    omnipy_methods,
    NULL,
    NULL,
    NULL,
    NULL
  };

  PyMODINIT_FUNC
  PyInit__omnipy(void)
  {
    // Make sure Python is running multi-threaded
    PyEval_InitThreads();

    PyObject* m = PyModule_Create(&omnipymodule);
    if (!m)
      return 0;

    initModule(m);
    return m;
  }

#endif
}
