// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyFixed.cc                 Created on: 2001/03/30
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2014 Apasphere Ltd
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
// Description:
//    Implementation of Fixed type for Python

#include <omnipy.h>
#include <pyFixed.h>
#include <stdio.h>

static PyObject*
fixedValueAsPyLong(const CORBA::Fixed& f)
{
  CORBA::String_var str = f.NP_asString();

  // Seek out and destroy any decimal point
  char* c;

  for (c = (char*)str; *c && *c != '.'; ++c);

  if (*c == '.') {
    for (; *c; ++c)
      *c = *(c+1);
  }
  return PyLong_FromString((char*)str, 0, 10);
}

PyObject*
omniPy::newFixedObject(const CORBA::Fixed& f)
{
  omnipyFixedObject* pyf = PyObject_New(omnipyFixedObject, &omnipyFixed_Type);
  OMNIORB_ASSERT(Py_TYPE(pyf)->tp_free);

  pyf->ob_fixed = new CORBA::Fixed(f);
  return (PyObject*)pyf;
}

PyObject*
omniPy::newFixedObject(PyObject* self, PyObject* args)
{
  try {
    int size = PyTuple_Size(args);
    if (size == 1) {
      PyObject* pyv = PyTuple_GetItem(args, 0);

      if (String_Check(pyv)) {
	CORBA::Fixed f(String_AsString(pyv));
	return omniPy::newFixedObject(f);
      }
#if (PY_VERSION_HEX <= 0x03000000)
      else if (PyInt_Check(pyv)) {
	long l = PyInt_AsLong(pyv);
	CORBA::Fixed f(l);
	return omniPy::newFixedObject(f);
      }
#endif
      else if (PyLong_Check(pyv)) {
	PyObject* pystr = PyObject_Str(pyv);
	omniPy::PyRefHolder pystr_holder(pystr);
	CORBA::Fixed f;
	f.NP_fromString(String_AsString(pystr), 1);
	return omniPy::newFixedObject(f);
      }
      else if (omnipyFixed_Check(pyv)) {
	return omniPy::newFixedObject(*((omnipyFixedObject*)pyv)->ob_fixed);
      }
    }
    else if (size == 3) {
      PyObject* pyd = PyTuple_GetItem(args, 0);
      PyObject* pys = PyTuple_GetItem(args, 1);
      PyObject* pyv = PyTuple_GetItem(args, 2);

      if (Int_Check(pyd) && Int_Check(pys)) {
	long digits = Int_AS_LONG(pyd);
	long scale  = Int_AS_LONG(pys);

	if (digits < 0 || digits > 31)
	  OMNIORB_THROW(DATA_CONVERSION,
			DATA_CONVERSION_RangeError,
			CORBA::COMPLETED_NO);

	if (scale < 0 || scale > digits)
	  OMNIORB_THROW(DATA_CONVERSION,
			DATA_CONVERSION_RangeError,
			CORBA::COMPLETED_NO);

	// The standard interface to fixed creation provides an int or
	// long representing the required value of the fixed times
	// 10**scale. We first convert the int/long into a Fixed with
	// an integer value, then hack the scale to be correct.

#if (PY_VERSION_HEX <= 0x03000000)
	if (PyInt_Check(pyv)) {
	  long l = PyInt_AsLong(pyv);
	  CORBA::Fixed f(l);
	  f.PR_changeScale(scale);
	  f.PR_setLimits(digits, scale);
	  return omniPy::newFixedObject(f);
	}
	else
#endif
        if (PyLong_Check(pyv)) {
	  PyObject* pystr = PyObject_Str(pyv);
	  omniPy::PyRefHolder pystr_holder(pystr);
	  CORBA::Fixed f;
	  f.NP_fromString(String_AsString(pystr), 1);
	  f.PR_changeScale(scale);
	  f.PR_setLimits(digits, scale);
	  return omniPy::newFixedObject(f);
	}
	else if (String_Check(pyv)) {
	  CORBA::Fixed f(String_AsString(pyv));
	  f.PR_setLimits(digits, scale);
	  return omniPy::newFixedObject(f);
	}
	else if (omnipyFixed_Check(pyv)) {
	  CORBA::Fixed f(*((omnipyFixedObject*)pyv)->ob_fixed);
	  f.PR_setLimits(digits, scale);
	  return omniPy::newFixedObject(f);
	}
      }
    }
  }
  OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

  PyErr_SetString(PyExc_TypeError,
		  "incorrect arguments; see docstring for details.");
  return 0;
}


extern "C" {

  static void
  fixed_dealloc(omnipyFixedObject* self)
  {
    delete self->ob_fixed;
    Py_TYPE(self)->tp_free((PyObject*)self);
  }

  static int
  fixed_print(omnipyFixedObject* f, FILE* fp, int flags)
  {
    CORBA::String_var str = f->ob_fixed->NP_asString();
    fputs((const char*)str, fp);
    return 0;
  }

  static PyObject*
  fixed_value(omnipyFixedObject* self, PyObject* args)
  {
    return fixedValueAsPyLong(*(self->ob_fixed));
  }

  static PyObject*
  fixed_precision(omnipyFixedObject* self, PyObject* args)
  {
    return Int_FromLong(self->ob_fixed->fixed_digits());
  }

  static PyObject*
  fixed_decimals(omnipyFixedObject* self, PyObject* args)
  {
    return Int_FromLong(self->ob_fixed->fixed_scale());
  }

  static PyObject*
  fixed_round(omnipyFixedObject* self, PyObject* args)
  {
    int scale;
    if (!PyArg_ParseTuple(args, (char*)"i", &scale)) return 0;

    try {
      CORBA::Fixed f(((omnipyFixedObject*)self)->ob_fixed->round(scale));
      return omniPy::newFixedObject(f);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  fixed_truncate(omnipyFixedObject* self, PyObject* args)
  {
    int scale;
    if (!PyArg_ParseTuple(args, (char*)"i", &scale)) return 0;

    try {
      CORBA::Fixed f(((omnipyFixedObject*)self)->ob_fixed->truncate(scale));
      return omniPy::newFixedObject(f);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyMethodDef fixed_methods[] = {
    {(char*)"value",     (PyCFunction)fixed_value,     METH_NOARGS},
    {(char*)"precision", (PyCFunction)fixed_precision, METH_NOARGS},
    {(char*)"decimals",  (PyCFunction)fixed_decimals,  METH_NOARGS},
    {(char*)"round",     (PyCFunction)fixed_round,     METH_VARARGS},
    {(char*)"truncate",  (PyCFunction)fixed_truncate,  METH_VARARGS},
    {0,0}
  };

#if (PY_VERSION_HEX < 0x03000000)

  static int
  fixed_compare(omnipyFixedObject* a, omnipyFixedObject* b)
  {
    return CORBA::Fixed::NP_cmp(*(a->ob_fixed), *(b->ob_fixed));
  }

#else

  static PyObject*
  fixed_rcompare(omnipyFixedObject* a, omnipyFixedObject* b, int op)
  {
    int cmp = CORBA::Fixed::NP_cmp(*(a->ob_fixed), (*b->ob_fixed));

    CORBA::Boolean r;

    switch (op) {
    case Py_LT: r = cmp <  0; break;
    case Py_LE: r = cmp <= 0; break;
    case Py_EQ: r = cmp == 0; break;
    case Py_NE: r = cmp != 0; break;
    case Py_GT: r = cmp >  0; break;
    case Py_GE: r = cmp >= 0; break;
    default:    r = 0;
    };
    
    PyObject* r_o = r ? Py_True : Py_False;
    Py_INCREF(r_o);
    return r_o;
  }

#endif

  static PyObject*
  fixed_repr(omnipyFixedObject* f)
  {
    CORBA::String_var str  = f->ob_fixed->NP_asString();
    CORBA::String_var repr = CORBA::string_alloc(strlen(str) + 10);
    sprintf((char*)repr, "fixed(\"%s\")", (const char*)str);

    return String_FromString((const char*)repr);
  }

  static long
  fixed_hash(omnipyFixedObject* f)
  {
    // Calculate the hash value by converting the digits to a Python
    // long and finding its hash. Then rotate the result left by the
    // fixed scale. This means that integer values hash to the same
    // value as normal Python integers or longs.

    long h;

    PyObject* pyl = fixedValueAsPyLong(*(f->ob_fixed));
    h = PyObject_Hash(pyl);
    Py_DECREF(pyl);

    unsigned long u = h;
    int shift = f->ob_fixed->fixed_scale();

    h = (u << shift) + (u >> (32 - shift));

    if (h == -1)
      h = -2;
    return h;
  }

  static PyObject*
  fixed_str(omnipyFixedObject* f)
  {
    CORBA::String_var str = f->ob_fixed->NP_asString();
    return String_FromString((const char*)str);
  }

  static PyObject*
  fixed_add(omnipyFixedObject* a, omnipyFixedObject* b)
  {
    try {
      return omniPy::newFixedObject(*a->ob_fixed + *b->ob_fixed);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  fixed_sub(omnipyFixedObject* a, omnipyFixedObject* b)
  {
    try {
      return omniPy::newFixedObject(*a->ob_fixed - *b->ob_fixed);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  fixed_mul(omnipyFixedObject* a, omnipyFixedObject* b)
  {
    try {
      return omniPy::newFixedObject(*a->ob_fixed * *b->ob_fixed);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  fixed_div(omnipyFixedObject* a, omnipyFixedObject* b)
  {
    try {
      return omniPy::newFixedObject(*a->ob_fixed / *b->ob_fixed);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  fixed_neg(omnipyFixedObject* f)
  {
    return omniPy::newFixedObject(- *f->ob_fixed);
  }

  static PyObject*
  fixed_pos(omnipyFixedObject* f)
  {
    Py_INCREF(f);
    return (PyObject*)f;
  }

  static PyObject*
  fixed_abs(omnipyFixedObject* f)
  {
    if (*f->ob_fixed < CORBA::Fixed(0))
      return fixed_neg(f);
    else
      return fixed_pos(f);
  }

  static int
  fixed_nonzero(omnipyFixedObject* f)
  {
    return *f->ob_fixed != CORBA::Fixed(0);
  }


#if (PY_VERSION_HEX <= 0x03000000)

  static int
  fixed_coerce(PyObject** pv, PyObject** pw)
  {
    if (PyInt_Check(*pw)) {
      long l = PyInt_AsLong(*pw);
      CORBA::Fixed f(l);
      *pw = omniPy::newFixedObject(f);
      Py_INCREF(*pv);
      return 0;
    }
    else if (PyLong_Check(*pw)) {
      PyObject* pystr = PyObject_Str(*pw);
      try {
	CORBA::Fixed f;
	f.NP_fromString(String_AsString(pystr), 1);
	*pw = omniPy::newFixedObject(f);
	Py_DECREF(pystr);
      }
      catch (...) {
	Py_DECREF(pystr);
	return 1; // Can't coerce
      }
      Py_INCREF(*pv);
      return 0;
    }
    return 1; // Can't coerce
  }
#endif

#if (PY_VERSION_HEX <= 0x03000000)
  static PyObject *
  fixed_int(PyObject* v)
  {
    try {
#ifdef HAS_LongLong
      CORBA::LongLong ll = *((omnipyFixedObject*)v)->ob_fixed;
      if (ll > _CORBA_LONGLONG_CONST(2147483647) ||
	  ll < _CORBA_LONGLONG_CONST(-2147483648))
	OMNIORB_THROW(DATA_CONVERSION,
		      DATA_CONVERSION_RangeError,
		      CORBA::COMPLETED_NO);

      return PyInt_FromLong((long)ll);
#else
      CORBA::Long l = *((omnipyFixedObject*)v)->ob_fixed;
      return PyInt_FromLong((long)l);
#endif
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }
#endif

  static PyObject*
  fixed_long(PyObject* v)
  {
    CORBA::Fixed f = ((omnipyFixedObject*)v)->ob_fixed->truncate(0);
    return fixedValueAsPyLong(f);
  }

#if (PY_VERSION_HEX <= 0x03000000)

  static PyNumberMethods fixed_as_number = {
    (binaryfunc) fixed_add,        /*nb_add*/
    (binaryfunc) fixed_sub,        /*nb_subtract*/
    (binaryfunc) fixed_mul,        /*nb_multiply*/
    (binaryfunc) fixed_div,        /*nb_divide*/
    0,                             /*nb_remainder*/
    0,                             /*nb_divmod*/
    0,                             /*nb_power*/
    (unaryfunc)	 fixed_neg,        /*nb_negative*/
    (unaryfunc)	 fixed_pos,        /*nb_positive*/
    (unaryfunc)  fixed_abs,        /*nb_absolute*/
    (inquiry)    fixed_nonzero,    /*nb_nonzero*/
    0,                             /*nb_invert*/
    0,                             /*nb_lshift*/
    0,                             /*nb_rshift*/
    0,                             /*nb_and*/
    0,                             /*nb_xor*/
    0,                             /*nb_or*/
    (coercion)   fixed_coerce,     /*nb_coerce*/
    (unaryfunc)	 fixed_int,        /*nb_int*/
    (unaryfunc)	 fixed_long,       /*nb_long*/
    0,                             /*nb_float*/
    0,                             /*nb_oct*/
    0,                             /*nb_hex*/
  };

#else

  static PyNumberMethods fixed_as_number = {
    (binaryfunc) fixed_add,        /*nb_add*/
    (binaryfunc) fixed_sub,        /*nb_subtract*/
    (binaryfunc) fixed_mul,        /*nb_multiply*/
    0,                             /*nb_remainder*/
    0,                             /*nb_divmod*/
    0,                             /*nb_power*/
    (unaryfunc)	 fixed_neg,        /*nb_negative*/
    (unaryfunc)	 fixed_pos,        /*nb_positive*/
    (unaryfunc)  fixed_abs,        /*nb_absolute*/
    (inquiry)    fixed_nonzero,    /*nb_bool*/
    0,                             /*nb_invert*/
    0,                             /*nb_lshift*/
    0,                             /*nb_rshift*/
    0,                             /*nb_and*/
    0,                             /*nb_xor*/
    0,                             /*nb_or*/
    (unaryfunc)	 fixed_long,       /*nb_int*/
    0,                             /*nb_reserved*/
    0,                             /*nb_float*/

    0,                             /*nb_inplace_add*/
    0,                             /*nb_inplace_subtract*/
    0,                             /*nb_inplace_multiply*/
    0,                             /*nb_inplace_remainder*/
    0,                             /*nb_inplace_power*/
    0,                             /*nb_inplace_lshift*/
    0,                             /*nb_inplace_rshift*/
    0,                             /*nb_inplace_and*/
    0,                             /*nb_inplace_xor*/
    0,                             /*nb_inplace_or*/

    0,                             /*nb_floor_divide*/
    (binaryfunc) fixed_div,        /*nb_true_divide*/
    0,                             /*nb_inplace_floor_divide*/
    0,                             /*nb_inplace_true_divide*/

    0,                             /*nb_index*/
  };

#endif

  static char fixed_type_doc [] =
  "CORBA fixed point type.\n"
  "\n"
  "Standard functions on fixed value f:\n"
  "\n"
  "  f.value()         -> Python long containing fixed value * 10**decimals\n"
  "  f.precision()     -> Total number of decimal digits\n"
  "  f.decimals()      -> Scale of the number. i.e. digits after the\n"
  "                       decimal point\n"
  "\n"
  "omniORB extensions:\n"
  "\n"
  "  str(f)            -> Value as a string\n"
  "  f.round(scale)    -> Value rounded to scale decimal places\n"
  "  f.truncate(scale) -> Value truncated to scale decimal places\n";

  PyTypeObject omnipyFixed_Type = {
    PyVarObject_HEAD_INIT(0,0)
    "omnipyFixed",
    sizeof(omnipyFixedObject),
    0,
    (destructor) fixed_dealloc,    /*tp_dealloc*/
    (printfunc)  fixed_print,      /*tp_print*/
    0,                             /*tp_getattr*/
    0,				   /*tp_setattr*/
#if (PY_VERSION_HEX <= 0x03000000)
    (cmpfunc)    fixed_compare,    /*tp_compare*/
#else
    0,                             /*tp_reserved*/
#endif
    (reprfunc)   fixed_repr,       /*tp_repr*/
                &fixed_as_number,  /*tp_as_number*/
    0,				   /*tp_as_sequence*/
    0,				   /*tp_as_mapping*/
    (hashfunc)   fixed_hash,       /*tp_hash*/
    0,				   /*tp_call*/
    (reprfunc)   fixed_str,	   /*tp_str*/
    0,				   /*tp_getattro*/
    0,				   /*tp_setattro*/
    0,                             /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    fixed_type_doc,                /*tp_doc*/
    0,                             /*tp_traverse*/
    0,                             /*tp_clear*/
#if (PY_VERSION_HEX < 0x03000000)
    0,                             /*tp_richcompare*/
#else
    (richcmpfunc)fixed_rcompare,   /*tp_richcompare*/
#endif
    0,                             /*tp_weaklistoffset*/
    0,                             /*tp_iter*/
    0,                             /*tp_iternext*/
    fixed_methods,                 /*tp_methods*/
  };
}

void
omniPy::initFixed(PyObject* mod)
{
  int r = PyType_Ready(&omnipyFixed_Type);
  OMNIORB_ASSERT(r == 0);
}
