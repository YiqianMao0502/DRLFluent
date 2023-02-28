// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyPOAManagerFunc.cc        Created on: 2000/02/04
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2008-2014 Apasphere Ltd
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
//    POAManager functions

#include <omnipy.h>


extern "C" {

  static void
  pyPM_dealloc(PyPOAManagerObject* self)
  {
    {
      omniPy::InterpreterUnlocker _u;
      CORBA::release(self->pm);
      CORBA::release(self->base.obj);
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
  }

  static PyObject*
  pyPM_activate(PyPOAManagerObject* self, PyObject* args)
  {
    try {
      omniPy::InterpreterUnlocker _u;
      self->pm->activate();
    }
    catch (PortableServer::POAManager::AdapterInactive& ex) {
      return omniPy::raiseScopedException(omniPy::pyPortableServerModule,
                                          "POAManager", "AdapterInactive");
    }
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject*
  pyPM_hold_requests(PyPOAManagerObject* self, PyObject* args)
  {
    int wfc;
    if (!PyArg_ParseTuple(args, (char*)"i", &wfc)) return NULL;

    try {
      omniPy::InterpreterUnlocker _u;
      self->pm->hold_requests(wfc);
    }
    catch (PortableServer::POAManager::AdapterInactive& ex) {
      return omniPy::raiseScopedException(omniPy::pyPortableServerModule,
                                          "POAManager", "AdapterInactive");
    }
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject*
  pyPM_discard_requests(PyPOAManagerObject* self, PyObject* args)
  {
    int wfc;
    if (!PyArg_ParseTuple(args, (char*)"i", &wfc)) return NULL;

    try {
      omniPy::InterpreterUnlocker _u;
      self->pm->discard_requests(wfc);
    }
    catch (PortableServer::POAManager::AdapterInactive& ex) {
      return omniPy::raiseScopedException(omniPy::pyPortableServerModule,
                                          "POAManager", "AdapterInactive");
    }
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject*
  pyPM_deactivate(PyPOAManagerObject* self, PyObject* args)
  {
    int eo, wfc;
    if (!PyArg_ParseTuple(args, (char*)"ii", &eo, &wfc)) return NULL;

    try {
      omniPy::InterpreterUnlocker _u;
      self->pm->deactivate(eo, wfc);
    }
    catch (PortableServer::POAManager::AdapterInactive& ex) {
      return omniPy::raiseScopedException(omniPy::pyPortableServerModule,
                                          "POAManager", "AdapterInactive");
    }
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject* pyPM_get_state(PyPOAManagerObject* self, PyObject* args)
  {
    PortableServer::POAManager::State s;
    {
      omniPy::InterpreterUnlocker _u;
      s = self->pm->get_state();
    }
    return Int_FromLong((int)s);
  }

  static PyMethodDef pyPM_methods[] = {
    {(char*)"activate",
     (PyCFunction)pyPM_activate,
     METH_NOARGS},

    {(char*)"hold_requests",
     (PyCFunction)pyPM_hold_requests,
     METH_VARARGS},

    {(char*)"discard_requests",
     (PyCFunction)pyPM_discard_requests,
     METH_VARARGS},

    {(char*)"deactivate",
     (PyCFunction)pyPM_deactivate,
     METH_VARARGS},

    {(char*)"get_state",
     (PyCFunction)pyPM_get_state,
     METH_NOARGS},

    {NULL,NULL}
  };

  static PyTypeObject PyPOAManagerType = {
    PyVarObject_HEAD_INIT(0,0)
    (char*)"_omnipy.PyPOAManagerObject",  /* tp_name */
    sizeof(PyPOAManagerObject),           /* tp_basicsize */
    0,                                 /* tp_itemsize */
    (destructor)pyPM_dealloc,          /* tp_dealloc */
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
    (char*)"Internal POAManager object", /* tp_doc */
    0,                                 /* tp_traverse */
    0,                                 /* tp_clear */
    0,                                 /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                 /* tp_iternext */
    pyPM_methods,                      /* tp_methods */
  };
}

PyObject*
omniPy::createPyPOAManagerObject(PortableServer::POAManager_ptr pm)
{
  PyPOAManagerObject* self = PyObject_New(PyPOAManagerObject,
                                          &PyPOAManagerType);
  self->pm = pm;
  self->base.obj = CORBA::Object::_duplicate(pm);

  omniPy::PyRefHolder args(PyTuple_New(1));
  PyTuple_SET_ITEM(args, 0, (PyObject*)self);

  return PyObject_CallObject(omniPy::pyPOAManagerClass, args);
}

CORBA::Boolean
omniPy::pyPOAManagerCheck(PyObject* pyobj)
{
  return pyobj->ob_type == &PyPOAManagerType;
}

void
omniPy::initPOAManagerFunc(PyObject* d)
{
  PyPOAManagerType.tp_base = omniPy::PyObjRefType;
  int r = PyType_Ready(&PyPOAManagerType);
  OMNIORB_ASSERT(r == 0);
}
