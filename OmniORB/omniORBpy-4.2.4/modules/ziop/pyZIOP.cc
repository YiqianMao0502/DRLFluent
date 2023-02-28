// -*- Mode: C++; -*-
//                            Package   : omniORB
// pyZIOP.cc                  Created on: 2013/05/24
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2013 Apasphere Ltd.
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
//    ZIOP support

#ifdef __WIN32__
#define DLL_EXPORT _declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#if defined(__VMS)
#include <Python.h>
#else
#include PYTHON_INCLUDE
#endif

#include <omniORB4/CORBA.h>
#include <omniORB4/omniZIOP.h>

#include <omniORBpy.h>
#include "../omnipy.h"


static omniORBpyAPI* api;


//
// Utilities

static CORBA::UShort
getUShort(PyObject* obj)
{
  long l = 0;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(obj)) {
    l = PyInt_AS_LONG(obj);
  }
  else
#endif
  if (PyLong_Check(obj)) {
    l = PyLong_AsLong(obj);
  }
  else {
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
  }

  if (l < 0 || l > 0xffff) {
    PyErr_Clear();
    OMNIORB_THROW(BAD_PARAM,
                  BAD_PARAM_PythonValueOutOfRange,
                  CORBA::COMPLETED_NO);
  }
  return l;
}


static CORBA::ULong
getULong(PyObject* obj)
{
  if (PyLong_Check(obj)) {
    unsigned long ul = PyLong_AsUnsignedLong(obj);
    if (ul == (unsigned long)-1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM,
                    BAD_PARAM_PythonValueOutOfRange,
                    CORBA::COMPLETED_NO);
    }
#if SIZEOF_LONG > 4
    if (ul > 0xffffffffL) {
      OMNIORB_THROW(BAD_PARAM,
                    BAD_PARAM_PythonValueOutOfRange,
                    CORBA::COMPLETED_NO);
    }
#endif
    return ul;
  }
#if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(obj)) {
    long l = PyInt_AS_LONG(obj);
#if SIZEOF_LONG > 4
    if (l < 0 || l > 0xffffffffL) {
      OMNIORB_THROW(BAD_PARAM,
                    BAD_PARAM_PythonValueOutOfRange,
                    CORBA::COMPLETED_NO);
    }
#else
    if (l < 0) {
      OMNIORB_THROW(BAD_PARAM,
                    BAD_PARAM_PythonValueOutOfRange,
                    CORBA::COMPLETED_NO);
    }
#endif
    return l;
  }
#endif
  else {
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
  }

  // Never reach here
  return 0;
}


static CORBA::Float
getFloat(PyObject* obj)
{
  double d = PyFloat_AsDouble(obj);

  if (PyErr_Occurred()) {
    PyErr_Clear();
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
  }
  return d;
}


//
// Policy conversion functions

static CORBA::Policy_ptr
convertCompressionEnablingPolicy(PyObject* pyval)
{
  return omniZIOP::create_compression_enabling_policy(PyObject_IsTrue(pyval));
}


static CORBA::Policy_ptr
convertCompressorIdLevelListPolicy(PyObject* pyval)
{
  Compression::CompressorIdLevelList cids;

  if (!PyList_Check(pyval))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
  
  cids.length(PyList_Size(pyval));

  for (CORBA::ULong idx=0; idx != cids.length(); ++idx) {
    PyObject* pycid = PyList_GetItem(pyval, idx);
    
    omniPy::PyRefHolder cid(PyObject_GetAttrString(pycid,
                                                   (char*)"compressor_id"));
    omniPy::PyRefHolder clv(PyObject_GetAttrString(pycid,
                                                   (char*)"compression_level"));

    if (!(cid.valid() && clv.valid())) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
    }
    cids[idx].compressor_id     = getUShort(cid);
    cids[idx].compression_level = getUShort(clv);
  }

  return omniZIOP::create_compression_id_level_list_policy(cids);
}


static CORBA::Policy_ptr
convertCompressionLowValuePolicy(PyObject* pyval)
{
  return omniZIOP::create_compression_low_value_policy(getULong(pyval));
}


static CORBA::Policy_ptr
convertCompressionMinRatioPolicy(PyObject* pyval)
{
  return omniZIOP::create_compression_min_ratio_policy(getFloat(pyval));
}


CORBA::PolicyList*
convertPolicies(PyObject* pyps)
{
  if (!PyList_Check(pyps))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);

  CORBA::ULong len = PyList_GET_SIZE(pyps);

  CORBA::PolicyList_var ps = new CORBA::PolicyList(len);
  ps->length(len);

  for (CORBA::ULong idx=0; idx != len; ++idx) {
    PyObject* pypolicy = PyList_GET_ITEM(pyps, idx);

    omniPy::PyRefHolder pyptype(PyObject_GetAttrString(pypolicy,
                                                       (char*)"_policy_type"));

    omniPy::PyRefHolder pyvalue(PyObject_GetAttrString(pypolicy,
                                                       (char*)"_value"));

    if (pyptype.valid() && pyvalue.valid()) {
      CORBA::ULong ptype = getULong(pyptype);

      if (ptype == ZIOP::COMPRESSION_ENABLING_POLICY_ID)
        ps[idx] = convertCompressionEnablingPolicy(pyvalue);

      else if (ptype == ZIOP::COMPRESSOR_ID_LEVEL_LIST_POLICY_ID)
        ps[idx] = convertCompressorIdLevelListPolicy(pyvalue);

      else if (ptype == ZIOP::COMPRESSION_LOW_VALUE_POLICY_ID)
        ps[idx] = convertCompressionLowValuePolicy(pyvalue);

      else if (ptype == ZIOP::COMPRESSION_MIN_RATIO_POLICY_ID)
        ps[idx] = convertCompressionMinRatioPolicy(pyvalue);

      else
        OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType,
                      CORBA::COMPLETED_NO);
    }
    else {
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType,
                    CORBA::COMPLETED_NO);
    }
  }
  return ps._retn();
}



static void
registerPolicyFn(PyObject*         policyFns,
                 CORBA::ULong      ptype,
                 omniORBpyPolicyFn fn)
{
#if (PY_VERSION_HEX < 0x03000000)
  omniPy::PyRefHolder pyptype(PyInt_FromLong(ptype));
  omniPy::PyRefHolder pyfn(PyCObject_FromVoidPtr((void*)fn, 0));
#else
  omniPy::PyRefHolder pyptype(PyLong_FromLong(ptype));
  omniPy::PyRefHolder pyfn(PyCapsule_New((void*)fn, 0, 0));
#endif

  PyDict_SetItem(policyFns, pyptype, pyfn);
}


extern "C" {

  static char setGlobalPolicies_doc[] =
  "setGlobalPolicies(policies)\n"
  "\n"
  "Sets global policies that apply to all POAs and all object references,\n"
  "equivalent to separately setting the policies in calls to the POAs and\n"
  "calling _set_policy_overrides() on all object references.\n"
  "\n"
  "There is no need to include a CompressionEnablingPolicy with value true,\n"
  "since calling this implicitly enables compression. If other policies are\n"
  "not set, default values are used, so to enable ZIOP with default settings,\n"
  "simply call setGlobalPolicies with empty policies.\n";

  static PyObject*
  pyZIOP_setGlobalPolicies(PyObject* self, PyObject* args)
  {
    PyObject* pyps;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyps))
      return 0;

    try {
      CORBA::PolicyList_var ps = convertPolicies(pyps);
      omniZIOP::setGlobalPolicies(ps);
    }
    OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    Py_INCREF(Py_None);
    return Py_None;
  }


  static char setServerPolicies_doc[] =
  "setServerPolicies(objref, policies)\n"
  "\n"
  "Returns a new object reference equivalent to obj, but with specified\n"
  "compression policies, as if the server-side ORB had set those policies.\n"
  "This is useful to enable ZIOP for an object reference constructed from a\n"
  "corbaloc URI.\n"
  "\n"
  "ZIOP is only valid for GIOP 1.2 (and later), so the object reference must\n"
  "be GIOP 1.2. For a corbaloc URI, specify it as\n"
  "\n"
  "  corbaloc::1.2@some.host.name/key\n"
  "\n"
  "Beware that this may cause the client to make ZIOP calls to a server that\n"
  "does not support ZIOP!\n";

  static PyObject*
  pyZIOP_setServerPolicies(PyObject* self, PyObject* args)
  {
    PyObject* pyobj;
    PyObject* pyps;

    if (!PyArg_ParseTuple(args, (char*)"OO", &pyobj, &pyps))
      return 0;

    try {
      CORBA::Object_var     obj = api->pyObjRefToCxxObjRef(pyobj, 1);
      CORBA::PolicyList_var ps  = convertPolicies(pyps);

      obj = omniZIOP::setServerPolicies(obj, ps);

      return api->cxxObjRefToPyObjRef(obj, 1);
    }
    OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }


  static inline void initModule(PyObject* m, PyObject* omnipy)
  {
    // Register policy creation functions
    omniPy::PyRefHolder policyFns(PyObject_GetAttrString(omnipy,
                                                         (char*)"policyFns"));
    if (!policyFns.valid())
      return;

    registerPolicyFn(policyFns,
                     ZIOP::COMPRESSION_ENABLING_POLICY_ID,
                     convertCompressionEnablingPolicy);

    registerPolicyFn(policyFns,
                     ZIOP::COMPRESSOR_ID_LEVEL_LIST_POLICY_ID,
                     convertCompressorIdLevelListPolicy);

    registerPolicyFn(policyFns,
                     ZIOP::COMPRESSION_LOW_VALUE_POLICY_ID,
                     convertCompressionLowValuePolicy);

    registerPolicyFn(policyFns,
                     ZIOP::COMPRESSION_MIN_RATIO_POLICY_ID,
                     convertCompressionMinRatioPolicy);
  }


  static PyMethodDef omniZIOP_methods[] = {
    {(char*)"setGlobalPolicies",
     pyZIOP_setGlobalPolicies, METH_VARARGS, setGlobalPolicies_doc},

    {(char*)"setServerPolicies",
     pyZIOP_setServerPolicies, METH_VARARGS, setServerPolicies_doc},

    {0,0}
  };

#if (PY_VERSION_HEX < 0x03000000)

  void DLL_EXPORT init_omniZIOP()
  {
    PyObject* m = Py_InitModule((char*)"_omniZIOP", omniZIOP_methods);

    // Get hold of the omniORBpy C++ API.
    PyObject* omnipy = PyImport_ImportModule((char*)"_omnipy");
    PyObject* pyapi  = PyObject_GetAttrString(omnipy, (char*)"API");
    api              = (omniORBpyAPI*)PyCObject_AsVoidPtr(pyapi);
    Py_DECREF(pyapi);

    initModule(m, omnipy);
  }

#else

  static struct PyModuleDef omniZIOPmodule = {
    PyModuleDef_HEAD_INIT,
    "_omniZIOP",
    "omniORBpy ZIOP support",
    -1,
    omniZIOP_methods,
    NULL,
    NULL,
    NULL,
    NULL
  };

  PyMODINIT_FUNC
  PyInit__omniZIOP(void)
  {
    PyObject* m = PyModule_Create(&omniZIOPmodule);
    if (!m)
      return 0;

    // Get hold of the omniORBpy C++ API.
    PyObject* omnipy = PyImport_ImportModule((char*)"_omnipy");
    PyObject* pyapi  = PyObject_GetAttrString(omnipy, (char*)"API");
    api              = (omniORBpyAPI*)PyCapsule_GetPointer(pyapi,
                                                           "_omnipy.API");
    Py_DECREF(pyapi);

    initModule(m, omnipy);
    return m;
  }

#endif
};
