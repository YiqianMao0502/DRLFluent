// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyExceptions.cc            Created on: 1999/07/29
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
//    Exception handling functions

#include <omnipy.h>
#include <pyThreadCache.h>


void
Py_BAD_PARAM::raise(const char* file, int line,
		    CORBA::ULong minor, CORBA::CompletionStatus completed,
		    PyObject* message)
{
  if (omniORB::traceExceptions) {
    omniORB::logger log;
    log << "throw BAD_PARAM from " << _OMNI_NS(omniExHelper)::strip(file)
	<< ":" << line << " (";

    const char* description =
      _OMNI_NS(minorCode2String)(_OMNI_NS(BAD_PARAM_LookupTable),minor);

    if (description) {
      log << omniORB::logger::exceptionStatus(completed, description) << ")\n";
    }
    else {
      log << omniORB::logger::exceptionStatus(completed, minor) << ")\n";
    }
  }
  throw Py_BAD_PARAM(minor, completed, message);
}

CORBA::Exception*
Py_BAD_PARAM::_NP_duplicate() const
{
  return new Py_BAD_PARAM(*this);
}

void
Py_BAD_PARAM::_raise() const
{
  throw *this;
}

const char*
Py_BAD_PARAM::_NP_typeId() const
{
  return "Exception/SystemException/BAD_PARAM/Py_BAD_PARAM";
}

Py_BAD_PARAM*
Py_BAD_PARAM::_downcast(CORBA::Exception* e)
{
  return (Py_BAD_PARAM*)
    _NP_is_a(e, "Exception/SystemException/BAD_PARAM/Py_BAD_PARAM");
}


PyObject*
omniPy::handleSystemException(const CORBA::SystemException& ex, PyObject* info)
{
  int dummy;
  PyObject* excc = PyDict_GetItemString(pyCORBAsysExcMap,
					(char*)ex._NP_repoId(&dummy));
  OMNIORB_ASSERT(excc);

  PyObject* exca;
  if (info) {
    exca = Py_BuildValue((char*)"(iiO)", ex.minor(), ex.completed(), info);

    if (omniORB::traceExceptions) {
      PyObject* info_repr = PyObject_Repr(info);
      omniORB::logger log;
      log << "BAD_PARAM info: " << String_AsString(info_repr) << "\n";
      Py_DECREF(info_repr);
    }
  }
  else {
    exca = Py_BuildValue((char*)"(ii)", ex.minor(), ex.completed());
  }

  PyObject* exci = PyEval_CallObject(excc, exca);
  Py_DECREF(exca);
  if (exci) {
    // If we couldn't create the exception object, there will be a
    // suitable error set already
    PyErr_SetObject(excc, exci);
    Py_DECREF(exci);
  }
  return 0;
}

PyObject*
omniPy::createPySystemException(const CORBA::SystemException& ex)
{
  int dummy;
  PyObject* excc = PyDict_GetItemString(pyCORBAsysExcMap,
					(char*)ex._NP_repoId(&dummy));
  OMNIORB_ASSERT(excc);

  PyObject* exca = Py_BuildValue((char*)"(ii)", ex.minor(), ex.completed());
  PyObject* exci = PyEval_CallObject(excc, exca);
  Py_DECREF(exca);

  return exci;
}


void
omniPy::produceSystemException(PyObject* eobj, PyObject* erepoId,
			       PyObject* etype, PyObject* etraceback)
{
  CORBA::ULong            minor  = 0;
  CORBA::CompletionStatus status = CORBA::COMPLETED_MAYBE;

  PyRefHolder m(PyObject_GetAttrString(eobj, (char*)"minor"));
  PyRefHolder c(PyObject_GetAttrString(eobj, (char*)"completed"));

  try {
    if (m.valid())
      minor = getULongVal(m);

    if (c.valid())
      status = (CORBA::CompletionStatus)getEnumVal(c);
  }
  catch (Py_BAD_PARAM& bp) {
    bp.logInfoAndDrop("Invalid data inside system exception");
  }

  // Clear any errors raised by the GetAttrs
  if (PyErr_Occurred()) PyErr_Clear();

  const char* repoId = String_AS_STRING(erepoId);

#define THROW_SYSTEM_EXCEPTION_IF_MATCH(ex) \
  if (omni::strMatch(repoId, "IDL:omg.org/CORBA/" #ex ":1.0")) { \
    if (omniORB::traceExceptions) { \
      { \
	PyObject* erepr = PyObject_Repr(eobj); \
        omniORB::logger l; \
        l << "Caught a CORBA system exception during up-call: " \
          << String_AsString(erepr) << "\n";                    \
        Py_DECREF(erepr); \
      } \
      PyErr_Restore(etype, eobj, etraceback); \
      PyErr_Print(); \
    } \
    else { \
      Py_XDECREF(eobj); Py_DECREF(etype); Py_XDECREF(etraceback); \
    } \
    Py_DECREF(erepoId); \
    OMNIORB_THROW(ex, minor, status); \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(THROW_SYSTEM_EXCEPTION_IF_MATCH)

#undef THROW_SYSTEM_EXCEPTION_IF_MATCH

  if (omniORB::trace(1)) {
    {
      PyObject* erepr = PyObject_Repr(eobj);
      omniORB::logger l;
      l << "Caught an unexpected CORBA exception during up-call: "
        << String_AsString(erepr) << "\n";
      Py_DECREF(erepr);
    }
    PyErr_Restore(etype, eobj, etraceback);
    PyErr_Print();
  }
  else {
    Py_XDECREF(eobj); Py_DECREF(etype); Py_XDECREF(etraceback);
  }
  Py_DECREF(erepoId);

  if (m.valid() && c.valid())
    OMNIORB_THROW(UNKNOWN, UNKNOWN_SystemException, CORBA::COMPLETED_MAYBE);
  else
    OMNIORB_THROW(UNKNOWN, UNKNOWN_UserException, CORBA::COMPLETED_MAYBE);
}


void
omniPy::handlePythonException()
{
  OMNIORB_ASSERT(PyErr_Occurred());

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
    OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException,
		  CORBA::COMPLETED_MAYBE);
  }

  // Is it a LOCATION_FORWARD?
  if (omni::strMatch(String_AS_STRING(erepoId), "omniORB.LOCATION_FORWARD")) {
    Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
    omniPy::handleLocationForward(evalue);
  }

  // System exception
  omniPy::produceSystemException(evalue, erepoId, etype, etraceback);
}


PyObject*
omniPy::raiseScopedException(PyObject* module, const char* scope,
                             const char* cls)
{
  omniPy::PyRefHolder excs(PyObject_GetAttrString(module, (char*)scope));
  omniPy::PyRefHolder excc(PyObject_GetAttrString(excs,   (char*)cls));
  omniPy::PyRefHolder exci(PyObject_CallObject(excc, omniPy::pyEmptyTuple));

  PyErr_SetObject(excc, exci);
  return 0;
}


void
omniPy::handleLocationForward(PyObject* evalue)
{
  PyObject* pyfwd  = PyObject_GetAttrString(evalue, (char*)"_forward");
  PyObject* pyperm = PyObject_GetAttrString(evalue, (char*)"_perm");
  OMNIORB_ASSERT(pyfwd);
  OMNIORB_ASSERT(pyperm);

  CORBA::Boolean perm = PyObject_IsTrue(pyperm);

  if (PyErr_Occurred()) {
    perm = 0;

    if (omniORB::trace(1)) {
      omniORB::logs(1, "Invalid 'permanent' attribute in LOCATION_FORWARD.");
      PyErr_Print();
    }
    else
      PyErr_Clear();
  }

  CORBA::Object_ptr fwd = omniPy::getObjRef(pyfwd);

  if (fwd)
    CORBA::Object::_duplicate(fwd);

  Py_DECREF(pyfwd);
  Py_DECREF(pyperm);
  Py_DECREF(evalue);

  if (fwd) {
    OMNIORB_ASSERT(CORBA::Object::_PR_is_valid(fwd));
    throw omniORB::LOCATION_FORWARD(fwd, perm);
  }
  else {
    omniORB::logs(1, "Invalid object reference inside "
		  "omniORB.LOCATION_FORWARD exception");
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
  }
}



//
// Implementation of PyUserException
//

omniPy::
PyUserException::PyUserException(PyObject* desc)
  : desc_(desc), exc_(0), decref_on_del_(0)
{
  OMNIORB_ASSERT(desc_);
  pd_insertToAnyFn    = 0;
  pd_insertToAnyFnNCP = 0;

  if (omniORB::trace(25)) {
    omniORB::logger l;
    const char* repoId = String_AS_STRING(PyTuple_GET_ITEM(desc_, 2));
    l << "Prepare to unmarshal Python user exception " << repoId << "\n";
  }
}

omniPy::
PyUserException::PyUserException(PyObject* desc, PyObject* exc,
				 CORBA::CompletionStatus comp_status)
  : desc_(desc), exc_(exc), decref_on_del_(1)
{
  OMNIORB_ASSERT(desc_);
  OMNIORB_ASSERT(exc_);

  if (omniORB::trace(25)) {
    omniORB::logger l;
    const char* repoId = String_AS_STRING(PyTuple_GET_ITEM(desc_, 2));
    l << "Construct Python user exception " << repoId << "\n";
  }

  try {
    omniPy::validateType(desc_, exc_, comp_status);
  }
  catch (...) {
    Py_DECREF(exc_);
    throw;
  }

  pd_insertToAnyFn    = 0;
  pd_insertToAnyFnNCP = 0;
}

omniPy::
PyUserException::PyUserException(const PyUserException& e)
  : desc_(e.desc_), exc_(e.exc_), decref_on_del_(1)
{
  pd_insertToAnyFn    = 0;
  pd_insertToAnyFnNCP = 0;

  // Oh dear. We need to mark the exception being copied to say that
  // the exception object should not be DECREF'd when it is deleted.
  ((PyUserException&)e).decref_on_del_ = 0;
}

omniPy::
PyUserException::~PyUserException()
{
  if (decref_on_del_) {
    if (omniORB::trace(25)) {
      omniORB::logger l;
      const char* repoId = String_AS_STRING(PyTuple_GET_ITEM(desc_, 2));
      l << "Python user exception state " << repoId << " dropped unused\n";
    }
    omnipyThreadCache::lock _t;
    OMNIORB_ASSERT(exc_);
    Py_DECREF(exc_);
  }
}

PyObject*
omniPy::
PyUserException::setPyExceptionState()
{
  OMNIORB_ASSERT(desc_);
  OMNIORB_ASSERT(exc_);

  PyObject* excclass = PyTuple_GET_ITEM(desc_, 1);
  
  if (omniORB::trace(25)) {
    omniORB::logger l;
    const char* repoId = String_AS_STRING(PyTuple_GET_ITEM(desc_, 2));
    l << "Set Python user exception state " << repoId << "\n";
  }
  PyErr_SetObject(excclass, exc_);
  Py_DECREF(exc_);
  decref_on_del_ = 0;
  exc_ = 0;
  return 0;
}

void
omniPy::
PyUserException::decrefPyException()
{
  OMNIORB_ASSERT(exc_);
  Py_DECREF(exc_);
  decref_on_del_ = 0;
  exc_ = 0;
}


void
omniPy::
PyUserException::operator>>=(cdrStream& stream) const
{
  OMNIORB_ASSERT(exc_);

  if (omniORB::trace(25)) {
    omniORB::logger l;
    const char* repoId = String_AS_STRING(PyTuple_GET_ITEM(desc_, 2));
    l << "Marshal Python user exception " << repoId << "\n";
  }

  PyUnlockingCdrStream pystream(stream);

  int cnt = (PyTuple_GET_SIZE(desc_) - 4) / 2;

  PyObject* name;
  PyObject* value;

  int i, j;
  for (i=0,j=4; i < cnt; i++) {
    name  = PyTuple_GET_ITEM(desc_, j++);
    value = PyObject_GetAttr(exc_, name);
    Py_DECREF(value); // Exception object still holds a reference.
    omniPy::marshalPyObject(pystream, PyTuple_GET_ITEM(desc_, j++), value);
  }
}

void
omniPy::
PyUserException::operator<<=(cdrStream& stream)
{
  if (omniORB::trace(25)) {
    omniORB::logger l;
    const char* repoId = String_AS_STRING(PyTuple_GET_ITEM(desc_, 2));
    l << "Unmarshal Python user exception " << repoId << "\n";
  }

  PyUnlockingCdrStream pystream(stream);

  PyObject* excclass = PyTuple_GET_ITEM(desc_, 1);

  int       cnt      = (PyTuple_GET_SIZE(desc_) - 4) / 2;
  PyObject* exctuple = PyTuple_New(cnt);
  omniPy::PyRefHolder exctuple_holder(exctuple);

  int i, j;
  for (i=0, j=5; i < cnt; i++, j+=2) {
    PyTuple_SET_ITEM(exctuple, i,
		     unmarshalPyObject(pystream,
				       PyTuple_GET_ITEM(desc_, j)));
  }
  exc_ = PyEval_CallObject(excclass, exctuple);

  if (!exc_) {
    // Oh dear. Python exception constructor threw an exception.
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l << "Caught unexpected error trying to create an exception:\n";
      }
      PyErr_Print();
    }
    else
      PyErr_Clear();

    OMNIORB_THROW(INTERNAL, 0, CORBA::COMPLETED_MAYBE);
  }
}

void
omniPy::
PyUserException::_raise() const
{
  OMNIORB_ASSERT(desc_);
  OMNIORB_ASSERT(exc_);

  if (omniORB::trace(25)) {
    omniORB::logger l;
    const char* repoId = String_AS_STRING(PyTuple_GET_ITEM(desc_, 2));
    l << "C++ throw of Python user exception " << repoId << "\n";
  }
  throw *this;
}

const char*
omniPy::
PyUserException::_NP_repoId(int* size) const
{
  PyObject* pyrepoId = PyTuple_GET_ITEM(desc_, 2);
  OMNIORB_ASSERT(String_Check(pyrepoId));

  CORBA::ULong len;
  const char*  repoId = String_AS_STRING_AND_SIZE(pyrepoId, len);

  *size = len + 1;
  return repoId;
}

void
omniPy::
PyUserException::_NP_marshal(cdrStream& stream) const
{
  omnipyThreadCache::lock _t;
  *this >>= stream;
}

CORBA::Exception*
omniPy::
PyUserException::_NP_duplicate() const
{
  return new PyUserException(*this);
}

const char*
omniPy::PyUserException::
_PD_typeId = "Exception/UserException/omniPy::PyUserException";

const char*
omniPy::
PyUserException::_NP_typeId() const
{
  return _PD_typeId;
}

omniPy::PyUserException*
omniPy::PyUserException::_downcast(CORBA::Exception* ex)
{
  return (omniPy::PyUserException*)_NP_is_a(ex, _PD_typeId);
}

const omniPy::PyUserException*
omniPy::PyUserException::_downcast(const CORBA::Exception* ex)
{
  return (const omniPy::PyUserException*)_NP_is_a(ex, _PD_typeId);
}
