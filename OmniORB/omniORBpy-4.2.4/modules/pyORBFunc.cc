// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyORBFunc.cc               Created on: 2000/02/04
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2014 Apasphere Ltd
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
//    ORB functions

#include <omnipy.h>
#include <corbaOrb.h>
#include <math.h>

extern "C" {

  static void
  pyORB_dealloc(PyORBObject* self)
  {
    {
      omniPy::InterpreterUnlocker _u;
      CORBA::release(self->orb);
      CORBA::release(self->base.obj);
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
  }

  static PyObject*
  pyORB_string_to_object(PyORBObject* self, PyObject* args)
  {
    char* s;

    if (!PyArg_ParseTuple(args, (char*)"s", &s))
      return NULL;

    if (!s || strlen(s) == 0) {
      CORBA::INV_OBJREF ex;
      return omniPy::handleSystemException(ex);
    }
    CORBA::Object_ptr objref;

    try {
      objref = omniPy::stringToObject(s);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    return omniPy::createPyCorbaObjRef(0, objref);
  }

  static PyObject*
  pyORB_object_to_string(PyORBObject* self, PyObject* args)
  {
    PyObject* pyobjref;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyobjref))
      return NULL;

    CORBA::Object_ptr objref;

    if (pyobjref == Py_None)
      objref = CORBA::Object::_nil();
    else
      objref = omniPy::getObjRef(pyobjref);

    RAISE_PY_BAD_PARAM_IF(!objref, BAD_PARAM_WrongPythonType);

    CORBA::String_var str;
    try {
      omniPy::InterpreterUnlocker _u;
      str = self->orb->object_to_string(objref);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    return String_FromString((char*)str);
  }

  static PyObject*
  pyORB_register_initial_reference(PyORBObject* self, PyObject* args)
  {
    char*     identifier;
    PyObject* pyobjref;

    if (!PyArg_ParseTuple(args, (char*)"sO", &identifier, &pyobjref))
      return NULL;

    CORBA::Object_ptr objref;

    if (pyobjref == Py_None)
      objref = CORBA::Object::_nil();
    else
      objref = omniPy::getObjRef(pyobjref);

    RAISE_PY_BAD_PARAM_IF(!objref, BAD_PARAM_WrongPythonType);

    try {
      omniPy::InterpreterUnlocker _u;
      self->orb->register_initial_reference(identifier, objref);
    }
    catch (CORBA::ORB::InvalidName& ex) {
      return omniPy::raiseScopedException(omniPy::pyCORBAmodule,
                                          "ORB", "InvalidName");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  pyORB_list_initial_services(PyORBObject* self, PyObject* args)
  {
    CORBA::ORB::ObjectIdList_var ids;
    try {
      omniPy::InterpreterUnlocker _u;
      ids = self->orb->list_initial_services();
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    PyObject* pyids = PyList_New(ids->length());

    for (CORBA::ULong i=0; i < ids->length(); i++) {
      PyList_SetItem(pyids, i, String_FromString(ids[i]));
    }
    return pyids;
  }

  static PyObject*
  pyORB_resolve_initial_references(PyORBObject* self, PyObject* args)
  {
    char* id;

    if (!PyArg_ParseTuple(args, (char*)"s", &id))
      return NULL;

    CORBA::Object_ptr objref;

    try {
      omniPy::InterpreterUnlocker _u;
      objref = self->orb->resolve_initial_references(id);

      if (!(CORBA::is_nil(objref) || objref->_NP_is_pseudo())) {
	omniObjRef* cxxref = objref->_PR_getobj();
	omniObjRef* pyref  = omniPy::createObjRef(CORBA::Object::_PD_repoId,
						  cxxref->_getIOR(), 0, 0);
	CORBA::release(objref);
	objref =
	  (CORBA::Object_ptr)pyref->_ptrToObjRef(CORBA::Object::_PD_repoId);
      }
    }
    catch (CORBA::ORB::InvalidName& ex) {
      return omniPy::raiseScopedException(omniPy::pyCORBAmodule,
                                          "ORB", "InvalidName");
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    return omniPy::createPyCorbaObjRef(0, objref);
  }

  static PyObject*
  pyORB_work_pending(PyORBObject* self, PyObject* args)
  {
    CORBA::Boolean pending;

    try {
      omniPy::InterpreterUnlocker _u;
      pending = self->orb->work_pending();
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    return PyBool_FromLong(pending);
  }

  static PyObject*
  pyORB_perform_work(PyORBObject* self, PyObject* args)
  {
    try {
      omniPy::InterpreterUnlocker _u;
      self->orb->perform_work();
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  pyORB_run_timeout(PyORBObject* self, PyObject* args)
  {
    double timeout;

    if (!PyArg_ParseTuple(args, (char*)"d", &timeout)) return NULL;

    CORBA::Boolean shutdown;
    
    try {
      omniPy::InterpreterUnlocker _u;
      unsigned long s, ns;
      s  = (unsigned long)floor(timeout);
      ns = (unsigned long)((timeout - (double)s) * 1000000000.0);
      omni_thread::get_time(&s, &ns, s, ns);
      shutdown = ((omniOrbORB*)self->orb)->run_timeout(s, ns);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    return PyBool_FromLong(shutdown);
  }

  static PyObject*
  pyORB_shutdown(PyORBObject* self, PyObject* args)
  {
    int wait;

    if (!PyArg_ParseTuple(args, (char*)"i", &wait)) return NULL;

    try {
      omniPy::InterpreterUnlocker _u;
      self->orb->shutdown(wait);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  pyORB_destroy(PyORBObject* self, PyObject* args)
  {
    try {
      omniPy::InterpreterUnlocker _u;
      self->orb->destroy();
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    Py_INCREF(Py_None);
    return Py_None;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Python method table                                                    //
  ////////////////////////////////////////////////////////////////////////////

  static PyMethodDef pyORB_methods[] = {
    {(char*)"string_to_object",
     (PyCFunction)pyORB_string_to_object,
     METH_VARARGS},

    {(char*)"object_to_string",
     (PyCFunction)pyORB_object_to_string,
     METH_VARARGS},

    {(char*)"register_initial_reference",
     (PyCFunction)pyORB_register_initial_reference,
     METH_VARARGS},

    {(char*)"list_initial_services",
     (PyCFunction)pyORB_list_initial_services,
     METH_NOARGS},

    {(char*)"resolve_initial_references",
     (PyCFunction)pyORB_resolve_initial_references,
     METH_VARARGS},

    {(char*)"work_pending",
     (PyCFunction)pyORB_work_pending,
     METH_NOARGS},

    {(char*)"perform_work",
     (PyCFunction)pyORB_perform_work,
     METH_NOARGS},

    {(char*)"run_timeout",
     (PyCFunction)pyORB_run_timeout,
     METH_VARARGS},

    {(char*)"shutdown",
     (PyCFunction)pyORB_shutdown,
     METH_VARARGS},

    {(char*)"destroy",
     (PyCFunction)pyORB_destroy,
     METH_NOARGS},

    {NULL,NULL}
  };

  static PyTypeObject PyORBType = {
    PyVarObject_HEAD_INIT(0,0)
    (char*)"_omnipy.PyORBObject",      /* tp_name */
    sizeof(PyORBObject),               /* tp_basicsize */
    0,                                 /* tp_itemsize */
    (destructor)pyORB_dealloc,         /* tp_dealloc */
    0,                                 /* tp_print */
    0,                                 /* tp_getattr */
    0,                                 /* tp_setattr */
    0,                                 /* tp_compare */
    0,                                 /* tp_repr */
    0,                                 /* tp_as_number */
    0,                                 /* tp_as_sequence */
    0,                                 /* tp_as_mapping */
    0,                                 /* tp_hash  */
    0,                                 /* tp_call */
    0,                                 /* tp_str */
    0,                                 /* tp_getattro */
    0,                                 /* tp_setattro */
    0,                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    (char*)"Internal ORB object",      /* tp_doc */
    0,                                 /* tp_traverse */
    0,                                 /* tp_clear */
    0,                                 /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                 /* tp_iternext */
    pyORB_methods,                     /* tp_methods */
  };
}


PyObject*
omniPy::createPyORBObject(CORBA::ORB_ptr orb)
{
  PyORBObject* self = PyObject_New(PyORBObject, &PyORBType);
  self->orb = orb;
  self->base.obj = CORBA::Object::_duplicate(orb);

  omniPy::PyRefHolder args(PyTuple_New(1));
  PyTuple_SET_ITEM(args, 0, (PyObject*)self);

  return PyObject_CallObject(omniPy::pyCORBAORBClass, args);
}

CORBA::Boolean
omniPy::pyORBCheck(PyObject* pyobj)
{
  return pyobj->ob_type == &PyORBType;
}

void
omniPy::initORBFunc(PyObject* d)
{
  PyORBType.tp_base = omniPy::PyObjRefType;
  int r = PyType_Ready(&PyORBType);
  OMNIORB_ASSERT(r == 0);
}
