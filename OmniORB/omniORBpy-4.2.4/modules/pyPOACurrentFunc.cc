// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyPOACurrentFunc.cc        Created on: 2001/06/11
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2005-2014 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//    PortableServer::Current functions

#include <omnipy.h>


extern "C" {

  static void
  pyPC_dealloc(PyPOACurrentObject* self)
  {
    {
      omniPy::InterpreterUnlocker _u;
      CORBA::release(self->pc);
      CORBA::release(self->base.obj);
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
  }

  static PyObject*
  pyPC_get_POA(PyPOACurrentObject* self, PyObject* args)
  {
    PortableServer::POA_ptr poa;
    try {
      omniPy::InterpreterUnlocker _u;
      poa = self->pc->get_POA();
    }
    catch (PortableServer::Current::NoContext& ex) {
      return omniPy::raiseScopedException(omniPy::pyPortableServerModule,
                                          "Current", "NoContext");
    }
    return omniPy::createPyPOAObject(poa);
  }

  static PyObject*
  pyPC_get_object_id(PyPOACurrentObject* self, PyObject* args)
  {
    PortableServer::ObjectId_var oid;
    try {
      omniPy::InterpreterUnlocker _u;
      oid = self->pc->get_object_id();
    }
    catch (PortableServer::Current::NoContext& ex) {
      return omniPy::raiseScopedException(omniPy::pyPortableServerModule,
                                          "Current", "NoContext");
    }
    return RawString_FromStringAndSize((const char*)oid->NP_data(),
                                       oid->length());
  }

  static PyObject*
  pyPC_get_reference(PyPOACurrentObject* self, PyObject* args)
  {
    CORBA::Object_ptr lobjref;
    const char* mdri;

    try {
      omniPy::InterpreterUnlocker _u;
      CORBA::Object_var objref;
      objref  = self->pc->get_reference();
      mdri    = objref->_PR_getobj()->_mostDerivedRepoId();
      lobjref = omniPy::makeLocalObjRef(mdri, objref);
    }
    catch (PortableServer::Current::NoContext& ex) {
      return omniPy::raiseScopedException(omniPy::pyPortableServerModule,
                                          "Current", "NoContext");
    }
    return omniPy::createPyCorbaObjRef(0, lobjref);
  }
      
  static PyObject*
  pyPC_get_servant(PyPOACurrentObject* self, PyObject* args)
  {
    PortableServer::Servant servant;
    omniPy::Py_omniServant* pyos;
    try {
      omniPy::InterpreterUnlocker _u;
      servant = self->pc->get_servant();
      pyos = (omniPy::Py_omniServant*)servant->
                               _ptrToInterface(omniPy::string_Py_omniServant);
    }
    catch (PortableServer::Current::NoContext& ex) {
      return omniPy::raiseScopedException(omniPy::pyPortableServerModule,
                                          "Current", "NoContext");
    }
    if (pyos) {
      PyObject* pyservant = pyos->pyServant();
      pyos->_locked_remove_ref();
      return pyservant;
    }
    else {
      // Oh dear -- the servant is C++, not Python. OBJ_ADAPTER
      // seems the most sensible choice of exception.
      {
	omniPy::InterpreterUnlocker _u;
	servant->_remove_ref();
      }
      CORBA::OBJ_ADAPTER ex(OBJ_ADAPTER_IncompatibleServant,
			    CORBA::COMPLETED_NO);
      return omniPy::handleSystemException(ex);
    }
  }

  static PyMethodDef pyPC_methods[] = {
    {(char*)"get_POA",
     (PyCFunction)pyPC_get_POA,
     METH_NOARGS},

    {(char*)"get_object_id",
     (PyCFunction)pyPC_get_object_id,
     METH_NOARGS},

    {(char*)"get_reference",
     (PyCFunction)pyPC_get_reference,
     METH_NOARGS},

    {(char*)"get_servant",
     (PyCFunction)pyPC_get_servant,
     METH_NOARGS},

    {NULL,NULL}
  };

  static PyTypeObject PyPOACurrentType = {
    PyVarObject_HEAD_INIT(0,0)
    (char*)"_omnipy.PyPOACurrentObject", /* tp_name */
    sizeof(PyPOACurrentObject),          /* tp_basicsize */
    0,                                   /* tp_itemsize */
    (destructor)pyPC_dealloc,            /* tp_dealloc */
    0,                                   /* tp_print */
    0,                                   /* tp_getattr */
    0,                                   /* tp_setattr */
    0,                                   /* tp_compare */
    0,                                   /* tp_repr */
    0,                                   /* tp_as_number */
    0,                                   /* tp_as_sequence */
    0,                                   /* tp_as_mapping */
    0,                                   /* tp_hash  */
    0,                                   /* tp_call */
    0,                                   /* tp_str */
    0,                                   /* tp_getattro */
    0,                                   /* tp_setattro */
    0,                                   /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    (char*)"Internal PortableServer::Current object", /* tp_doc */
    0,                                   /* tp_traverse */
    0,                                   /* tp_clear */
    0,                                   /* tp_richcompare */
    0,                                   /* tp_weaklistoffset */
    0,                                   /* tp_iter */
    0,                                   /* tp_iternext */
    pyPC_methods,                        /* tp_methods */
  };

}

PyObject*
omniPy::createPyPOACurrentObject(PortableServer::Current_ptr pc)
{
  PyPOACurrentObject* self = PyObject_New(PyPOACurrentObject,
                                          &PyPOACurrentType);
  self->pc = pc;
  self->base.obj = CORBA::Object::_duplicate(pc);

  omniPy::PyRefHolder args(PyTuple_New(1));
  PyTuple_SET_ITEM(args, 0, (PyObject*)self);

  return PyObject_CallObject(omniPy::pyPOACurrentClass, args);
}

CORBA::Boolean
omniPy::pyPOACurrentCheck(PyObject* pyobj)
{
  return pyobj->ob_type == &PyPOACurrentType;
}

void
omniPy::initPOACurrentFunc(PyObject* d)
{
  PyPOACurrentType.tp_base = omniPy::PyObjRefType;
  int r = PyType_Ready(&PyPOACurrentType);
  OMNIORB_ASSERT(r == 0);
}
