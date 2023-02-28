// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyAbstractIntf.cc          Created on: 2003/05/21
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2003-2014 Apasphere Ltd.
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
//    Abstract interface support

#include <omnipy.h>

OMNI_USING_NAMESPACE(omni)

void
omniPy::
validateTypeAbstractInterface(PyObject* d_o, PyObject* a_o,
			      CORBA::CompletionStatus compstatus,
			      PyObject* track)
{ // repoId, name

  // Nil?
  if (a_o == Py_None)
    return;

  // Object reference?
  if (omniPy::getObjRef(a_o))
    return;

  // Value?
  if (PyObject_IsInstance(a_o, omniPy::pyCORBAValueBase)) {
    // Does it support the interface?
    PyObject* repoId    = PyTuple_GET_ITEM(d_o, 1);
    PyObject* skelclass = PyDict_GetItem(omniPy::pyomniORBskeletonMap, repoId);

    if (!skelclass) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("No skeleton class for %r",
					      "O", repoId));
    }

    if (!PyObject_IsInstance(a_o, skelclass)) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Valuetype %r does not support "
					      "abstract interface %r",
					      "OO",
					      a_o->ob_type,
					      PyTuple_GET_ITEM(d_o, 2)));
    }

    // Check the instance matches the valuetype it claims to be.
    repoId = PyObject_GetAttr(a_o, omniPy::pyNP_RepositoryId);
    if (!repoId) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Valuetype %r has no "
					      "repository id",
					      "O", a_o->ob_type));
    }

    PyObject* valuedesc = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

    Py_DECREF(repoId);

    if (!valuedesc) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Unknown valuetype %r",
					      "O", repoId));
    }

    try {
      omniPy::validateTypeValue(valuedesc, a_o, compstatus, track);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.add(omniPy::formatString("Valuetype in abstract interface %r", "O",
				  PyTuple_GET_ITEM(d_o, 2)));
      throw;
    }
    return;
  }
  THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		     omniPy::formatString("Expecting abstract interface %r, "
					  "got %r",
					  "OO",
					  PyTuple_GET_ITEM(d_o, 2),
					  a_o->ob_type));
}


void
omniPy::
marshalPyObjectAbstractInterface(cdrStream& stream,
				 PyObject* d_o, PyObject* a_o)
{ // repoId, name

  if (a_o == Py_None) {
    // GIOP spec says treat as a null valuetype
    stream.marshalBoolean(0);
    CORBA::Long tag = 0;
    tag >>= stream;
    return;
  }

  // Object reference?
  CORBA::Object_ptr obj = omniPy::getObjRef(a_o);
  if (obj) {
    stream.marshalBoolean(1);
    CORBA::Object::_marshalObjRef(obj, stream);
    return;
  }

  // Valuetype
  stream.marshalBoolean(0);
  omniPy::marshalPyObjectValue(stream, omniPy::pyCORBAValueBaseDesc, a_o);
}

PyObject*
omniPy::
unmarshalPyObjectAbstractInterface(cdrStream& stream, PyObject* d_o)
{ // repoId, name

  CORBA::Boolean is_objref = stream.unmarshalBoolean();

  if (is_objref) {
    PyObject* pyrepoId = PyTuple_GET_ITEM(d_o, 1);
    const char* repoId = String_AS_STRING(pyrepoId);

    CORBA::Object_ptr obj = omniPy::UnMarshalObjRef(repoId, stream);
    return omniPy::createPyCorbaObjRef(repoId, obj);
  }
  else {
    return omniPy::unmarshalPyObjectValue(stream,omniPy::pyCORBAValueBaseDesc);
  }
}

PyObject*
omniPy::
copyArgumentAbstractInterface(PyObject* d_o, PyObject* a_o,
			      CORBA::CompletionStatus compstatus)
{
  if (a_o == Py_None) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  if (omniPy::getObjRef(a_o)) {
    return omniPy::copyObjRefArgument(PyTuple_GET_ITEM(d_o, 1),
				      a_o, compstatus);
  }
  if (PyObject_IsInstance(a_o, omniPy::pyCORBAValueBase)) {
    // Does it support the interface?
    PyObject* repoId    = PyTuple_GET_ITEM(d_o, 1);
    PyObject* skelclass = PyDict_GetItem(omniPy::pyomniORBskeletonMap, repoId);

    if (!skelclass) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("No skeleton class for %r",
					      "O", repoId));
    }

    if (!PyObject_IsInstance(a_o, skelclass)) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Valuetype %r does not support "
					      "abstract interface %r",
					      "OO",
					      a_o->ob_type,
					      PyTuple_GET_ITEM(d_o, 2)));
    }

    // Check the instance matches the valuetype it claims to be.
    repoId = PyObject_GetAttr(a_o, omniPy::pyNP_RepositoryId);
    if (!repoId) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Valuetype %r has no "
					      "repository id",
					      "O", a_o->ob_type));
    }

    PyObject* valuedesc = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

    Py_DECREF(repoId);

    if (!valuedesc) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Unknown valuetype %r",
					      "O", repoId));
    }

    try {
      return omniPy::copyArgumentValue(valuedesc, a_o, compstatus);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.add(omniPy::formatString("Valuetype in abstract interface %r", "O",
				  PyTuple_GET_ITEM(d_o, 2)));
      throw;
    }
  }
  THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		     omniPy::formatString("Expecting abstract interface %r, "
					  "got %r",
					  "OO",
					  PyTuple_GET_ITEM(d_o, 2),
					  a_o->ob_type));
  return 0;
}
