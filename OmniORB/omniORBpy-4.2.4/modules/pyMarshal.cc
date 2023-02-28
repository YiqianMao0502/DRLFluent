// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyMarshal.cc               Created on: 1999/07/05
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2014 Apasphere Ltd
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
//    Marshalling / unmarshalling of Python objects

#include <omnipy.h>
#include <pyFixed.h>
#include <codeSetUtil.h>

OMNI_USING_NAMESPACE(omni)


// Small function to indicate whether a descriptor represents a type
// for which we have unrolled sequence marshalling code
static inline CORBA::Boolean
sequenceOptimisedType(PyObject* desc, CORBA::ULong& tk)
{
  static CORBA::Boolean optmap[] = {
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0
  };

#if (PY_VERSION_HEX < 0x03000000)

  if (PyInt_Check(desc)) {
    tk = PyInt_AS_LONG(desc);
    OMNIORB_ASSERT(tk <= 33);
    return optmap[tk];
  }

#else

  if (PyLong_Check(desc)) {
    tk = PyLong_AsLong(desc);
    OMNIORB_ASSERT(tk <= 33);
    return optmap[tk];
  }

#endif

  return 0;
}
	


static void
validateTypeNull(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus,
		 PyObject* track)
{
  if (a_o != Py_None)
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting None, got %r",
					    "O", a_o->ob_type));
}

static void
validateTypeVoid(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus,
		 PyObject* track)
{
  if (a_o != Py_None)
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting None, got %r",
					    "O", a_o->ob_type));
}

static void
validateTypeShort(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{
  long l = 0;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o)) {
    l = PyInt_AS_LONG(a_o);
  }
  else
#endif
  if (PyLong_Check(a_o)) {
    l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for short",
					      "O", a_o));
    }
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting short, got %r",
					    "O", a_o->ob_type));
  }

  if (l < -0x8000 || l > 0x7fff) {
    THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
		       omniPy::formatString("%s is out of range for short",
					    "O", a_o));
  }
}

static void
validateTypeLong(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus,
		 PyObject* track)
{
  long l = 0;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o)) {
    l = PyInt_AS_LONG(a_o);
  }
  else
#endif
  if (PyLong_Check(a_o)) {
    l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for long",
					      "O", a_o));
    }
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting long, got %r",
					    "O", a_o->ob_type));
  }

#if SIZEOF_LONG > 4
  if (l < -0x80000000L || l > 0x7fffffffL) {
    THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
		       omniPy::formatString("%s is out of range for long",
					    "O", a_o));
  }
#endif
}

static void
validateTypeUShort(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{
  long l = 0;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o)) {
    l = PyInt_AS_LONG(a_o);
  }
  else
#endif
  if (PyLong_Check(a_o)) {
    l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned short",
					      "O", a_o));
    }
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting unsigned short, got %r",
					    "O", a_o->ob_type));
  }

  if (l < 0 || l > 0xffff) {
    THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
		       omniPy::formatString("%s is out of range for "
					    "unsigned short",
					    "O", a_o));
  }
}

static void
validateTypeULong(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{
  if (PyLong_Check(a_o)) {
    unsigned long ul = PyLong_AsUnsignedLong(a_o);
    if (ul == (unsigned long)-1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long",
					      "O", a_o));
    }
#if SIZEOF_LONG > 4
    if (ul > 0xffffffffL) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long",
					      "O", a_o));
    }
#endif
  }
#if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
#  if SIZEOF_LONG > 4
    if (l < 0 || l > 0xffffffffL) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long",
					      "O", a_o));
    }
#  else
    if (l < 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long",
					      "O", a_o));
    }
#  endif
  }
#endif
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting unsigned long, got %r",
					    "O", a_o->ob_type));
  }
}

static void
validateTypeFloat(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{
#if (PY_VERSION_HEX < 0x03000000)
  if (PyFloat_Check(a_o) || PyInt_Check(a_o))
    return;
#else
  if (PyFloat_Check(a_o))
    return;
#endif

  if (PyLong_Check(a_o)) {
    double d = PyLong_AsDouble(a_o);
    if (d == -1.0 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for float",
					      "O", a_o));
    }
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting float, got %r",
					    "O", a_o->ob_type));
  }
}

static void
validateTypeDouble(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{
#if (PY_VERSION_HEX < 0x03000000)
  if (PyFloat_Check(a_o) || PyInt_Check(a_o))
    return;
#else
  if (PyFloat_Check(a_o))
    return;
#endif

  if (PyLong_Check(a_o)) {
    double d = PyLong_AsDouble(a_o);
    if (d == -1.0 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for double",
					      "O", a_o));
    }
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting double, got %r",
					    "O", a_o->ob_type));
  }
}

static void
validateTypeBoolean(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus,
		    PyObject* track)
{
  if (PyObject_IsTrue(a_o) == -1) {
    if (omniORB::trace(1))
      PyErr_Print();
    else
      PyErr_Clear();

    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting bool, got %r",
					    "O", a_o->ob_type));
  }
}

static void
validateTypeChar(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus,
		 PyObject* track)
{
  if (!String_Check(a_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting string, got %r",
					    "O", a_o->ob_type));
  }
  if (String_GET_SIZE(a_o) != 1) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting string of length 1, "
					    "got %r",
					    "O", a_o));
  }

#if (PY_VERSION_HEX >= 0x03030000) // Python 3.3 or later
  Py_UCS4 uc = PyUnicode_READ_CHAR(a_o, 0);
  if (uc > 255)
    OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_CannotMapChar, compstatus);

#elif (PY_VERSION_HEX >= 0x03000000) // Python 3.0 - 3.2
  Py_UNICODE* us = PyUnicode_AS_UNICODE(a_o);
  if (*us > 255)
    OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_CannotMapChar, compstatus);
#endif
}

static void
validateTypeOctet(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{
  long l = 0;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o)) {
    l = PyInt_AS_LONG(a_o);
  }
  else
#endif
  if (PyLong_Check(a_o)) {
    l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for octet",
					      "O", a_o));
    }
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting octet, got %r",
					    "O", a_o->ob_type));
  }

  if (l < 0 || l > 0xff) {
    THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
		       omniPy::formatString("%s is out of range for octet",
					    "O", a_o));
  }
}

static void
validateTypeAny(PyObject* d_o, PyObject* a_o,
		CORBA::CompletionStatus compstatus,
		PyObject* track)
{
  if (!PyObject_IsInstance(a_o, omniPy::pyCORBAAnyClass)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting Any, got %r",
					    "O", a_o->ob_type));
  }

  // Validate TypeCode
  omniPy::PyRefHolder t_o(PyObject_GetAttrString(a_o, (char*)"_t"));

  if (!t_o.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       String_FromString("Any has no TypeCode _t"));
  }

  if (!PyObject_IsInstance(t_o, omniPy::pyCORBATypeCodeClass)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting TypeCode in Any, got %r",
					    "O", a_o->ob_type));
  }

  omniPy::PyRefHolder desc(PyObject_GetAttrString(t_o, (char*)"_d"));
  if (!desc.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       String_FromString("TypeCode in Any has no "
                                         "descriptor _d"));
  }

  // Any's contents
  t_o = PyObject_GetAttrString(a_o, (char*)"_v");
  if (!t_o.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       String_FromString("Any has no value _v"));
  }
  try {
    omniPy::validateType(desc, t_o, compstatus, track);
  }
  catch (Py_BAD_PARAM& bp) {
    bp.add(String_FromString("Value inside Any"));
    throw;
  }
}


static void
validateTypeTypeCode(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus,
		     PyObject* track)
{
  if (!PyObject_IsInstance(a_o, omniPy::pyCORBATypeCodeClass)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting TypeCode, got %r",
					    "O", a_o->ob_type));
  }
  PyObject* t_o = PyObject_GetAttrString(a_o, (char*)"_d");

  if (!t_o) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       String_FromString("TypeCode in has no descriptor _d"));
  }

  Py_DECREF(t_o);
}

static void
validateTypePrincipal(PyObject* d_o, PyObject* a_o,
		      CORBA::CompletionStatus compstatus,
		      PyObject* track)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
}


static void
validateTypeObjref(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{ // repoId, name
  if (a_o != Py_None) {
    CORBA::Object_ptr obj = omniPy::getObjRef(a_o);
    if (!obj) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting object reference, "
					      "got %r",
					      "O", a_o->ob_type));
    }
  }
}

static void
validateTypeStruct(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{ // class, repoId, struct name, name, descriptor, ...

  // The descriptor tuple has twice the number of struct members,
  // plus 4 -- the typecode kind, the Python class, the repoId,
  // and the struct name
  int                 i, j;
  int                 cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;
  PyObject*           name;
  omniPy::PyRefHolder value;

  for (i=0,j=4; i < cnt; i++,j++) {
    name = PyTuple_GET_ITEM(d_o, j++);
    OMNIORB_ASSERT(String_Check(name));

    value = PyObject_GetAttr(a_o, name);

    if (!value.valid()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Struct %r instance %r "
					      "has no %r member",
					      "OOO",
					      PyTuple_GET_ITEM(d_o, 3),
					      a_o->ob_type,
					      name));
    }
    try {
      omniPy::validateType(PyTuple_GET_ITEM(d_o, j), value, compstatus, track);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.add(omniPy::formatString("Struct %r member %r", "OO",
				  PyTuple_GET_ITEM(d_o, 3), name));
      throw;
    }
  }
}

static void
validateTypeUnion(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{ // class,
  // repoId,
  // name,
  // discriminant descr,
  // default used,
  // ((label value, member name, member descr), ...),
  // default (label, name, descr) or None,
  // {label: (label, name, descr), ...}

  omniPy::PyRefHolder discriminant(PyObject_GetAttrString(a_o, (char*)"_d"));
  if (!discriminant.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting union, got %r",
					    "O", a_o->ob_type));
  }

  omniPy::PyRefHolder value(PyObject_GetAttrString(a_o, (char*)"_v"));
  if (!value.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting union, got %r",
					    "O", a_o->ob_type));
  }

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 4); // Discriminant descriptor
  try {
    omniPy::validateType(t_o, discriminant, compstatus, track);
  }
  catch (Py_BAD_PARAM& bp) {
    bp.add(String_FromString("Union discriminant"));
    throw;
  }

  PyObject* cdict = PyTuple_GET_ITEM(d_o, 8);
  OMNIORB_ASSERT(PyDict_Check(cdict));

  t_o = PyDict_GetItem(cdict, discriminant);

  if (!t_o) {
    // Discriminant not found in case dictionary. Is there a default case?
    t_o = PyTuple_GET_ITEM(d_o, 7);
    if (t_o == Py_None)
      t_o = 0;
  }
  if (t_o) {
    OMNIORB_ASSERT(PyTuple_Check(t_o));
    try {
      omniPy::validateType(PyTuple_GET_ITEM(t_o, 2), value, compstatus, track);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.add(omniPy::formatString("Union member %r", "O",
				  PyTuple_GET_ITEM(t_o, 1)));
      throw;
    }
  }
}


static void
validateTypeEnum(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus,
		 PyObject* track)
{ // repoId, name, item list

  omniPy::PyRefHolder ev(PyObject_GetAttrString(a_o, (char*)"_v"));

  if (!ev.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting enum %r item, got %r",
					    "OO",
					    PyTuple_GET_ITEM(d_o, 2),
					    a_o->ob_type));
  }

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 3);

  CORBA::ULong e;
  try {
    e = omniPy::getULongVal(ev);
  }
  catch (Py_BAD_PARAM& bp) {
    bp.add(omniPy::formatString("Expecting enum %r item, got %r",
                                "OO",
                                PyTuple_GET_ITEM(d_o, 2),
                                a_o));
    throw;
  }

  if (e >= PyTuple_GET_SIZE(t_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_EnumValueOutOfRange, compstatus,
		       omniPy::formatString("Expecting enum %r item, got %r",
					    "OO",
					    PyTuple_GET_ITEM(d_o, 2),
					    a_o));
  }

  if (PyTuple_GET_ITEM(t_o, e) != a_o) {
    // EnumItem object is not the one we expected -- are they equivalent?
    int cmp;

#if (PY_VERSION_HEX < 0x03000000)

    if (PyObject_Cmp(PyTuple_GET_ITEM(t_o, e), a_o, &cmp) == -1)
      omniPy::handlePythonException();

    if (cmp != 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting enum %r item, "
					      "got %r",
					      "OO",
					      PyTuple_GET_ITEM(d_o, 2),
					      a_o));
    }

#else
    cmp = PyObject_RichCompareBool(PyTuple_GET_ITEM(t_o, e), a_o, Py_EQ);
    if (cmp == -1)
      omniPy::handlePythonException();

    if (cmp != 1) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting enum %r item, "
					      "got %r",
					      "OO",
					      PyTuple_GET_ITEM(d_o, 2),
					      a_o));
    }
#endif
  }
}

static void
validateTypeString(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{ // max_length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);
  OMNIORB_ASSERT(Int_Check(t_o));

  CORBA::ULong max_len = Int_AS_LONG(t_o);

  if (!String_Check(a_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting string, got %r",
					    "O", a_o->ob_type));
  }

  CORBA::ULong len;
  const char*  str = String_AS_STRING_AND_SIZE(a_o, len);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, compstatus);

  // Annoyingly, we have to scan the string to check there are no
  // nulls
  for (CORBA::ULong i=0; i<len; i++) {
    if (str[i] == '\0') {
      THROW_PY_BAD_PARAM(BAD_PARAM_EmbeddedNullInPythonString, compstatus,
			 omniPy::formatString("Embedded null in string "
					      "at position %d",
					      "i", i));
    }
  }
}


static inline
PyObject* listGet(PyObject* lst, CORBA::ULong idx)
{
  return PyList_GET_ITEM(lst, idx);
}

static inline
PyObject* tupleGet(PyObject* lst, CORBA::ULong idx)
{
  return PyTuple_GET_ITEM(lst, idx);
}

template<class G>
static inline void
validateOptSequenceItems(CORBA::ULong            len,
                         PyObject*               a_o,
                         CORBA::ULong            etk,
                         CORBA::CompletionStatus compstatus,
                         const char*             seq_arr,
                         G                       getFn)
{
  CORBA::ULong  i;
  PyObject*     t_o;
  long          long_val   = 0;
  unsigned long ulong_val  = 0;
  double        double_val = 0.0;;

  switch (etk) {

  case CORBA::tk_short:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
      if (PyInt_Check(t_o)) {
        long_val = PyInt_AS_LONG(t_o);
      }
      else
#endif
      if (PyLong_Check(t_o)) {
        long_val = PyLong_AsLong(t_o);
        if (long_val == -1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "short", "siO",
                                                  seq_arr, i, t_o));
        }
      }
      else {
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting short, "
                                                "got %r", "siO",
                                                seq_arr, i, t_o->ob_type));
      }

      if (long_val < -0x8000 || long_val > 0x7fff) {
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "short", "siO",
                                                seq_arr, i, t_o));
      }
    }
    return;

  case CORBA::tk_long:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
      if (PyInt_Check(t_o)) {
        long_val = PyInt_AS_LONG(t_o);
      }
      else
#endif
      if (PyLong_Check(t_o)) {
        long_val = PyLong_AsLong(t_o);
        if (long_val == -1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "long", "siO",
                                                  seq_arr, i, t_o));
        }
      }
      else {
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting long, "
                                                "got %r", "siO",
                                                seq_arr, i, t_o->ob_type));
      }

#if SIZEOF_LONG > 4
      if (long_val < -0x80000000L || long_val > 0x7fffffffL) {
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "long", "siO",
                                                seq_arr, i, t_o));
      }
#endif
    }
    return;

  case CORBA::tk_ushort:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
      if (PyInt_Check(t_o)) {
        long_val = PyInt_AS_LONG(t_o);
      }
      else
#endif
      if (PyLong_Check(t_o)) {
        long_val = PyLong_AsLong(t_o);
        if (long_val == -1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "unsigned short", "siO",
                                                  seq_arr, i, t_o));
        }
      }
      else {
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting unsigned short, "
                                                "got %r","siO",
                                                seq_arr, i, t_o->ob_type));
      }

      if (long_val < 0 || long_val > 0xffff) {
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "unsigned short", "siO",
                                                seq_arr, i, t_o));
      }
    }
    return;

  case CORBA::tk_ulong:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

      if (PyLong_Check(t_o)) {
        ulong_val = PyLong_AsUnsignedLong(t_o);
        if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "unsigned long", "siO",
                                                  seq_arr, i, t_o));
        }
#if SIZEOF_LONG > 4
        if (ulong_val > 0xffffffffL) {
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "unsigned long", "siO",
                                                  seq_arr, i, t_o));
        }
#endif
      }
#if (PY_VERSION_HEX < 0x03000000)
      else if (PyInt_Check(t_o)) {
        long_val = PyInt_AS_LONG(t_o);

#  if SIZEOF_LONG > 4
        if (long_val < 0 || long_val > 0xffffffffL)
#  else
          if (long_val < 0)
#  endif
	    {
	      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
				 omniPy::formatString("%s item %d: "
						      "%s is out of range for "
						      "unsigned long", "siO",
                                                      seq_arr, i, t_o));
	    }
      }
#endif
      else {
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting unsigned long, "
                                                "got %r", "siO",
                                                seq_arr, i, t_o->ob_type));
      }
    }
    return;

  case CORBA::tk_float:
  case CORBA::tk_double:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);
      if (PyFloat_Check(t_o)) {
        // OK
      }
#if (PY_VERSION_HEX < 0x03000000)
      else if (PyInt_Check(t_o)) {
        // OK
      }
#endif
      else if (PyLong_Check(t_o)) {
        double_val = PyLong_AsDouble(t_o);
        if (double_val == -1.0 && PyErr_Occurred()) {
          PyErr_Clear();
          const char* tname = etk == CORBA::tk_float ? "float" : "double";

          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range "
                                                  "for %s", "siOs",
                                                  seq_arr, i, t_o, tname));
        }
      }
      else {
        const char* tname = etk == CORBA::tk_float ? "float" : "double";
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting %s, "
                                                "got %r", "sisO",
                                                seq_arr, i, tname,
                                                t_o->ob_type));
      }
    }
    return;

  case CORBA::tk_boolean:
    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

      if (PyObject_IsTrue(t_o) == -1) {
        if (omniORB::trace(1))
          PyErr_Print();
        else
          PyErr_Clear();

        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting bool, "
                                                "got %r", "siO",
                                                seq_arr, i, t_o->ob_type));
      }
    }
    return;

#ifdef HAS_LongLong

  case CORBA::tk_longlong:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

      if (PyLong_Check(t_o)) {
        CORBA::LongLong ll = PyLong_AsLongLong(t_o);
        if (ll == -1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "long long", "siO",
                                                  seq_arr, i, t_o));
        }
      }
#if (PY_VERSION_HEX < 0x03000000)
      else if (!PyInt_Check(t_o)) {
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting long long, "
                                                "got %r", "siO",
                                                seq_arr, i, t_o->ob_type));
      }
#endif
    }
    return;

  case CORBA::tk_ulonglong:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

      if (PyLong_Check(t_o)) {
        CORBA::ULongLong ull = PyLong_AsUnsignedLongLong(t_o);
        if (ull == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "unsigned long long", "siO",
                                                  seq_arr, i, t_o));
        }
      }
#if (PY_VERSION_HEX < 0x03000000)
      else if (PyInt_Check(t_o)) {
        long l = PyInt_AS_LONG(t_o);
        if (l < 0) {
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "unsigned long long", "siO",
                                                  seq_arr, i, t_o));
        }
      }
#endif
      else {
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting unsigned "
                                                "long long, got %r", "siO",
                                                seq_arr, i, t_o->ob_type));
      }
    }
    return;
#else // No long long
  case 23:
    OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);

  case 24:
    OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
  default:
    OMNIORB_ASSERT(0);
  }
}


static void
validateTypeSequence(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus,
		     PyObject* track)
{ // element_desc, max_length
  PyObject*    t_o      = PyTuple_GET_ITEM(d_o, 2);
  OMNIORB_ASSERT(Int_Check(t_o));

  CORBA::ULong max_len  = Int_AS_LONG(t_o);
  PyObject*    elm_desc = PyTuple_GET_ITEM(d_o, 1);

  CORBA::ULong len, i;
  CORBA::ULong etk;

  if (sequenceOptimisedType(elm_desc, etk)) {

    if (etk == CORBA::tk_octet) {
      // Mapping says sequence<octet> uses a string. For Python 3 we
      // use a bytes.
      if (!RawString_Check(a_o)) {
	THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			   omniPy::formatString("Expecting bytes, got %r",
						"O", a_o->ob_type));
      }
      len = RawString_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);
      return;
    }
    else if (etk == CORBA::tk_char) {
      // Mapping says sequence<char> uses a string
      if (!String_Check(a_o)) {
	THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			   omniPy::formatString("Expecting string, got %r",
						"O", a_o->ob_type));
      }
      len = String_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

#if (PY_VERSION_HEX >= 0x03030000) // Python 3.3 or later
      int   kind = PyUnicode_KIND(a_o);
      void* data = PyUnicode_DATA(a_o);

      for (i=0; i != len; ++i) {
        Py_UCS4 uc = PyUnicode_READ(kind, data, i);
        if (uc > 255)
          OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_CannotMapChar,
                        compstatus);
      }

#elif (PY_VERSION_HEX >= 0x03000000) // Python 3.0 - 3.2
      Py_UNICODE* us = PyUnicode_AS_UNICODE(a_o);
      for (i=0; i != len; ++i) {
        if (us[i] > 255)
          OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_CannotMapChar,
                        compstatus);
      }
#endif
      return;
    }
    else if (PyList_Check(a_o)) {
      CORBA::ULong len = PyList_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
        OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      validateOptSequenceItems(len, a_o, etk, compstatus, "Sequence", listGet);
    }
    else if (PyTuple_Check(a_o)) {
      CORBA::ULong len = PyTuple_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
        OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      validateOptSequenceItems(len, a_o, etk, compstatus, "Sequence", tupleGet);
    }
    else {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting sequence, got %r",
					      "O", a_o->ob_type));
    }
  }
  else { // Complex type
    if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      for (i=0; i < len; i++) {
	try {
	  omniPy::validateType(elm_desc, PyList_GET_ITEM(a_o, i),
			       compstatus, track);
	}
	catch (Py_BAD_PARAM& bp) {
	  bp.add(omniPy::formatString("Sequence item %d", "i", i));
	  throw;
	}
      }
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      for (i=0; i < len; i++) {
	try {
	  omniPy::validateType(elm_desc, PyTuple_GET_ITEM(a_o, i),
			       compstatus, track);
	}
	catch (Py_BAD_PARAM& bp) {
	  bp.add(omniPy::formatString("Sequence item %d", "i", i));
	  throw;
	}
      }
    }
    else {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting sequence, got %r",
					      "O", a_o->ob_type));
    }
  }
}

static void
validateTypeArray(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{ // element_desc, length

  PyObject*    t_o      = PyTuple_GET_ITEM(d_o, 2);
  OMNIORB_ASSERT(Int_Check(t_o));

  CORBA::ULong arr_len  = Int_AS_LONG(t_o);
  PyObject*    elm_desc = PyTuple_GET_ITEM(d_o, 1);

  CORBA::ULong len, i;
  CORBA::ULong etk;

  if (sequenceOptimisedType(elm_desc, etk)) {

    if (etk == CORBA::tk_octet) {
      // Mapping says array of octet uses a string. For Python 3 we
      // use a bytes.
      if (!RawString_Check(a_o)) {
	THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			   omniPy::formatString("Expecting bytes, got %r",
						"O", a_o->ob_type));
      }
      len = RawString_GET_SIZE(a_o);
      if (len != arr_len) {
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting bytes length %d, "
						"got %d",
						"ii", arr_len, len));
      }
      return;
    }
    else if (etk == CORBA::tk_char) {
      // Mapping says array of char uses a string
      if (!String_Check(a_o)) {
	THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			   omniPy::formatString("Expecting string, got %r",
						"O", a_o->ob_type));
      }
      len = String_GET_SIZE(a_o);
      if (len != arr_len) {
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting string length %d, "
						"got %d",
						"ii", arr_len, len));
      }

#if (PY_VERSION_HEX >= 0x03030000) // Python 3.3 or later
      int   kind = PyUnicode_KIND(a_o);
      void* data = PyUnicode_DATA(a_o);

      for (i=0; i != len; ++i) {
        Py_UCS4 uc = PyUnicode_READ(kind, data, i);
        if (uc > 255)
          OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_CannotMapChar,
                        compstatus);
      }

#elif (PY_VERSION_HEX >= 0x03000000) // Python 3.0 - 3.2
      Py_UNICODE* us = PyUnicode_AS_UNICODE(a_o);
      for (i=0; i != len; ++i) {
        if (us[i] > 255)
          OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_CannotMapChar,
                        compstatus);
      }
#endif
      return;
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (len != arr_len) {
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting array length %d, "
						"got %d",
						"ii", arr_len, len));
      }
      validateOptSequenceItems(len, a_o, etk, compstatus, "Array", listGet);
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (len != arr_len) {
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting array length %d, "
						"got %d",
						"ii", arr_len, len));
      }
      validateOptSequenceItems(len, a_o, etk, compstatus, "Array", tupleGet);
    }
    else {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting array, got %r",
					      "O", a_o->ob_type));
    }
  }
  else { // Complex type
    if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (len != arr_len) {
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting array length %d, "
						"got %d",
						"ii", arr_len, len));
      }

      for (i=0; i < len; i++) {
	try {
	  omniPy::validateType(elm_desc, PyList_GET_ITEM(a_o, i),
			       compstatus, track);
	}
	catch (Py_BAD_PARAM& bp) {
	  bp.add(omniPy::formatString("Array item %d", "i", i));
	  throw;
	}
      }
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (len != arr_len) {
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting array length %d, "
						"got %d",
						"ii", arr_len, len));
      }

      for (i=0; i < len; i++) {
	try {
	  omniPy::validateType(elm_desc, PyTuple_GET_ITEM(a_o, i),
			       compstatus, track);
	}
	catch (Py_BAD_PARAM& bp) {
	  bp.add(omniPy::formatString("Array item %d", "i", i));
	  throw;
	}
      }
    }
    else {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting array, got %r",
					      "O", a_o->ob_type));
    }
  }
}

static void
validateTypeAlias(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{ // repoId, name, descr

  omniPy::validateType(PyTuple_GET_ITEM(d_o, 3), a_o, compstatus, track);
}

static void
validateTypeExcept(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{ // class, repoId, exc name, name, descriptor, ...

  // As with structs, the descriptor tuple has twice the number of
  // members plus 4.
  int cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  PyObject* name;
  PyObject* value;

  int i, j;
  for (i=0,j=4; i < cnt; i++,j++) {
    name = PyTuple_GET_ITEM(d_o, j++);
    OMNIORB_ASSERT(String_Check(name));

    value = PyObject_GetAttr(a_o, name);

    if (!value) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Exception %r instance %r "
					      "has no %r member",
					      "OOO",
					      PyTuple_GET_ITEM(d_o, 3),
					      a_o->ob_type,
					      name));
    }
    omniPy::PyRefHolder h(value);
    try {
      omniPy::validateType(PyTuple_GET_ITEM(d_o, j), value, compstatus, track);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.add(omniPy::formatString("Exception %r member %r", "OO",
				  PyTuple_GET_ITEM(d_o, 3), name));
      throw;
    }
  }
}

static void
validateTypeLongLong(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus,
		     PyObject* track)
{
#ifdef HAS_LongLong
  if (PyLong_Check(a_o)) {
    CORBA::LongLong ll = PyLong_AsLongLong(a_o);
    if (ll == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "long long",
					      "O", a_o));
    }
  }
#  if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(a_o)) {
    // OK
  }
#  endif
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting long long, got %r",
					    "O", a_o->ob_type));
  }

#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
}

static void
validateTypeULongLong(PyObject* d_o, PyObject* a_o,
		      CORBA::CompletionStatus compstatus,
		      PyObject* track)
{
#ifdef HAS_LongLong
  if (PyLong_Check(a_o)) {
    CORBA::ULongLong ull = PyLong_AsUnsignedLongLong(a_o);
    if (ull == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long long",
					      "O", a_o));
    }
  }
#  if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    if (l < 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long long",
					      "O", a_o));
    }
  }
#  endif
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting long long, got %r",
					    "O", a_o->ob_type));
  }
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
}

static void
validateTypeLongDouble(PyObject* d_o, PyObject* a_o,
		       CORBA::CompletionStatus compstatus,
		       PyObject* track)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
}

static void
validateTypeWChar(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{
  if (!PyUnicode_Check(a_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting unicode, got %r",
					    "O", a_o->ob_type));
  }
  if (Unicode_GET_SIZE(a_o) != 1) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting unicode of length 1, "
					    "got %r",
					    "O", a_o));
  }
#if (PY_VERSION_HEX >= 0x03030000) // New Unicode API
  Py_UCS4 uc = PyUnicode_READ_CHAR(a_o, 0);
  if (uc > 0xffff)
    OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_CannotMapChar, compstatus);
#endif
}

static void
validateTypeWString(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus,
		    PyObject* track)
{ // max_length
  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);
  OMNIORB_ASSERT(Int_Check(t_o));

  CORBA::ULong max_len = Int_AS_LONG(t_o);

  if (!PyUnicode_Check(a_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting unicode, got %r",
					    "O", a_o->ob_type));
  }

#if (PY_VERSION_HEX < 0x03030000) // Earlier than Python 3.3

  CORBA::ULong len = PyUnicode_GET_SIZE(a_o);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, compstatus);

  // Check for nulls
  Py_UNICODE* str = PyUnicode_AS_UNICODE(a_o);
  for (CORBA::ULong i=0; i<len; i++) {
    if (str[i] == 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_EmbeddedNullInPythonString, compstatus,
			 omniPy::formatString("Embedded null in unicode "
					      "at position %d", "i", i));
    }
  }

#else // New Unicode API

  CORBA::ULong len = PyUnicode_GET_LENGTH(a_o);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, compstatus);

  // Check for nulls
  int   kind = PyUnicode_KIND(a_o);
  void* data = PyUnicode_DATA(a_o);

  for (CORBA::ULong i=0; i<len; i++) {
    if (PyUnicode_READ(kind, data, i) == 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_EmbeddedNullInPythonString, compstatus,
			 omniPy::formatString("Embedded null in unicode "
					      "at position %d", "i", i));
    }
  }
#endif
}

static void
validateTypeFixed(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{ // digits, scale
  if (!omnipyFixed_Check(a_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting fixed, got %r",
					    "O", a_o->ob_type));
  }

  PyObject* t_o;

  t_o = PyTuple_GET_ITEM(d_o, 1); int dlimit = Int_AS_LONG(t_o);
  t_o = PyTuple_GET_ITEM(d_o, 2); int slimit = Int_AS_LONG(t_o);

  int digits = ((omnipyFixedObject*)a_o)->ob_fixed->fixed_digits();
  int scale  = ((omnipyFixedObject*)a_o)->ob_fixed->fixed_scale();

  if (scale > slimit) {
    digits -= (scale - slimit);
    scale   = slimit;
  }
  if (digits > dlimit)
    OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_RangeError, compstatus);
}

// validateTypeValue and validateTypeValueBox are in pyValueType.cc

static void
validateTypeNative(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
}

// validateTypeAbstractInterface is in pyAbstractIntf.cc

static void
validateTypeLocalInterface(PyObject* d_o, PyObject* a_o,
			   CORBA::CompletionStatus compstatus,
			   PyObject* track)
{
  OMNIORB_THROW(MARSHAL, MARSHAL_LocalObject, compstatus);
}

void
omniPy::
validateTypeIndirect(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus,
		     PyObject* track)
{
  PyObject* l = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(PyList_Check(l));
  PyObject* d = PyList_GET_ITEM(l, 0);

  if (String_Check(d)) {
    // Indirection to a repoId -- find the corresponding descriptor
    d = PyDict_GetItem(pyomniORBtypeMap, d);
    if (!d) OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_IncompletePythonType,
			  compstatus);
    Py_INCREF(d);
    PyList_SetItem(l, 0, d);
  }
  validateType(d, a_o, compstatus, track);
}


const omniPy::ValidateTypeFn omniPy::validateTypeFns[] = {
  validateTypeNull,
  validateTypeVoid,
  validateTypeShort,
  validateTypeLong,
  validateTypeUShort,
  validateTypeULong,
  validateTypeFloat,
  validateTypeDouble,
  validateTypeBoolean,
  validateTypeChar,
  validateTypeOctet,
  validateTypeAny,
  validateTypeTypeCode,
  validateTypePrincipal,
  validateTypeObjref,
  validateTypeStruct,
  validateTypeUnion,
  validateTypeEnum,
  validateTypeString,
  validateTypeSequence,
  validateTypeArray,
  validateTypeAlias,
  validateTypeExcept,
  validateTypeLongLong,
  validateTypeULongLong,
  validateTypeLongDouble,
  validateTypeWChar,
  validateTypeWString,
  validateTypeFixed,
  omniPy::validateTypeValue,
  omniPy::validateTypeValueBox,
  validateTypeNative,
  omniPy::validateTypeAbstractInterface,
  validateTypeLocalInterface
};


static void
marshalPyObjectNull(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
}

static void
marshalPyObjectVoid(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
}

static void
marshalPyObjectShort(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Short s;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o))
    s = PyInt_AS_LONG(a_o);
  else
#endif
    s = PyLong_AsLong(a_o);

  s >>= stream;
}

static void
marshalPyObjectLong(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Long l;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o))
    l = PyInt_AS_LONG(a_o);
  else
#endif
    l = PyLong_AsLong(a_o);

  l >>= stream;
}

static void
marshalPyObjectUShort(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::UShort us;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o))
    us = PyInt_AS_LONG(a_o);
  else
#endif
    us = PyLong_AsLong(a_o);

  us >>= stream;
}

static void
marshalPyObjectULong(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::ULong ul;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyLong_Check(a_o))
    ul = PyLong_AsUnsignedLong(a_o);
  else // It's an int
    ul = PyInt_AS_LONG(a_o);
#else
  ul = PyLong_AsUnsignedLong(a_o);
#endif

  ul >>= stream;
}

static void
marshalPyObjectFloat(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Float f;

  if (PyFloat_Check(a_o))
    f = (CORBA::Float)PyFloat_AS_DOUBLE(a_o);

#if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(a_o))
    f = (CORBA::Float)PyInt_AS_LONG(a_o);
#endif

  else
    f = (CORBA::Float)PyLong_AsDouble(a_o);

  f >>= stream;
}

static void
marshalPyObjectDouble(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Double d = 0; // Initialised to stop egcs complaining

  if (PyFloat_Check(a_o))
    d = (CORBA::Double)PyFloat_AS_DOUBLE(a_o);

#if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(a_o))
    d = (CORBA::Double)PyInt_AS_LONG(a_o);
#endif

  else
    d = (CORBA::Double)PyLong_AsDouble(a_o);

  d >>= stream;
}

static void
marshalPyObjectBoolean(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Boolean b = PyObject_IsTrue(a_o);
  stream.marshalBoolean(b);
}

static void
marshalPyObjectChar(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
#if (PY_VERSION_HEX < 0x03000000)
  char* str = PyString_AS_STRING(a_o);
  stream.marshalChar(str[0]);

#elif (PY_VERSION_HEX < 0x03030000)
  Py_UNICODE* us = PyUnicode_AS_UNICODE(a_o);
  stream.marshalChar(*us);

#else
  Py_UCS4 uc = PyUnicode_READ_CHAR(a_o, 0);
  stream.marshalChar(uc);

#endif
}

static void
marshalPyObjectOctet(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Octet o;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o))
    o = PyInt_AS_LONG(a_o);
  else
#endif
    o = PyLong_AsLong(a_o);

  stream.marshalOctet(o);
}

static void
marshalPyObjectAny(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  // TypeCode
  omniPy::PyRefHolder t_o (PyObject_GetAttrString(a_o, (char*)"_t"));
  omniPy::PyRefHolder desc(PyObject_GetAttrString(t_o, (char*)"_d"));

  omniPy::marshalTypeCode(stream, desc);

  // Any's contents
  t_o = PyObject_GetAttrString(a_o, (char*)"_v");
  omniPy::marshalPyObject(stream, desc, t_o);
}

static void
marshalPyObjectTypeCode(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  omniPy::PyRefHolder t_o(PyObject_GetAttrString(a_o, (char*)"_d"));
  omniPy::marshalTypeCode(stream, t_o);
}

static void
marshalPyObjectPrincipal(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  OMNIORB_ASSERT(0);
}

static void
marshalPyObjectObjref(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // repoId, name

  CORBA::Object_ptr obj;

  if (a_o == Py_None)
    obj = CORBA::Object::_nil();
  else
    obj = omniPy::getObjRef(a_o);

  CORBA::Object::_marshalObjRef(obj, stream);
}

static void
marshalPyObjectStruct(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // class, repoId, struct name, {name, descriptor}

  int                 i, j;
  int                 cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;
  PyObject*           name;
  omniPy::PyRefHolder value;

  for (i=0,j=4; i < cnt; i++,j++) {
    name  = PyTuple_GET_ITEM(d_o, j++);
    value = PyObject_GetAttr(a_o, name);
    omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(d_o, j), value);
  }
}

static void
marshalPyObjectUnion(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // class,
  // repoId,
  // name,
  // discriminant descr,
  // default used,
  // ((label value, member name, member descr), ...),
  // default (label, name, descr) or None,
  // {label: (label, name, descr), ...}

  omniPy::PyRefHolder discriminant(PyObject_GetAttrString(a_o, (char*)"_d"));
  omniPy::PyRefHolder value(PyObject_GetAttrString(a_o, (char*)"_v"));

  PyObject* t_o          = PyTuple_GET_ITEM(d_o, 4); // Discriminant descriptor
  PyObject* cdict        = PyTuple_GET_ITEM(d_o, 8);

  omniPy::marshalPyObject(stream, t_o, discriminant);

  t_o = PyDict_GetItem(cdict, discriminant);
  if (t_o) {
    // Discriminant found in case dictionary
    omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(t_o, 2), value);
  }
  else {
    // Is there a default case?
    t_o = PyTuple_GET_ITEM(d_o, 7);
    if (t_o != Py_None) {
      omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(t_o, 2), value);
    }
  }
}

static void
marshalPyObjectEnum(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // repoId, name, item list

  omniPy::PyRefHolder ev(PyObject_GetAttrString(a_o, (char*)"_v"));
  CORBA::ULong e = omniPy::getULongVal(ev);
  e >>= stream;
}

static void
marshalPyObjectString(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // max_length

#if (PY_VERSION_HEX < 0x03000000)
  orbParameters::nativeCharCodeSet->marshalString(stream, stream.TCS_C(), 0,
						  PyString_GET_SIZE(a_o),
						  PyString_AS_STRING(a_o));
#else
  CORBA::ULong size;
  const char*  str = String_AS_STRING_AND_SIZE(a_o, size);

  omniPy::ncs_c_utf_8->marshalString(stream, stream.TCS_C(), 0, size, str);
#endif
}


template<class G>
inline void
marshalOptSequenceItems(cdrStream&   stream,
                        CORBA::ULong len,
                        PyObject*    a_o,
                        CORBA::ULong etk,
                        G            getFn)
{
  CORBA::ULong  i;
  PyObject*     t_o;
  long          long_val;
  unsigned long ulong_val;
  double        double_val;

  switch (etk) {

  case CORBA::tk_short:
    {
      CORBA::Short e;
      for (i=0; i < len; i++) {
        t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
        if (PyInt_Check(t_o))
          e = PyInt_AS_LONG(t_o);
        else
#endif
          e = PyLong_AsLong(t_o);

        e >>= stream;
      }
    }
    break;

  case CORBA::tk_long:
    {
      CORBA::Long e;
      for (i=0; i < len; i++) {
        t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
        if (PyInt_Check(t_o))
          e = PyInt_AS_LONG(t_o);
        else
#endif
          e = PyLong_AsLong(t_o);

        e >>= stream;
      }
    }
    break;

  case CORBA::tk_ushort:
    {
      CORBA::UShort e;
      for (i=0; i < len; i++) {
        t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
        if (PyInt_Check(t_o))
          e = PyInt_AS_LONG(t_o);
        else
#endif
          e = PyLong_AsLong(t_o);

        e >>= stream;
      }
    }
    break;

  case CORBA::tk_ulong:
    {
      CORBA::ULong e;
      for (i=0; i < len; i++) {
        t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
        if (PyLong_Check(t_o))
#endif
          e = PyLong_AsUnsignedLong(t_o);

#if (PY_VERSION_HEX < 0x03000000)
        else
          e = PyInt_AS_LONG(t_o);
#endif
        e >>= stream;
      }
    }
    break;

  case CORBA::tk_float:
    {
      CORBA::Float e;
      for (i=0; i < len; i++) {
        t_o = getFn(a_o, i);

        if (PyFloat_Check(t_o))
          e = PyFloat_AS_DOUBLE(t_o);

#if (PY_VERSION_HEX < 0x03000000)
        else if (PyInt_Check(t_o))
          e = PyInt_AS_LONG(t_o);
#endif
        else
          e = PyLong_AsDouble(t_o);

        e >>= stream;
      }
    }
    break;

  case CORBA::tk_double:
    {
      CORBA::Double e;
      for (i=0; i < len; i++) {
        t_o = getFn(a_o, i);

        if (PyFloat_Check(t_o))
          e = PyFloat_AS_DOUBLE(t_o);

#if (PY_VERSION_HEX < 0x03000000)
        else if (PyInt_Check(t_o))
          e = PyInt_AS_LONG(t_o);
#endif
        else
          e = PyLong_AsDouble(t_o);

        e >>= stream;
      }
    }
    break;

  case CORBA::tk_boolean:
    {
      CORBA::Boolean e;
      for (i=0; i < len; i++) {
        t_o = getFn(a_o, i);

        stream.marshalBoolean(PyObject_IsTrue(t_o));
      }
    }
    break;

#ifdef HAS_LongLong

  case CORBA::tk_longlong:
    {
      CORBA::LongLong e;
      for (i=0; i < len; i++) {
        t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
        if (PyLong_Check(t_o))
#endif
          e = PyLong_AsLongLong(t_o);

#if (PY_VERSION_HEX < 0x03000000)
        else
          e = PyInt_AS_LONG(t_o);
#endif
        e >>= stream;
      }
    }
    break;

  case CORBA::tk_ulonglong:
    {
      CORBA::ULongLong e;
      for (i=0; i < len; i++) {
        t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
        if (PyLong_Check(t_o))
#endif
          e = PyLong_AsLongLong(t_o);

#if (PY_VERSION_HEX < 0x03000000)
        else
          e = PyInt_AS_LONG(t_o);
#endif
        e >>= stream;
      }
    }
    break;
#endif
  default:
    OMNIORB_ASSERT(0);
  }

}


static void
marshalPyObjectSequence(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // element_desc, max_length

  PyObject* elm_desc = PyTuple_GET_ITEM(d_o, 1);
  PyObject* t_o;

  CORBA::ULong i, len;
  CORBA::ULong etk;

  if (sequenceOptimisedType(elm_desc, etk)) {
    if (etk == CORBA::tk_octet) {
      len = RawString_GET_SIZE(a_o);
      len >>= stream;
      CORBA::Octet *l = (CORBA::Octet*)RawString_AS_STRING(a_o);
      stream.put_octet_array((const CORBA::Octet*)l, len);
    }
    else if (etk == CORBA::tk_char) {
      len = String_GET_SIZE(a_o);
      len >>= stream;

#if (PY_VERSION_HEX < 0x03000000)
      CORBA::Char *l = (CORBA::Char*)PyString_AS_STRING(a_o);
      for (i=0; i != len; ++i)
	stream.marshalChar(l[i]);

#elif (PY_VERSION_HEX >= 0x03030000) // Python 3.3 or later
      int   kind = PyUnicode_KIND(a_o);
      void* data = PyUnicode_DATA(a_o);
      
      for (i=0; i != len; ++i) {
        Py_UCS4 uc = PyUnicode_READ(kind, data, i);
        stream.marshalChar((char)uc);
      }

#elif (PY_VERSION_HEX >= 0x03000000) // Python 3.0 - 3.2
      Py_UNICODE* us = PyUnicode_AS_UNICODE(a_o);
      for (i=0; i != len; ++i) {
        stream.marshalChar((char)us[i]);
      }
#endif
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      len >>= stream;

      marshalOptSequenceItems(stream, len, a_o, etk, listGet);
    }
    else {
      OMNIORB_ASSERT(PyTuple_Check(a_o));
      len = PyTuple_GET_SIZE(a_o);
      len >>= stream;

      marshalOptSequenceItems(stream, len, a_o, etk, tupleGet);
    }
  }
  else if (PyList_Check(a_o)) {
    len = PyList_GET_SIZE(a_o);
    len >>= stream;
    for (i=0; i < len; i++)
      omniPy::marshalPyObject(stream, elm_desc, PyList_GET_ITEM(a_o, i));
  }
  else {
    len = PyTuple_GET_SIZE(a_o);
    len >>= stream;
    for (i=0; i < len; i++)
      omniPy::marshalPyObject(stream, elm_desc, PyTuple_GET_ITEM(a_o, i));
  }
}


static void
marshalPyObjectArray(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // element_desc, length

  PyObject* elm_desc = PyTuple_GET_ITEM(d_o, 1);
  PyObject* t_o;

  CORBA::ULong i, len;
  CORBA::ULong etk;

  if (sequenceOptimisedType(elm_desc, etk)) {
    if (etk == CORBA::tk_octet) {
      len = RawString_GET_SIZE(a_o);
      CORBA::Octet *l = (CORBA::Octet*)RawString_AS_STRING(a_o);
      stream.put_octet_array(l, len);
    }
    else if (etk == CORBA::tk_char) {
      len = String_GET_SIZE(a_o);

#if (PY_VERSION_HEX < 0x03000000)
      CORBA::Char *l = (CORBA::Char*)PyString_AS_STRING(a_o);
      for (i=0; i<len; i++)
	stream.marshalChar(l[i]);

#elif (PY_VERSION_HEX >= 0x03030000) // Python 3.3 or later
      int   kind = PyUnicode_KIND(a_o);
      void* data = PyUnicode_DATA(a_o);
      
      for (i=0; i != len; ++i) {
        Py_UCS4 uc = PyUnicode_READ(kind, data, i);
        stream.marshalChar((char)uc);
      }

#elif (PY_VERSION_HEX >= 0x03000000) // Python 3.0 - 3.2
      Py_UNICODE* us = PyUnicode_AS_UNICODE(a_o);
      for (i=0; i != len; ++i) {
        stream.marshalChar((char)us[i]);
      }
#endif
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      marshalOptSequenceItems(stream, len, a_o, etk, listGet);
    }
    else {
      OMNIORB_ASSERT(PyTuple_Check(a_o));
      len = PyTuple_GET_SIZE(a_o);
      marshalOptSequenceItems(stream, len, a_o, etk, tupleGet);
    }
  }
  else if (PyList_Check(a_o)) {
    len = PyList_GET_SIZE(a_o);
    for (i=0; i < len; i++)
      omniPy::marshalPyObject(stream, elm_desc, PyList_GET_ITEM(a_o, i));
  }
  else {
    len = PyTuple_GET_SIZE(a_o);
    for (i=0; i < len; i++)
      omniPy::marshalPyObject(stream, elm_desc, PyTuple_GET_ITEM(a_o, i));
  }
}

static void
marshalPyObjectAlias(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // repoId, name, descr

  omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(d_o, 3), a_o);
}

static void
marshalPyObjectExcept(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // class, repoId, exc name, name, descriptor, ...

  PyObject*    t_o  = PyTuple_GET_ITEM(d_o, 2);

  CORBA::ULong slen;
  const char*  str = String_AS_STRING_AND_SIZE(t_o, slen);
  ++slen;

  slen >>= stream;
  stream.put_octet_array((const CORBA::Octet*)str, slen);

  int cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  PyObject* name;
  PyObject* value;

  int i, j;
  for (i=0,j=4; i < cnt; i++) {
    name  = PyTuple_GET_ITEM(d_o, j++);
    value = PyObject_GetAttr(a_o, name);
    Py_DECREF(value);
    omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(d_o, j++), value);
  }
}

static void
marshalPyObjectLongLong(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
#ifdef HAS_LongLong
  CORBA::LongLong ll;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyLong_Check(a_o))
#endif
    ll = PyLong_AsLongLong(a_o);

#if (PY_VERSION_HEX < 0x03000000)
  else // It's an int
    ll = PyInt_AS_LONG(a_o);
#endif

  ll >>= stream;
#else
  OMNIORB_ASSERT(0);
#endif
}

static void
marshalPyObjectULongLong(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
#ifdef HAS_LongLong
  CORBA::ULongLong ull;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyLong_Check(a_o))
#endif
    ull = PyLong_AsUnsignedLongLong(a_o);

#if (PY_VERSION_HEX < 0x03000000)
  else // It's an int
    ull = PyInt_AS_LONG(a_o);
#endif

  ull >>= stream;
#else
  OMNIORB_ASSERT(0);
#endif
}

static void
marshalPyObjectLongDouble(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  OMNIORB_ASSERT(0);
}

static void
marshalPyObjectWChar(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  OMNIORB_CHECK_TCS_W_FOR_MARSHAL(stream.TCS_W(), stream);

#if (PY_VERSION_HEX < 0x03030000) // Old Unicode API
  Py_UNICODE* str = PyUnicode_AS_UNICODE(a_o);
  stream.TCS_W()->marshalWChar(stream, str[0]);
#else
  Py_UCS4 uc = PyUnicode_READ_CHAR(a_o, 0);
  stream.TCS_W()->marshalWChar(stream, uc);
#endif
}

static void
marshalPyObjectWString(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  OMNIORB_CHECK_TCS_W_FOR_MARSHAL(stream.TCS_W(), stream);

#if defined(Py_UNICODE_WIDE) || (PY_VERSION_HEX >= 0x03030000)
  omniPy::PyRefHolder ustr(PyUnicode_AsUTF16String(a_o));
  if (!ustr.valid()) {
    // Now we're in trouble...
    if (omniORB::trace(1)) {
      PyErr_Print();
    }
    PyErr_Clear();
    OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException,
		  (CORBA::CompletionStatus)stream.completion());
  }
  OMNIORB_ASSERT(RawString_Check(ustr));

  char*        str = RawString_AS_STRING(ustr) + 2; // Skip BOM
  CORBA::ULong len = (RawString_GET_SIZE(ustr) - 2) / 2;

#else
  Py_UNICODE*  str = PyUnicode_AS_UNICODE(a_o);
  CORBA::ULong len = PyUnicode_GET_SIZE(a_o);

#endif
  stream.TCS_W()->marshalWString(stream, 0, len,
				 (const omniCodeSet::UniChar*)str);
}

static void
marshalPyObjectFixed(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // digits, scale
  PyObject* t_o;

  t_o = PyTuple_GET_ITEM(d_o, 1); int dlimit = Int_AS_LONG(t_o);
  t_o = PyTuple_GET_ITEM(d_o, 2); int slimit = Int_AS_LONG(t_o);

  CORBA::Fixed f(*((omnipyFixedObject*)a_o)->ob_fixed);
  f.PR_setLimits(dlimit, slimit);
  f >>= stream;
}

// marshalPyObjectValue and marshalPyObjectValueBox are in pyValueType.cc

static void
marshalPyObjectNative(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  OMNIORB_ASSERT(0);
}

// marshalPyObjectAbstractInterface is in pyAbstractIntf.cc

static void
marshalPyObjectLocalInterface(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  OMNIORB_ASSERT(0);
}

void
omniPy::
marshalPyObjectIndirect(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  PyObject* l = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(PyList_Check(l));
  PyObject* d = PyList_GET_ITEM(l, 0);

  OMNIORB_ASSERT(!String_Check(d));
  marshalPyObject(stream, d, a_o);
}


const omniPy::MarshalPyObjectFn omniPy::marshalPyObjectFns[] = {
  marshalPyObjectNull,
  marshalPyObjectVoid,
  marshalPyObjectShort,
  marshalPyObjectLong,
  marshalPyObjectUShort,
  marshalPyObjectULong,
  marshalPyObjectFloat,
  marshalPyObjectDouble,
  marshalPyObjectBoolean,
  marshalPyObjectChar,
  marshalPyObjectOctet,
  marshalPyObjectAny,
  marshalPyObjectTypeCode,
  marshalPyObjectPrincipal,
  marshalPyObjectObjref,
  marshalPyObjectStruct,
  marshalPyObjectUnion,
  marshalPyObjectEnum,
  marshalPyObjectString,
  marshalPyObjectSequence,
  marshalPyObjectArray,
  marshalPyObjectAlias,
  marshalPyObjectExcept,
  marshalPyObjectLongLong,
  marshalPyObjectULongLong,
  marshalPyObjectLongDouble,
  marshalPyObjectWChar,
  marshalPyObjectWString,
  marshalPyObjectFixed,
  omniPy::marshalPyObjectValue,
  omniPy::marshalPyObjectValueBox,
  marshalPyObjectNative,
  omniPy::marshalPyObjectAbstractInterface,
  marshalPyObjectLocalInterface
};



static PyObject*
unmarshalPyObjectNull(cdrStream& stream, PyObject* d_o)
{
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
unmarshalPyObjectVoid(cdrStream& stream, PyObject* d_o)
{
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
unmarshalPyObjectShort(cdrStream& stream, PyObject* d_o)
{
  CORBA::Short s;
  s <<= stream;
  return Int_FromLong(s);
}

static PyObject*
unmarshalPyObjectLong(cdrStream& stream, PyObject* d_o)
{
  CORBA::Long l;
  l <<= stream;
  return Int_FromLong(l);
}

static PyObject*
unmarshalPyObjectUShort(cdrStream& stream, PyObject* d_o)
{
  CORBA::UShort us;
  us <<= stream;
  return Int_FromLong(us);
}

static PyObject*
unmarshalPyObjectULong(cdrStream& stream, PyObject* d_o)
{
  CORBA::ULong ul;
  ul <<= stream;
  return PyLong_FromUnsignedLong(ul);
}

static PyObject*
unmarshalPyObjectFloat(cdrStream& stream, PyObject* d_o)
{
  CORBA::Float f;
  f <<= stream;
  return PyFloat_FromDouble((double)f);
}

static PyObject*
unmarshalPyObjectDouble(cdrStream& stream, PyObject* d_o)
{
  CORBA::Double d;
  d <<= stream;
  return PyFloat_FromDouble(d);
}

static PyObject*
unmarshalPyObjectBoolean(cdrStream& stream, PyObject* d_o)
{
  CORBA::Boolean b = stream.unmarshalBoolean();
  return PyBool_FromLong(b);
}

static PyObject*
unmarshalPyObjectChar(cdrStream& stream, PyObject* d_o)
{
#if (PY_VERSION_HEX < 0x03000000)
  CORBA::Char c = stream.unmarshalChar();
  return PyString_FromStringAndSize((const char*)&c, 1);

#elif (PY_VERSION_HEX < 0x03030000)
  Py_UNICODE uc = stream.unmarshalChar();
  return PyUnicode_FromUnicode(&uc, 1);

#else
  Py_UCS4   uc  = stream.unmarshalChar();
  PyObject* r_o = PyUnicode_New(1, uc);

  PyUnicode_WriteChar(r_o, 0, uc);
  return r_o;

#endif
}

static PyObject*
unmarshalPyObjectOctet(cdrStream& stream, PyObject* d_o)
{
  CORBA::Octet o = stream.unmarshalOctet();
  return Int_FromLong(o);
}

static PyObject*
unmarshalPyObjectAny(cdrStream& stream, PyObject* d_o)
{
  // TypeCode
  PyObject* desc = omniPy::unmarshalTypeCode(stream);
  omniPy::PyRefHolder argtuple(PyTuple_New(1));

  PyTuple_SET_ITEM(argtuple, 0, desc);

  omniPy::PyRefHolder tcobj(PyObject_CallObject(omniPy::pyCreateTypeCode,
                                                argtuple));

  if (!tcobj.valid()) {
    // Return exception to caller
    return 0;
  }

  PyObject* value = omniPy::unmarshalPyObject(stream, desc);

  argtuple = PyTuple_New(2);
  PyTuple_SET_ITEM(argtuple, 0, tcobj.retn());
  PyTuple_SET_ITEM(argtuple, 1, value);

  return PyObject_CallObject(omniPy::pyCORBAAnyClass, argtuple);
}

static PyObject*
unmarshalPyObjectTypeCode(cdrStream& stream, PyObject* d_o)
{
  PyObject* t_o      = omniPy::unmarshalTypeCode(stream);
  PyObject* argtuple = PyTuple_New(1);
  PyTuple_SET_ITEM(argtuple, 0, t_o);
  PyObject* r_o      = PyObject_CallObject(omniPy::pyCreateTypeCode, argtuple);
  Py_DECREF(argtuple);
  return r_o;
}

static PyObject*
unmarshalPyObjectPrincipal(cdrStream& stream, PyObject* d_o)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
}

static PyObject*
unmarshalPyObjectObjref(cdrStream& stream, PyObject* d_o)
{ // repoId, name

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);

  const char* targetRepoId;

  if (t_o == Py_None) {
    targetRepoId = 0;
  }
  else {
    OMNIORB_ASSERT(String_Check(t_o));
    targetRepoId = String_AS_STRING(t_o);
    if (targetRepoId[0] == '\0') { // Empty string => CORBA.Object
      targetRepoId = CORBA::Object::_PD_repoId;
    }
  }
  CORBA::Object_ptr obj = omniPy::UnMarshalObjRef(targetRepoId, stream);
  return omniPy::createPyCorbaObjRef(targetRepoId, obj);
}

static PyObject*
unmarshalPyObjectStruct(cdrStream& stream, PyObject* d_o)
{ // class, repoId, struct name, name, descriptor, ...

  PyObject* strclass = PyTuple_GET_ITEM(d_o, 1);
  int       cnt      = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  omniPy::PyRefHolder strtuple(PyTuple_New(cnt));

  int i, j;
  for (i=0, j=5; i < cnt; i++, j+=2) {
    PyTuple_SET_ITEM(strtuple, i,
		     omniPy::unmarshalPyObject(stream,
					       PyTuple_GET_ITEM(d_o, j)));
  }
  return PyObject_CallObject(strclass, strtuple);
}

static PyObject*
unmarshalPyObjectUnion(cdrStream& stream, PyObject* d_o)
{ // class,
  // repoId,
  // name,
  // discriminant descr,
  // default used,
  // ((label value, member name, member descr), ...),
  // default (label, name, descr) or None,
  // {label: (label, name, descr), ...}

  PyObject* unclass = PyTuple_GET_ITEM(d_o, 1);
  PyObject* t_o     = PyTuple_GET_ITEM(d_o, 4);
  PyObject* cdict   = PyTuple_GET_ITEM(d_o, 8);

  omniPy::PyRefHolder discriminant(omniPy::unmarshalPyObject(stream, t_o));
  PyObject* value;

  t_o = PyDict_GetItem(cdict, discriminant);
  if (t_o) {
    // Discriminant found in case dictionary
    OMNIORB_ASSERT(PyTuple_Check(t_o));
    value = omniPy::unmarshalPyObject(stream, PyTuple_GET_ITEM(t_o, 2));
  }
  else {
    // Is there a default case?
    t_o = PyTuple_GET_ITEM(d_o, 7);
    if (t_o != Py_None) {
      OMNIORB_ASSERT(PyTuple_Check(t_o));
      value = omniPy::unmarshalPyObject(stream, PyTuple_GET_ITEM(t_o, 2));
    }
    else {
      Py_INCREF(Py_None);
      value = Py_None;
    }
  }
  omniPy::PyRefHolder untuple(PyTuple_New(2));
  PyTuple_SET_ITEM(untuple, 0, discriminant.retn());
  PyTuple_SET_ITEM(untuple, 1, value);

  return PyObject_CallObject(unclass, untuple);
}

static PyObject*
unmarshalPyObjectEnum(cdrStream& stream, PyObject* d_o)
{ // repoId, name, item list

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 3);

  OMNIORB_ASSERT(PyTuple_Check(t_o));

  CORBA::ULong e;
  e <<= stream;

  if (e >= (CORBA::ULong)PyTuple_GET_SIZE(t_o))
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEnumValue,
		  (CORBA::CompletionStatus)stream.completion());

  PyObject* ev = PyTuple_GET_ITEM(t_o, e);
  Py_INCREF(ev);
  return ev;
}

static PyObject*
unmarshalPyObjectString(cdrStream& stream, PyObject* d_o)
{ // max_length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);

  OMNIORB_ASSERT(Int_Check(t_o));

  CORBA::ULong max_len = Int_AS_LONG(t_o);
  CORBA::ULong len;
  char*        s;

#if (PY_VERSION_HEX < 0x03000000)
  len = orbParameters::nativeCharCodeSet->unmarshalString(stream,
                                                          stream.TCS_C(),
                                                          max_len, s);
#else
  len = omniPy::ncs_c_utf_8->unmarshalString(stream, stream.TCS_C(),
                                             max_len, s);
#endif

  PyObject* r_o = String_FromStringAndSize(s, len);
  _CORBA_String_helper::dealloc(s);
  return r_o;
}


static PyObject*
unmarshalPyObjectSeqArray(cdrStream& stream, PyObject* d_o, CORBA::ULong len)
{
  omniPy::PyRefHolder r_o;

  PyObject* elm_desc = PyTuple_GET_ITEM(d_o, 1);

  // If the sequence length field is greater than the number of
  // octets left in the message, the sequence length is invalid.
  if (!stream.checkInputOverrun(1, len)) {

    if (Int_Check(elm_desc) && Int_AS_LONG(elm_desc) <= 1) {
      // Sequence is a bizarre sequence of void or null, meaning that
      // the data takes up no space!  The overrun is therefore not an
      // error.
    }
    else {
      OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
                    (CORBA::CompletionStatus)stream.completion());
    }
  }

  CORBA::ULong i;
  CORBA::ULong etk;

  if (sequenceOptimisedType(elm_desc, etk)) {
    if (etk == CORBA::tk_octet) {
      r_o = RawString_FromStringAndSize(0, len);
      CORBA::Octet* c = (CORBA::Octet*)RawString_AS_STRING(r_o);
      stream.get_octet_array(c, len);
      return r_o.retn();
    }
    else if (etk == CORBA::tk_char) {

#if (PY_VERSION_HEX < 0x03000000)
      r_o = PyString_FromStringAndSize(0, len);
      CORBA::Char* c = (CORBA::Char*)PyString_AS_STRING(r_o);

      for (i=0; i<len; i++)
        c[i] = stream.unmarshalChar();

#elif (PY_VERSION_HEX < 0x03030000)
      r_o = PyUnicode_FromUnicode(0, len);
      Py_UNICODE* uc = PyUnicode_AS_UNICODE(r_o);

      for (i=0; i<len; i++)
        uc[i] = stream.unmarshalChar();

#else
      r_o = PyUnicode_New(len, 255);

      int   kind = PyUnicode_KIND(r_o);
      void* data = PyUnicode_DATA(r_o);

      for (i=0; i<len; i++)
        PyUnicode_WRITE(kind, data, i, stream.unmarshalChar());
#endif

      return r_o.retn();
    }
    else {
      r_o = PyList_New(len);

      switch(etk) {
      case CORBA::tk_short:
	{
	  CORBA::Short e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, Int_FromLong(e));
	  }
	}
	return r_o.retn();

      case CORBA::tk_long:
	{
	  CORBA::Long e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, Int_FromLong(e));
	  }
	}
	return r_o.retn();

      case CORBA::tk_ushort:
	{
	  CORBA::UShort e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, Int_FromLong(e));
	  }
	}
	return r_o.retn();

      case CORBA::tk_ulong:
	{
	  CORBA::ULong e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyLong_FromUnsignedLong(e));
	  }
	}
	return r_o.retn();

      case CORBA::tk_float:
	{
	  CORBA::Float e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(e));
	  }
	}
	return r_o.retn();

      case CORBA::tk_double:
	{
	  CORBA::Double e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(e));
	  }
	}
	return r_o.retn();

      case CORBA::tk_boolean:
	{
	  CORBA::Boolean e;
	  for (i=0; i < len; i++) {
	    e = stream.unmarshalBoolean();
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(e));
	  }
	}
	return r_o.retn();
	    
#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	{
	  CORBA::LongLong e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyLong_FromLongLong(e));
	  }
	}
	return r_o.retn();

      case CORBA::tk_ulonglong:
	{
	  CORBA::ULongLong e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyLong_FromUnsignedLongLong(e));
	  }
	}
	return r_o.retn();
#else
      case 23:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		      (CORBA::CompletionStatus)stream.completion());
	return 0;

      case 24:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		      (CORBA::CompletionStatus)stream.completion());
	return 0;
#endif
      default:
	OMNIORB_ASSERT(0);
	return 0;
      }
    }
  }
  else {
    r_o = PyList_New(len);

    for (i=0; i < len; i++)
      PyList_SET_ITEM(r_o, i, omniPy::unmarshalPyObject(stream, elm_desc));

    return r_o.retn();
  }
}


static PyObject*
unmarshalPyObjectSequence(cdrStream& stream, PyObject* d_o)
{ // element_desc, max_length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 2);
  OMNIORB_ASSERT(Int_Check(t_o));

  CORBA::ULong max_len = Int_AS_LONG(t_o);
  CORBA::ULong len;
  len <<= stream;

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong,
		  (CORBA::CompletionStatus)stream.completion());

  return unmarshalPyObjectSeqArray(stream, d_o, len);
}


static PyObject*
unmarshalPyObjectArray(cdrStream& stream, PyObject* d_o)
{ // element_desc, length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 2);
  OMNIORB_ASSERT(Int_Check(t_o));

  CORBA::ULong len      = Int_AS_LONG(t_o);
  PyObject*    elm_desc = PyTuple_GET_ITEM(d_o, 1);

  return unmarshalPyObjectSeqArray(stream, d_o, len);
}


static PyObject*
unmarshalPyObjectAlias(cdrStream& stream, PyObject* d_o)
{ // repoId, name, descr

  return omniPy::unmarshalPyObject(stream, PyTuple_GET_ITEM(d_o, 3));
}

static PyObject*
unmarshalPyObjectExcept(cdrStream& stream, PyObject* d_o)
{ // class, repoId, exc name, name, descriptor, ...

  // Throw away the repoId. By the time we get here, we already
  // know it.
  // *** Should maybe check to see if it's what we're expecting
  CORBA::ULong len; len <<= stream;
  stream.skipInput(len);

  PyObject* strclass = PyTuple_GET_ITEM(d_o, 1);
  int       cnt      = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  omniPy::PyRefHolder strtuple(PyTuple_New(cnt));

  int i, j;
  for (i=0, j=5; i < cnt; i++, j+=2) {
    PyTuple_SET_ITEM(strtuple, i,
		     omniPy::unmarshalPyObject(stream,
					       PyTuple_GET_ITEM(d_o, j)));
  }
  return PyObject_CallObject(strclass, strtuple);
}

static PyObject*
unmarshalPyObjectLongLong(cdrStream& stream, PyObject* d_o)
{
#ifdef HAS_LongLong
  CORBA::LongLong ll;
  ll <<= stream;
  return PyLong_FromLongLong(ll);
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
#endif
}

static PyObject*
unmarshalPyObjectULongLong(cdrStream& stream, PyObject* d_o)
{
#ifdef HAS_LongLong
  CORBA::ULongLong ull;
  ull <<= stream;
  return PyLong_FromUnsignedLongLong(ull);
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
#endif
}

static PyObject*
unmarshalPyObjectLongDouble(cdrStream& stream, PyObject* d_o)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
}

static PyObject*
unmarshalPyObjectWChar(cdrStream& stream, PyObject* d_o)
{
  OMNIORB_CHECK_TCS_W_FOR_UNMARSHAL(stream.TCS_W(), stream);

  Py_UNICODE  c   = stream.TCS_W()->unmarshalWChar(stream);
  PyObject*   r_o = PyUnicode_FromUnicode(0, 1);
  Py_UNICODE* str = PyUnicode_AS_UNICODE(r_o);
  str[0]          = c;
  str[1]          = 0;
  return r_o;
}

static PyObject*
unmarshalPyObjectWString(cdrStream& stream, PyObject* d_o)
{ // max_length

  OMNIORB_CHECK_TCS_W_FOR_UNMARSHAL(stream.TCS_W(), stream);

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);

  OMNIORB_ASSERT(Int_Check(t_o));

  CORBA::ULong max_len = Int_AS_LONG(t_o);

  omniCodeSet::UniChar* us;
  CORBA::ULong len = stream.TCS_W()->unmarshalWString(stream, max_len, us);

#if defined(Py_UNICODE_WIDE) || (PY_VERSION_HEX >= 0x03030000)

#  if _OMNIORB_HOST_BYTE_ORDER_ == 0
  int byteorder = 1;  // Big endian
#  else
  int byteorder = -1; // Little endian
#  endif
  PyObject* r_o = PyUnicode_DecodeUTF16((const char*)us, len*2, 0, &byteorder);

#else
  PyObject* r_o = PyUnicode_FromUnicode((Py_UNICODE*)us, len);
#endif

  omniCodeSetUtil::freeU(us);
  return r_o;
}

static PyObject*
unmarshalPyObjectFixed(cdrStream& stream, PyObject* d_o)
{ // digits, scale
  PyObject* t_o;

  t_o = PyTuple_GET_ITEM(d_o, 1); int dlimit = Int_AS_LONG(t_o);
  t_o = PyTuple_GET_ITEM(d_o, 2); int slimit = Int_AS_LONG(t_o);

  CORBA::Fixed f;
  f.PR_setLimits(dlimit, slimit);

  f <<= stream;

  return omniPy::newFixedObject(f);
}

// unmarshalPyObjectValue is in pyValueType.cc. It does both values
// and valueboxes.

static PyObject*
unmarshalPyObjectNative(cdrStream& stream, PyObject* d_o)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
}

// unmarshalPyObjectAbstractInterface is in pyAbstractIntf.cc

static PyObject*
unmarshalPyObjectLocalInterface(cdrStream& stream, PyObject* d_o)
{
  OMNIORB_THROW(MARSHAL, MARSHAL_LocalObject,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
}

PyObject*
omniPy::
unmarshalPyObjectIndirect(cdrStream& stream, PyObject* d_o)
{
  PyObject* l = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(PyList_Check(l));
  PyObject* d = PyList_GET_ITEM(l, 0);

  if (String_Check(d)) {
    // Indirection to a repoId -- find the corresponding descriptor
    d = PyDict_GetItem(pyomniORBtypeMap, d);
    if (!d)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_IncompletePythonType,
		    (CORBA::CompletionStatus)stream.completion());
    Py_INCREF(d);
    PyList_SetItem(l, 0, d);
  }
  return unmarshalPyObject(stream, d);
}


const omniPy::UnmarshalPyObjectFn omniPy::unmarshalPyObjectFns[] = {
  unmarshalPyObjectNull,
  unmarshalPyObjectVoid,
  unmarshalPyObjectShort,
  unmarshalPyObjectLong,
  unmarshalPyObjectUShort,
  unmarshalPyObjectULong,
  unmarshalPyObjectFloat,
  unmarshalPyObjectDouble,
  unmarshalPyObjectBoolean,
  unmarshalPyObjectChar,
  unmarshalPyObjectOctet,
  unmarshalPyObjectAny,
  unmarshalPyObjectTypeCode,
  unmarshalPyObjectPrincipal,
  unmarshalPyObjectObjref,
  unmarshalPyObjectStruct,
  unmarshalPyObjectUnion,
  unmarshalPyObjectEnum,
  unmarshalPyObjectString,
  unmarshalPyObjectSequence,
  unmarshalPyObjectArray,
  unmarshalPyObjectAlias,
  unmarshalPyObjectExcept,
  unmarshalPyObjectLongLong,
  unmarshalPyObjectULongLong,
  unmarshalPyObjectLongDouble,
  unmarshalPyObjectWChar,
  unmarshalPyObjectWString,
  unmarshalPyObjectFixed,
  omniPy::unmarshalPyObjectValue,
  omniPy::unmarshalPyObjectValue, // Same function as value
  unmarshalPyObjectNative,
  omniPy::unmarshalPyObjectAbstractInterface,
  unmarshalPyObjectLocalInterface
};



static PyObject*
copyArgumentNull(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus)
{
  if (a_o != Py_None)
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting None, got %r",
					    "O", a_o->ob_type));

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
copyArgumentVoid(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus)
{
  if (a_o != Py_None)
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting None, got %r",
					    "O", a_o->ob_type));

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
copyArgumentShort(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  long l;

#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o)) {
    l = PyInt_AS_LONG(a_o);
    if (l < -0x8000 || l > 0x7fff) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for short",
					      "O", a_o));
    }
    Py_INCREF(a_o); return a_o;
  }
  else
#endif
  if (PyLong_Check(a_o)) {
    l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for short",
					      "O", a_o));
    }
    else if (l < -0x8000 || l > 0x7fff) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for short",
					      "O", a_o));
    }
#if (PY_VERSION_HEX < 0x03000000)
    return PyInt_FromLong(l);
#else
    Py_INCREF(a_o); return a_o;
#endif
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting short, got %r",
					    "O", a_o->ob_type));
  }
  return 0;
}

static PyObject*
copyArgumentLong(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus)
{
#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o)) {
#  if SIZEOF_LONG > 4
    long l = PyInt_AS_LONG(a_o);
    if (l < -0x80000000L || l > 0x7fffffffL) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for long",
					      "O", a_o));
    }
#  endif
    Py_INCREF(a_o); return a_o;
  }
  else
#endif
  if (PyLong_Check(a_o)) {
    long l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for long",
					      "O", a_o));
    }
#if SIZEOF_LONG > 4
    if (l < -0x80000000L || l > 0x7fffffffL) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for long",
					      "O", a_o));
    }
#endif

#if (PY_VERSION_HEX < 0x03000000)
    return PyInt_FromLong(l);
#else
    Py_INCREF(a_o); return a_o;
#endif
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting long, got %r",
					    "O", a_o->ob_type));
  }
  return 0;
}

static PyObject*
copyArgumentUShort(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{
#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    if (l < 0 || l > 0xffff) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned short",
					      "O", a_o));
    }
    Py_INCREF(a_o); return a_o;
  }
  else
#endif
  if (PyLong_Check(a_o)) {
    long l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned short",
					      "O", a_o));
    }
    else if (l < 0 || l > 0xffff) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned short",
					      "O", a_o));
    }
#if (PY_VERSION_HEX < 0x03000000)
    return PyInt_FromLong(l);
#else
    Py_INCREF(a_o); return a_o;
#endif
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting unsigned short, got %r",
					    "O", a_o->ob_type));
  }
  return 0;
}

static PyObject*
copyArgumentULong(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  if (PyLong_Check(a_o)) {
    unsigned long ul = PyLong_AsUnsignedLong(a_o);
    if (ul == (unsigned long)-1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long",
					      "O", a_o));
    }
#if SIZEOF_LONG > 4
    if (ul > 0xffffffffL) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long",
					      "O", a_o));
    }
#endif
    Py_INCREF(a_o); return a_o;
  }
#if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
#  if SIZEOF_LONG > 4
    if (l < 0 || l > 0xffffffffL) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long",
					      "O", a_o));
    }
#  else
    if (l < 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long",
					      "O", a_o));
    }
#  endif
    return PyLong_FromLong(l);
  }
#endif
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting unsigned long, got %r",
					    "O", a_o->ob_type));
  }
  return 0;
}

static PyObject*
copyArgumentFloat(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  // *** This accepts values that are too big to fit in a float. It
  // *** should complain.

  if (PyFloat_Check(a_o)) {
    Py_INCREF(a_o); return a_o;
  }
#if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(a_o)) {
    return PyFloat_FromDouble((double)(PyInt_AS_LONG(a_o)));
  }
#endif
  else if (PyLong_Check(a_o)) {
    double d;
    d = PyLong_AsDouble(a_o);
    if (d == -1.0 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for float",
					      "O", a_o));
    }
    return PyFloat_FromDouble(d);
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting float, got %r",
					    "O", a_o->ob_type));
  }
  return 0;
}

static PyObject*
copyArgumentDouble(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{
  if (PyFloat_Check(a_o)) {
    Py_INCREF(a_o); return a_o;
  }
#if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(a_o)) {
    return PyFloat_FromDouble((double)(PyInt_AS_LONG(a_o)));
  }
#endif
  else if (PyLong_Check(a_o)) {
    double d;
    d = PyLong_AsDouble(a_o);
    if (d == -1.0 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for double",
					      "O", a_o));
    }
    return PyFloat_FromDouble(d);
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting double, got %r",
					    "O", a_o->ob_type));
  }
  return 0;
}

static PyObject*
copyArgumentBoolean(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus)
{
  if (PyBool_Check(a_o)) {
    Py_INCREF(a_o);
    return a_o;
  }

  int i = PyObject_IsTrue(a_o);

  if (i == -1) {
    if (omniORB::trace(1))
      PyErr_Print();
    else
      PyErr_Clear();

    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting bool, got %r",
					    "O", a_o->ob_type));
  }

  PyObject* r_o = i ? Py_True : Py_False;
  Py_INCREF(r_o);
  return r_o;
}

static PyObject*
copyArgumentChar(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus)
{
  if (!String_Check(a_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting string, got %r",
					    "O", a_o->ob_type));
  }
  if (String_GET_SIZE(a_o) != 1) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting string of length 1, "
					    "got %r",
					    "O", a_o));
  }
  Py_INCREF(a_o); return a_o;
}

static PyObject*
copyArgumentOctet(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
#if (PY_VERSION_HEX < 0x03000000)
  if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    if (l < 0 || l > 0xff) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for octet",
					      "O", a_o));
    }
    Py_INCREF(a_o); return a_o;
  }
  else
#endif
  if (PyLong_Check(a_o)) {
    long l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for octet",
					      "O", a_o));
    }
    if (l < 0 || l > 0xff) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for octet",
					      "O", a_o));
    }
#if (PY_VERSION_HEX < 0x03000000)
    return PyInt_FromLong(l);
#else
    Py_INCREF(a_o); return a_o;
#endif
  }
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting octet, got %r",
					    "O", a_o->ob_type));
  }
  return 0;
}

static PyObject*
copyArgumentAny(PyObject* d_o, PyObject* a_o,
		CORBA::CompletionStatus compstatus)
{
  if (!PyObject_IsInstance(a_o, omniPy::pyCORBAAnyClass)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting Any, got %r",
					    "O", a_o->ob_type));
  }

  // TypeCode
  omniPy::PyRefHolder tc(PyObject_GetAttrString(a_o, (char*)"_t"));

  if (!tc.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       String_FromString("Any has no TypeCode _t"));
  }

  if (!PyObject_IsInstance(tc, omniPy::pyCORBATypeCodeClass)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting TypeCode in Any, got %r",
					    "O", a_o->ob_type));
  }

  omniPy::PyRefHolder desc(PyObject_GetAttrString(tc, (char*)"_d"));
  if (!desc.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       String_FromString("TypeCode in Any has no "
                                         "descriptor _d"));
  }

  // Any's contents
  omniPy::PyRefHolder val(PyObject_GetAttrString(a_o, (char*)"_v"));
  if (!val.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       String_FromString("Any has no value _v"));
  }

  // Copy contents
  PyObject* cval;

  try {
    cval = omniPy::copyArgument(desc, val, compstatus);
  }
  catch (Py_BAD_PARAM& bp) {
    bp.add(String_FromString("Value inside Any"));
    throw;
  }

  // Construct new Any
  omniPy::PyRefHolder argtuple(PyTuple_New(2));
  Py_INCREF(tc);
  PyTuple_SET_ITEM(argtuple, 0, tc);
  PyTuple_SET_ITEM(argtuple, 1, cval);
  
  return PyObject_CallObject(omniPy::pyCORBAAnyClass, argtuple);
}

static PyObject*
copyArgumentTypeCode(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus)
{
  if (!PyObject_IsInstance(a_o, omniPy::pyCORBATypeCodeClass)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting TypeCode, got %r",
					    "O", a_o->ob_type));
  }
  PyObject* desc = PyObject_GetAttrString(a_o, (char*)"_d");

  if (!desc) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       String_FromString("TypeCode in has no descriptor _d"));
  }
  Py_DECREF(desc);
  Py_INCREF(a_o); return a_o;
}

static PyObject*
copyArgumentPrincipal(PyObject* d_o, PyObject* a_o,
		      CORBA::CompletionStatus compstatus)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
  return 0;
}

static PyObject*
copyArgumentObjref(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{ // repoId, name

  return omniPy::copyObjRefArgument(PyTuple_GET_ITEM(d_o, 1),
				    a_o, compstatus);
}

static PyObject*
copyArgumentStruct(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{ // class, repoId, struct name, name, descriptor, ...

  // The descriptor tuple has twice the number of struct members,
  // plus 4 -- the typecode kind, the Python class, the repoId,
  // and the struct name
  int cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  PyObject* t_o;
  PyObject* name;
  omniPy::PyRefHolder value;
  omniPy::PyRefHolder argtuple(PyTuple_New(cnt));

  int i, j;

  for (i=0,j=4; i < cnt; i++,j++) {
    name  = PyTuple_GET_ITEM(d_o, j++); OMNIORB_ASSERT(String_Check(name));
    value = PyObject_GetAttr(a_o, name);

    if (value.valid()) {
      try {
	t_o = omniPy::copyArgument(PyTuple_GET_ITEM(d_o, j),
				   value, compstatus);
      }
      catch (Py_BAD_PARAM& bp) {
	bp.add(omniPy::formatString("Struct %r member %r", "OO",
				    PyTuple_GET_ITEM(d_o, 3), name));
	throw;
      }
      PyTuple_SET_ITEM(argtuple, i, t_o);
    }
    else {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Struct %r instance %r "
					      "has no %r member",
					      "OOO",
					      PyTuple_GET_ITEM(d_o, 3),
					      a_o->ob_type,
					      name));
    }
  }
  return PyObject_CallObject(PyTuple_GET_ITEM(d_o, 1), argtuple);
}

static PyObject*
copyArgumentUnion(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{ // class,
  // repoId,
  // name,
  // discriminant descr,
  // default used,
  // ((label value, member name, member descr), ...),
  // default (label, name, descr) or None,
  // {label: (label, name, descr), ...}

  omniPy::PyRefHolder discr(PyObject_GetAttrString(a_o, (char*)"_d"));
  if (!discr.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting union, got %r",
					    "O", a_o->ob_type));
  }

  omniPy::PyRefHolder value(PyObject_GetAttrString(a_o, (char*)"_v"));
  if (!value.valid()) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting union, got %r",
					    "O", a_o->ob_type));
  }

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 4);
  PyObject* cdiscr;

  try {
    cdiscr = omniPy::copyArgument(t_o, discr, compstatus);
  }
  catch (Py_BAD_PARAM& bp) {
    bp.add(String_FromString("Union discriminant"));
    throw;
  }

  omniPy::PyRefHolder cdiscr_holder(cdiscr);

  PyObject* cvalue = 0;
  PyObject* cdict  = PyTuple_GET_ITEM(d_o, 8);
  t_o              = PyDict_GetItem(cdict, discr);
  if (t_o) {
    // Discriminant found in case dictionary
    OMNIORB_ASSERT(PyTuple_Check(t_o));
    cvalue = omniPy::copyArgument(PyTuple_GET_ITEM(t_o, 2), value, compstatus);
  }
  else {
    // Is there a default case?
    t_o = PyTuple_GET_ITEM(d_o, 7);
    if (t_o == Py_None) {
      // No default
      Py_INCREF(Py_None);
      cvalue = Py_None;
    }
    else {
      OMNIORB_ASSERT(PyTuple_Check(t_o));
      try {
	cvalue = omniPy::copyArgument(PyTuple_GET_ITEM(t_o, 2),
				      value, compstatus);
      }
      catch (Py_BAD_PARAM& bp) {
	bp.add(omniPy::formatString("Union member %r", "O",
				    PyTuple_GET_ITEM(t_o, 1)));
	throw;
      }
    }
  }
  t_o = PyTuple_New(2);
  PyTuple_SET_ITEM(t_o, 0, cdiscr_holder.retn());
  PyTuple_SET_ITEM(t_o, 1, cvalue);
  PyObject* r_o = PyObject_CallObject(PyTuple_GET_ITEM(d_o, 1), t_o);
  Py_DECREF(t_o);
  return r_o;
}


static PyObject*
copyArgumentEnum(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus)
{ // repoId, name, item list

  omniPy::PyRefHolder ev(PyObject_GetAttrString(a_o, (char*)"_v"));

  if (!(ev.valid() && Int_Check(ev))) {
    PyErr_Clear();
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting enum %r item, got %r",
					    "OO",
					    PyTuple_GET_ITEM(d_o, 2),
					    a_o->ob_type));
  }

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 3);
  long      e   = Int_AS_LONG(ev);

  if (e >= PyTuple_GET_SIZE(t_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_EnumValueOutOfRange, compstatus,
		       omniPy::formatString("Expecting enum %r item, got %r",
					    "OO",
					    PyTuple_GET_ITEM(d_o, 2),
					    a_o));
  }
  if (PyTuple_GET_ITEM(t_o, e) != a_o) {
    // EnumItem object is not the one we expected -- are they equivalent?
    int cmp;

#if (PY_VERSION_HEX < 0x03000000)

    if (PyObject_Cmp(PyTuple_GET_ITEM(t_o, e), a_o, &cmp) == -1)
      omniPy::handlePythonException();

    if (cmp != 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting enum %r item, "
					      "got %r",
					      "OO",
					      PyTuple_GET_ITEM(d_o, 2),
					      a_o));
    }

#else
    cmp = PyObject_RichCompareBool(PyTuple_GET_ITEM(t_o, e), a_o, Py_EQ);
    if (cmp == -1)
      omniPy::handlePythonException();

    if (cmp != 1) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting enum %r item, "
					      "got %r",
					      "OO",
					      PyTuple_GET_ITEM(d_o, 2),
					      a_o));
    }
#endif
    a_o = PyTuple_GET_ITEM(t_o, e);
  }
  Py_INCREF(a_o); return a_o;
}


static PyObject*
copyArgumentString(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{ // max_length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);
  OMNIORB_ASSERT(Int_Check(t_o));

  CORBA::ULong max_len = Int_AS_LONG(t_o);

  if (!String_Check(a_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting string, got %r",
					    "O", a_o->ob_type));
  }

  CORBA::ULong len = String_GET_SIZE(a_o);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, compstatus);

  // Annoyingly, we have to scan the string to check there are no
  // nulls

#if (PY_VERSION_HEX < 0x03000000)

  char* str = PyString_AS_STRING(a_o);
  for (CORBA::ULong i=0; i != len; ++i) {
    if (str[i] == '\0') {
      THROW_PY_BAD_PARAM(BAD_PARAM_EmbeddedNullInPythonString, compstatus,
			 omniPy::formatString("Embedded null in string "
					      "at position %d",
					      "i", i));
    }
  }

#elif (PY_VERSION_HEX < 0x03030000)

  Py_UNICODE* us = PyUnicode_AS_UNICODE(a_o);
  for (CORBA::ULong i=0; i != len; ++i) {
    if (us[i] == 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_EmbeddedNullInPythonString, compstatus,
			 omniPy::formatString("Embedded null in string "
					      "at position %d",
					      "i", i));
    }
  }

#else

  int   kind = PyUnicode_KIND(a_o);
  void* data = PyUnicode_DATA(a_o);

  for (CORBA::ULong i=0; i != len; ++i) {
    Py_UCS4 uc = PyUnicode_READ(kind, data, i);
    if (uc == 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_EmbeddedNullInPythonString, compstatus,
			 omniPy::formatString("Embedded null in string "
					      "at position %d",
					      "i", i));
    }
  }
#endif

  // After all that, we don't actually have to copy the string,
  // since they're immutable
  Py_INCREF(a_o);
  return a_o;
}


template<class G>
inline PyObject*
copyOptSequenceItems(CORBA::ULong            len,
                     PyObject*               a_o,
                     CORBA::ULong            etk,
                     CORBA::CompletionStatus compstatus,
                     const char*             seq_arr,
                     G                       getFn)
{
  omniPy::PyRefHolder r_o(PyList_New(len));

  PyObject*        t_o;
  CORBA::ULong     i;

  long             long_val;
  unsigned long    ulong_val;
  double           double_val;
#ifdef HAS_LongLong
  CORBA::LongLong  llong_val;
  CORBA::ULongLong ullong_val;
#endif

  switch (etk) {

  case CORBA::tk_short:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
      if (PyInt_Check(t_o)) {
        long_val = PyInt_AS_LONG(t_o);
        if (long_val >= -0x8000 && long_val <= 0x7fff) {
          Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
        }
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "short", "siO",
                                                seq_arr, i, t_o));
      }
      else
#endif
      if (PyLong_Check(t_o)) {
        long_val = PyLong_AsLong(t_o);
        if (long_val == -1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "short", "siO",
                                                  seq_arr, i, t_o));
        }
        if (long_val >= -0x8000 && long_val <= 0x7fff) {
#if (PY_VERSION_HEX < 0x03000000)
          PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
#else
          Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
#endif
        }
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "short", "siO",
                                                seq_arr, i, t_o));
      }
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                         omniPy::formatString("%s item %d: "
                                              "expecting short, "
                                              "got %r", "siO",
                                              seq_arr, i, t_o->ob_type));
    }
    break;

  case CORBA::tk_long:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
      if (PyInt_Check(t_o)) {
#  if SIZEOF_LONG > 4
        long_val = PyInt_AS_LONG(t_o);
        if (long_val >= -0x80000000L && long_val <= 0x7fffffffL) {
#  endif
          Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
#  if SIZEOF_LONG > 4
        }
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "long", "siO",
                                                seq_arr, i, t_o));
#  endif
      }
      else
#endif
      if (PyLong_Check(t_o)) {
        long_val = PyLong_AsLong(t_o);
        if (long_val == -1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "long", "siO",
                                                  seq_arr, i, t_o));
        }
#if SIZEOF_LONG > 4
        if (long_val >= -0x80000000L && long_val <= 0x7fffffffL) {
#endif
#if (PY_VERSION_HEX < 0x03000000)
          PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
#else
          Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
#endif
#if SIZEOF_LONG > 4
        }
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "long", "siO",
                                                seq_arr, i, t_o));
#endif
      }
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                         omniPy::formatString("%s item %d: "
                                              "expecting long, "
                                              "got %r", "siO",
                                              seq_arr, i, t_o->ob_type));
    }
    break;

  case CORBA::tk_ushort:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

#if (PY_VERSION_HEX < 0x03000000)
      if (PyInt_Check(t_o)) {
        long_val = PyInt_AS_LONG(t_o);
        if (long_val >= 0 && long_val <= 0xffff) {
          Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
        }
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "unsigned short", "siO",
                                                seq_arr, i, t_o));
      }
      else
#endif
      if (PyLong_Check(t_o)) {
        long_val = PyLong_AsLong(t_o);
        if (long_val == -1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "unsigned short", "siO",
                                                  seq_arr, i, t_o));
        }
        if (long_val >= 0 && long_val <= 0xffff) {
#if (PY_VERSION_HEX < 0x03000000)
          PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
#else
          Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
#endif
        }
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "unsigned short", "siO",
                                                seq_arr, i, t_o));
      }
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                         omniPy::formatString("%s item %d: "
                                              "expecting unsigned short, "
                                              "got %r", "siO",
                                              seq_arr, i, t_o->ob_type));
    }
    break;

  case CORBA::tk_ulong:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

      if (PyLong_Check(t_o)) {
        ulong_val = PyLong_AsUnsignedLong(t_o);
        if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "unsigned long", "siO",
                                                  seq_arr, i, t_o));
        }
#if SIZEOF_LONG > 4
        if (ulong_val > 0xffffffffL) {
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "unsigned long", "siO",
                                                  seq_arr, i, t_o));
        }
#endif
        Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
      }
#if (PY_VERSION_HEX < 0x03000000)
      else if (PyInt_Check(t_o)) {
        long_val = PyInt_AS_LONG(t_o);
#  if SIZEOF_LONG > 4
        if (long_val >= 0 && long_val <= 0xffffffffL) {
          PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
          continue;
        }
#  else
        if (long_val >= 0) {
          PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
          continue;
        }
#  endif
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "unsigned long", "siO",
                                                seq_arr, i, t_o));
      }
#endif
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                         omniPy::formatString("%s item %d: "
                                              "expecting unsigned long, "
                                              "got %r", "siO",
                                              seq_arr, i, t_o->ob_type));
    }
    break;

  case CORBA::tk_float:
  case CORBA::tk_double:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

      if (PyFloat_Check(t_o)) {
        Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o);
      }
#if (PY_VERSION_HEX < 0x03000000)
      else if (PyInt_Check(t_o)) {
        PyList_SET_ITEM(r_o, i,
                        PyFloat_FromDouble((double)PyInt_AS_LONG(t_o)));
      }
#endif
      else if (PyLong_Check(t_o)) {
        double_val = PyLong_AsDouble(t_o);
        if (double_val == -1.0 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "double", "siO",
                                                  seq_arr, i, t_o));
        }
        PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(double_val));
      }
      else {
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting double, "
                                                "got %r", "siO",
                                                seq_arr, i, t_o->ob_type));
      }
    }
    break;

  case CORBA::tk_boolean:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

      int b = PyObject_IsTrue(t_o);
      if (b == -1) {
        if (omniORB::trace(1))
          PyErr_Print();
        else
          PyErr_Clear();

        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting bool, "
                                                "got %r", "siO",
                                                seq_arr, i, t_o->ob_type));
      }
      t_o = b ? Py_True : Py_False;
      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o);
    }
    break;

#ifdef HAS_LongLong

  case CORBA::tk_longlong:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

      if (PyLong_Check(t_o)) {
        llong_val = PyLong_AsLongLong(t_o);
        if (llong_val == -1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "long long", "siO",
                                                  seq_arr, i, t_o));
        }
        Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
      }
#if (PY_VERSION_HEX < 0x03000000)
      else if (PyInt_Check(t_o)) {
        long_val = PyInt_AS_LONG(t_o);
        PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
      }
#endif
      else {
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting long long, "
                                                "got %r", "siO",
                                                seq_arr, i, t_o->ob_type));
      }
    }
    break;

  case CORBA::tk_ulonglong:

    for (i=0; i<len; i++) {
      t_o = getFn(a_o, i);

      if (PyLong_Check(t_o)) {
        ullong_val = PyLong_AsUnsignedLongLong(t_o);
        if (ullong_val == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
          PyErr_Clear();
          THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                             omniPy::formatString("%s item %d: "
                                                  "%s is out of range for "
                                                  "unsigned long long", "siO",
                                                  seq_arr, i, t_o));
        }
        Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
      }
#if (PY_VERSION_HEX < 0x03000000)
      else if (PyInt_Check(t_o)) {
        long_val = PyInt_AS_LONG(t_o);
        if (long_val >= 0) {
          PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
          continue;
        }
        THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "%s is out of range for "
                                                "unsigned long long", "siO",
                                                seq_arr, i, t_o));
      }
#endif
      else {
        THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
                           omniPy::formatString("%s item %d: "
                                                "expecting unsigned "
                                                "long long, got %r", "siO",
                                                seq_arr, i, t_o->ob_type));
      }
    }
    break;
#else
  case 23:
  case 24:
    {
      OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
    }
#endif
  default:
    OMNIORB_ASSERT(0);
  }
  return r_o.retn();
}


static PyObject*
copyArgumentSequence(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus)
{ // element_desc, max_length

  PyObject*    t_o      = PyTuple_GET_ITEM(d_o, 2);
  OMNIORB_ASSERT(Int_Check(t_o));
  CORBA::ULong max_len  = Int_AS_LONG(t_o);
  PyObject*    elm_desc = PyTuple_GET_ITEM(d_o, 1);

  CORBA::ULong len, i;
  CORBA::ULong etk;

  if (sequenceOptimisedType(elm_desc, etk)) { // Simple type

    if (etk == CORBA::tk_octet) {
      // Mapping says octet and char use a string
      if (!RawString_Check(a_o)) {
	THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			   omniPy::formatString("Expecting bytes, got %r",
						"O", a_o->ob_type));
      }
      len = RawString_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      Py_INCREF(a_o);
      return a_o;
    }
    else if (etk == CORBA::tk_char) {
      // Mapping says octet and char use a string
      if (!String_Check(a_o)) {
	THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			   omniPy::formatString("Expecting string, got %r",
						"O", a_o->ob_type));
      }
      len = String_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      Py_INCREF(a_o);
      return a_o;
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      return copyOptSequenceItems(len, a_o, etk, compstatus,
                                  "Sequence", listGet);
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      return copyOptSequenceItems(len, a_o, etk, compstatus,
                                  "Sequence", tupleGet);
    }
    else {
      // Not a list or a tuple
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting sequence, got %r",
					      "O", a_o->ob_type));
    }
  }
  else {
    // Complex type

    if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      omniPy::PyRefHolder r_o(PyList_New(len));

      for (i=0; i < len; i++) {
	try {
	  t_o = omniPy::copyArgument(elm_desc, PyList_GET_ITEM(a_o, i),
				     compstatus);
	}
	catch (Py_BAD_PARAM& bp) {
	  bp.add(omniPy::formatString("Sequence item %d", "i", i));
	  throw;
	}
	PyList_SET_ITEM(r_o, i, t_o);
      }
      return r_o.retn();
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      omniPy::PyRefHolder r_o(PyList_New(len));

      for (i=0; i < len; i++) {
	try {
	  t_o = omniPy::copyArgument(elm_desc, PyTuple_GET_ITEM(a_o, i),
				     compstatus);
	}
	catch (Py_BAD_PARAM& bp) {
	  bp.add(omniPy::formatString("Sequence item %d", "i", i));
	  throw;
	}
	PyList_SET_ITEM(r_o, i, t_o);
      }
      return r_o.retn();
    }
    else {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting sequence, got %r",
					      "O", a_o->ob_type));
    }
  }
  return 0;
}

static PyObject*
copyArgumentArray(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{ // element_desc, length

  PyObject*    t_o      = PyTuple_GET_ITEM(d_o, 2);
  OMNIORB_ASSERT(Int_Check(t_o));
  CORBA::ULong arr_len  = Int_AS_LONG(t_o);
  PyObject*    elm_desc = PyTuple_GET_ITEM(d_o, 1);

  CORBA::ULong len, i;
  CORBA::ULong etk;

  if (sequenceOptimisedType(elm_desc, etk)) { // Simple type

    if (etk == CORBA::tk_octet) {
      // Mapping says octet and char use a string
      if (!RawString_Check(a_o)) {
	THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			   omniPy::formatString("Expecting bytes, got %r",
						"O", a_o->ob_type));
      }

      len = RawString_GET_SIZE(a_o);
      if (len != arr_len) {
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting bytes length %d, "
						"got %d",
						"ii", arr_len, len));
      }
      Py_INCREF(a_o);
      return a_o;
    }
    else if (etk == CORBA::tk_char) {
      // Mapping says octet and char use a string
      if (!String_Check(a_o)) {
	THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			   omniPy::formatString("Expecting string, got %r",
						"O", a_o->ob_type));
      }

      len = String_GET_SIZE(a_o);
      if (len != arr_len) {
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting string length %d, "
						"got %d",
						"ii", arr_len, len));
      }
      Py_INCREF(a_o);
      return a_o;
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (len != arr_len)
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting array length %d, "
						"got %d",
						"ii", arr_len, len));

      
      return copyOptSequenceItems(len, a_o, etk, compstatus,
                                  "Array", listGet);
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (len != arr_len)
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting array length %d, "
						"got %d",
						"ii", arr_len, len));

      return copyOptSequenceItems(len, a_o, etk, compstatus,
                                  "Array", tupleGet);

    }
    else {
      // Not a list or a tuple
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting array, got %r",
					      "O", a_o->ob_type));
    }
  }
  else {
    // Complex type

    if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (len != arr_len) {
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting array length %d, "
						"got %d",
						"ii", arr_len, len));
      }

      omniPy::PyRefHolder r_o(PyList_New(len));

      for (i=0; i < len; i++) {
	try {
	  t_o = omniPy::copyArgument(elm_desc, PyList_GET_ITEM(a_o, i),
				     compstatus);
	}
	catch (Py_BAD_PARAM& bp) {
	  bp.add(omniPy::formatString("Array item %d", "i", i));
	  throw;
	}
	PyList_SET_ITEM(r_o, i, t_o);
      }
      return r_o.retn();
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (len != arr_len) {
	THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			   omniPy::formatString("Expecting array length %d, "
						"got %d",
						"ii", arr_len, len));
      }

      omniPy::PyRefHolder r_o(PyList_New(len));

      for (i=0; i < len; i++) {
	try {
	  t_o = omniPy::copyArgument(elm_desc, PyTuple_GET_ITEM(a_o, i),
				     compstatus);
	}
	catch (Py_BAD_PARAM& bp) {
	  bp.add(omniPy::formatString("Array item %d", "i", i));
	  throw;
	}
	PyList_SET_ITEM(r_o, i, t_o);
      }
      return r_o.retn();
    }
    else {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Expecting array, got %r",
					      "O", a_o->ob_type));
    }
  }
  return 0;
}

static PyObject*
copyArgumentAlias(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{ // repoId, name, descr

  return omniPy::copyArgument(PyTuple_GET_ITEM(d_o, 3), a_o, compstatus);
}

static PyObject*
copyArgumentExcept(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{ // class, repoId, exc name, name, descriptor, ...

  // As with structs, the descriptor tuple has twice the number of
  // members plus 4.
  int cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  PyObject* t_o;
  PyObject* name;
  omniPy::PyRefHolder value;
  omniPy::PyRefHolder argtuple(PyTuple_New(cnt));

  int i, j;

  for (i=0,j=4; i < cnt; i++,j++) {
    name = PyTuple_GET_ITEM(d_o, j++); OMNIORB_ASSERT(String_Check(name));
    value = PyObject_GetAttr(a_o, name);

    if (value.valid()) {
      try {
	t_o = omniPy::copyArgument(PyTuple_GET_ITEM(d_o, j),
				   value, compstatus);
      }
      catch (Py_BAD_PARAM& bp) {
	bp.add(omniPy::formatString("Exception %r member %r", "OO",
				    PyTuple_GET_ITEM(d_o, 3), name));
	throw;
      }
      PyTuple_SET_ITEM(argtuple, i, t_o);
    }
    else {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
			 omniPy::formatString("Exception %r instance %r "
					      "has no %r member",
					      "OOO",
					      PyTuple_GET_ITEM(d_o, 3),
					      a_o->ob_type,
					      name));
    }
  }
  return PyObject_CallObject(PyTuple_GET_ITEM(d_o, 1), argtuple);
}


static PyObject*
copyArgumentLongLong(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus)
{
#ifdef HAS_LongLong
  if (PyLong_Check(a_o)) {
    CORBA::LongLong ll = PyLong_AsLongLong(a_o);
    if (ll == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "long long",
					      "O", a_o));
    }
    Py_INCREF(a_o); return a_o;
  }
#if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    return PyLong_FromLong(l);
  }
#endif
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting long long, got %r",
					    "O", a_o->ob_type));
  }
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
  return 0;
}


static PyObject*
copyArgumentULongLong(PyObject* d_o, PyObject* a_o,
		      CORBA::CompletionStatus compstatus)
{
#ifdef HAS_LongLong
  if (PyLong_Check(a_o)) {
    CORBA::ULongLong ll = PyLong_AsUnsignedLongLong(a_o);
    if (ll == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
      PyErr_Clear();
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long long",
					      "O", a_o));
    }
    Py_INCREF(a_o); return a_o;
  }
#if (PY_VERSION_HEX < 0x03000000)
  else if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    if (l < 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_PythonValueOutOfRange, compstatus,
			 omniPy::formatString("%s is out of range for "
					      "unsigned long long",
					      "O", a_o));
    }
    return PyLong_FromLong(l);
  }
#endif
  else {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting long long, got %r",
					    "O", a_o->ob_type));
  }
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
  return 0;
}

static PyObject*
copyArgumentLongDouble(PyObject* d_o, PyObject* a_o,
		       CORBA::CompletionStatus compstatus)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
  return 0;
}

static PyObject*
copyArgumentWChar(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  if (!PyUnicode_Check(a_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting unicode, got %r",
					    "O", a_o->ob_type));
  }
  if (PyUnicode_GET_SIZE(a_o) != 1) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting unicode of length 1, "
					    "got %r",
					    "O", a_o));
  }
  Py_INCREF(a_o); return a_o;
}


static PyObject*
copyArgumentWString(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus)
{ // max_length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);
  OMNIORB_ASSERT(Int_Check(t_o));

  CORBA::ULong max_len = Int_AS_LONG(t_o);

  if (!PyUnicode_Check(a_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting unicode, got %r",
					    "O", a_o->ob_type));
  }

#if (PY_VERSION_HEX < 0x03030000) // Earlier than Python 3.3

  CORBA::ULong len = PyUnicode_GET_SIZE(a_o);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, compstatus);

  // Check for nulls
  Py_UNICODE* str = PyUnicode_AS_UNICODE(a_o);
  for (CORBA::ULong i=0; i<len; i++) {
    if (str[i] == 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_EmbeddedNullInPythonString, compstatus,
			 omniPy::formatString("Embedded null in unicode "
					      "at position %d",
					      "i", i));
    }
  }

#else // New Unicode API

  CORBA::ULong len = PyUnicode_GET_LENGTH(a_o);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, compstatus);

  // Check for nulls
  int   kind = PyUnicode_KIND(a_o);
  void* data = PyUnicode_DATA(a_o);

  for (CORBA::ULong i=0; i<len; i++) {
    if (PyUnicode_READ(kind, data, i) == 0) {
      THROW_PY_BAD_PARAM(BAD_PARAM_EmbeddedNullInPythonString, compstatus,
			 omniPy::formatString("Embedded null in unicode "
					      "at position %d", "i", i));
    }
  }
#endif

  // After all that, we don't actually have to copy the string,
  // since they're immutable
  Py_INCREF(a_o);
  return a_o;
}


static PyObject*
copyArgumentFixed(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  if (!omnipyFixed_Check(a_o)) {
    THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, compstatus,
		       omniPy::formatString("Expecting fixed, got %r",
					    "O", a_o->ob_type));
  }

  PyObject* t_o;

  t_o = PyTuple_GET_ITEM(d_o, 1); int dlimit = Int_AS_LONG(t_o);
  t_o = PyTuple_GET_ITEM(d_o, 2); int slimit = Int_AS_LONG(t_o);

  CORBA::Fixed f(*((omnipyFixedObject*)a_o)->ob_fixed);
  f.PR_setLimits(dlimit, slimit);
  return omniPy::newFixedObject(f);
}


static PyObject*
copyArgumentNative(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
  return 0;
}


// copyArgumentAbstractInterface is in pyAbstractIntf.cc

static PyObject*
copyArgumentLocalInterface(PyObject* d_o, PyObject* a_o,
			   CORBA::CompletionStatus compstatus)
{
  Py_INCREF(a_o);
  return a_o;
}

PyObject*
omniPy::
copyArgumentIndirect(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus)
{
  PyObject* l = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(PyList_Check(l));
  PyObject* d = PyList_GET_ITEM(l, 0);

  if (String_Check(d)) {
    // Indirection to a repoId -- find the corresponding descriptor
    d = PyDict_GetItem(pyomniORBtypeMap, d);
    if (!d) OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_IncompletePythonType,
			  compstatus);

    Py_INCREF(d);
    PyList_SetItem(l, 0, d);
  }
  return copyArgument(d, a_o, compstatus);
}


const omniPy::CopyArgumentFn omniPy::copyArgumentFns[] = {
  copyArgumentNull,
  copyArgumentVoid,
  copyArgumentShort,
  copyArgumentLong,
  copyArgumentUShort,
  copyArgumentULong,
  copyArgumentFloat,
  copyArgumentDouble,
  copyArgumentBoolean,
  copyArgumentChar,
  copyArgumentOctet,
  copyArgumentAny,
  copyArgumentTypeCode,
  copyArgumentPrincipal,
  copyArgumentObjref,
  copyArgumentStruct,
  copyArgumentUnion,
  copyArgumentEnum,
  copyArgumentString,
  copyArgumentSequence,
  copyArgumentArray,
  copyArgumentAlias,
  copyArgumentExcept,
  copyArgumentLongLong,
  copyArgumentULongLong,
  copyArgumentLongDouble,
  copyArgumentWChar,
  copyArgumentWString,
  copyArgumentFixed,
  omniPy::copyArgumentValue,
  omniPy::copyArgumentValueBox,
  copyArgumentNative,
  omniPy::copyArgumentAbstractInterface,
  copyArgumentLocalInterface
};



//
// PyUnlockingCdrStream
//

void
omniPy::
PyUnlockingCdrStream::put_octet_array(const _CORBA_Octet* b, int size,
				      omni::alignment_t align)
{
  omniPy::InterpreterUnlocker _u;
  cdrStreamAdapter::put_octet_array(b, size, align);
}

void
omniPy::
PyUnlockingCdrStream::get_octet_array(_CORBA_Octet* b,int size,
				      omni::alignment_t align)
{
  omniPy::InterpreterUnlocker _u;
  cdrStreamAdapter::get_octet_array(b, size, align);
}
  
void
omniPy::
PyUnlockingCdrStream::skipInput(_CORBA_ULong size)
{
  omniPy::InterpreterUnlocker _u;
  cdrStreamAdapter::skipInput(size);
}

void
omniPy::
PyUnlockingCdrStream::copy_to(cdrStream& stream, int size,
			      omni::alignment_t align)
{
  omniPy::InterpreterUnlocker _u;
  cdrStreamAdapter::copy_to(stream, size, align);
}

void
omniPy::
PyUnlockingCdrStream::fetchInputData(omni::alignment_t align, size_t required)
{
  omniPy::InterpreterUnlocker _u;
  cdrStreamAdapter::fetchInputData(align, required);
}

_CORBA_Boolean
omniPy::
PyUnlockingCdrStream::
reserveOutputSpaceForPrimitiveType(omni::alignment_t align, size_t required)
{
  omniPy::InterpreterUnlocker _u;
  return cdrStreamAdapter::reserveOutputSpaceForPrimitiveType(align, required);
}

_CORBA_Boolean
omniPy::
PyUnlockingCdrStream::
maybeReserveOutputSpace(omni::alignment_t align, size_t required)
{
  omniPy::InterpreterUnlocker _u;
  return cdrStreamAdapter::maybeReserveOutputSpace(align, required);
}
