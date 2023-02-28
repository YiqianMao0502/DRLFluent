// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyServant.cc               Created on: 1999/07/29
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
// Description:
//    Implementation of Python servant object

#include <omnipy.h>
#include <pyThreadCache.h>

#include <omniORB4/callHandle.h>
#include <omniORB4/IOP_S.h>


//
// Python type used to hold a pointer to a Py_omniServant

extern "C" {

  struct pyServantObj {
    PyObject_HEAD
    omniPy::Py_omniServant* svt;
  };

  static void
  pyServantObj_dealloc(pyServantObj* self)
  {
    PyObject_Del((PyObject*)self);
  }

  static PyMethodDef pyServantObj_methods[] = {
    {0,0}
  };

  static PyTypeObject pyServantType = {
    PyVarObject_HEAD_INIT(0,0)
    (char*)"_omnipy.pyServantObj",     /* tp_name */
    sizeof(pyServantObj),              /* tp_basicsize */
    0,                                 /* tp_itemsize */
    (destructor)pyServantObj_dealloc,  /* tp_dealloc */
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
    Py_TPFLAGS_DEFAULT,                /* tp_flags */
    (char*)"omni Servant",             /* tp_doc */
    0,                                 /* tp_traverse */
    0,                                 /* tp_clear */
    0,                                 /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                 /* tp_iternext */
    pyServantObj_methods,              /* tp_methods */
  };
}

static inline PyObject*
pyServantObj_alloc(omniPy::Py_omniServant* svt)
{
  pyServantObj* self = PyObject_New(pyServantObj, &pyServantType);
  self->svt = svt;
  return (PyObject*)self;
}

static inline void
setSvt(PyObject* obj, omniPy::Py_omniServant* svt)
{
  PyObject* pysvt = pyServantObj_alloc(svt);
  PyObject_SetAttr(obj, omniPy::pyservantAttr, pysvt);
  Py_DECREF(pysvt);
}

static inline omniPy::Py_omniServant*
getSvt(PyObject* obj)
{
  omniPy::Py_omniServant* svt;
  PyObject* pysvt = PyObject_GetAttr(obj, omniPy::pyservantAttr);
  if (pysvt) {
    svt = ((pyServantObj*)pysvt)->svt;
    Py_DECREF(pysvt);
  }
  else {
    PyErr_Clear();
    svt = 0;
  }
  return svt;
}

static inline void
remSvt(PyObject* obj)
{
  PyObject_DelAttr(obj, omniPy::pyservantAttr);
}


void
omniPy::initServant(PyObject* mod)
{
  int r = PyType_Ready(&pyServantType);
  OMNIORB_ASSERT(r == 0);
}



//
// Implementation classes for ServantManagers and AdapterActivator

class Py_ServantActivatorSvt :
  public virtual POA_PortableServer::ServantActivator,
  public virtual omniPy::Py_omniServant
{
public:
  Py_ServantActivatorSvt(PyObject* pysa, PyObject* opdict, const char* repoId)
    : PY_OMNISERVANT_BASE(pysa, opdict, repoId), impl_(pysa) { }

  virtual ~Py_ServantActivatorSvt() { }

  PortableServer::Servant incarnate(const PortableServer::ObjectId& oid,
				    PortableServer::POA_ptr         poa)
  {
    return impl_.incarnate(oid, poa);
  }

  void etherealize(const PortableServer::ObjectId& oid,
		   PortableServer::POA_ptr         poa,
		   PortableServer::Servant         serv,
		   CORBA::Boolean                  cleanup_in_progress,
		   CORBA::Boolean                  remaining_activations)
  {
    impl_.etherealize(oid, poa, serv, cleanup_in_progress,
		      remaining_activations);
  }

  void* _ptrToInterface(const char* repoId);

  CORBA::Boolean _is_a(const char* logical_type_id) {
    return PY_OMNISERVANT_BASE::_is_a(logical_type_id);
  }
  PortableServer::POA_ptr _default_POA() {
    return PY_OMNISERVANT_BASE::_default_POA();
  }
  const char* _mostDerivedRepoId() {
    return PY_OMNISERVANT_BASE::_mostDerivedRepoId();
  }
  CORBA::Boolean _dispatch(omniCallHandle& handle) {
    return PY_OMNISERVANT_BASE::_dispatch(handle);
  }

private:
  omniPy::Py_ServantActivator impl_;

  // Not implemented
  Py_ServantActivatorSvt(const Py_ServantActivatorSvt&);
  Py_ServantActivatorSvt& operator=(const Py_ServantActivatorSvt&);
};

class Py_ServantLocatorSvt :
  public virtual POA_PortableServer::ServantLocator,
  public virtual omniPy::Py_omniServant
{
public:
  Py_ServantLocatorSvt(PyObject* pysl, PyObject* opdict, const char* repoId)
    : PY_OMNISERVANT_BASE(pysl, opdict, repoId), impl_(pysl) { }

  virtual ~Py_ServantLocatorSvt() { }

  PortableServer::Servant preinvoke(const PortableServer::ObjectId& oid,
				    PortableServer::POA_ptr         poa,
				    const char*                     operation,
				    void*&                          cookie)
  {
    return impl_.preinvoke(oid, poa, operation, cookie);
  }

  void postinvoke(const PortableServer::ObjectId& oid,
		  PortableServer::POA_ptr         poa,
		  const char*                     operation,
		  void*                           cookie,
		  PortableServer::Servant         serv)
  {
    impl_.postinvoke(oid, poa, operation, cookie, serv);
  }

  void* _ptrToInterface(const char* repoId);

  CORBA::Boolean _is_a(const char* logical_type_id) {
    return PY_OMNISERVANT_BASE::_is_a(logical_type_id);
  }
  PortableServer::POA_ptr _default_POA() {
    return PY_OMNISERVANT_BASE::_default_POA();
  }
  const char* _mostDerivedRepoId() {
    return PY_OMNISERVANT_BASE::_mostDerivedRepoId();
  }
  CORBA::Boolean _dispatch(omniCallHandle& handle) {
    return PY_OMNISERVANT_BASE::_dispatch(handle);
  }

private:
  omniPy::Py_ServantLocator impl_;

  // Not implemented
  Py_ServantLocatorSvt(const Py_ServantLocatorSvt&);
  Py_ServantLocatorSvt& operator=(const Py_ServantLocatorSvt&);
};


class Py_AdapterActivatorSvt :
  public virtual POA_PortableServer::AdapterActivator,
  public virtual omniPy::Py_omniServant
{
public:
  Py_AdapterActivatorSvt(PyObject* pyaa, PyObject* opdict, const char* repoId)
    : PY_OMNISERVANT_BASE(pyaa, opdict, repoId), impl_(pyaa) { }

  virtual ~Py_AdapterActivatorSvt() { }

  CORBA::Boolean unknown_adapter(PortableServer::POA_ptr parent,
				 const char*             name)
  {
    return impl_.unknown_adapter(parent, name);
  }

  void* _ptrToInterface(const char* repoId);

  CORBA::Boolean _is_a(const char* logical_type_id) {
    return PY_OMNISERVANT_BASE::_is_a(logical_type_id);
  }
  PortableServer::POA_ptr _default_POA() {
    return PY_OMNISERVANT_BASE::_default_POA();
  }
  const char* _mostDerivedRepoId() {
    return PY_OMNISERVANT_BASE::_mostDerivedRepoId();
  }
  CORBA::Boolean _dispatch(omniCallHandle& handle) {
    return PY_OMNISERVANT_BASE::_dispatch(handle);
  }

private:
  omniPy::Py_AdapterActivator impl_;

  // Not implemented
  Py_AdapterActivatorSvt(const Py_AdapterActivatorSvt&);
  Py_AdapterActivatorSvt& operator=(const Py_AdapterActivatorSvt&);
};



// Implementation of Py_omniServant

omniPy::
Py_omniServant::Py_omniServant(PyObject* pyservant, PyObject* opdict,
			       const char* repoId)
  : pyservant_(pyservant), opdict_(opdict), refcount_(1)
{
  repoId_ = CORBA::string_dup(repoId);

  OMNIORB_ASSERT(PyDict_Check(opdict));
  Py_INCREF(pyservant_);
  Py_INCREF(opdict_);

  pyskeleton_ = PyObject_GetAttrString(pyservant_, (char*)"_omni_skeleton");
  OMNIORB_ASSERT(pyskeleton_);

  setSvt(pyservant, this);
}

omniPy::
Py_omniServant::~Py_omniServant()
{
  remSvt(pyservant_);
  Py_DECREF(pyservant_);
  Py_DECREF(opdict_);
  Py_DECREF(pyskeleton_);
  CORBA::string_free(repoId_);
}


void
omniPy::
Py_omniServant::_add_ref()
{
  omnipyThreadCache::lock _t;
  OMNIORB_ASSERT(refcount_ > 0);
  ++refcount_;
}

void
omniPy::
Py_omniServant::_locked_add_ref()
{
  OMNIORB_ASSERT(refcount_ > 0);
  ++refcount_;
}

void
omniPy::
Py_omniServant::_remove_ref()
{
  omnipyThreadCache::lock _t;
  if (--refcount_ > 0) return;

  OMNIORB_ASSERT(refcount_ == 0);
  delete this;
}

void
omniPy::
Py_omniServant::_locked_remove_ref()
{
  if (--refcount_ > 0) return;

  OMNIORB_ASSERT(refcount_ == 0);
  delete this;
}


void*
omniPy::
Py_omniServant::_ptrToInterface(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if (omni::ptrStrMatch(repoId, omniPy::string_Py_omniServant))
    return (Py_omniServant*)this;

  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))
    return (void*)1;

  return 0;
}


const char*
omniPy::
Py_omniServant::_mostDerivedRepoId()
{
  return repoId_;
}


PortableServer::POA_ptr
omniPy::
Py_omniServant::_default_POA()
{
  {
    omnipyThreadCache::lock _t;

    omniPy::PyRefHolder
      pyPOA(PyObject_CallMethod(pyservant_, (char*)"_default_POA", 0));

    if (pyPOA.valid()) {
      omniPy::PyRefHolder
        pyobj(PyObject_GetAttrString(pyPOA, (char*)"_obj"));

      if (pyobj.valid() && omniPy::pyPOACheck(pyobj)) {
        return PortableServer::POA::_duplicate(((PyPOAObject*)pyobj)->poa);
      }
      else {
        PyErr_Clear();
        omniORB::logs(1, "Python servant returned an invalid object from "
                      "_default_POA.");
        OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType,
                      CORBA::COMPLETED_NO);
      }      
    }
    else {
      // The call raised a Python exception
      omniORB::logs(1, "Python servant raised an exception in _default_POA.");
      omniPy::handlePythonException();
#ifdef NEED_DUMMY_THROW
      throw 0;
#endif
    }
  }
  return 0;
}

CORBA::Boolean
omniPy::
Py_omniServant::_non_existent()
{
  omnipyThreadCache::lock _t;

  omniPy::PyRefHolder result(PyObject_CallMethod(pyservant_,
                                                 (char*)"_non_existent", 0));
  if (!result.valid()) {
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l << "Exception trying to call _non_existent. Raising UNKNOWN.\n";
      }
      PyErr_Print();
    }
    else {
      PyErr_Clear();
    }
    OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException, CORBA::COMPLETED_NO);
  }

  return PyObject_IsTrue(result);
}


PyObject*
omniPy::
Py_omniServant::py_this()
{
  CORBA::Object_ptr lobjref;
  {
    omniPy::InterpreterUnlocker _u;
    {
      CORBA::Object_var objref;
      objref  = (CORBA::Object_ptr)_do_this(CORBA::Object::_PD_repoId);
      lobjref = omniPy::makeLocalObjRef(repoId_, objref);
    }
  }
  return omniPy::createPyCorbaObjRef(repoId_, lobjref);
}


CORBA::Boolean
omniPy::
Py_omniServant::_is_a(const char* logical_type_id)
{
  if (omni::ptrStrMatch(logical_type_id, repoId_))
    return 1;
  else if (omni::ptrStrMatch(logical_type_id, CORBA::Object::_PD_repoId))
    return 1;
  else {
    omnipyThreadCache::lock _t;
    omniPy::PyRefHolder pyisa(PyObject_CallMethod(omniPy::pyomniORBmodule,
                                                  (char*)"static_is_a",
                                                  (char*)"Os",
                                                  pyskeleton_,
                                                  logical_type_id));
    if (!pyisa.valid()) {
      if (omniORB::trace(1))
        PyErr_Print();
      else
        PyErr_Clear();

      OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException, CORBA::COMPLETED_NO);
    }

    CORBA::Boolean isa = PyObject_IsTrue(pyisa);

    if (isa)
      return 1;

    // Last resort -- does the servant have an _is_a method?
    if (PyObject_HasAttrString(pyservant_, (char*)"_is_a")) {

      pyisa = PyObject_CallMethod(pyservant_, (char*)"_is_a",
				  (char*)"s", logical_type_id);

      if (pyisa.valid())
        return PyObject_IsTrue(pyisa);
      else
	omniPy::handlePythonException();
    }
  }
  return 0;
}


CORBA::Boolean
omniPy::
Py_omniServant::_dispatch(omniCallHandle& handle)
{
  int i;
  omnipyThreadCache::lock _t;

  const char* op   = handle.operation_name();
  PyObject*   desc = PyDict_GetItemString(opdict_, (char*)op);

  if (!desc) {
    if (omni::strMatch(op, "_interface")) {
      // Special case: _interface call maps to _get_interface
      //
      // If the IR stubs have been loaded, the descriptor for the
      // _interface call will be in the CORBA module. If they have not
      // been loaded, there will be no descriptor, and we fall back to
      // the ORB core version of _interface.

      desc = PyObject_GetAttrString(omniPy::pyCORBAmodule,
				    (char*)"_d_Object_interface");
      if (desc)
	Py_DECREF(desc);
      else
	PyErr_Clear();
    }
    if (!desc)
      return 0; // Unknown operation name
  }

  OMNIORB_ASSERT(PyTuple_Check(desc));

  PyObject* in_d  = PyTuple_GET_ITEM(desc,0);
  PyObject* out_d = PyTuple_GET_ITEM(desc,1);
  PyObject* exc_d = PyTuple_GET_ITEM(desc,2);
  PyObject* ctxt_d;

  OMNIORB_ASSERT(PyTuple_Check(in_d));
  OMNIORB_ASSERT(out_d == Py_None || PyTuple_Check(out_d));
  OMNIORB_ASSERT(exc_d == Py_None || PyDict_Check(exc_d));

  if (PyTuple_GET_SIZE(desc) >= 4) {
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

  Py_omniCallDescriptor call_desc(op, 0,
				  (out_d == Py_None),
				  in_d, out_d, exc_d, ctxt_d);
  try {
    omniPy::InterpreterUnlocker _u;
    handle.upcall(this, call_desc);
  }
  catch (omniPy::PyUserException& ex) {
    if (handle.iop_s()) {
      try {
	omniPy::InterpreterUnlocker _u;
	handle.iop_s()->SendException(&ex);
      }
      catch (...) {
	ex.decrefPyException();
	throw;
      }
      ex.decrefPyException();
    }
    else
      throw;
  }
  return 1;
}

void
omniPy::
Py_omniServant::remote_dispatch(Py_omniCallDescriptor* pycd)
{
  const char* op      = pycd->op();
  PyObject*   propget = 0;
  PyObject*   propset = 0;
  PyObject*   method  = PyObject_GetAttrString(pyservant_, (char*)op);

  omniPy::PyRefHolder holder(method);

  if (!method) {
    PyErr_Clear();

    if (op[0] == '_') {
      if (op[1] == 'g' && op[2] == 'e' && op[3] == 't' && op[4] == '_') {
	propget = holder.change(String_FromString(op + 5));
	PyObject* word = PyDict_GetItem(omniPy::pyomniORBwordMap, propget);
	if (word) {
	  Py_INCREF(word);
	  propget = holder.change(word);
	}
      }
      else if (op[1] == 's' && op[2] == 'e' && op[3] == 't' && op[4] == '_') {
	propset = holder.change(String_FromString(op + 5));
	PyObject* word = PyDict_GetItem(omniPy::pyomniORBwordMap, propset);
	if (word) {
	  Py_INCREF(word);
	  propset = holder.change(word);
	}
      }
      else if (omni::strMatch(op, "_interface")) {
	method = holder.change(PyObject_GetAttrString(pyservant_,
						      (char*)"_get_interface"));
      }
    }
    else {
      PyObject* word = PyDict_GetItemString(omniPy::pyomniORBwordMap, op);
      if (word) {
	// Keyword -- look up mangled name
	method = holder.change(PyObject_GetAttr(pyservant_, word));
      }
    }
    if (!method && !propget && !propset) {
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "Python servant for `" << repoId_ << "' has no method named `"
	  << op << "'.\n";
      }
      PyErr_Clear();
      OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_NoPythonMethod,
		    CORBA::COMPLETED_NO);
    }
  }

  //
  // Do the call

  PyObject* args = pycd->args();
  PyObject* result;

  if (method) {
    result = PyObject_CallObject(method, args);
  }
  else if (propget) {
    if (PyTuple_GET_SIZE(args) != 0)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);

    result = PyObject_GetAttr(pyservant_, propget);
  }
  else {
    OMNIORB_ASSERT(propset);

    if (PyTuple_GET_SIZE(args) != 1)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);

    if (PyObject_SetAttr(pyservant_, propset, PyTuple_GetItem(args, 0)) != -1) {
      Py_INCREF(Py_None);
      result = Py_None;
    }
    else {
      result = 0;
    }
  }

  if (result) {
    // No exception was thrown. Set the return value
    try {
      pycd->setAndValidateReturnedValues(result);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.logInfoAndThrow();
    }
  }
  else {
    // An exception of some sort was thrown
    PyObject *etype, *evalue, *etraceback;
    PyObject *erepoId = 0;
    PyErr_Fetch(&etype, &evalue, &etraceback);
    PyErr_NormalizeException(&etype, &evalue, &etraceback);
    OMNIORB_ASSERT(etype);

    if (evalue)
      erepoId = PyObject_GetAttrString(evalue, (char*)"_NP_RepositoryId");

    if (!(erepoId && String_Check(erepoId))) {
      PyErr_Clear();
      Py_XDECREF(erepoId);
      if (omniORB::trace(1)) {
	{
	  omniORB::logger l;
	  l << "Caught an unexpected Python exception during up-call.\n";
	}
	PyErr_Restore(etype, evalue, etraceback);
	PyErr_Print();
      }
      else {
        Py_DECREF(etype); Py_XDECREF(evalue); Py_XDECREF(etraceback);
      }
      OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException, CORBA::COMPLETED_MAYBE);
    }

    PyObject* exc_d = pycd->exc_d();

    // Is it a user exception?
    if (exc_d != Py_None) {
      OMNIORB_ASSERT(PyDict_Check(exc_d));

      PyObject* edesc = PyDict_GetItem(exc_d, erepoId);

      if (edesc) {
	Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
        try {
          PyUserException ex(edesc, evalue, CORBA::COMPLETED_MAYBE);
          ex._raise();
        }
        catch (Py_BAD_PARAM& bp) {
          bp.logInfoAndThrow();
        }
      }
    }

    // Is it a LOCATION_FORWARD?
    if (omni::strMatch(String_AS_STRING(erepoId), "omniORB.LOCATION_FORWARD")) {
      Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
      omniPy::handleLocationForward(evalue);
    }

    // System exception or unknown user exception
    omniPy::produceSystemException(evalue, erepoId, etype, etraceback);
  }
}


void
omniPy::
Py_omniServant::local_dispatch(Py_omniCallDescriptor* pycd)
{
  const char* op      = pycd->op();
  PyObject*   propget = 0;
  PyObject*   propset = 0;
  PyObject*   method  = PyObject_GetAttrString(pyservant_, (char*)op);

  omniPy::PyRefHolder holder(method);

  if (!method) {
    PyErr_Clear();

    if (op[0] == '_') {
      if (op[1] == 'g' && op[2] == 'e' && op[3] == 't' && op[4] == '_') {
	propget = holder.change(String_FromString(op + 5));
	PyObject* word = PyDict_GetItem(omniPy::pyomniORBwordMap, propget);
	if (word) {
	  Py_INCREF(word);
	  propget = holder.change(word);
	}
      }
      else if (op[1] == 's' && op[2] == 'e' && op[3] == 't' && op[4] == '_') {
	propset = holder.change(String_FromString(op + 5));
	PyObject* word = PyDict_GetItem(omniPy::pyomniORBwordMap, propset);
	if (word) {
	  Py_INCREF(word);
	  propset = holder.change(word);
	}
      }
      else if (omni::strMatch(op, "_interface")) {
	method = holder.change(PyObject_GetAttrString(pyservant_,
						      (char*)"_get_interface"));
      }
    }
    else {
      PyObject* word = PyDict_GetItemString(omniPy::pyomniORBwordMap, op);
      if (word) {
	// Keyword -- look up mangled name
	method = holder.change(PyObject_GetAttr(pyservant_, word));
      }
    }
    if (!method && !propget && !propset) {
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "Python servant for `" << repoId_ << "' has no method named `"
	  << op << "'.\n";
      }
      PyErr_Clear();
      OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_NoPythonMethod,
		    CORBA::COMPLETED_NO);
    }
  }

  PyObject* in_d;
  int       in_l;
  PyObject* out_d;
  int       out_l;
  PyObject* exc_d;
  PyObject* ctxt_d;
  pycd->setDescriptors(in_d, in_l, out_d, out_l, exc_d, ctxt_d);

  PyObject* args = pycd->args();

  // Copy args which would otherwise have reference semantics
  PyObject* argtuple = PyTuple_New(in_l + (ctxt_d ? 1 : 0));
  omniPy::PyRefHolder argtuple_holder(argtuple);

  PyObject* t_o;

  int i;

  for (i=0; i < in_l; ++i) {
    try {
      t_o = copyArgument(PyTuple_GET_ITEM(in_d, i),
			 PyTuple_GET_ITEM(args, i),
			 CORBA::COMPLETED_NO);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.add(omniPy::formatString("Operation %r parameter %d", "si",
				  pycd->op(), i));
      throw;
    }
    OMNIORB_ASSERT(t_o);
    PyTuple_SET_ITEM(argtuple, i, t_o);
  }
  if (ctxt_d) {
    t_o = filterContext(ctxt_d, PyTuple_GET_ITEM(args, in_l));
    OMNIORB_ASSERT(t_o);
    PyTuple_SET_ITEM(argtuple, in_l, t_o);
  }

  //
  // Do the call

  PyObject* result;

  if (method) {
    result = PyObject_CallObject(method, argtuple);
  }
  else if (propget) {
    if (PyTuple_GET_SIZE(argtuple) != 0)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);

    result = PyObject_GetAttr(pyservant_, propget);
  }
  else {
    OMNIORB_ASSERT(propset);
    if (PyTuple_GET_SIZE(argtuple) != 1)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);

    if (PyObject_SetAttr(pyservant_, propset,
			 PyTuple_GetItem(argtuple, 0)) != -1) {
      Py_INCREF(Py_None);
      result = Py_None;
    }
    else {
      result = 0;
    }
  }

  if (result) {
    PyObject* retval = 0;

    try {
      if (out_l == -1 || out_l == 0) {
	if (result == Py_None) {
	  pycd->setReturnedValues(result);
	  return;
	}
	else {
	  THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_MAYBE,
			     omniPy::formatString("Operation %r should return "
						  "None, got %r",
						  "sO",
						  pycd->op(),
						  result->ob_type));
	}
      }
      else if (out_l == 1) {
	try {
	  retval = copyArgument(PyTuple_GET_ITEM(out_d, 0),
				result, CORBA::COMPLETED_MAYBE);
	}
	catch (Py_BAD_PARAM& bp) {
	  bp.add(omniPy::formatString("Operation %r return value",
				      "s", pycd->op()));
	  throw;
	}
      }
      else {
	if (!PyTuple_Check(result) || PyTuple_GET_SIZE(result) != out_l) {
	  THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_MAYBE,
			     omniPy::formatString("Operation %r should return "
						  "%d-tuple, got %r",
						  "siO",
						  pycd->op(), out_l,
						  result->ob_type));
	}
	retval = PyTuple_New(out_l);
	
	for (i=0; i < out_l; ++i) {
	  try {
	    t_o = copyArgument(PyTuple_GET_ITEM(out_d, i),
			       PyTuple_GET_ITEM(result, i),
			       CORBA::COMPLETED_MAYBE);
	  }
	  catch (Py_BAD_PARAM& bp) {
	    bp.add(omniPy::formatString("Operation %r return value %d",
					"si", pycd->op(), i));
	    throw;
	  }
	  PyTuple_SET_ITEM(retval, i, t_o);
	}
      }
    }
    catch (...) {
      Py_DECREF(result);
      Py_XDECREF(retval);
      throw;
    }
    Py_DECREF(result);
    pycd->setReturnedValues(retval);
  }
  else {
    // The call raised a Python exception
    PyObject *etype, *evalue, *etraceback;
    PyObject *erepoId = 0;
    PyErr_Fetch(&etype, &evalue, &etraceback);
    PyErr_NormalizeException(&etype, &evalue, &etraceback);
    OMNIORB_ASSERT(etype);

    if (evalue)
      erepoId = PyObject_GetAttrString(evalue, (char*)"_NP_RepositoryId");

    if (!(erepoId && String_Check(erepoId))) {
      PyErr_Clear();
      Py_XDECREF(erepoId);
      if (omniORB::trace(1)) {
	{
	  omniORB::logger l;
	  l << "Caught an unexpected Python exception during up-call.\n";
	}
	PyErr_Restore(etype, evalue, etraceback);
	PyErr_Print();
      }
      else {
        Py_DECREF(etype); Py_XDECREF(evalue); Py_XDECREF(etraceback);
      }
      OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException, CORBA::COMPLETED_MAYBE);
    }

    // Is it a user exception?
    if (exc_d != Py_None) {
      OMNIORB_ASSERT(PyDict_Check(exc_d));

      PyObject* edesc = PyDict_GetItem(exc_d, erepoId);

      if (edesc) {
	Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
	PyUserException ex(edesc, evalue, CORBA::COMPLETED_MAYBE);
	ex._raise();
      }
    }

    // Is it a LOCATION_FORWARD?
    if (omni::strMatch(String_AS_STRING(erepoId), "omniORB.LOCATION_FORWARD")) {
      Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
      omniPy::handleLocationForward(evalue);
    }

    // System exception or unknown user exception
    omniPy::produceSystemException(evalue, erepoId, etype, etraceback);
  }
}


// Py_ServantActivatorSvt

void*
Py_ServantActivatorSvt::_ptrToInterface(const char* repoId)
{
  if (omni::ptrStrMatch(repoId, PortableServer::ServantActivator::_PD_repoId))
    return (PortableServer::_impl_ServantActivator*)this;
  if (omni::ptrStrMatch(repoId, omniPy::string_Py_omniServant))
    return (omniPy::Py_omniServant*)this;
  if (omni::ptrStrMatch(repoId, PortableServer::ServantManager::_PD_repoId))
    return (PortableServer::_impl_ServantManager*)this;
  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))
    return (void*)1;

  return 0;
}

// Py_ServantLocatorSvt

void*
Py_ServantLocatorSvt::_ptrToInterface(const char* repoId)
{
  if (omni::ptrStrMatch(repoId, PortableServer::ServantLocator::_PD_repoId))
    return (PortableServer::_impl_ServantLocator*)this;
  if (omni::ptrStrMatch(repoId, omniPy::string_Py_omniServant))
    return (omniPy::Py_omniServant*)this;
  if (omni::ptrStrMatch(repoId, PortableServer::ServantManager::_PD_repoId))
    return (PortableServer::_impl_ServantManager*)this;
  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))
    return (void*)1;

  return 0;
}


// Py_AdapterActivatorSvt

void*
Py_AdapterActivatorSvt::_ptrToInterface(const char* repoId)
{
  if (omni::ptrStrMatch(repoId, PortableServer::AdapterActivator::_PD_repoId))
    return (PortableServer::_impl_AdapterActivator*)this;
  if (omni::ptrStrMatch(repoId, omniPy::string_Py_omniServant))
    return (omniPy::Py_omniServant*)this;
  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))
    return (void*)1;

  return 0;
}

// Functions to create Py_omniServant objects

static
omniPy::Py_omniServant*
newSpecialServant(PyObject* pyservant, PyObject* opdict, const char* repoId)
{
  if (omni::ptrStrMatch(repoId, PortableServer::ServantActivator::_PD_repoId))
    return new Py_ServantActivatorSvt(pyservant, opdict, repoId);

  if (omni::ptrStrMatch(repoId, PortableServer::ServantLocator::_PD_repoId))
    return new Py_ServantLocatorSvt(pyservant, opdict, repoId);

  if (omni::ptrStrMatch(repoId, PortableServer::AdapterActivator::_PD_repoId))
    return new Py_AdapterActivatorSvt(pyservant, opdict, repoId);

  OMNIORB_ASSERT(0);
  return 0;
}


omniPy::Py_omniServant*
omniPy::getServantForPyObject(PyObject* pyservant)
{
  Py_omniServant* pyos;

  // Is there a Py_omniServant already?
  pyos = getSvt(pyservant);
  if (pyos) {
    pyos->_locked_add_ref();
    return pyos;
  }

  // Is it an instance of the right class?
  if (!PyObject_IsInstance(pyservant, omniPy::pyServantClass))
    return 0;

  PyObject* opdict = PyObject_GetAttrString(pyservant, (char*)"_omni_op_d");
  if (!(opdict && PyDict_Check(opdict)))
    return 0;

  PyObject* pyrepoId = PyObject_GetAttr(pyservant, pyNP_RepositoryId);

  if (!(pyrepoId && String_Check(pyrepoId))) {
    Py_DECREF(opdict);
    return 0;
  }
  if (PyObject_HasAttrString(pyservant, (char*)"_omni_special")) {

    pyos = newSpecialServant(pyservant, opdict, String_AS_STRING(pyrepoId));
  }
  else {
    pyos = new omniPy::Py_omniServant(pyservant, opdict,
				      String_AS_STRING(pyrepoId));
  }
  Py_DECREF(opdict);
  Py_DECREF(pyrepoId);

  return pyos;
}
