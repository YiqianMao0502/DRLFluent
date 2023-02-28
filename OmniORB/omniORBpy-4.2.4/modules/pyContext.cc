// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyContext.cc               Created on: 2002/01/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2014 Apasphere Ltd
//    Copyright (C) 2002 AT&T Laboratories Cambridge
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
//    Context support

#include <omnipy.h>

OMNI_USING_NAMESPACE(omni)


void
omniPy::validateContext(PyObject* c_o, CORBA::CompletionStatus compstatus)
{
  if (!PyObject_IsInstance(c_o, pyCORBAContextClass))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}

void
omniPy::marshalContext(cdrStream& stream, PyObject* p_o, PyObject* c_o)
{
  PyObject* values = PyObject_CallMethod(c_o, (char*)"_get_values",
					 (char*)"O", p_o);
  if (values) {
    PyObject*    items = PyDict_Items(values);
    CORBA::ULong count = PyList_GET_SIZE(items);
    CORBA::ULong mlen  = count * 2;
    mlen >>= stream;

    for (CORBA::ULong i=0; i < count; i++) {
      PyObject* item = PyList_GET_ITEM(items, i);
      omniPy::marshalRawPyString(stream, PyTuple_GET_ITEM(item, 0));
      omniPy::marshalRawPyString(stream, PyTuple_GET_ITEM(item, 1));
    }
    Py_DECREF(values);
  }
  else {
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l << "Exception trying to get Context values:\n";
      }
      PyErr_Print();
    }
    else
      PyErr_Clear();
    
    OMNIORB_THROW(TRANSIENT, TRANSIENT_PythonExceptionInORB,
		  CORBA::COMPLETED_NO);
  }
}

PyObject*
omniPy::unmarshalContext(cdrStream& stream)
{
  PyObject* dict = PyDict_New();
  CORBA::ULong mlen;
  mlen <<= stream;

  if (mlen % 2)
    OMNIORB_THROW(MARSHAL,
		  MARSHAL_InvalidContextList,
		  CORBA::COMPLETED_MAYBE);

  CORBA::ULong count = mlen / 2;

  for (CORBA::ULong i=0; i < count; i++) {
    PyObject* k = omniPy::unmarshalRawPyString(stream);
    PyObject* v = omniPy::unmarshalRawPyString(stream);
    PyDict_SetItem(dict, k, v);
    Py_DECREF(k);
    Py_DECREF(v);
  }
  PyObject* r = PyObject_CallFunction(omniPy::pyCORBAContextClass,
				      (char*)"sON", "", Py_None, dict);
  if (!r) {
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l << "Exception trying to construct Context:\n";
      }
      PyErr_Print();
    }
    else
      PyErr_Clear();

    OMNIORB_THROW(TRANSIENT, TRANSIENT_PythonExceptionInORB,
		  CORBA::COMPLETED_NO);
  }
  return r;
}


PyObject*
omniPy::filterContext(PyObject* p_o, PyObject* c_o)
{
  PyObject* values = PyObject_CallMethod(c_o, (char*)"_get_values",
					 (char*)"O", p_o);

  if (values) {
    PyObject* r = PyObject_CallFunction(omniPy::pyCORBAContextClass,
					(char*)"sON", "", Py_None, values);
    if (r) return r;
  }
  if (omniORB::trace(1)) {
    {
      omniORB::logger l;
      l << "Exception trying to filter Context:\n";
    }
    PyErr_Print();
  }
  else
    PyErr_Clear();

  OMNIORB_THROW(TRANSIENT, TRANSIENT_PythonExceptionInORB,
		CORBA::COMPLETED_NO);
  return 0;
}
