// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyInterceptors.cc          Created on: 2003/05/27
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2003-2012 Apasphere Ltd.
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
//    Python interceptors

#include <omnipy.h>
#include <pyThreadCache.h>
#include <omniORB4/omniInterceptors.h>
#include <giopStream.h>
#include <giopStrand.h>
#include <giopRope.h>
#include <GIOP_S.h>
#include <GIOP_C.h>


OMNI_USING_NAMESPACE(omni)


// Python lists of interceptor functions
static PyObject* clientSendRequestFns         = 0;
static PyObject* clientReceiveReplyFns        = 0;
static PyObject* clientReceiveReplyCredsFns   = 0;
static PyObject* serverReceiveRequestFns      = 0;
static PyObject* serverReceiveRequestCredsFns = 0;
static PyObject* serverSendReplyFns           = 0;
static PyObject* serverSendExceptionFns       = 0;
static PyObject* assignUpcallThreadFns        = 0;
static PyObject* assignAMIThreadFns           = 0;


static
void
callInterceptorsAndSetContexts(PyObject*                fnlist,
			       const char*              opname,
			       const char*              exrepoid,
			       IOP::ServiceContextList& service_contexts,
			       CORBA::CompletionStatus  completion)
{
  omniPy::PyRefHolder argtuple(PyTuple_New(exrepoid ? 3 : 2));

  PyObject* ctxtlist = PyList_New(0);
  PyTuple_SetItem(argtuple, 0, String_FromString(opname));
  PyTuple_SetItem(argtuple, 1, ctxtlist);

  if (exrepoid)
    PyTuple_SetItem(argtuple, 2, String_FromString(exrepoid));

  CORBA::ULong sclen = service_contexts.length();
  CORBA::ULong sci   = sclen;

  try {
    for (int i=0; i < PyList_GET_SIZE(fnlist); i++) {
      PyObject* interceptor = PyList_GET_ITEM(fnlist, i);
      PyObject* result      = PyObject_CallObject(interceptor, argtuple);

      if (!result) {
	omniPy::handlePythonException();
      }
      if (result != Py_None) {
	Py_DECREF(result);
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, completion);
      }
      Py_DECREF(result);

      if (PyList_GET_SIZE(ctxtlist) > 0) {
	sclen += PyList_GET_SIZE(ctxtlist);
	service_contexts.length(sclen);

	for (int j=0; j < PyList_GET_SIZE(ctxtlist); j++, sci++) {
	  PyObject* sc = PyList_GET_ITEM(ctxtlist, j);

	  if (!PyTuple_Check(sc) || PyTuple_GET_SIZE(sc) != 2) {
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, completion);
	  }
	  service_contexts[sci].context_id =
	    omniPy::getULongVal(PyTuple_GET_ITEM(sc, 0), completion);

	  PyObject* data = PyTuple_GET_ITEM(sc, 1);

	  if (!RawString_Check(data))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, completion);

          CORBA::ULong size;
          const char*  str = RawString_AS_STRING_AND_SIZE(data, size);

	  service_contexts[sci].context_data.length(size);

	  memcpy(service_contexts[sci].context_data.NP_data(), str, size);
	}
	PyList_SetSlice(ctxtlist, 0, PyList_GET_SIZE(ctxtlist), 0);
      }
    }
  }
  catch (Py_BAD_PARAM& bp) {
    bp.logInfoAndThrow();
  }
}

static
void
getContextsAndCallInterceptors(PyObject*                fnlist,
                               const char*              opname,
                               int                      pass_peer_info,
                               const char*              peer_address,
                               const char*              peer_identity,
                               IOP::ServiceContextList& service_contexts,
                               CORBA::CompletionStatus  completion)
{
  int i;
  int sclen = service_contexts.length();

  omniPy::PyRefHolder argtuple(PyTuple_New(pass_peer_info ? 3 : 2));

  PyObject* sctuple = PyTuple_New(sclen);
  PyTuple_SET_ITEM(argtuple, 0, String_FromString(opname));
  PyTuple_SET_ITEM(argtuple, 1, sctuple);

  if (pass_peer_info) {
    PyObject* peer_info = PyDict_New();
    PyObject* value;
    if (peer_address) {
      value = String_FromString(peer_address);
    }
    else {
      Py_INCREF(Py_None);
      value = Py_None;
    }
    PyDict_SetItemString(peer_info, "address", value);
    Py_DECREF(value);

    if (peer_identity) {
      value = String_FromString(peer_identity);
    }
    else {
      Py_INCREF(Py_None);
      value = Py_None;
    }
    PyDict_SetItemString(peer_info, "identity", value);
    Py_DECREF(value);

    PyTuple_SET_ITEM(argtuple, 2, peer_info);
  }

  for (i=0; i < sclen; i++) {
    PyObject* sc = PyTuple_New(2);
    PyTuple_SET_ITEM(sc, 0,
		     PyLong_FromUnsignedLong(service_contexts[i].context_id));

    const char* data = (const char*)service_contexts[i].context_data.NP_data();
    int len = service_contexts[i].context_data.length();
    
    PyTuple_SET_ITEM(sc, 1, RawString_FromStringAndSize(data, len));
    PyTuple_SET_ITEM(sctuple, i, sc);
  }

  try {
    for (i=0; i < PyList_GET_SIZE(fnlist); i++) {
      PyObject* interceptor = PyList_GET_ITEM(fnlist, i);
      PyObject* result      = PyObject_CallObject(interceptor, argtuple);

      if (!result) {
	omniPy::handlePythonException();
      }
      if (result != Py_None) {
	Py_DECREF(result);
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, completion);
      }
      Py_DECREF(result);
    }
  }
  catch (Py_BAD_PARAM& bp) {
    bp.logInfoAndThrow();
  }
}


static
CORBA::Boolean
pyClientSendRequestFn(omniInterceptors::clientSendRequest_T::info_T& info)
{
  OMNIORB_ASSERT(clientSendRequestFns);

  omnipyThreadCache::lock _t;

  callInterceptorsAndSetContexts(clientSendRequestFns,
				 info.giop_c.operation(),
                                 0,
				 info.service_contexts,
				 CORBA::COMPLETED_NO);

  return 1;
}

static
CORBA::Boolean
pyClientReceiveReplyFn(omniInterceptors::clientReceiveReply_T::info_T& info)
{
  OMNIORB_ASSERT(clientReceiveReplyFns);

  omnipyThreadCache::lock _t;

  if (PyList_Size(clientReceiveReplyFns)) {

    getContextsAndCallInterceptors(clientReceiveReplyFns,
				   info.giop_c.operation(),
				   0, 0, 0,
				   info.service_contexts,
				   (CORBA::CompletionStatus)
				   info.giop_c.completion());
  }

  if (PyList_Size(clientReceiveReplyCredsFns)) {

    giopStrand&     strand     = info.giop_c.strand();
    giopConnection* connection = strand.connection;
    const char*     address    = connection->peeraddress();
    const char*     identity   = connection->peeridentity();

    getContextsAndCallInterceptors(clientReceiveReplyCredsFns,
				   info.giop_c.operation(),
				   1, address, identity,
				   info.service_contexts,
				   (CORBA::CompletionStatus)
				   info.giop_c.completion());
  }
  return 1;
}

static
CORBA::Boolean
pyServerReceiveRequestFn(omniInterceptors::
			 serverReceiveRequest_T::info_T& info)
{
  OMNIORB_ASSERT(serverReceiveRequestFns);

  omnipyThreadCache::lock _t;

  if (PyList_Size(serverReceiveRequestFns)) {
    getContextsAndCallInterceptors(serverReceiveRequestFns,
				   info.giop_s.operation(),
				   0, 0, 0,
				   info.giop_s.service_contexts(),
				   (CORBA::CompletionStatus)
				   info.giop_s.completion());
  }

  if (PyList_Size(serverReceiveRequestCredsFns)) {

    giopStrand&     strand     = info.giop_s.strand();
    giopConnection* connection = strand.connection;
    const char*     address    = connection->peeraddress();
    const char*     identity   = connection->peeridentity();

    getContextsAndCallInterceptors(serverReceiveRequestCredsFns,
				   info.giop_s.operation(),
				   1, address, identity,
				   info.giop_s.service_contexts(),
				   (CORBA::CompletionStatus)
				   info.giop_s.completion());
  }
  return 1;
}

static
CORBA::Boolean
pyServerSendReplyFn(omniInterceptors::serverSendReply_T::info_T& info)
{
  OMNIORB_ASSERT(serverSendReplyFns);

  omnipyThreadCache::lock _t;

  callInterceptorsAndSetContexts(serverSendReplyFns,
				 info.giop_s.operation(), 0,
				 info.giop_s.service_contexts(),
				 (CORBA::CompletionStatus)
				 info.giop_s.completion());

  return 1;
}

static
CORBA::Boolean
pyServerSendExceptionFn(omniInterceptors::serverSendException_T::info_T& info)
{
  OMNIORB_ASSERT(serverSendExceptionFns);

  omnipyThreadCache::lock _t;

  callInterceptorsAndSetContexts(serverSendExceptionFns,
				 info.giop_s.operation(),
				 info.exception->_rep_id(),
				 info.giop_s.service_contexts(),
				 (CORBA::CompletionStatus)
				 info.giop_s.completion());
  return 1;
}


template<class infoT>
static
void
assignThreadFn(infoT& info, PyObject* fns)
{
  OMNIORB_ASSERT(fns);

  omnipyThreadCache::lock _t;

  omniPy::PyRefHolder post_list(PyList_New(0));

  int i;
  for (i=0; i < PyList_GET_SIZE(fns); ++i) {
    PyObject* interceptor = PyList_GET_ITEM(fns, i);
    PyObject* result      = PyObject_CallObject(interceptor, 0);

    if (!result)
      omniPy::handlePythonException();
    
    if (result == Py_None) {
      // Simple function
      Py_DECREF(result);
    }
    else {
      if (!PyIter_Check(result))
        OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);

      // A generator function. Call next() on it once
      PyList_Append(post_list, result);

      result = PyIter_Next(result);
      
      if (!result) {
        if (PyErr_Occurred())
          omniPy::handlePythonException();

        // The iterator terminated too soon
        OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
      }
      Py_DECREF(result);
    }
  }
  {
    omniPy::InterpreterUnlocker _u;
    info.run();
  }

  // Reverse-iterate over functions
  for (i = PyList_GET_SIZE(post_list) - 1; i >= 0; --i) {

    PyObject* gen    = PyList_GET_ITEM(post_list, i);
    PyObject* result = PyIter_Next(gen);

    if (result) {
      // Not expecting this -- next() should have raised StopIteration
      Py_DECREF(result);
    }
    else {
      // If an error occurred, we just swallow it.
      if (PyErr_Occurred())
        PyErr_Clear();
    }
  }
}

static
void
pyAssignUpcallThreadFn(omniInterceptors::assignUpcallThread_T::info_T& info)
{
  assignThreadFn(info, assignUpcallThreadFns);
}

static
void
pyAssignAMIThreadFn(omniInterceptors::assignAMIThread_T::info_T& info)
{
  assignThreadFn(info, assignAMIThreadFns);
}


void
omniPy::
registerInterceptors()
{
  omniInterceptors* interceptors = omniORB::getInterceptors();

  if (clientSendRequestFns)
    interceptors->clientSendRequest.add(pyClientSendRequestFn);

  if (clientReceiveReplyFns || clientReceiveReplyCredsFns)
    interceptors->clientReceiveReply.add(pyClientReceiveReplyFn);

  if (serverReceiveRequestFns || serverReceiveRequestCredsFns)
    interceptors->serverReceiveRequest.add(pyServerReceiveRequestFn);

  if (serverSendReplyFns)
    interceptors->serverSendReply.add(pyServerSendReplyFn);

  if (serverSendExceptionFns)
    interceptors->serverSendException.add(pyServerSendExceptionFn);

  if (assignUpcallThreadFns)
    interceptors->assignUpcallThread.add(pyAssignUpcallThreadFn);

  if (assignAMIThreadFns)
    interceptors->assignAMIThread.add(pyAssignAMIThreadFn);
}


#define CHECK_ORB_NOT_INITIALISED() \
  do { \
    if (omniPy::orb) { \
      CORBA::BAD_INV_ORDER _ex(BAD_INV_ORDER_InvalidPortableInterceptorCall, \
                               CORBA::COMPLETED_NO); \
      return omniPy::handleSystemException(_ex); \
    } \
  } while(0)


extern "C" {
  static char addClientSendRequest_doc [] =
  "addClientSendRequest(interceptor) -> None\n"
  "\n"
  "Install an interceptor for when clients send requests.\n";

  static PyObject* pyInterceptor_addClientSendRequest(PyObject* self,
						      PyObject* args)
  {
    PyObject* interceptor;

    if (!PyArg_ParseTuple(args, (char*)"O", &interceptor))
      return 0;

    RAISE_PY_BAD_PARAM_IF(!PyCallable_Check(interceptor),
			  BAD_PARAM_WrongPythonType);

    CHECK_ORB_NOT_INITIALISED();

    if (!clientSendRequestFns)
      clientSendRequestFns = PyList_New(0);

    PyList_Append(clientSendRequestFns, interceptor);
    Py_INCREF(Py_None);
    return Py_None;
  }

  static char addClientReceiveReply_doc [] =
  "addClientReceiveReply(interceptor) -> None\n"
  "\n"
  "Install an interceptor for when clients receive replies.\n";

  static PyObject* pyInterceptor_addClientReceiveReply(PyObject* self,
						       PyObject* args)
  {
    PyObject* interceptor;

    int pass_creds = 0;
    if (!PyArg_ParseTuple(args, (char*)"O|i", &interceptor, &pass_creds))
      return 0;

    RAISE_PY_BAD_PARAM_IF(!PyCallable_Check(interceptor),
			  BAD_PARAM_WrongPythonType);

    CHECK_ORB_NOT_INITIALISED();

    if (!clientReceiveReplyFns) {
      clientReceiveReplyFns      = PyList_New(0);
      clientReceiveReplyCredsFns = PyList_New(0);
    }

    if (pass_creds)
      PyList_Append(clientReceiveReplyCredsFns, interceptor);
    else
      PyList_Append(clientReceiveReplyFns, interceptor);

    Py_INCREF(Py_None);
    return Py_None;
  }

  static char addServerReceiveRequest_doc [] =
  "addServerReceiveRequest(interceptor) -> None\n"
  "\n"
  "Install an interceptor for when servers receive requests.\n";

  static PyObject* pyInterceptor_addServerReceiveRequest(PyObject* self,
							 PyObject* args)
  {
    PyObject* interceptor;

    int pass_creds = 0;
    if (!PyArg_ParseTuple(args, (char*)"O|i", &interceptor, &pass_creds))
      return 0;

    RAISE_PY_BAD_PARAM_IF(!PyCallable_Check(interceptor),
			  BAD_PARAM_WrongPythonType);

    CHECK_ORB_NOT_INITIALISED();

    if (!serverReceiveRequestFns) {
      serverReceiveRequestFns      = PyList_New(0);
      serverReceiveRequestCredsFns = PyList_New(0);
    }

    if (pass_creds)
      PyList_Append(serverReceiveRequestCredsFns, interceptor);
    else
      PyList_Append(serverReceiveRequestFns, interceptor);

    Py_INCREF(Py_None);
    return Py_None;
  }

  static char addServerSendReply_doc [] =
  "addServerSendReply(interceptor) -> None\n"
  "\n"
  "Install an interceptor for when servers send replies.\n";

  static PyObject* pyInterceptor_addServerSendReply(PyObject* self,
						    PyObject* args)
  {
    PyObject* interceptor;

    if (!PyArg_ParseTuple(args, (char*)"O", &interceptor))
      return 0;

    RAISE_PY_BAD_PARAM_IF(!PyCallable_Check(interceptor),
			  BAD_PARAM_WrongPythonType);

    CHECK_ORB_NOT_INITIALISED();

    if (!serverSendReplyFns)
      serverSendReplyFns = PyList_New(0);

    PyList_Append(serverSendReplyFns, interceptor);
    Py_INCREF(Py_None);
    return Py_None;
  }

  static char addServerSendException_doc [] =
  "addServerSendException(interceptor) -> None\n"
  "\n"
  "Install an interceptor for when servers send exceptions.\n";

  static PyObject* pyInterceptor_addServerSendException(PyObject* self,
							PyObject* args)
  {
    PyObject* interceptor;

    if (!PyArg_ParseTuple(args, (char*)"O", &interceptor))
      return 0;

    RAISE_PY_BAD_PARAM_IF(!PyCallable_Check(interceptor),
			  BAD_PARAM_WrongPythonType);

    CHECK_ORB_NOT_INITIALISED();

    if (!serverSendExceptionFns)
      serverSendExceptionFns = PyList_New(0);

    PyList_Append(serverSendExceptionFns, interceptor);
    Py_INCREF(Py_None);
    return Py_None;
  }

  static char addAssignUpcallThread_doc [] =
  "addAssignUpcallThread(interceptor) -> None\n"
  "\n"
  "Install an interceptor for when a thread is assigned to perform upcalls.\n";

  static PyObject* pyInterceptor_addAssignUpcallThread(PyObject* self,
                                                       PyObject* args)
  {
    PyObject* interceptor;

    if (!PyArg_ParseTuple(args, (char*)"O", &interceptor))
      return 0;

    RAISE_PY_BAD_PARAM_IF(!PyCallable_Check(interceptor),
			  BAD_PARAM_WrongPythonType);

    CHECK_ORB_NOT_INITIALISED();

    if (!assignUpcallThreadFns)
      assignUpcallThreadFns = PyList_New(0);

    PyList_Append(assignUpcallThreadFns, interceptor);
    Py_INCREF(Py_None);
    return Py_None;
  }

  static char addAssignAMIThread_doc [] =
  "addAssignAMIThread(interceptor) -> None\n"
  "\n"
  "Install an interceptor for when a thread is assigned to perform AMI calls.\n";

  static PyObject* pyInterceptor_addAssignAMIThread(PyObject* self,
                                                    PyObject* args)
  {
    PyObject* interceptor;

    if (!PyArg_ParseTuple(args, (char*)"O", &interceptor))
      return 0;

    RAISE_PY_BAD_PARAM_IF(!PyCallable_Check(interceptor),
			  BAD_PARAM_WrongPythonType);

    CHECK_ORB_NOT_INITIALISED();

    if (!assignAMIThreadFns)
      assignAMIThreadFns = PyList_New(0);

    PyList_Append(assignAMIThreadFns, interceptor);
    Py_INCREF(Py_None);
    return Py_None;
  }


  static PyMethodDef pyInterceptor_methods[] = {
    {(char*)"addClientSendRequest",
     pyInterceptor_addClientSendRequest,
     METH_VARARGS,
     addClientSendRequest_doc},

    {(char*)"addClientReceiveReply",
     pyInterceptor_addClientReceiveReply,
     METH_VARARGS,
     addClientReceiveReply_doc},

    {(char*)"addServerReceiveRequest",
     pyInterceptor_addServerReceiveRequest,
     METH_VARARGS,
     addServerReceiveRequest_doc},

    {(char*)"addServerSendReply",
     pyInterceptor_addServerSendReply,
     METH_VARARGS,
     addServerSendReply_doc},

    {(char*)"addServerSendException",
     pyInterceptor_addServerSendException,
     METH_VARARGS,
     addServerSendException_doc},

    {(char*)"addAssignUpcallThread",
     pyInterceptor_addAssignUpcallThread,
     METH_VARARGS,
     addAssignUpcallThread_doc},

    {(char*)"addAssignAMIThread",
     pyInterceptor_addAssignAMIThread,
     METH_VARARGS,
     addAssignAMIThread_doc},

    {NULL,NULL}
  };
}

#if (PY_VERSION_HEX < 0x03000000)

void
omniPy::initInterceptorFunc(PyObject* d)
{
  PyObject* m = Py_InitModule((char*)"_omnipy.interceptor_func",
			      pyInterceptor_methods);
  PyDict_SetItemString(d, (char*)"interceptor_func", m);
}

#else

 static struct PyModuleDef interceptor_func_module = {
   PyModuleDef_HEAD_INIT,
   "_omnipy.interceptor_func",
   "omniORB Interceptor API",
   -1,
   pyInterceptor_methods,
   NULL,
   NULL,
   NULL,
   NULL
 };

void
omniPy::initInterceptorFunc(PyObject* d)
{
  PyObject* m = PyModule_Create(&interceptor_func_module);
  if (!m)
    return;

  PyDict_SetItemString(d, (char*)"interceptor_func", m);
}

#endif
