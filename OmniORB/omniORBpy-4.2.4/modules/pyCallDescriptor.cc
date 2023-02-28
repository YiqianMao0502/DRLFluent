// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyCallDescriptor.cc        Created on: 2000/02/02
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2014 Apasphere Ltd
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
//    Implementation of Python call descriptor object

#include <omnipy.h>
#include <pyThreadCache.h>
#include <omniORB4/IOP_C.h>

#ifdef HAS_Cplusplus_Namespace
namespace {
#endif
  class cdLockHolder {
  public:
    inline cdLockHolder(omniPy::Py_omniCallDescriptor* cd)
      : ul_(cd->unlocker()), cn_(0)
    {
      if (ul_)
        ul_->lock();
      else
        cn_ = omnipyThreadCache::acquire();
    }
    inline ~cdLockHolder() {
      if (ul_)
        ul_->unlock();
      else
        omnipyThreadCache::release(cn_);
    }
  private:
    omniPy::InterpreterUnlocker*  ul_;
    omnipyThreadCache::CacheNode* cn_;
  };
#ifdef HAS_Cplusplus_Namespace
};
#endif


OMNI_USING_NAMESPACE(omni)


extern "C" {

  //
  // Forward declarations
  
  struct PyCDObj;
  static PyObject* PyPSetObj_alloc(PyObject* poller);


  //
  // Python type used to hold call descriptor and implement AMI Poller.

  struct PyCDObj {
    PyObject_HEAD
    omniPy::Py_omniCallDescriptor* cd;
    CORBA::Boolean                 from_poller;
    CORBA::Boolean                 retrieved;
  };

  static void
  PyCDObj_dealloc(PyCDObj* self)
  {
    delete self->cd;
    PyObject_Del((PyObject*)self);
  }

  static PyObject*
  PyCDObj_poll(PyCDObj* self, PyObject* args)
  {
    omniPy::Py_omniCallDescriptor* cd = self->cd;

    const char*  op;
    Py_ssize_t   op_len;
    PyObject*    pytimeout;
    CORBA::ULong timeout;

    if (!PyArg_ParseTuple(args, (char*)"s#O", &op, &op_len, &pytimeout))
      return 0;

    timeout = PyLong_AsUnsignedLong(pytimeout);
    if (timeout == (CORBA::ULong)-1 && PyErr_Occurred())
      return 0;

    try {
      if ((CORBA::ULong)op_len+1 != cd->op_len() ||
          !omni::strMatch(op, cd->op())) {

        OMNIORB_THROW(BAD_OPERATION,
                      BAD_OPERATION_WrongPollerOperation,
                      CORBA::COMPLETED_NO);
      }

      if (self->retrieved) {
        OMNIORB_THROW(OBJECT_NOT_EXIST,
                      OBJECT_NOT_EXIST_PollerAlreadyDeliveredReply,
                      CORBA::COMPLETED_NO);
      }

      {
        omniPy::InterpreterUnlocker u;

        if (!cd->isReady(timeout)) {
          if (timeout == 0) {
            OMNIORB_THROW(NO_RESPONSE,
                          NO_RESPONSE_ReplyNotAvailableYet,
                          CORBA::COMPLETED_NO);
          }
          else {
            OMNIORB_THROW(TIMEOUT,
                          TIMEOUT_NoPollerResponseInTime,
                          CORBA::COMPLETED_NO);
          }
        }
      }

      self->retrieved = 1;

      if (cd->exceptionOccurred())
        return cd->raisePyException();

      return cd->result();
    }
    catch (CORBA::SystemException& ex) {
      // Only catches exceptions thrown by the poller.
      self->from_poller = 1;
      return omniPy::handleSystemException(ex);
    }
  }

  static PyObject*
  PyCDObj_is_ready(PyCDObj* self, PyObject* args)
  {
    PyObject*    pytimeout;
    CORBA::ULong timeout;

    if (!PyArg_ParseTuple(args, (char*)"O", &pytimeout))
      return 0;
    
    timeout = PyLong_AsUnsignedLong(pytimeout);
    if (timeout == (CORBA::ULong)-1 && PyErr_Occurred())
      return 0;

    CORBA::Boolean result;
    {
      omniPy::InterpreterUnlocker u;
      result = self->cd->isReady(timeout);
    }
    return PyBool_FromLong(result);
  }

  static PyObject*
  PyCDObj_create_pollable_set(PyCDObj* self, PyObject* args)
  {
    PyObject* poller;
    if (!PyArg_ParseTuple(args, (char*)"O", &poller))
      return 0;

    return PyPSetObj_alloc(poller);
  }

  static PyObject*
  PyCDObj_operation_target(PyCDObj* self, PyObject* args)
  {
    omniObjRef* objref = self->cd->objref();
    omni::duplicateObjRef(objref);

    CORBA::Object_ptr obj =
      (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);

    return omniPy::createPyCorbaObjRef(0, obj);
  }

  static PyObject*
  PyCDObj_operation_name(PyCDObj* self, PyObject* args)
  {
    return String_FromString(self->cd->op());
  }

  static PyObject*
  PyCDObj_get_handler(PyCDObj* self, PyObject* args)
  {
    return self->cd->callback();
  }

  static PyObject*
  PyCDObj_set_handler(PyCDObj* self, PyObject* args)
  {
    PyObject* pyhandler;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyhandler))
      return 0;
    
    self->cd->callback(pyhandler);
    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  PyCDObj_is_from_poller(PyCDObj* self, PyObject* args)
  {
    return PyBool_FromLong(self->from_poller);
  }

  static PyObject*
  PyCDObj_raise_exception(PyCDObj* self, PyObject* args)
  {
    return self->cd->raisePyException();
  }

  static PyMethodDef PyCDObj_methods[] = {
    {(char*)"poll",             (PyCFunction)PyCDObj_poll,        METH_VARARGS},
    {(char*)"is_ready",         (PyCFunction)PyCDObj_is_ready,    METH_VARARGS},
    {(char*)"create_pollable_set",
                                (PyCFunction)PyCDObj_create_pollable_set,
                                                                  METH_VARARGS},
    {(char*)"operation_target", (PyCFunction)PyCDObj_operation_target,
                                                                  METH_NOARGS},
    {(char*)"operation_name",   (PyCFunction)PyCDObj_operation_name,
                                                                  METH_NOARGS},
    {(char*)"get_handler",      (PyCFunction)PyCDObj_get_handler, METH_VARARGS},
    {(char*)"set_handler",      (PyCFunction)PyCDObj_set_handler, METH_VARARGS},
    {(char*)"is_from_poller",   (PyCFunction)PyCDObj_is_from_poller,
                                                                  METH_NOARGS},
    {(char*)"raise_exception",  (PyCFunction)PyCDObj_raise_exception,
                                                                  METH_NOARGS},
    {0,0}
  };

  static PyTypeObject PyCDType = {
    PyVarObject_HEAD_INIT(0,0)
    (char*)"_omnipy.PyCDObj",          /* tp_name */
    sizeof(PyCDObj),                   /* tp_basicsize */
    0,                                 /* tp_itemsize */
    (destructor)PyCDObj_dealloc,       /* tp_dealloc */
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
    (char*)"Call descriptor",          /* tp_doc */
    0,                                 /* tp_traverse */
    0,                                 /* tp_clear */
    0,                                 /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                 /* tp_iternext */
    PyCDObj_methods,                   /* tp_methods */
  };

  // Return borrowed reference to PyCDObj inside a Python poller. Sets
  // exception state and returns 0 if invalid.
  static inline
  PyCDObj* getPyCDObj(PyObject* poller)
  {
    omniPy::PyRefHolder pycd(PyObject_GetAttrString(poller, (char*)"_poller"));
    if (!pycd.valid())
      return 0;

    if (pycd.obj()->ob_type != &PyCDType) {
      CORBA::BAD_PARAM ex(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
      omniPy::handleSystemException(ex);
      return 0;
    }
    return (PyCDObj*)pycd.obj();
  }


  //
  // Python type implementing AMI PollableSet

  struct PyPSetObj {
    PyObject_HEAD
    omni_tracedcondition* cond;
    PyObject*             pollers;
  };

  static void
  PyPSetObj_dealloc(PyPSetObj* self)
  {
    {
      omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);

      CORBA::ULong len = PyList_GET_SIZE(self->pollers);
      for (CORBA::ULong idx=0; idx != len; ++idx) {
        PyCDObj* pycd = getPyCDObj(PyList_GET_ITEM(self->pollers, idx));
        OMNIORB_ASSERT(pycd);

        pycd->cd->remFromSet(self->cond);
      }
    }
    delete self->cond;
    Py_DECREF(self->pollers);

    PyObject_Del((PyObject*)self);
  }

  static PyObject*
  PyPSetObj_add_pollable(PyPSetObj* self, PyObject* args)
  {
    PyObject* poller;

    if (!PyArg_ParseTuple(args, (char*)"O", &poller))
      return 0;

    PyCDObj* pycd = getPyCDObj(poller);
    if (!pycd)
      return 0;

    if (pycd->retrieved) {
      CORBA::OBJECT_NOT_EXIST ex(OBJECT_NOT_EXIST_PollerAlreadyDeliveredReply,
                                 CORBA::COMPLETED_NO);
      return omniPy::handleSystemException(ex);
    }

    CORBA::Boolean ok;
    {
      omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);
      ok = pycd->cd->addToSet(self->cond);
    }
    if (ok) {
      Py_INCREF(poller);
      PyList_Append(self->pollers, (PyObject*)poller);
    }
    else {
      CORBA::BAD_PARAM ex(BAD_PARAM_PollableAlreadyInPollableSet,
                          CORBA::COMPLETED_NO);
      return omniPy::handleSystemException(ex);
    }
    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  PyPSetObj_getAndRemoveReadyPollable(PyPSetObj* self)
  {
    CORBA::ULong len = PyList_GET_SIZE(self->pollers);

    if (len == 0) {
      // No possible pollable
      return omniPy::raiseScopedException(omniPy::pyCORBAmodule,
                                          "PollableSet", "NoPossiblePollable");
    }

    CORBA::ULong idx;
    PyObject*    poller;
    PyCDObj*     pycd;

    {
      omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);

      for (idx=0; idx != len; ++idx) {
        poller = PyList_GET_ITEM(self->pollers, idx);
        pycd   = getPyCDObj(poller);

        if (pycd->cd->lockedIsComplete()) {
          pycd->cd->remFromSet(self->cond);
          break;
        }
      }
    }
    
    if (idx != len) {
      // Poller was complete.
      Py_INCREF(poller);

      if (--len > idx) {
        // Copy last item from the list.
        PyObject* last = PyList_GET_ITEM(self->pollers, len);
        Py_INCREF(last);

        // Overwrite the completed poller.
        PyList_SetItem(self->pollers, idx, last);
      }

      // Shrink the list. Releases one reference to last.
      PyList_SetSlice(self->pollers, len, len+1, 0);

      return poller;
    }
    return 0;
  }

  static PyObject*
  PyPSetObj_get_ready_pollable(PyPSetObj* self, PyObject* args)
  {
    PyObject*    pytimeout;
    CORBA::ULong timeout;

    if (!PyArg_ParseTuple(args, (char*)"O", &pytimeout))
      return 0;

#if (PY_VERSION_HEX <= 0x03000000)
    if (PyInt_Check(pytimeout))
      timeout = PyInt_AsLong(pytimeout);
    else
#endif
      timeout = PyLong_AsUnsignedLong(pytimeout);
    
    if (PyErr_Occurred())
      return 0;

    PyObject* pollable;
    
    pollable = PyPSetObj_getAndRemoveReadyPollable(self);
    if (pollable || PyErr_Occurred())
      return pollable;

    if (timeout == 0) {
      CORBA::NO_RESPONSE ex(NO_RESPONSE_ReplyNotAvailableYet,
                            CORBA::COMPLETED_NO);
      return omniPy::handleSystemException(ex);
    }

    if (timeout == 0xffffffff) {
      do {
        {
          omniPy::InterpreterUnlocker ul;
          omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);
          self->cond->wait();
        }
        pollable = PyPSetObj_getAndRemoveReadyPollable(self);
        if (pollable || PyErr_Occurred())
          return pollable;

      } while(1);      
    }

    {
      omniPy::InterpreterUnlocker ul;

      omni_time_t timeout_tt(timeout / 1000, (timeout % 1000) * 1000000);
      omni_time_t deadline;
      omni_thread::get_time(deadline, timeout_tt);

      omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);
      self->cond->timedwait(deadline);
    }

    pollable = PyPSetObj_getAndRemoveReadyPollable(self);
    if (pollable || PyErr_Occurred())
      return pollable;

    CORBA::TIMEOUT ex(TIMEOUT_NoPollerResponseInTime, CORBA::COMPLETED_NO);
    return omniPy::handleSystemException(ex);
  }


  static PyObject*
  PyPSetObj_remove(PyPSetObj* self, PyObject* args)
  {
    PyObject* poller;

    if (!PyArg_ParseTuple(args, (char*)"O", &poller))
      return 0;

    CORBA::ULong len = PyList_GET_SIZE(self->pollers);
    CORBA::ULong idx;

    for (idx=0; idx != len; ++idx) {
      if (PyList_GET_ITEM(self->pollers, idx) == poller) {
        // Found it

        if (--len > idx) {
          // Copy last item from the list.
          PyObject* last = PyList_GET_ITEM(self->pollers, len);
          Py_INCREF(last);

          // Overwrite the removed poller, releasing the list's reference to it.
          PyList_SetItem(self->pollers, idx, last);
        }

        // Shrink the list. Releases one reference to last.
        PyList_SetSlice(self->pollers, len, len+1, 0);

        // Tell the call descriptor it has been removed from the set
        PyCDObj* pycd = getPyCDObj(poller);
        {
          omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);
          pycd->cd->remFromSet(self->cond);
        }

        Py_INCREF(Py_None);
        return Py_None;
      }
    }
    return omniPy::raiseScopedException(omniPy::pyCORBAmodule,
                                        "PollableSet", "UnknownPollable");
  }

  static PyObject*
  PyPSetObj_number_left(PyPSetObj* self, PyObject* args)
  {
    // Return is CORBA::UShort, so we clip to 0xffff.
    int len = PyList_GET_SIZE(self->pollers);
    if (len > 0xffff)
      len = 0xffff;

    return Int_FromLong(len);
  }

  static PyMethodDef PyPSetObj_methods[] = {
    {(char*)"add_pollable",  (PyCFunction)PyPSetObj_add_pollable, METH_VARARGS},

    {(char*)"get_ready_pollable",
                             (PyCFunction)PyPSetObj_get_ready_pollable,
                                                                  METH_VARARGS},

    {(char*)"remove",        (PyCFunction)PyPSetObj_remove,       METH_VARARGS},
    {(char*)"number_left",   (PyCFunction)PyPSetObj_number_left,  METH_NOARGS},
    {0,0}
  };

  static PyTypeObject PyPSetType = {
    PyVarObject_HEAD_INIT(0,0)
    (char*)"_omnipy.PyPSetObj",        /* tp_name */
    sizeof(PyPSetObj),                 /* tp_basicsize */
    0,                                 /* tp_itemsize */
    (destructor)PyPSetObj_dealloc,     /* tp_dealloc */
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
    (char*)"Pollable set",             /* tp_doc */
    0,                                 /* tp_traverse */
    0,                                 /* tp_clear */
    0,                                 /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                 /* tp_iternext */
    PyPSetObj_methods,                 /* tp_methods */
  };

  static PyObject*
  PyPSetObj_alloc(PyObject* poller)
  {
    PyCDObj* pycd = getPyCDObj(poller);
    if (!pycd)
      return 0;

    if (pycd->retrieved) {
      CORBA::OBJECT_NOT_EXIST ex(OBJECT_NOT_EXIST_PollerAlreadyDeliveredReply,
                                 CORBA::COMPLETED_NO);
      return omniPy::handleSystemException(ex);
    }

    omni_tracedcondition* cond =
      new omni_tracedcondition(&omniAsyncCallDescriptor::sd_lock);

    CORBA::Boolean ok;
    {
      omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);
      ok = pycd->cd->addToSet(cond);
    }

    if (ok) {
      PyPSetObj* self = PyObject_New(PyPSetObj, &PyPSetType);
      self->cond      = cond;
      self->pollers   = PyList_New(1);

      Py_INCREF(poller);
      PyList_SetItem(self->pollers, 0, (PyObject*)poller);
      
      return (PyObject*)self;
    }
    else {
      // Pollable was already in a set.
      delete cond;
      CORBA::BAD_PARAM ex(BAD_PARAM_PollableAlreadyInPollableSet,
                          CORBA::COMPLETED_NO);
      return omniPy::handleSystemException(ex);
    }
  }
}


static inline PyCDObj*
PyCDObj_alloc(omniPy::Py_omniCallDescriptor* cd)
{
  PyCDObj* self = PyObject_New(PyCDObj, &PyCDType);

  self->cd          = cd;
  self->from_poller = 0;
  self->retrieved   = 0;

  return self;
}

void
omniPy::initCallDescriptor(PyObject* mod)
{
  int r;

  r = PyType_Ready(&PyCDType);
  OMNIORB_ASSERT(r == 0);

  r = PyType_Ready(&PyPSetType);
  OMNIORB_ASSERT(r == 0);
}


omniPy::Py_omniCallDescriptor::~Py_omniCallDescriptor()
{
  OMNIORB_ASSERT(!unlocker_);
}


//
// Client side

void
omniPy::Py_omniCallDescriptor::initialiseCall(cdrStream&)
{
  // initialiseCall() is called with the interpreter lock
  // released. Reacquire it so we can touch the descriptor objects
  // safely
  cdLockHolder _l(this);

  for (int i=0; i < in_l_; i++) {
    try {
      omniPy::validateType(PyTuple_GET_ITEM(in_d_,i),
                           PyTuple_GET_ITEM(args_,i),
                           CORBA::COMPLETED_NO);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.add(omniPy::formatString("Operation %r parameter %d",
                                  "si", op(), i));
      throw;
    }
  }
}


void
omniPy::Py_omniCallDescriptor::marshalArguments(cdrStream& stream)
{
  int i;
  if (in_marshal_) {
    omniORB::logs(25, "Python marshalArguments re-entered.");

    // marshalArguments can be re-entered when using GIOP 1.0, to
    // calculate the message size if the message is too big for a
    // single buffer. In that case, the interpreter lock has been
    // released by the PyUnlockingCdrStream, meaning the call
    // descriptor does not have the lock details. We have to use the
    // thread cache lock.

    omnipyThreadCache::lock _t;

    for (i=0; i < in_l_; i++)
      omniPy::marshalPyObject(stream,
                              PyTuple_GET_ITEM(in_d_,i),
                              PyTuple_GET_ITEM(args_,i));
    if (ctxt_d_.valid())
      omniPy::marshalContext(stream, ctxt_d_, PyTuple_GET_ITEM(args_, i));
  }
  else {
    cdLockHolder _l(this);

    in_marshal_ = 1;
    PyUnlockingCdrStream pystream(stream);

    try {
      for (i=0; i < in_l_; i++)
        omniPy::marshalPyObject(pystream,
                                PyTuple_GET_ITEM(in_d_,i),
                                PyTuple_GET_ITEM(args_,i));
      if (ctxt_d_.valid())
        omniPy::marshalContext(pystream, ctxt_d_, PyTuple_GET_ITEM(args_, i));
    }
    catch (...) {
      in_marshal_ = 0;
      throw;
    }
    in_marshal_ = 0;
  }
}


void
omniPy::Py_omniCallDescriptor::unmarshalReturnedValues(cdrStream& stream)
{
  if (out_l_ == -1) return;  // Oneway operation

  cdLockHolder _l(this);

  if (out_l_ == 0) {
    Py_INCREF(Py_None);
    result_ = Py_None;
  }
  else {
    PyUnlockingCdrStream pystream(stream);

    if (out_l_ == 1)
      result_ = omniPy::unmarshalPyObject(pystream,
                                          PyTuple_GET_ITEM(out_d_, 0));
    else {
      result_ = PyTuple_New(out_l_);
      if (!result_.valid())
        OMNIORB_THROW(NO_MEMORY, 0,
                      (CORBA::CompletionStatus)stream.completion());

      for (int i=0; i < out_l_; i++) {
        PyTuple_SET_ITEM(result_, i,
                         omniPy::unmarshalPyObject(pystream,
                                                   PyTuple_GET_ITEM(out_d_,
                                                                    i)));
      }
    }
  }
}


void
omniPy::Py_omniCallDescriptor::userException(cdrStream&  stream,
                                             IOP_C*      iop_client,
                                             const char* repoId)
{
  CORBA::Boolean skip = 0;

  try {
    cdLockHolder _l(this);

    PyObject* d_o = 0;

    if (exc_d_ != Py_None)
      d_o = PyDict_GetItemString(exc_d_, (char*)repoId);

    if (d_o) { // class, repoId, exc name, name, descriptor, ...
      PyUserException ex(d_o);
      ex <<= stream;
      ex._raise();
    }
    else {
      // Unexpected exception. Skip the remaining data in the reply
      // and throw UNKNOWN.
      skip = 1;
      OMNIORB_THROW(UNKNOWN, UNKNOWN_UserException,
                    (CORBA::CompletionStatus)stream.completion());
    }
  }
  catch (...) {
    // The code above always throws, so this always executes.
    if (iop_client)
      iop_client->RequestCompleted(skip);
    throw;
  }
}


//
// AMI

static PyObject* pyEHClass = 0;

static inline PyObject*
getPyEHClass()
{
  if (!pyEHClass) {
    omniPy::PyRefHolder mod(PyImport_ImportModule((char*)"omniORB.ami"));
    
    if (mod.valid())
      pyEHClass = PyObject_GetAttrString(mod, (char*)"ExceptionHolderImpl");

    if (!pyEHClass) {
      if (omniORB::trace(1))
        PyErr_Print();
      else
        PyErr_Clear();
    }      
  }
  return pyEHClass;
}


void
omniPy::Py_omniCallDescriptor::completeCallback()
{
  omnipyThreadCache::lock _t;

  // If there is a poller, ensure our reference to it is released when
  // the function completes.
  PyRefHolder poller(poller_.retn());

  if (callback_.valid() && callback_.obj() != Py_None) {
    PyRefHolder method;
    PyRefHolder args;
    PyRefHolder result;

    if (!exceptionOccurred()) {
      method = PyObject_GetAttrString(callback_, (char*)op());
      if (PyTuple_Check(result_)) {
        args = result_.dup();
      }
      else {
        args = PyTuple_New(1);
        PyTuple_SET_ITEM(args, 0, result_.dup());
      }
    }
    else {
      // Exception. We need a poller.
      if (!poller.valid())
        poller = (PyObject*)PyCDObj_alloc(this);

      method = PyObject_GetAttr(callback_, excep_name_);

      PyObject* ehc = getPyEHClass();
      if (ehc) {
        PyObject* eh = PyObject_CallFunctionObjArgs(ehc, (PyObject*)poller, 0);
        if (eh) {
          args = PyTuple_New(1);
          PyTuple_SET_ITEM(args, 0, eh);
        }
      }
    }

    if (method.valid() && args.valid())
      result = PyObject_CallObject(method, args);

    if (!result.valid()) {
      if (omniORB::trace(1)) {
        omniORB::logs(1, "Exception performing AMI callback:");
        PyErr_Print();
      }
      else {
        PyErr_Clear();
      }
    }
  }

  if (!poller.valid()) {
    // No poller so this call descriptor is finished with.
    delete this;
  }
}


PyObject*
omniPy::Py_omniCallDescriptor::raisePyException()
{
  OMNIORB_ASSERT(pd_exception);

  PyUserException* ue = PyUserException::_downcast(pd_exception);
  if (ue)
    return ue->setPyExceptionState();

  Py_BAD_PARAM* bp = Py_BAD_PARAM::_downcast(pd_exception);
  if (bp)
    return handleSystemException(*bp, bp->getInfo());

  CORBA::SystemException* se = CORBA::SystemException::_downcast(pd_exception);
  if (se)
    return handleSystemException(*se);

  try {
    OMNIORB_THROW(UNKNOWN, UNKNOWN_UserException, CORBA::COMPLETED_NO);
  }
  catch (CORBA::UNKNOWN& ex) {
    return handleSystemException(ex);
  }
}


PyObject*
omniPy::Py_omniCallDescriptor::makePoller()
{
  return (PyObject*)PyCDObj_alloc(this);
}


//
// Server-side

void
omniPy::Py_localCallBackFunction(omniCallDescriptor* cd, omniServant* svnt)
{
  Py_omniCallDescriptor* pycd = (Py_omniCallDescriptor*)cd;
  Py_omniServant*        pyos =
    (Py_omniServant*)svnt->_ptrToInterface(omniPy::string_Py_omniServant);

  // We can't use the call descriptor's unlocker to re-lock, because
  // this call-back may be running in a different thread to the
  // creator of the call descriptor.

  if (cd->is_upcall()) {
    omnipyThreadCache::lock _t;
    pyos->remote_dispatch(pycd);
  }
  else {
    omnipyThreadCache::lock _t;
    pyos->local_dispatch(pycd);
  }
}


void
omniPy::Py_omniCallDescriptor::unmarshalArguments(cdrStream& stream)
{
  OMNIORB_ASSERT(!args_.valid());

  omnipyThreadCache::lock _t;

  if (ctxt_d_.valid())
    args_ = PyTuple_New(in_l_ + 1);
  else
    args_ = PyTuple_New(in_l_);


  PyUnlockingCdrStream pystream(stream);

  int i;
  for (i=0; i < in_l_; i++) {
    PyTuple_SET_ITEM(args_, i,
                     omniPy::unmarshalPyObject(pystream,
                                               PyTuple_GET_ITEM(in_d_, i)));
  }
  if (ctxt_d_.valid())
    PyTuple_SET_ITEM(args_, i, omniPy::unmarshalContext(pystream));
}

void
omniPy::Py_omniCallDescriptor::setAndValidateReturnedValues(PyObject* result)
{
  OMNIORB_ASSERT(!result_.valid());
  result_ = result;

  if (out_l_ == -1 || out_l_ == 0) {
    if (result_ != Py_None) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_MAYBE,
                         omniPy::formatString("Operation %r should return "
                                              "None, got %r",
                                              "sO", op(), result->ob_type));
    }
  }
  else if (out_l_ == 1) {
    try {
      omniPy::validateType(PyTuple_GET_ITEM(out_d_,0),
                           result,
                           CORBA::COMPLETED_MAYBE);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.add(omniPy::formatString("Operation %r return value",
                                  "s", op()));
      throw;
    }
  }
  else {
    if (!PyTuple_Check(result) || PyTuple_GET_SIZE(result) != out_l_) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_MAYBE,
                         omniPy::formatString("Operation %r should return "
                                              "%d-tuple, got %r",
                                              "siO",
                                              op(), out_l_, result->ob_type));
    }

    for (int i=0; i < out_l_; i++) {
      try {
        omniPy::validateType(PyTuple_GET_ITEM(out_d_,i),
                             PyTuple_GET_ITEM(result,i),
                             CORBA::COMPLETED_MAYBE);
      }
      catch (Py_BAD_PARAM& bp) {
        bp.add(omniPy::formatString("Operation %r return value %d",
                                    "si", op(), i));
        throw;
      }
    }
  }
}

void
omniPy::Py_omniCallDescriptor::marshalReturnedValues(cdrStream& stream)
{
  omnipyThreadCache::lock _t;
  PyUnlockingCdrStream pystream(stream);

  if (out_l_ == 1) {
    omniPy::marshalPyObject(pystream,
                            PyTuple_GET_ITEM(out_d_, 0),
                            result_);
  }
  else {
    for (int i=0; i < out_l_; i++) {
      omniPy::marshalPyObject(pystream,
                              PyTuple_GET_ITEM(out_d_,i),
                              PyTuple_GET_ITEM(result_,i));
    }
  }
}
