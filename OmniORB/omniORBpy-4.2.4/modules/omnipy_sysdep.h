// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// omnipy_sysdep.h            Created on: 2000/03/07
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2014 Apasphere Ltd
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
//    Additional system dependencies

#ifndef _omnipy_sysdep_h_
#define _omnipy_sysdep_h_


//
// Python version dependencies

#if (PY_VERSION_HEX < 0x03000000) // Python 2

#  define String_Check(o)                  PyString_Check(o)
#  define String_AsString(o)               PyString_AsString(o)
#  define String_FromString(s)             PyString_FromString(s)
#  define String_FromStringAndSize(s,l)    PyString_FromStringAndSize(s,l)
#  define String_Format(f,a)               PyString_Format(f,a)
#  define String_GET_SIZE(o)               PyString_GET_SIZE(o)
#  define String_AS_STRING(o)              PyString_AS_STRING(o)

#  define Unicode_GET_SIZE(o)              PyUnicode_GET_SIZE(o)

static inline const char*
String_AS_STRING_AND_SIZE(PyObject* obj, CORBA::ULong& size)
{
  size = PyString_GET_SIZE(obj);
  return PyString_AS_STRING(obj);
}

#  define RawString_Check(o)                PyString_Check(o)
#  define RawString_GET_SIZE(o)             PyString_GET_SIZE(o)
#  define RawString_AS_STRING(o)            PyString_AS_STRING(o)
#  define RawString_AS_STRING_AND_SIZE(o,s) String_AS_STRING_AND_SIZE(o,s)
#  define RawString_FromStringAndSize(o,s)  PyString_FromStringAndSize(o,s)
#  define RawString_FromString(s)           PyString_FromString(s)

#  define Int_Check(o)                      PyInt_Check(o)
#  define Int_FromLong(l)                   PyInt_FromLong(l)
#  define Int_AS_LONG(o)                    PyInt_AS_LONG(o)

#else // Python 3

#  if (PY_VERSION_HEX >= 0x03030000) // Python 3.3

#    define String_AsString(o)               PyUnicode_AsUTF8(o)
#    define String_GET_SIZE(o)               PyUnicode_GET_LENGTH(o)
#    define String_AS_STRING(o)              PyUnicode_AsUTF8(o)

#    define Unicode_GET_SIZE(o)              PyUnicode_GET_LENGTH(o)

static inline const char*
String_AS_STRING_AND_SIZE(PyObject* obj, CORBA::ULong& size)
{
  Py_ssize_t ss;
  const char* str = PyUnicode_AsUTF8AndSize(obj, &ss);
  size = ss;
  return str;
}

#  else // Python 3.0, 3.1, 3.2

static inline char* String_AsString(PyObject* obj)
{
  char* str;
  PyArg_Parse(obj, (char*)"s", &str);
  return str;
}

static inline const char*
String_AS_STRING_AND_SIZE(PyObject* obj, CORBA::ULong& size)
{
  char*      str;
  Py_ssize_t ss;

  PyArg_Parse(obj, (char*)"s#", &str, &ss);

  size = ss;
  return str;
}

#    define String_AS_STRING(o)            String_AsString(o)
#    define String_GET_SIZE(o)             PyUnicode_GetSize(o)

#    define Unicode_GET_SIZE(o)            PyUnicode_GET_SIZE(o)

#  endif

static inline const char*
RawString_AS_STRING_AND_SIZE(PyObject* obj, CORBA::ULong& size)
{
  size = PyBytes_GET_SIZE(obj);
  return PyBytes_AS_STRING(obj);
}

#  define String_Check(o)                  PyUnicode_Check(o)
#  define String_FromString(s)             PyUnicode_FromString(s)
#  define String_FromStringAndSize(s,l)    PyUnicode_FromStringAndSize(s,l)
#  define String_Format(f,a)               PyUnicode_Format(f,a)

#  define RawString_Check(o)               PyBytes_Check(o)
#  define RawString_GET_SIZE(o)            PyBytes_GET_SIZE(o)
#  define RawString_AS_STRING(o)           PyBytes_AS_STRING(o)
#  define RawString_FromStringAndSize(o,s) PyBytes_FromStringAndSize(o,s)
#  define RawString_FromString(s)          PyBytes_FromString(s)

#  define Int_Check(o)                     PyLong_Check(o)
#  define Int_FromLong(l)                  PyLong_FromLong(l)
#  define Int_AS_LONG(o)                   PyLong_AsLong(o)

#endif



//
// Compiler dependencies

// Defaults for things we'd like to do

#define PY_OMNISERVANT_BASE omniPy::Py_omniServant

// Some compilers will do some flow analysis and might get tricked if
// a function always throws an exception.

#ifdef NEED_DUMMY_RETURN
#  define NEED_DUMMY_THROW
#elif defined(__DECCXX)
#  define NEED_DUMMY_THROW
#endif

// Things that are broken

#if defined(_MSC_VER)
#  undef  PY_OMNISERVANT_BASE
#  define PY_OMNISERVANT_BASE Py_omniServant

#endif

#endif // _omnipy_sysdep_h_
