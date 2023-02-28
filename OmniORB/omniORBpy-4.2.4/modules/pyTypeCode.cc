// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyTypeCode.cc              Created on: 1999/07/19
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
//    TypeCode support

#include <omnipy.h>


extern "C" {
  
  // Simple Python type to hold a PyObject pointer and compare by pointer.

  struct PyPointerObj {
    PyObject_HEAD
    PyObject* ptr;
  };

  static void
  PyPointerObj_dealloc(PyPointerObj* self)
  {
    PyObject_Del((PyObject*)self);
  }

#if (PY_VERSION_HEX < 0x03000000)
  static int
  PyPointerObj_cmp(PyPointerObj* t1, PyPointerObj* t2)
  {
    if      (t1->ptr == t2->ptr) return 0;
    else if (t1->ptr >  t2->ptr) return 1;
    else                         return -1;
  }

#else

  static PyObject*
  PyPointerObj_rcmp(PyPointerObj* t1, PyPointerObj* t2, int op)
  {
    CORBA::Boolean r = 0;

    PyObject* p1 = t1->ptr;
    PyObject* p2 = t2->ptr;

    switch (op) {
    case Py_LT: r = p1 <  p2; break;
    case Py_LE: r = p1 <= p2; break;
    case Py_EQ: r = p1 == p2; break;
    case Py_NE: r = p1 != p2; break;
    case Py_GT: r = p1 >  p2; break;
    case Py_GE: r = p1 >= p2; break;
    };
    
    PyObject* r_o = r ? Py_True : Py_False;
    Py_INCREF(r_o);
    return r_o;
  }
#endif

  static long
  PyPointerObj_hash(PyPointerObj* self)
  {
    return (long)self->ptr;
  }

  static PyTypeObject PyPointerType = {
    PyVarObject_HEAD_INIT(0,0)
    (char*)"_omnipy.PyPointerObj",     /* tp_name */
    sizeof(PyPointerObj),              /* tp_basicsize */
    0,                                 /* tp_itemsize */
    (destructor)PyPointerObj_dealloc,  /* tp_dealloc */
    0,                                 /* tp_print */
    0,                                 /* tp_getattr */
    0,                                 /* tp_setattr */
#if (PY_VERSION_HEX < 0x03000000)
    (cmpfunc)PyPointerObj_cmp,         /* tp_compare */
#else
    0,                                 /* tp_reserved */
#endif
    0,                                 /* tp_repr */
    0,                                 /* tp_as_number */
    0,                                 /* tp_as_sequence */
    0,                                 /* tp_as_mapping */
    (hashfunc)PyPointerObj_hash,       /* tp_hash  */
    0,                                 /* tp_call */
    0,                                 /* tp_str */
    0,                                 /* tp_getattro */
    0,                                 /* tp_setattro */
    0,                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                /* tp_flags */
    (char*)"PyPointerObj",             /* tp_doc */
    0,                                 /* tp_traverse */
    0,                                 /* tp_clear */
#if (PY_VERSION_HEX < 0x03000000)
    0,                                 /* tp_richcompare */
#else
    (richcmpfunc)PyPointerObj_rcmp,    /* tp_richcompare */
#endif
  };
}

static inline PyPointerObj*
PyPointerObj_alloc(PyObject* ptr)
{
  PyPointerObj* self = PyObject_New(PyPointerObj, &PyPointerType);
  self->ptr = ptr;
  return self;
}


OMNI_USING_NAMESPACE(omni)

// Objects to map descriptors to typecode offsets and vice-versa:

class DescriptorOffsetMap {
public:
  DescriptorOffsetMap() :
    dict_(PyDict_New()), base_(0)
  {
  }

  DescriptorOffsetMap(DescriptorOffsetMap& dom, CORBA::Long offset) :
    dict_(dom.getDict()), base_(dom.getBase() + offset)
  {
  }

  ~DescriptorOffsetMap() {
    Py_DECREF(dict_);
  }

  inline void add(PyObject* desc, CORBA::Long offset) {
    PyObject* desc_o = (PyObject*)PyPointerObj_alloc(desc);
    PyObject* oo     = Int_FromLong(offset + base_);
    PyDict_SetItem(dict_, desc_o, oo);
    Py_DECREF(desc_o);
    Py_DECREF(oo);
  }

  inline CORBA::Boolean lookup(PyObject* desc, CORBA::Long& offset) {
    PyObject* desc_o = (PyObject*)PyPointerObj_alloc(desc);
    PyObject* oo     = PyDict_GetItem(dict_, desc_o);
    Py_DECREF(desc_o);
    if (oo) {
      offset = Int_AS_LONG(oo) - base_;
      return 1;
    }
    return 0;
  }

protected:
  inline PyObject*   getDict() { Py_INCREF(dict_); return dict_; }
  inline CORBA::Long getBase() { return base_; }

private:
  PyObject*   dict_;
  CORBA::Long base_;

  DescriptorOffsetMap(const DescriptorOffsetMap&);
  DescriptorOffsetMap& operator=(const DescriptorOffsetMap&);
};

class OffsetDescriptorMap {
public:
  OffsetDescriptorMap() :
    dict_(PyDict_New()), base_(0)
  {
  }

  OffsetDescriptorMap(OffsetDescriptorMap& odm, CORBA::Long offset) :
    dict_(odm.getDict()), base_(odm.getBase() + offset)
  {
  }

  ~OffsetDescriptorMap() {
    Py_DECREF(dict_);
  }

  inline void add(PyObject* desc, CORBA::Long offset) {
    PyObject* oo = Int_FromLong(offset + base_);
    PyDict_SetItem(dict_, oo, desc);
    Py_DECREF(oo);
  }

  inline CORBA::Boolean lookup(PyObject*& desc, CORBA::Long offset) {
    PyObject* oo = Int_FromLong(offset + base_);
    desc = PyDict_GetItem(dict_, oo);
    Py_DECREF(oo);
    if (desc) {
      Py_INCREF(desc);
      return 1;
    }
    return 0;
  }

protected:
  inline PyObject*   getDict() { Py_INCREF(dict_); return dict_; }
  inline CORBA::Long getBase() { return base_; }

private:
  PyObject*   dict_;
  CORBA::Long base_;

  OffsetDescriptorMap(const OffsetDescriptorMap&);
  OffsetDescriptorMap& operator=(const OffsetDescriptorMap&);
};



static void
r_marshalTypeCode(cdrStream&           stream,
		  PyObject*            d_o,
		  DescriptorOffsetMap& dom)
{
  CORBA::Long tc_offset;

  // If this TypeCode has already been sent, use an indirection:
  if (orbParameters::useTypeCodeIndirections && dom.lookup(d_o, tc_offset)) {

    CORBA::ULong tk_ind = 0xffffffff;
    tk_ind >>= stream;

    CORBA::Long  offset = tc_offset - stream.currentOutputPtr();
    offset >>= stream;
  }
  else {
    CORBA::ULong   tk;
    PyObject*      t_o;

    if (PyTuple_Check(d_o)) {
      t_o = PyTuple_GET_ITEM(d_o, 0); OMNIORB_ASSERT(Int_Check(t_o));
      tk  = Int_AS_LONG(t_o);
    }
    else {
      OMNIORB_ASSERT(Int_Check(d_o));
      tk  = Int_AS_LONG(d_o);
    }

    // Marshal the kind
    if (tk != 0xffffffff) {
      tk >>= stream;
      // Offset of this TypeCode (within the current encapsulation if any):
      tc_offset = stream.currentOutputPtr() - 4;
    }

    switch (tk) {
    case CORBA::tk_null:
    case CORBA::tk_void:
    case CORBA::tk_short:
    case CORBA::tk_long:
    case CORBA::tk_ushort:
    case CORBA::tk_ulong:
    case CORBA::tk_float:
    case CORBA::tk_double:
    case CORBA::tk_boolean:
    case CORBA::tk_char:
    case CORBA::tk_octet:
    case CORBA::tk_any:
    case CORBA::tk_TypeCode:
    case CORBA::tk_Principal:
#ifdef HAS_LongLong
    case CORBA::tk_longlong:
    case CORBA::tk_ulonglong:
#endif
    case CORBA::tk_wchar:
      // Nothing more to be sent
      break;

    case CORBA::tk_string:
      {
	// Send max length
	t_o = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(Int_Check(t_o));
	CORBA::ULong len = Int_AS_LONG(t_o);
	len >>= stream;
      }
      break;

    case CORBA::tk_wstring:
      {
	// Send max length
	t_o = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(Int_Check(t_o));
	CORBA::ULong len = Int_AS_LONG(t_o);
	len >>= stream;
      }
      break;

    case CORBA::tk_fixed:
      {
	// Send digits followed by scale
	t_o = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(Int_Check(t_o));
	CORBA::UShort digits = Int_AS_LONG(t_o);
	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(Int_Check(t_o));
	CORBA::Short scale = Int_AS_LONG(t_o);
	digits >>= stream;
	scale  >>= stream;
      }
      break;

    case CORBA::tk_objref:
      {
	// Add entry to descriptor offset map:
	dom.add(d_o, tc_offset);

	// Stream for the encapsulation
	cdrEncapsulationStream encap;

	// RepoId and name
	t_o = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case CORBA::tk_struct:
      {
	dom.add(d_o, tc_offset);

	cdrEncapsulationStream encap;
	DescriptorOffsetMap edom(dom, tc_offset + 8);

	// RepoId and name
	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	t_o = PyTuple_GET_ITEM(d_o, 3); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	// Count
	CORBA::ULong cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;
	cnt >>= encap;

	CORBA::ULong i, j, slen;
	const char*  str;

	for (i=0, j=4; i < cnt; i++) {
	  // member name
	  t_o  = PyTuple_GET_ITEM(d_o, j++); OMNIORB_ASSERT(String_Check(t_o));
          str  = String_AS_STRING_AND_SIZE(t_o, slen);

          ++slen;
	  if (str[0] == '_') { --slen; ++str; }

	  slen >>= encap;
          encap.put_small_octet_array((const CORBA::Octet*)str, slen);

	  // member type
	  r_marshalTypeCode(encap, PyTuple_GET_ITEM(d_o, j++), edom);
	}
	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case CORBA::tk_union:
      {
	dom.add(d_o, tc_offset);

	cdrEncapsulationStream encap;
	DescriptorOffsetMap edom(dom, tc_offset + 8);

	// RepoId and name
	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	t_o = PyTuple_GET_ITEM(d_o, 3); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	// Discriminant type
	PyObject* discriminant = PyTuple_GET_ITEM(d_o, 4);
	r_marshalTypeCode(encap, discriminant, edom);

	// Default used
	t_o              = PyTuple_GET_ITEM(d_o, 5);
	OMNIORB_ASSERT(Int_Check(t_o));
	CORBA::Long defu = Int_AS_LONG(t_o);
	defu >>= encap;

	PyObject* mems;
	PyObject* mem;

	// Count
	mems = PyTuple_GET_ITEM(d_o, 6); OMNIORB_ASSERT(PyTuple_Check(mems));
	CORBA::ULong cnt = PyTuple_GET_SIZE(mems);
	cnt >>= encap;

	CORBA::ULong slen;
	const char*  str;

	for (CORBA::ULong i=0; i < cnt; i++) {
	  mem = PyTuple_GET_ITEM(mems, i); OMNIORB_ASSERT(PyTuple_Check(mem));

	  // Label value
	  omniPy::marshalPyObject(encap, discriminant,
				  PyTuple_GET_ITEM(mem, 0));

	  // Member name
	  t_o  = PyTuple_GET_ITEM(mem, 1); OMNIORB_ASSERT(String_Check(t_o));
          str  = String_AS_STRING_AND_SIZE(t_o, slen);

          ++slen;
	  if (str[0] == '_') { --slen; ++str; }

	  slen >>= encap;
          encap.put_small_octet_array((const CORBA::Octet*)str, slen);

	  // Member typecode
	  r_marshalTypeCode(encap, PyTuple_GET_ITEM(mem, 2), edom);
	}
	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case CORBA::tk_enum:
      {
	dom.add(d_o, tc_offset);

	cdrEncapsulationStream encap;

	// RepoId and name
	t_o = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	PyObject* mems;
	PyObject* mem;

	// Count
	mems = PyTuple_GET_ITEM(d_o, 3); OMNIORB_ASSERT(PyTuple_Check(mems));
	CORBA::ULong cnt = PyTuple_GET_SIZE(mems);
	cnt >>= encap;

	for (CORBA::ULong i=0; i < cnt; i++) {
	  mem = PyTuple_GET_ITEM(mems, i);

	  // Member name
	  t_o = PyObject_GetAttrString(mem, (char*)"_n");
	  omniPy::PyRefHolder h(t_o);
	  omniPy::marshalRawPyString(encap, t_o);
	}
      	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case CORBA::tk_sequence:
      {
	dom.add(d_o, tc_offset);

	cdrEncapsulationStream encap;
	DescriptorOffsetMap edom(dom, tc_offset + 8);

	// Element type
	r_marshalTypeCode(encap, PyTuple_GET_ITEM(d_o, 1), edom);

	// Max length
	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(Int_Check(t_o));
	CORBA::ULong max_len = Int_AS_LONG(t_o);
	max_len >>= encap;

	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case CORBA::tk_array:
      {
	dom.add(d_o, tc_offset);

	cdrEncapsulationStream encap;
	DescriptorOffsetMap edom(dom, tc_offset + 8);

	// Element type
	r_marshalTypeCode(encap, PyTuple_GET_ITEM(d_o, 1), edom);

	// Length
	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(Int_Check(t_o));
	CORBA::ULong arr_len = Int_AS_LONG(t_o);
	arr_len >>= encap;

	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case CORBA::tk_alias:
      {
	dom.add(d_o, tc_offset);

	cdrEncapsulationStream encap;
	DescriptorOffsetMap edom(dom, tc_offset + 8);

	// RepoId and name
	t_o = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	// TypeCode
	r_marshalTypeCode(encap, PyTuple_GET_ITEM(d_o, 3), edom);

      	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case CORBA::tk_except:
      {
	dom.add(d_o, tc_offset);

	cdrEncapsulationStream encap;
	DescriptorOffsetMap edom(dom, tc_offset + 8);

	// RepoId and name
	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	t_o = PyTuple_GET_ITEM(d_o, 3); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	// Count
	CORBA::ULong cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;
	cnt >>= encap;

	CORBA::ULong i, j, slen;
	const char*  str;

	for (i=0, j=4; i < cnt; i++) {
	  // member name
	  t_o = PyTuple_GET_ITEM(d_o, j++); OMNIORB_ASSERT(String_Check(t_o));
          str = String_AS_STRING_AND_SIZE(t_o, slen);

          ++slen;
	  if (str[0] == '_') { --slen; ++str; }

	  slen >>= encap;
          encap.put_small_octet_array((const CORBA::Octet*)str, slen);

	  // member type
	  r_marshalTypeCode(encap, PyTuple_GET_ITEM(d_o, j++), edom);
	}
	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case CORBA::tk_value:
      {
	dom.add(d_o, tc_offset);

	cdrEncapsulationStream encap;
	DescriptorOffsetMap edom(dom, tc_offset + 8);

	// RepoId and name
	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	t_o = PyTuple_GET_ITEM(d_o, 3); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	// ValueModifier
	t_o = PyTuple_GET_ITEM(d_o, 4);	OMNIORB_ASSERT(Int_Check(t_o));
	CORBA::ValueModifier mod = Int_AS_LONG(t_o);
	mod >>= encap;

	// Concrete base
	t_o = PyTuple_GET_ITEM(d_o, 6);
	if (t_o == Py_None) {
	  // No base.
	  CORBA::ULong nulltk = CORBA::tk_null;
	  nulltk >>= encap;
	}
	else {
	  r_marshalTypeCode(encap, t_o, edom);
	}

	// Count
	CORBA::ULong cnt = (PyTuple_GET_SIZE(d_o) - 7) / 3;
	cnt >>= encap;

	CORBA::ULong i, j, slen;
	const char*  str;

	for (i=0, j=7; i < cnt; i++) {
	  // member name
	  t_o = PyTuple_GET_ITEM(d_o, j++); OMNIORB_ASSERT(String_Check(t_o));
          str = String_AS_STRING_AND_SIZE(t_o, slen);

          ++slen;
	  if (str[0] == '_') { --slen; ++str; }

	  slen >>= encap;
          encap.put_small_octet_array((const CORBA::Octet*)str, slen);

	  // member type
	  r_marshalTypeCode(encap, PyTuple_GET_ITEM(d_o, j++), edom);

	  // member visiblity
	  t_o = PyTuple_GET_ITEM(d_o, j++); OMNIORB_ASSERT(Int_Check(t_o));
	  CORBA::UShort vis = Int_AS_LONG(t_o);
	  vis >>= encap;
	}
	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case CORBA::tk_value_box:
      {
	dom.add(d_o, tc_offset);

	cdrEncapsulationStream encap;
	DescriptorOffsetMap edom(dom, tc_offset + 8);

	// RepoId and name
	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	t_o = PyTuple_GET_ITEM(d_o, 3); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	// Boxed type
	t_o = PyTuple_GET_ITEM(d_o, 4);
	r_marshalTypeCode(encap, t_o, edom);

	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case CORBA::tk_abstract_interface:
      {
	dom.add(d_o, tc_offset);

	cdrEncapsulationStream encap;
	DescriptorOffsetMap edom(dom, tc_offset + 8);

	// RepoId and name
	t_o = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	t_o = PyTuple_GET_ITEM(d_o, 2); OMNIORB_ASSERT(String_Check(t_o));
	omniPy::marshalRawPyString(encap, t_o);

	// Send encapsulation
	::operator>>=((CORBA::ULong)encap.bufSize(), stream);
	stream.put_octet_array((CORBA::Octet*)encap.bufPtr(), encap.bufSize());
      }
      break;

    case 0xffffffff:
      {
	PyObject* l = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(PyList_Check(l));

	t_o = PyList_GET_ITEM(l, 0); OMNIORB_ASSERT(t_o);
        
	if (String_Check(t_o)) {
	  // Indirection to a repoId -- find the corresponding descriptor
	  t_o = PyDict_GetItem(omniPy::pyomniORBtypeMap, t_o);
	  if (!t_o)
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_IncompletePythonType,
			  (CORBA::CompletionStatus)stream.completion());

	  Py_INCREF(t_o);
	  PyList_SetItem(l, 0, t_o);
	}

	CORBA::Long position, offset;

	if (dom.lookup(t_o, position)) {
	  tk     >>= stream;
	  offset   = position - stream.currentOutputPtr();
	  offset >>= stream;
	}
	else {
	  r_marshalTypeCode(stream, t_o, dom);
	}
      }
      break;

    default:
      OMNIORB_THROW(BAD_TYPECODE, BAD_TYPECODE_UnknownKind,
		    (CORBA::CompletionStatus)stream.completion());
    }
  }
}



static inline void
skipString(cdrStream& stream)
{
  CORBA::ULong len; len <<= stream;

  if (!stream.checkInputOverrun(1, len))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)stream.completion());

  stream.skipInput(len);
}


PyObject*
r_unmarshalTypeCode(cdrStream& stream, OffsetDescriptorMap& odm)
{
  PyObject* d_o = 0; // Descriptor object to build
  PyObject* t_o;

  // Read kind
  CORBA::ULong tk; tk <<= stream;

  // Offset of current TypeCode
  CORBA::Long tc_offset = stream.currentInputPtr() - 4;

  switch (tk) {
  case CORBA::tk_null:
  case CORBA::tk_void:
  case CORBA::tk_short:
  case CORBA::tk_long:
  case CORBA::tk_ushort:
  case CORBA::tk_ulong:
  case CORBA::tk_float:
  case CORBA::tk_double:
  case CORBA::tk_boolean:
  case CORBA::tk_char:
  case CORBA::tk_octet:
  case CORBA::tk_any:
  case CORBA::tk_TypeCode:
  case CORBA::tk_Principal:
#ifdef HAS_LongLong
  case CORBA::tk_longlong:
  case CORBA::tk_ulonglong:
#endif
  case CORBA::tk_wchar:
    {
      d_o = Int_FromLong(tk); odm.add(d_o, tc_offset);
    }
    break;

  case CORBA::tk_string:
    {
      d_o = PyTuple_New(2); odm.add(d_o, tc_offset);
      PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));

      CORBA::ULong len; len <<= stream;

      PyTuple_SET_ITEM(d_o, 1, Int_FromLong(len));
    }
    break;

  case CORBA::tk_wstring:
    {
      d_o = PyTuple_New(2); odm.add(d_o, tc_offset);
      PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));

      CORBA::ULong len; len <<= stream;

      PyTuple_SET_ITEM(d_o, 1, Int_FromLong(len));
    }
    break;

  case CORBA::tk_fixed:
    {
      d_o = PyTuple_New(3); odm.add(d_o, tc_offset);
      PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));

      CORBA::UShort digits; digits <<= stream;
      CORBA::Short  scale;  scale  <<= stream;

      PyTuple_SET_ITEM(d_o, 1, Int_FromLong(digits));
      PyTuple_SET_ITEM(d_o, 2, Int_FromLong(scale));
    }
    break;

  case CORBA::tk_objref:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      d_o = PyTuple_New(3); odm.add(d_o, tc_offset);
      PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));

      // RepoId and name
      t_o = omniPy::unmarshalRawPyString(encap); PyTuple_SET_ITEM(d_o, 1, t_o);
      t_o = omniPy::unmarshalRawPyString(encap); PyTuple_SET_ITEM(d_o, 2, t_o);
    }
    break;

  case CORBA::tk_struct:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      PyObject* repoId = omniPy::unmarshalRawPyString(encap);

      d_o = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

      if (d_o) {
	// Static knowledge of the structure
	Py_INCREF(d_o);
	Py_DECREF(repoId);

	// We could unmarshal the whole TypeCode and check it for
	// equivalence, but we don't bother. We still have to recurse
	// into the TypeCode in case there are later indirections into
	// it.
	odm.add(d_o, tc_offset);
	OffsetDescriptorMap eodm(odm, tc_offset + 8);

	skipString(encap); // Name
	CORBA::ULong cnt; cnt <<= encap;

	for (CORBA::ULong i=0; i < cnt; i++) {
	  // Member name and type
	  skipString(encap);
	  t_o = r_unmarshalTypeCode(encap, eodm); Py_DECREF(t_o);
	}
      }
      else {
	// Don't know this structure
	OffsetDescriptorMap eodm(odm, tc_offset + 8);

	PyObject* name = omniPy::unmarshalRawPyString(encap);

	CORBA::ULong cnt; cnt <<= encap;

	d_o = PyTuple_New(cnt * 2 + 4);	odm.add(d_o, tc_offset);
	PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));
	PyTuple_SET_ITEM(d_o, 2, repoId);
	PyTuple_SET_ITEM(d_o, 3, name);

	PyObject* mems = PyTuple_New(cnt);

	CORBA::ULong i, j;

	PyObject* word;

	for (i=0, j=4; i < cnt; i++) {
	  // member name
	  t_o = omniPy::unmarshalRawPyString(encap);

	  if ((word = PyDict_GetItem(omniPy::pyomniORBwordMap, t_o))) {
	    Py_INCREF(word);
	    Py_DECREF(t_o);
	    t_o = word;
	  }
	  PyTuple_SET_ITEM(d_o, j++, t_o);
	  Py_INCREF(t_o);
	  PyTuple_SET_ITEM(mems, i, t_o);

	  // member type
	  t_o = r_unmarshalTypeCode(encap, eodm);
	  PyTuple_SET_ITEM(d_o, j++, t_o);
	}

	// Create class object:
	// *** Could be made faster by finding the createUnknownStruct
	// function only once, and manually building the argument tuple
	t_o = PyObject_GetAttrString(omniPy::pyomniORBmodule,
				     (char*)"createUnknownStruct");
	OMNIORB_ASSERT(t_o && PyFunction_Check(t_o));

	t_o = PyObject_CallFunction(t_o, (char*)"OO", repoId, mems);
	OMNIORB_ASSERT(t_o);

	Py_DECREF(mems);

	PyTuple_SET_ITEM(d_o, 1, t_o);
      }
    }
    break;

  case CORBA::tk_union:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      PyObject* repoId = omniPy::unmarshalRawPyString(encap);

      d_o = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

      if (d_o) {
	// Static knowledge of the union
	Py_INCREF(d_o);
	Py_DECREF(repoId);

	// We could unmarshal the whole TypeCode and check it for
	// equivalence, but we don't bother. We still have to recurse
	// into the TypeCode in case there are later indirections into
	// it.

	odm.add(d_o, tc_offset);
	OffsetDescriptorMap eodm(odm, tc_offset + 8);

	skipString(encap); // Name
	PyObject* discriminant = r_unmarshalTypeCode(encap, eodm);

	CORBA::Long  def_used; def_used <<= encap;
	CORBA::ULong cnt;      cnt      <<= encap;

	for (CORBA::ULong i=0; i<cnt; i++) {
	  // Label value
	  t_o = omniPy::unmarshalPyObject(encap, discriminant); Py_DECREF(t_o);

	  // Member name
	  skipString(encap);

	  // Member type
	  t_o = r_unmarshalTypeCode(encap, eodm); Py_DECREF(t_o);
	}
	Py_DECREF(discriminant);
      }
      else {
	// Don't know this union
	OffsetDescriptorMap eodm(odm, tc_offset + 8);

	d_o = PyTuple_New(9); odm.add(d_o, tc_offset);
	PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));
	PyTuple_SET_ITEM(d_o, 2, repoId);

	// name
	t_o = omniPy::unmarshalRawPyString(encap);
	PyTuple_SET_ITEM(d_o, 3, t_o);

	// discriminant type
	PyObject* discriminant = r_unmarshalTypeCode(encap, eodm);
	PyTuple_SET_ITEM(d_o, 4, discriminant);

	// default used
	CORBA::Long def_used; def_used <<= encap;
	PyTuple_SET_ITEM(d_o, 5, Int_FromLong(def_used));

	if (def_used < 0) {
	  Py_INCREF(Py_None);
	  PyTuple_SET_ITEM(d_o, 7, Py_None);
	}

	// count
	CORBA::ULong cnt; cnt <<= encap;
	PyObject* mems = PyTuple_New(cnt);
	PyTuple_SET_ITEM(d_o, 6, mems);

	PyObject* dict = PyDict_New();
	PyTuple_SET_ITEM(d_o, 8, dict);

	PyObject* mem;
	PyObject* label;
	PyObject* word;

	for (CORBA::ULong i=0; i<cnt; i++) {
	  mem = PyTuple_New(3);

	  // Label value
	  label = omniPy::unmarshalPyObject(encap, discriminant);
	  PyTuple_SET_ITEM(mem, 0, label);

	  // Member name
	  t_o = omniPy::unmarshalRawPyString(encap);

	  if ((word = PyDict_GetItem(omniPy::pyomniORBwordMap, t_o))) {
	    Py_INCREF(word);
	    Py_DECREF(t_o);
	    t_o = word;
	  }
	  PyTuple_SET_ITEM(mem, 1, t_o);

	  // Member type
	  t_o = r_unmarshalTypeCode(encap, eodm);
	  PyTuple_SET_ITEM(mem, 2, t_o);

	  PyTuple_SET_ITEM(mems, i, mem);

	  if (def_used > 0 && i == (CORBA::ULong)def_used) {
	    PyTuple_SET_ITEM(d_o, 7, mem);
	  }
	  else {
            PyDict_SetItem(dict, label, mem);
            Py_DECREF(mem);
	  }
	}

	// Create class object
	t_o = PyObject_GetAttrString(omniPy::pyomniORBmodule,
				     (char*)"createUnknownUnion");
	OMNIORB_ASSERT(t_o && PyFunction_Check(t_o));

	t_o = PyObject_CallFunction(t_o, (char*)"OiO", repoId, def_used, mems);
	OMNIORB_ASSERT(t_o);

	PyTuple_SET_ITEM(d_o, 1, t_o);
      }
    }
    break;

  case CORBA::tk_enum:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      PyObject* repoId = omniPy::unmarshalRawPyString(encap);

      d_o = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

      if (d_o) {
	// Static knowledge of the enum
	Py_INCREF(d_o);
	Py_DECREF(repoId);
	odm.add(d_o, tc_offset);
      }
      else {
	// Don't know this enum
	d_o = PyTuple_New(4); odm.add(d_o, tc_offset);
	PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));
	PyTuple_SET_ITEM(d_o, 1, repoId);

	// name
	t_o = omniPy::unmarshalRawPyString(encap);
	PyTuple_SET_ITEM(d_o, 2, t_o);

	// count
	CORBA::ULong cnt; cnt <<= encap;
	PyObject* mems = PyTuple_New(cnt);
	PyTuple_SET_ITEM(d_o, 3, mems);

	PyObject* eclass = PyObject_GetAttrString(omniPy::pyomniORBmodule,
						  (char*)"EnumItem");
	PyObject* aclass = PyObject_GetAttrString(omniPy::pyomniORBmodule,
						  (char*)"AnonymousEnumItem");
	OMNIORB_ASSERT(eclass);
	OMNIORB_ASSERT(aclass);

	// members
	for (CORBA::ULong i=0; i<cnt; i++) {
	  PyObject* mname = omniPy::unmarshalRawPyString(encap);

	  if (String_GET_SIZE(mname) > 0)
	    t_o = PyObject_CallFunction(eclass, (char*)"Oi", mname, i);
	  else
	    t_o = PyObject_CallFunction(aclass, (char*)"i", i);

	  Py_DECREF(mname);

	  OMNIORB_ASSERT(t_o);
          PyObject_SetAttrString(t_o, (char*)"_parent_id", repoId);
	  PyTuple_SET_ITEM(mems, i, t_o);
	}
      }
    }
    break;

  case CORBA::tk_sequence:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      OffsetDescriptorMap eodm(odm, tc_offset + 8);

      d_o = PyTuple_New(3); odm.add(d_o, tc_offset);
      PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));

      // Element type
      t_o = r_unmarshalTypeCode(encap, eodm);
      PyTuple_SET_ITEM(d_o, 1, t_o);

      // Max length
      CORBA::ULong len; len <<= encap;
      PyTuple_SET_ITEM(d_o, 2, Int_FromLong(len));
    }
    break;

  case CORBA::tk_array:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      OffsetDescriptorMap eodm(odm, tc_offset + 8);

      d_o = PyTuple_New(3); odm.add(d_o, tc_offset);
      PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));

      // Element type
      t_o = r_unmarshalTypeCode(encap, eodm);
      PyTuple_SET_ITEM(d_o, 1, t_o);

      // Length
      CORBA::ULong len; len <<= encap;
      PyTuple_SET_ITEM(d_o, 2, Int_FromLong(len));
    }
    break;

  case CORBA::tk_alias:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      PyObject* repoId = omniPy::unmarshalRawPyString(encap);

      d_o = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

      if (d_o) {
	// Static knowledge of the alias
	Py_INCREF(d_o);
	Py_DECREF(repoId);

	odm.add(d_o, tc_offset);
	OffsetDescriptorMap eodm(odm, tc_offset + 8);
	skipString(encap); // name
	t_o = r_unmarshalTypeCode(encap, eodm); Py_DECREF(t_o);
      }
      else {
	OffsetDescriptorMap eodm(odm, tc_offset + 8);

	d_o = PyTuple_New(4); odm.add(d_o, tc_offset);
	PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));
	PyTuple_SET_ITEM(d_o, 1, repoId);
	
	// name
	t_o = omniPy::unmarshalRawPyString(encap);
	PyTuple_SET_ITEM(d_o, 2, t_o);

	// TypeCode
	t_o = r_unmarshalTypeCode(encap, eodm);
	PyTuple_SET_ITEM(d_o, 3, t_o);
      }
    }
    break;

  case CORBA::tk_except:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      PyObject* repoId = omniPy::unmarshalRawPyString(encap);

      d_o = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

      if (d_o) {
	// Static knowledge of the exception
	Py_INCREF(d_o);
	Py_DECREF(repoId);

	odm.add(d_o, tc_offset);
	OffsetDescriptorMap eodm(odm, tc_offset + 8);

	skipString(encap); // name
	CORBA::ULong cnt; cnt <<= encap;

	for (CORBA::ULong i=0; i < cnt; i++) {
	  // Member name and type
	  skipString(encap);
	  t_o = r_unmarshalTypeCode(encap, eodm); Py_DECREF(t_o);
	}
      }
      else {
	// Don't know this exception
	OffsetDescriptorMap eodm(odm, tc_offset + 8);

	PyObject* name = omniPy::unmarshalRawPyString(encap);

	CORBA::ULong cnt; cnt <<= encap;

	d_o = PyTuple_New(cnt * 2 + 4); odm.add(d_o, tc_offset);
	PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));
	PyTuple_SET_ITEM(d_o, 2, repoId);
	PyTuple_SET_ITEM(d_o, 3, name);

	PyObject* mems = PyTuple_New(cnt);

	CORBA::ULong i, j;

	PyObject* word;

	for (i=0, j=4; i < cnt; i++) {
	  // member name
	  t_o = omniPy::unmarshalRawPyString(encap);

	  if ((word = PyDict_GetItem(omniPy::pyomniORBwordMap, t_o))) {
	    Py_INCREF(word);
	    Py_DECREF(t_o);
	    t_o = word;
	  }
	  PyTuple_SET_ITEM(d_o, j++, t_o);
	  Py_INCREF(t_o);
	  PyTuple_SET_ITEM(mems, i, t_o);

	  // member type
	  t_o = r_unmarshalTypeCode(encap, eodm);
	  PyTuple_SET_ITEM(d_o, j++, t_o);
	}

	// Create class object:
	// *** Could be made faster by finding the createUnknownUserException
	// function only once, and manually building the argument tuple
	t_o = PyObject_GetAttrString(omniPy::pyomniORBmodule,
				     (char*)"createUnknownUserException");
	OMNIORB_ASSERT(t_o && PyFunction_Check(t_o));

	t_o = PyObject_CallFunction(t_o, (char*)"OO", repoId, mems);
	OMNIORB_ASSERT(t_o);

	Py_DECREF(mems);

	PyTuple_SET_ITEM(d_o, 1, t_o);
      }
    }
    break;

  case CORBA::tk_value:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      PyObject* repoId = omniPy::unmarshalRawPyString(encap);

      d_o = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

      if (d_o) {
	// Static knowledge of the value
	Py_INCREF(d_o);
	Py_DECREF(repoId);

	// Unmarshal the rest in case of later indirections
	odm.add(d_o, tc_offset);
	OffsetDescriptorMap eodm(odm, tc_offset + 8);

	skipString(encap); // Name
	CORBA::ValueModifier mod; mod <<= encap; // Modifier
	
	// Skip concrete base
	t_o = r_unmarshalTypeCode(encap, eodm); Py_DECREF(t_o);
	
	CORBA::ULong cnt; cnt <<= encap;
	for (CORBA::ULong i=0; i < cnt; i++) {
	  // Member name, type, visibility
	  skipString(encap);
	  t_o = r_unmarshalTypeCode(encap, eodm); Py_DECREF(t_o);
	  CORBA::UShort vis; vis <<= encap;
	}
      }
      else {
	// Don't know this value
	OffsetDescriptorMap eodm(odm, tc_offset + 8);

	PyObject* name = omniPy::unmarshalRawPyString(encap);
	CORBA::ValueModifier mod; mod <<= encap;
	PyObject* base = r_unmarshalTypeCode(encap, eodm);

	CORBA::ULong basekind = omniPy::descriptorToTK(base);
	if (!(basekind == CORBA::tk_value || basekind == CORBA::tk_null))
	  OMNIORB_THROW(MARSHAL, MARSHAL_InvalidTypeCodeKind,
			(CORBA::CompletionStatus)stream.completion());

	CORBA::ULong cnt; cnt <<= encap;

	d_o = PyTuple_New(cnt * 3 + 7);	odm.add(d_o, tc_offset);
	PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));
	PyTuple_SET_ITEM(d_o, 2, repoId);
	PyTuple_SET_ITEM(d_o, 3, name);
	PyTuple_SET_ITEM(d_o, 4, Int_FromLong(mod));
	Py_INCREF(Py_None);
	PyTuple_SET_ITEM(d_o, 5, Py_None); // Empty truncatable bases
	PyTuple_SET_ITEM(d_o, 6, base);

	CORBA::ULong i, j;

	PyObject* word;

	for (i=0, j=7; i < cnt; i++) {
	  // member name
	  t_o = omniPy::unmarshalRawPyString(encap);

	  if ((word = PyDict_GetItem(omniPy::pyomniORBwordMap, t_o))) {
	    Py_INCREF(word);
	    Py_DECREF(t_o);
	    t_o = word;
	  }
	  PyTuple_SET_ITEM(d_o, j++, t_o);
	  Py_INCREF(t_o);

	  // member type
	  t_o = r_unmarshalTypeCode(encap, eodm);
	  PyTuple_SET_ITEM(d_o, j++, t_o);

	  // member visibility
	  CORBA::UShort vis; vis <<= encap;
	  PyTuple_SET_ITEM(d_o, j++, Int_FromLong(vis));
	}

	// Create class object
	t_o = PyObject_GetAttrString(omniPy::pyomniORBmodule,
				     (char*)"createUnknownValue");
	OMNIORB_ASSERT(t_o && PyFunction_Check(t_o));

	t_o = PyObject_CallFunction(t_o, (char*)"OO", repoId, base);
	OMNIORB_ASSERT(t_o);

	PyTuple_SET_ITEM(d_o, 1, t_o);
      }
    }
    break;

  case CORBA::tk_value_box:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      PyObject* repoId = omniPy::unmarshalRawPyString(encap);

      d_o = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

      if (d_o) {
	// Static knowledge of the valuebox
	Py_INCREF(d_o);
	Py_DECREF(repoId);

	odm.add(d_o, tc_offset);
	OffsetDescriptorMap eodm(odm, tc_offset + 8);
	skipString(encap); // name
	t_o = r_unmarshalTypeCode(encap, eodm); Py_DECREF(t_o);
      }
      else {
	OffsetDescriptorMap eodm(odm, tc_offset + 8);

	d_o = PyTuple_New(5); odm.add(d_o, tc_offset);
	PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));

	// Member 1 is valuebox class in static stubs, but it's not used.
	Py_INCREF(Py_None);
	PyTuple_SET_ITEM(d_o, 1, Py_None);

	PyTuple_SET_ITEM(d_o, 2, repoId);
	
	// name
	t_o = omniPy::unmarshalRawPyString(encap);
	PyTuple_SET_ITEM(d_o, 3, t_o);

	// TypeCode
	t_o = r_unmarshalTypeCode(encap, eodm);
	PyTuple_SET_ITEM(d_o, 4, t_o);
      }
    }
    break;

  case CORBA::tk_abstract_interface:
    {
      CORBA::ULong size; size <<= stream;
      cdrEncapsulationStream encap(stream, size);

      d_o = PyTuple_New(3); odm.add(d_o, tc_offset);
      PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));

      // RepoId and name
      t_o = omniPy::unmarshalRawPyString(encap); PyTuple_SET_ITEM(d_o, 1, t_o);
      t_o = omniPy::unmarshalRawPyString(encap); PyTuple_SET_ITEM(d_o, 2, t_o);
    }
    break;

  case 0xffffffff:
    {
      CORBA::Long position, offset;

      offset  <<= stream;
      position  = tc_offset + 4 + offset;

      if (!odm.lookup(t_o, position)) {
	if (omniORB::trace(10)) {
	  omniORB::logger log;
	  log << "Invalid indirection " << offset << " to " << position
	      << ".\n";
	}
	OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIndirection,
		      (CORBA::CompletionStatus)stream.completion());
      }

      d_o = PyTuple_New(2); odm.add(d_o, tc_offset);
      PyTuple_SET_ITEM(d_o, 0, Int_FromLong(tk));

      PyObject* olist = PyList_New(1);
      PyList_SET_ITEM(olist, 0, t_o);
      PyTuple_SET_ITEM(d_o, 1, olist);
    }
    break;

  default:
    OMNIORB_THROW(BAD_TYPECODE, BAD_TYPECODE_UnknownKind,
		  (CORBA::CompletionStatus)stream.completion());
  }
  return d_o;
}

void
omniPy::marshalTypeCode(cdrStream& stream, PyObject* d_o)
{
  DescriptorOffsetMap dom;
  r_marshalTypeCode(stream, d_o, dom);
}


PyObject*
omniPy::unmarshalTypeCode(cdrStream& stream)
{
  OffsetDescriptorMap odm;
  return r_unmarshalTypeCode(stream, odm);
}


void
omniPy::initTypeCode(PyObject* d)
{
  int r = PyType_Ready(&PyPointerType);
  OMNIORB_ASSERT(r == 0);
}
