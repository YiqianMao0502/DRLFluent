// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pysslTP.cc                 Created on: 2002/09/06
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2002 Apasphere Ltd.
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
//    SSL transport library

#ifdef __WIN32__
#  define DLL_EXPORT _declspec(dllexport)
#else
#  define DLL_EXPORT
#endif

#if defined(__VMS)
#  include <Python.h>
#else
#  include PYTHON_INCLUDE
#endif

#include <omniORB4/CORBA.h>
#include <omniORB4/sslContext.h>

#include "../omnipy_sysdep.h"

extern "C" {

  static char certificate_authority_file_doc[] =
  "certificate_authority_file(PEM filename)\n"
  "\n"
  "Set the certificate authority file for SSL validation.\n"
  "Call with no argument to retrieve the current value.\n";

  static PyObject* pysslTP_certificate_authority_file(PyObject* self,
						      PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      if (sslContext::certificate_authority_file)
	return String_FromString(sslContext::certificate_authority_file);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    char *name;
    if (!PyArg_ParseTuple(args, (char*)"s", &name)) return 0;

    // Leak here, but we can't do anything else about it.
    sslContext::certificate_authority_file = CORBA::string_dup(name);

    Py_INCREF(Py_None); return Py_None;
  }

  static char certificate_authority_path_doc[] =
  "certificate_authority_path(path)\n"
  "\n"
  "Set the path for certificate authority files for SSL validation.\n"
  "Call with no argument to retrieve the current value.\n";

  static PyObject* pysslTP_certificate_authority_path(PyObject* self,
						      PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      if (sslContext::certificate_authority_path)
	return String_FromString(sslContext::certificate_authority_path);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    char *name;
    if (!PyArg_ParseTuple(args, (char*)"s", &name)) return 0;

    // Leak here, but we can't do anything else about it.
    sslContext::certificate_authority_path = CORBA::string_dup(name);

    Py_INCREF(Py_None); return Py_None;
  }

  static char key_file_doc[] =
  "key_file(PEM filename)\n"
  "\n"
  "Set the key file for SSL encryption.\n"
  "Call with no argument to retrieve the current value.\n";

  static PyObject* pysslTP_key_file(PyObject* self,
				    PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      if (sslContext::key_file)
	return String_FromString(sslContext::key_file);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    char *name;
    if (!PyArg_ParseTuple(args, (char*)"s", &name)) return 0;

    // Leak here, but we can't do anything else about it.
    sslContext::key_file = CORBA::string_dup(name);

    Py_INCREF(Py_None); return Py_None;
  }

  static char key_file_password_doc[] =
  "key_file_password(password string)\n"
  "\n"
  "Set the password for the key file.\n"
  "Call with no argument to retrieve the current value.\n";

  static PyObject* pysslTP_key_file_password(PyObject* self,
					     PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      if (sslContext::key_file_password)
	return String_FromString(sslContext::key_file_password);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    char *pw;
    if (!PyArg_ParseTuple(args, (char*)"s", &pw)) return 0;

    // Leak here, but we can't do anything else about it.
    sslContext::key_file_password = CORBA::string_dup(pw);

    Py_INCREF(Py_None); return Py_None;
  }


  static PyMethodDef omnisslTP_methods[] = {
    {(char*)"certificate_authority_file",
     pysslTP_certificate_authority_file, METH_VARARGS,
     certificate_authority_file_doc},

    {(char*)"certificate_authority_path",
     pysslTP_certificate_authority_path, METH_VARARGS,
     certificate_authority_path_doc},

    {(char*)"key_file",
     pysslTP_key_file, METH_VARARGS,
     key_file_doc},

    {(char*)"key_file_password",
     pysslTP_key_file_password, METH_VARARGS,
     key_file_password_doc},

    {0,0}
  };

#if (PY_VERSION_HEX < 0x03000000)

  void DLL_EXPORT init_omnisslTP()
  {
    PyObject* m = Py_InitModule((char*)"_omnisslTP", omnisslTP_methods);
  }

#else

  static struct PyModuleDef omnisslTPmodule = {
    PyModuleDef_HEAD_INIT,
    "_omnisslTP",
    "omniORBpy SSL transport",
    -1,
    omnisslTP_methods,
    NULL,
    NULL,
    NULL,
    NULL
  };

  PyMODINIT_FUNC
  PyInit__omnisslTP(void)
  {
    return PyModule_Create(&omnisslTPmodule);
  }

#endif
};
