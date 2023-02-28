// -*- c++ -*-
//                          Package   : omniidl
// idlpython.cc             Created on: 1999/10/27
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2014 Apasphere Ltd
//    Copyright (C) 1999      AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see http://www.gnu.org/licenses/
//
// Description:
//   
//   Python interface to front-end

#if defined(__VMS)
#  include <Python.h>
#else
#  include PYTHON_INCLUDE
#endif

#include <idlsysdep.h>
#include <idlast.h>
#include <idltype.h>
#include <idlscope.h>
#include <idlvisitor.h>
#include <idldump.h>
#include <idlerr.h>
#include <idlconfig.h>


// PyLongFromLongLong is broken in Python 1.5.2. Workaround here:
#ifdef HAS_LongLong
#  if !defined(PY_VERSION_HEX) || (PY_VERSION_HEX < 0x01050200)
#    error "omniidl requires Python 1.5.2 or higher"

#  elif (PY_VERSION_HEX < 0x02000000)

// Don't know when it was fixed -- certainly in 2.0.0

static inline PyObject* MyPyLong_FromLongLong(IDL_LongLong ll)
{
  if (ll >= 0) // Positive numbers work OK
    return PyLong_FromLongLong(ll);
  else {
    IDL_ULongLong ull = (~ll) + 1; // Hope integers are 2's complement...
    PyObject* p = PyLong_FromUnsignedLongLong(ull);
    PyObject* n = PyNumber_Negative(p);
    Py_DECREF(p);
    return n;
  }
}
#  else
#    define MyPyLong_FromLongLong(ll) PyLong_FromLongLong(ll)
#  endif
#endif


//
// Python 3 support

#if PY_VERSION_HEX >= 0x03000000
#  define OMNIIDL_PY3 1
#  define PyString_Check PyUnicode_Check
#  define PyInt_FromLong PyLong_FromLong

static inline PyObject* PyString_FromString(const char* str)
{
  Py_ssize_t size = strlen(str);
  return PyUnicode_DecodeLatin1(str, size, 0);
}

static inline char* PyString_AsString(PyObject* obj)
{
  char* str;
  PyArg_Parse(obj, (char*)"s", &str);
  return str;
}

static inline PyObject* PyString_FromChar(unsigned char c)
{
  return Py_BuildValue((char*)"C", (int)c);
}

#else

static inline PyObject* PyString_FromChar(unsigned char c)
{
  return Py_BuildValue((char*)"c", c);
}

#endif


#define ASSERT_RESULT     if (!result_) PyErr_Print(); assert(result_)
#define ASSERT_PYOBJ(pyo) if (!pyo)     PyErr_Print(); assert(pyo)

class PythonVisitor : public AstVisitor, public TypeVisitor {
public:
  PythonVisitor();
  virtual ~PythonVisitor();

  void visitAST              (AST*);
  void visitModule           (Module*);
  void visitInterface        (Interface*);
  void visitForward          (Forward*);
  void visitConst            (Const*);
  void visitDeclarator       (Declarator*);
  void visitTypedef          (Typedef*);
  void visitMember           (Member*);
  void visitStruct           (Struct*);
  void visitStructForward    (StructForward*);
  void visitException        (Exception*);
  void visitCaseLabel        (CaseLabel*);
  void visitUnionCase        (UnionCase*);
  void visitUnion            (Union*);
  void visitUnionForward     (UnionForward*);
  void visitEnumerator       (Enumerator*);
  void visitEnum             (Enum*);
  void visitAttribute        (Attribute*);
  void visitParameter        (Parameter*);
  void visitOperation        (Operation*);
  void visitNative           (Native*);
  void visitStateMember      (StateMember*);
  void visitFactory          (Factory*);
  void visitValueForward     (ValueForward*);
  void visitValueBox         (ValueBox*);
  void visitValueAbs         (ValueAbs*);
  void visitValue            (Value*);

  void visitBaseType    (BaseType*);
  void visitStringType  (StringType*);
  void visitWStringType (WStringType*);
  void visitSequenceType(SequenceType*);
  void visitFixedType   (FixedType*);
  void visitDeclaredType(DeclaredType*);

  PyObject* result() { return result_; }

  static PyObject* scopedNameToList(const ScopedName* sn);
  static PyObject* wstringToList(const IDL_WChar* ws);

private:
  PyObject* pragmasToList(const Pragma* ps);
  PyObject* commentsToList(const Comment* cs);
  void      registerPyDecl(const ScopedName* sn, PyObject* pydecl);
  PyObject* findPyDecl(const ScopedName* sn);

  PyObject* idlast_;
  PyObject* idltype_;

  PyObject* result_; // Current working value
};

PythonVisitor::
PythonVisitor()
{
  idlast_  = PyImport_ImportModule((char*)"omniidl.idlast");
  idltype_ = PyImport_ImportModule((char*)"omniidl.idltype");
  ASSERT_PYOBJ(idlast_);
  ASSERT_PYOBJ(idltype_);
}

PythonVisitor::
~PythonVisitor()
{
  Py_DECREF(idlast_);
  Py_DECREF(idltype_);
}


PyObject*
PythonVisitor::
scopedNameToList(const ScopedName* sn)
{
  ScopedName::Fragment* f;
  int i;

  for (i=0, f = sn->scopeList(); f; f = f->next(), ++i);

  PyObject* pylist = PyList_New(i);

  for (i=0, f = sn->scopeList(); f; f = f->next(), ++i)
    PyList_SetItem(pylist, i, PyString_FromString(f->identifier()));

  return pylist;
}


PyObject*
PythonVisitor::
pragmasToList(const Pragma* ps)
{
  const Pragma* p;
  int i;

  for (i=0, p = ps; p; p = p->next(), ++i);

  PyObject* pylist = PyList_New(i);
  PyObject* pypragma;

  for (i=0, p = ps; p; p = p->next(), ++i) {

    pypragma = PyObject_CallMethod(idlast_, (char*)"Pragma", (char*)"ssi",
				   p->pragmaText(), p->file(), p->line());
    ASSERT_PYOBJ(pypragma);
    PyList_SetItem(pylist, i, pypragma);
  }
  return pylist;
}

PyObject*
PythonVisitor::
commentsToList(const Comment* cs)
{
  const Comment* c;
  int i;

  for (i=0, c = cs; c; c = c->next(), ++i);

  PyObject* pylist = PyList_New(i);
  PyObject* pycomment;

  for (i=0, c = cs; c; c = c->next(), ++i) {

    pycomment = PyObject_CallMethod(idlast_, (char*)"Comment", (char*)"ssi",
				    c->commentText(), c->file(), c->line());
    ASSERT_PYOBJ(pycomment);
    PyList_SetItem(pylist, i, pycomment);
  }
  return pylist;
}


void
PythonVisitor::
registerPyDecl(const ScopedName* sn, PyObject* pydecl)
{
  PyObject* pysn = scopedNameToList(sn);
  PyObject* r    = PyObject_CallMethod(idlast_, (char*)"registerDecl",
				       (char*)"NO", pysn, pydecl);
  ASSERT_PYOBJ(r); Py_DECREF(r);
}

PyObject*
PythonVisitor::
findPyDecl(const ScopedName* sn)
{
  PyObject* pysn   = scopedNameToList(sn);
  PyObject* pydecl = PyObject_CallMethod(idlast_, (char*)"findDecl",
					 (char*)"N", pysn);
  ASSERT_PYOBJ(pydecl);
  return pydecl;
}

PyObject*
PythonVisitor::
wstringToList(const IDL_WChar* ws)
{
  int i;
  const IDL_WChar* wc;

  for (i=0, wc=ws; *wc; ++wc, ++i);
  PyObject* pyl = PyList_New(i);

  for (i=0, wc=ws; *wc; ++wc, ++i)
    PyList_SetItem(pyl, i, PyInt_FromLong(*wc));

  return pyl;
}

//
// AST visit functions
//

void
PythonVisitor::
visitAST(AST* a)
{
  Decl* d;
  int   i;
  for (i=0, d = a->declarations(); d; d = d->next(), ++i);

  PyObject* pydecls = PyList_New(i);

  for (i=0, d = a->declarations(); d; d = d->next(), ++i) {
    d->accept(*this);
    PyList_SetItem(pydecls, i, result_);
  }
  result_ = PyObject_CallMethod(idlast_, (char*)"AST", (char*)"sNNN",
				a->file(), pydecls,
				pragmasToList(a->pragmas()),
				commentsToList(a->comments()));
  ASSERT_RESULT;
}

void
PythonVisitor::
visitModule(Module* m)
{
  Decl* d;
  int   i;
  for (i=0, d = m->definitions(); d; d = d->next(), ++i);

  PyObject* pydecls = PyList_New(i);

  for (i=0, d = m->definitions(); d; d = d->next(), ++i) {
    d->accept(*this);
    PyList_SetItem(pydecls, i, result_);
  }
  result_ = PyObject_CallMethod(idlast_, (char*)"Module", (char*)"siiNNsNsN",
				m->file(), m->line(), (int)m->mainFile(),
				pragmasToList(m->pragmas()),
				commentsToList(m->comments()),
				m->identifier(),
				scopedNameToList(m->scopedName()),
				m->repoId(),
				pydecls);
  ASSERT_RESULT;
  registerPyDecl(m->scopedName(), result_);
}

void
PythonVisitor::
visitInterface(Interface* i)
{
  int       l;
  PyObject* pyobj;
  Decl*     d;

  // Inherited interfaces
  InheritSpec* inh;

  for (l=0, inh = i->inherits(); inh; inh = inh->next(), ++l);
  PyObject* pyinherits = PyList_New(l);

  for (l=0, inh = i->inherits(); inh; inh = inh->next(), ++l) {
    d = inh->decl();
    if (d->kind() == Decl::D_INTERFACE)
      pyobj = findPyDecl(((Interface*)d)->scopedName());
    else if (d->kind() == Decl::D_DECLARATOR)
      pyobj = findPyDecl(((Declarator*)d)->scopedName());
    else
      assert(0);
    PyList_SetItem(pyinherits, l, pyobj);
  }

  PyObject* pyintf =
    PyObject_CallMethod(idlast_, (char*)"Interface", (char*)"siiNNsNsiiN",
			i->file(), i->line(), (int)i->mainFile(),
			pragmasToList(i->pragmas()),
			commentsToList(i->comments()),
			i->identifier(),
			scopedNameToList(i->scopedName()),
			i->repoId(),
			(int)i->abstract(), (int)i->local(), pyinherits);
  ASSERT_PYOBJ(pyintf);
  registerPyDecl(i->scopedName(), pyintf);

  // Contents
  for (l=0, d = i->contents(); d; d = d->next(), ++l);
  PyObject* pycontents = PyList_New(l);

  for (l=0, d = i->contents(); d; d = d->next(), ++l) {
    d->accept(*this);
    PyList_SetItem(pycontents, l, result_);
  }
  PyObject* r = PyObject_CallMethod(pyintf, (char*)"_setContents",
				    (char*)"N", pycontents);
  ASSERT_PYOBJ(r); Py_DECREF(r);
  result_ = pyintf;
}

void
PythonVisitor::
visitForward(Forward* f)
{
  result_ = PyObject_CallMethod(idlast_, (char*)"Forward", (char*)"siiNNsNsii",
				f->file(), f->line(), (int)f->mainFile(),
				pragmasToList(f->pragmas()),
				commentsToList(f->comments()),
				f->identifier(),
				scopedNameToList(f->scopedName()),
				f->repoId(),
				(int)f->abstract(), (int)f->local());
  ASSERT_RESULT;
  registerPyDecl(f->scopedName(), result_);
}

void
PythonVisitor::
visitConst(Const* c)
{
  c->constType()->accept(*this);
  PyObject* pytype = result_;
  PyObject* pyv;

  switch(c->constKind()) {
  case IdlType::tk_short:  pyv = PyInt_FromLong(c->constAsShort());  break;
  case IdlType::tk_long:   pyv = PyInt_FromLong(c->constAsLong());   break;
  case IdlType::tk_ushort: pyv = PyInt_FromLong(c->constAsUShort()); break;
  case IdlType::tk_ulong:
    pyv = PyLong_FromUnsignedLong(c->constAsULong()); break;

  case IdlType::tk_float:  pyv = PyFloat_FromDouble(c->constAsFloat());  break;
  case IdlType::tk_double: pyv = PyFloat_FromDouble(c->constAsDouble()); break;

  case IdlType::tk_boolean:
    pyv = PyInt_FromLong(c->constAsBoolean()); break;

  case IdlType::tk_char:  pyv = PyString_FromChar(c->constAsChar());
    break;
  case IdlType::tk_octet: pyv = PyInt_FromLong(c->constAsOctet()); break;

  case IdlType::tk_string:
    pyv = PyString_FromString(c->constAsString()); break;

#ifdef HAS_LongLong
  case IdlType::tk_longlong:
    pyv = MyPyLong_FromLongLong(c->constAsLongLong()); break;

  case IdlType::tk_ulonglong:
    pyv = PyLong_FromUnsignedLongLong(c->constAsULongLong()); break;

#endif
#ifdef HAS_LongDouble
  case IdlType::tk_longdouble:
    pyv = PyFloat_FromDouble(c->constAsLongDouble());
    IdlWarning(c->file(), c->line(),
	       "long double constant truncated to double. Sorry.");
    break;
#endif
  case IdlType::tk_wchar:   pyv = PyInt_FromLong(c->constAsWChar());  break;
  case IdlType::tk_wstring: pyv = wstringToList(c->constAsWString()); break;

  case IdlType::tk_fixed:
    {
      IDL_Fixed* fv = c->constAsFixed();
      char*      fs = fv->asString();
      pyv           = PyString_FromString(fs);
      delete [] fs;
      delete    fv;
    }
    break;

  case IdlType::tk_enum:
    pyv = findPyDecl(c->constAsEnumerator()->scopedName());
    break;

  default:
    assert(0);
  }
  result_ = PyObject_CallMethod(idlast_, (char*)"Const", (char*)"siiNNsNsNiN",
				c->file(), c->line(), (int)c->mainFile(),
				pragmasToList(c->pragmas()),
				commentsToList(c->comments()),
				c->identifier(),
				scopedNameToList(c->scopedName()),
				c->repoId(),
				pytype, (int)c->constKind(), pyv);
  ASSERT_RESULT;
  registerPyDecl(c->scopedName(), result_);
}

void
PythonVisitor::
visitDeclarator(Declarator* d)
{
  ArraySize* s;
  int        i;

  for (i=0, s = d->sizes(); s; s = s->next(), ++i);
  PyObject* pysizes = PyList_New(i);

  for (i=0, s = d->sizes(); s; s = s->next(), ++i)
    PyList_SetItem(pysizes, i, PyInt_FromLong(s->size()));

  result_ =
    PyObject_CallMethod(idlast_, (char*)"Declarator",(char*)"siiNNsNsN",
			d->file(), d->line(), (int)d->mainFile(),
			pragmasToList(d->pragmas()),
			commentsToList(d->comments()),
			d->identifier(),
			scopedNameToList(d->scopedName()),
			d->repoId(),
			pysizes);
  ASSERT_RESULT;
  registerPyDecl(d->scopedName(), result_);
}

void
PythonVisitor::
visitTypedef(Typedef* t)
{
  if (t->constrType()) {
    ((DeclaredType*)t->aliasType())->decl()->accept(*this);
    Py_DECREF(result_);
  }
  t->aliasType()->accept(*this);
  PyObject* pyaliasType = result_;

  Declarator* d;
  int         i, l;

  for (l=0, d = t->declarators(); d; d = (Declarator*)d->next(), ++l);
  PyObject* pydeclarators = PyList_New(l);

  for (i=0, d = t->declarators(); d; d = (Declarator*)d->next(), ++i) {
    d->accept(*this);
    PyList_SetItem(pydeclarators, i, result_);
  }
  Py_INCREF(pydeclarators);
  result_ = PyObject_CallMethod(idlast_, (char*)"Typedef", (char*)"siiNNNiN",
				t->file(), t->line(), (int)t->mainFile(),
				pragmasToList(t->pragmas()),
				commentsToList(t->comments()),
				pyaliasType, (int)t->constrType(),
				pydeclarators);
  ASSERT_RESULT;

  // Give each Declarator a reference to the Typedef. This creates a
  // loop which Python's GC won't collect :-(
  for (i=0; i<l; ++i) {
    PyObject_CallMethod(PyList_GetItem(pydeclarators, i),
			(char*)"_setAlias", (char*)"O", result_);
  }
  Py_DECREF(pydeclarators);
}

void
PythonVisitor::
visitMember(Member* m)
{
  if (m->constrType()) {
    ((DeclaredType*)m->memberType())->decl()->accept(*this);
    Py_DECREF(result_);
  }
  m->memberType()->accept(*this);
  PyObject* pymemberType = result_;

  Declarator* d;
  int         i;

  for (i=0, d = m->declarators(); d; d = (Declarator*)d->next(), ++i);
  PyObject* pydeclarators = PyList_New(i);

  for (i=0, d = m->declarators(); d; d = (Declarator*)d->next(), ++i) {
    d->accept(*this);
    PyList_SetItem(pydeclarators, i, result_);
  }
  result_ = PyObject_CallMethod(idlast_, (char*)"Member", (char*)"siiNNNiN",
				m->file(), m->line(), (int)m->mainFile(),
				pragmasToList(m->pragmas()),
				commentsToList(m->comments()),
				pymemberType, (int)m->constrType(),
				pydeclarators);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitStruct(Struct* s)
{
  Member* m;
  int     i;

  PyObject* pystruct = 
    PyObject_CallMethod(idlast_, (char*)"Struct", (char*)"siiNNsNsi",
			s->file(), s->line(), (int)s->mainFile(),
			pragmasToList(s->pragmas()),
			commentsToList(s->comments()),
			s->identifier(),
			scopedNameToList(s->scopedName()),
			s->repoId(),
			(int)s->recursive());
  ASSERT_PYOBJ(pystruct);
  registerPyDecl(s->scopedName(), pystruct);

  for (i=0, m = s->members(); m; m = (Member*)m->next(), ++i);
  PyObject* pymembers = PyList_New(i);

  for (i=0, m = s->members(); m; m = (Member*)m->next(), ++i) {
    m->accept(*this);
    PyList_SetItem(pymembers, i, result_);
  }
  PyObject* r = PyObject_CallMethod(pystruct, (char*)"_setMembers",
				    (char*)"N", pymembers);
  ASSERT_PYOBJ(r); Py_DECREF(r);
  result_ = pystruct;
}

void
PythonVisitor::
visitStructForward(StructForward* f)
{
  result_ = PyObject_CallMethod(idlast_, (char*)"StructForward",
				(char*)"siiNNsNs",
				f->file(), f->line(), (int)f->mainFile(),
				pragmasToList(f->pragmas()),
				commentsToList(f->comments()),
				f->identifier(),
				scopedNameToList(f->scopedName()),
				f->repoId());
  ASSERT_RESULT;
  registerPyDecl(f->scopedName(), result_);
}


void
PythonVisitor::
visitException(Exception* e)
{
  Member* m;
  int     i;

  for (i=0, m = e->members(); m; m = (Member*)m->next(), ++i);
  PyObject* pymembers = PyList_New(i);

  for (i=0, m = e->members(); m; m = (Member*)m->next(), ++i) {
    m->accept(*this);
    PyList_SetItem(pymembers, i, result_);
  }
  result_ =
    PyObject_CallMethod(idlast_, (char*)"Exception", (char*)"siiNNsNsN",
			e->file(), e->line(), (int)e->mainFile(),
			pragmasToList(e->pragmas()),
			commentsToList(e->comments()),
			e->identifier(),
			scopedNameToList(e->scopedName()),
			e->repoId(),
			pymembers);
  ASSERT_RESULT;
  registerPyDecl(e->scopedName(), result_);
}

void
PythonVisitor::
visitCaseLabel(CaseLabel* l)
{
  PyObject* pyv;

  switch(l->labelKind()) {
  case IdlType::tk_short:  pyv = PyInt_FromLong(l->labelAsShort());  break;
  case IdlType::tk_long:   pyv = PyInt_FromLong(l->labelAsLong());   break;
  case IdlType::tk_ushort: pyv = PyInt_FromLong(l->labelAsUShort()); break;
  case IdlType::tk_ulong:
    pyv = PyLong_FromUnsignedLong(l->labelAsULong()); break;

  case IdlType::tk_boolean: pyv = PyInt_FromLong(l->labelAsBoolean());  break;
  case IdlType::tk_char:    pyv = PyString_FromChar(l->labelAsChar());
    break;
#ifdef HAS_LongLong
  case IdlType::tk_longlong:
    pyv = MyPyLong_FromLongLong(l->labelAsLongLong());
    break;
  case IdlType::tk_ulonglong:
    pyv = PyLong_FromUnsignedLongLong(l->labelAsULongLong());
    break;
#endif
  case IdlType::tk_wchar:   pyv = PyInt_FromLong(l->labelAsWChar());  break;
  case IdlType::tk_enum:
    pyv = findPyDecl(l->labelAsEnumerator()->scopedName());
    break;
  default:
    assert(0);
  }
  result_ = PyObject_CallMethod(idlast_, (char*)"CaseLabel", (char*)"siiNNiNi",
				l->file(), l->line(), (int)l->mainFile(),
				pragmasToList(l->pragmas()),
				commentsToList(l->comments()),
				(int)l->isDefault(), pyv,
				(int)l->labelKind());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitUnionCase(UnionCase* c)
{
  if (c->constrType()) {
    ((DeclaredType*)c->caseType())->decl()->accept(*this);
    Py_DECREF(result_);
  }
  CaseLabel* l;
  int        i;

  for (i=0, l = c->labels(); l; l = (CaseLabel*)l->next(), ++i);
  PyObject* pylabels = PyList_New(i);

  for (i=0, l = c->labels(); l; l = (CaseLabel*)l->next(), ++i) {
    l->accept(*this);
    PyList_SetItem(pylabels, i, result_);
  }
  c->caseType()->accept(*this);
  PyObject* pycaseType = result_;

  c->declarator()->accept(*this);
  PyObject* pydeclarator = result_;

  result_ =
    PyObject_CallMethod(idlast_, (char*)"UnionCase", (char*)"siiNNNNiN",
			c->file(), c->line(), (int)c->mainFile(),
			pragmasToList(c->pragmas()),
			commentsToList(c->comments()),
			pylabels, pycaseType, (int)c->constrType(),
			pydeclarator);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitUnion(Union* u)
{
  if (u->constrType()) {
    ((DeclaredType*)u->switchType())->decl()->accept(*this);
    Py_DECREF(result_);
  }
  u->switchType()->accept(*this);
  PyObject* pyswitchType = result_;

  PyObject* pyunion =
    PyObject_CallMethod(idlast_, (char*)"Union", (char*)"siiNNsNsNii",
			u->file(), u->line(), (int)u->mainFile(),
			pragmasToList(u->pragmas()),
			commentsToList(u->comments()),
			u->identifier(),
			scopedNameToList(u->scopedName()),
			u->repoId(),
			pyswitchType, (int)u->constrType(),
			(int)u->recursive());
  ASSERT_PYOBJ(pyunion);
  registerPyDecl(u->scopedName(), pyunion);

  UnionCase* c;
  int        i;
  for (i=0, c = u->cases(); c; c = (UnionCase*)c->next(), ++i);
  PyObject* pycases = PyList_New(i);

  for (i=0, c = u->cases(); c; c = (UnionCase*)c->next(), ++i) {
    c->accept(*this);
    PyList_SetItem(pycases, i, result_);
  }

  PyObject* r = PyObject_CallMethod(pyunion, (char*)"_setCases",
				    (char*)"N", pycases);
  ASSERT_PYOBJ(r); Py_DECREF(r);
  result_ = pyunion;
}

void
PythonVisitor::
visitUnionForward(UnionForward* f)
{
  result_ = PyObject_CallMethod(idlast_, (char*)"UnionForward",
				(char*)"siiNNsNs",
				f->file(), f->line(), (int)f->mainFile(),
				pragmasToList(f->pragmas()),
				commentsToList(f->comments()),
				f->identifier(),
				scopedNameToList(f->scopedName()),
				f->repoId());
  ASSERT_RESULT;
  registerPyDecl(f->scopedName(), result_);
}

void
PythonVisitor::
visitEnumerator(Enumerator* e)
{
  result_ =
    PyObject_CallMethod(idlast_, (char*)"Enumerator", (char*)"siiNNsNsi",
			e->file(), e->line(), (int)e->mainFile(),
			pragmasToList(e->pragmas()),
			commentsToList(e->comments()),
			e->identifier(),
			scopedNameToList(e->scopedName()),
			e->repoId(),
			e->value());
  ASSERT_RESULT;
  registerPyDecl(e->scopedName(), result_);
}

void
PythonVisitor::
visitEnum(Enum* e)
{
  Enumerator* n;
  int         i;
  for (i=0, n = e->enumerators(); n; n = (Enumerator*)n->next(), ++i);
  PyObject* pyenumerators = PyList_New(i);

  for (i=0, n = e->enumerators(); n; n = (Enumerator*)n->next(), ++i) {
    n->accept(*this);
    PyList_SetItem(pyenumerators, i, result_);
  }
  result_ = PyObject_CallMethod(idlast_, (char*)"Enum", (char*)"siiNNsNsN",
				e->file(), e->line(), (int)e->mainFile(),
				pragmasToList(e->pragmas()),
				commentsToList(e->comments()),
				e->identifier(),
				scopedNameToList(e->scopedName()),
				e->repoId(),
				pyenumerators);
  ASSERT_RESULT;
  registerPyDecl(e->scopedName(), result_);
}

void
PythonVisitor::
visitAttribute(Attribute* a)
{
  a->attrType()->accept(*this);
  PyObject* pyattrType = result_;

  Declarator* d;
  int         i, l;

  for (l=0, d = a->declarators(); d; d = (Declarator*)d->next(), ++l);
  PyObject* pydeclarators = PyList_New(l);

  for (i=0, d = a->declarators(); d; d = (Declarator*)d->next(), ++i) {
    d->accept(*this);
    PyList_SetItem(pydeclarators, i, result_);
  }
  result_ = PyObject_CallMethod(idlast_, (char*)"Attribute", (char*)"siiNNiNN",
				a->file(), a->line(), (int)a->mainFile(),
				pragmasToList(a->pragmas()),
				commentsToList(a->comments()),
				(int)a->readonly(), pyattrType,
				pydeclarators);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitParameter(Parameter* p)
{
  p->paramType()->accept(*this);
  PyObject* pyparamType = result_;

  result_ = PyObject_CallMethod(idlast_, (char*)"Parameter", (char*)"siiNNiNs",
				p->file(), p->line(), (int)p->mainFile(),
				pragmasToList(p->pragmas()),
				commentsToList(p->comments()),
				p->direction(), pyparamType, p->identifier());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitOperation(Operation* o)
{
  o->returnType()->accept(*this);
  PyObject* pyreturnType = result_;

  Parameter* p;
  int        i;
  for (i=0, p = o->parameters(); p; p = (Parameter*)p->next(), ++i);
  PyObject* pyparameters = PyList_New(i);

  for (i=0, p = o->parameters(); p; p = (Parameter*)p->next(), ++i) {
    p->accept(*this);
    PyList_SetItem(pyparameters, i, result_);
  }

  RaisesSpec* r;
  for (i=0, r = o->raises(); r; r = r->next(), ++i);
  PyObject* pyraises = PyList_New(i);

  for (i=0, r = o->raises(); r; r = r->next(), ++i)
    PyList_SetItem(pyraises, i, findPyDecl(r->exception()->scopedName()));

  ContextSpec* c;
  for (i=0, c = o->contexts(); c; c = c->next(), ++i);
  PyObject* pycontexts = PyList_New(i);

  for (i=0, c = o->contexts(); c; c = c->next(), ++i)
    PyList_SetItem(pycontexts, i, PyString_FromString(c->context()));

  result_ =
    PyObject_CallMethod(idlast_,(char*)"Operation",(char*)"siiNNiNsNsNNN",
			o->file(), o->line(), (int)o->mainFile(),
			pragmasToList(o->pragmas()),
			commentsToList(o->comments()),
			(int)o->oneway(), pyreturnType,
			o->identifier(),
			scopedNameToList(o->scopedName()),
			o->repoId(),
			pyparameters,
			pyraises, pycontexts);
  ASSERT_RESULT;
  registerPyDecl(o->scopedName(), result_);
}

void
PythonVisitor::
visitNative(Native* n)
{
  result_ = PyObject_CallMethod(idlast_, (char*)"Native", (char*)"siiNNsNs",
				n->file(), n->line(), (int)n->mainFile(),
				pragmasToList(n->pragmas()),
				commentsToList(n->comments()),
				n->identifier(),
				scopedNameToList(n->scopedName()),
				n->repoId());
  ASSERT_RESULT;
  registerPyDecl(n->scopedName(), result_);
}

void
PythonVisitor::
visitStateMember(StateMember* s)
{
  if (s->constrType()) {
    ((DeclaredType*)s->memberType())->decl()->accept(*this);
    Py_DECREF(result_);
  }
  s->memberType()->accept(*this);
  PyObject* pymemberType = result_;

  Declarator* d;
  int         i;

  for (i=0, d = s->declarators(); d; d = (Declarator*)d->next(), ++i);
  PyObject* pydeclarators = PyList_New(i);

  for (i=0, d = s->declarators(); d; d = (Declarator*)d->next(), ++i) {
    d->accept(*this);
    PyList_SetItem(pydeclarators, i, result_);
  }
  result_ =
    PyObject_CallMethod(idlast_,(char*)"StateMember",(char*)"siiNNiNiN",
			s->file(), s->line(), (int)s->mainFile(),
			pragmasToList(s->pragmas()),
			commentsToList(s->comments()),
			s->memberAccess(), pymemberType,
			(int)s->constrType(), pydeclarators);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitFactory(Factory* f)
{
  Parameter* p;
  int        i;
  for (i=0, p = f->parameters(); p; p = (Parameter*)p->next(), ++i);
  PyObject* pyparameters = PyList_New(i);

  for (i=0, p = f->parameters(); p; p = (Parameter*)p->next(), ++i) {
    p->accept(*this);
    PyList_SetItem(pyparameters, i, result_);
  }

  RaisesSpec* r;
  for (i=0, r = f->raises(); r; r = r->next(), ++i);
  PyObject* pyraises = PyList_New(i);

  for (i=0, r = f->raises(); r; r = r->next(), ++i)
    PyList_SetItem(pyraises, i, findPyDecl(r->exception()->scopedName()));

  result_ = PyObject_CallMethod(idlast_, (char*)"Factory", (char*)"siiNNsNN",
				f->file(), f->line(), (int)f->mainFile(),
				pragmasToList(f->pragmas()),
				commentsToList(f->comments()),
				f->identifier(), pyparameters, pyraises);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitValueForward(ValueForward* f)
{
  result_ = PyObject_CallMethod(idlast_,
				(char*)"ValueForward", (char*)"siiNNsNsi",
				f->file(), f->line(), (int)f->mainFile(),
				pragmasToList(f->pragmas()),
				commentsToList(f->comments()),
				f->identifier(),
				scopedNameToList(f->scopedName()),
				f->repoId(),
				(int)f->abstract());
  ASSERT_RESULT;
  registerPyDecl(f->scopedName(), result_);
}

void
PythonVisitor::
visitValueBox(ValueBox* b)
{
  if (b->constrType()) {
    ((DeclaredType*)b->boxedType())->decl()->accept(*this);
    Py_DECREF(result_);
  }
  b->boxedType()->accept(*this);
  PyObject* pyboxedType = result_;

  result_ =
    PyObject_CallMethod(idlast_, (char*)"ValueBox", (char*)"siiNNsNsNi",
			b->file(), b->line(), (int)b->mainFile(),
			pragmasToList(b->pragmas()),
			commentsToList(b->comments()),
			b->identifier(),
			scopedNameToList(b->scopedName()),
			b->repoId(),
			pyboxedType, (int)b->constrType());
  ASSERT_RESULT;
  registerPyDecl(b->scopedName(), result_);
}

void
PythonVisitor::
visitValueAbs(ValueAbs* a)
{
  int       l;
  PyObject* pyobj;
  Decl*     d;

  // Inherited values and interfaces
  InheritSpec*      inh;
  ValueInheritSpec* vinh;

  for (l=0, vinh = a->inherits(); vinh; vinh = vinh->next(), ++l);
  PyObject* pyinherits = PyList_New(l);

  for (l=0, vinh = a->inherits(); vinh; vinh = vinh->next(), ++l) {
    d = vinh->decl();
    if (d->kind() == Decl::D_VALUEABS)
      pyobj = findPyDecl(((ValueAbs*)d)->scopedName());
    else if (d->kind() == Decl::D_DECLARATOR)
      pyobj = findPyDecl(((Declarator*)d)->scopedName());
    else
      assert(0);
    PyList_SetItem(pyinherits, l, pyobj);
  }
  for (l=0, inh = a->supports(); inh; inh = inh->next(), ++l);
  PyObject* pysupports = PyList_New(l);

  for (l=0, inh = a->supports(); inh; inh = inh->next(), ++l) {
    d = inh->decl();
    if (d->kind() == Decl::D_INTERFACE)
      pyobj = findPyDecl(((Interface*)d)->scopedName());
    else if (d->kind() == Decl::D_DECLARATOR)
      pyobj = findPyDecl(((Declarator*)d)->scopedName());
    else
      assert(0);
    PyList_SetItem(pysupports, l, pyobj);
  }

  PyObject* pyvalue =
    PyObject_CallMethod(idlast_, (char*)"ValueAbs", (char*)"siiNNsNsNN",
			a->file(), a->line(), (int)a->mainFile(),
			pragmasToList(a->pragmas()),
			commentsToList(a->comments()),
			a->identifier(),
			scopedNameToList(a->scopedName()),
			a->repoId(),
			pyinherits, pysupports);
  ASSERT_PYOBJ(pyvalue);
  registerPyDecl(a->scopedName(), pyvalue);

  // Contents
  for (l=0, d = a->contents(); d; d = d->next(), ++l);
  PyObject* pycontents = PyList_New(l);

  for (l=0, d = a->contents(); d; d = d->next(), ++l) {
    d->accept(*this);
    PyList_SetItem(pycontents, l, result_);
  }
  PyObject* r = PyObject_CallMethod(pyvalue, (char*)"_setContents",
				    (char*)"N", pycontents);
  ASSERT_PYOBJ(r); Py_DECREF(r);
  result_ = pyvalue;
}

void
PythonVisitor::
visitValue(Value* v)
{
  int       l;
  PyObject* pyobj;
  Decl*     d;

  // Inherited values and interfaces
  InheritSpec*      inh;
  ValueInheritSpec* vinh;
  int               truncatable = 0;

  if (v->inherits()) truncatable = v->inherits()->truncatable();

  for (l=0, vinh = v->inherits(); vinh; vinh = vinh->next(), ++l);
  PyObject* pyinherits = PyList_New(l);

  for (l=0, vinh = v->inherits(); vinh; vinh = vinh->next(), ++l) {
    d = vinh->decl();
    if (d->kind() == Decl::D_VALUE)
      pyobj = findPyDecl(((Value*)d)->scopedName());
    else if (d->kind() == Decl::D_VALUEABS)
      pyobj = findPyDecl(((ValueAbs*)d)->scopedName());
    else if (d->kind() == Decl::D_DECLARATOR)
      pyobj = findPyDecl(((Declarator*)d)->scopedName());
    else
      assert(0);
    PyList_SetItem(pyinherits, l, pyobj);
  }
  for (l=0, inh = v->supports(); inh; inh = inh->next(), ++l);
  PyObject* pysupports = PyList_New(l);

  for (l=0, inh = v->supports(); inh; inh = inh->next(), ++l) {
    d = inh->decl();
    if (d->kind() == Decl::D_INTERFACE)
      pyobj = findPyDecl(((Interface*)d)->scopedName());
    else if (d->kind() == Decl::D_DECLARATOR)
      pyobj = findPyDecl(((Declarator*)d)->scopedName());
    else
      assert(0);
    PyList_SetItem(pysupports, l, pyobj);
  }

  PyObject* pyvalue =
    PyObject_CallMethod(idlast_, (char*)"Value", (char*)"siiNNsNsiNiN",
			v->file(), v->line(), (int)v->mainFile(),
			pragmasToList(v->pragmas()),
			commentsToList(v->comments()),
			v->identifier(),
			scopedNameToList(v->scopedName()),
			v->repoId(),
			(int)v->custom(), pyinherits,
			truncatable, pysupports);
  ASSERT_PYOBJ(pyvalue);
  registerPyDecl(v->scopedName(), pyvalue);

  // Contents
  for (l=0, d = v->contents(); d; d = d->next(), ++l);
  PyObject* pycontents = PyList_New(l);

  for (l=0, d = v->contents(); d; d = d->next(), ++l) {
    d->accept(*this);
    PyList_SetItem(pycontents, l, result_);
  }
  PyObject* r = PyObject_CallMethod(pyvalue, (char*)"_setContents",
				    (char*)"N", pycontents);
  ASSERT_PYOBJ(r); Py_DECREF(r);
  result_ = pyvalue;
}

// Types

void
PythonVisitor::
visitBaseType(BaseType* t)
{
  result_ = PyObject_CallMethod(idltype_, (char*)"baseType", (char*)"i",
				(int)t->kind());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitStringType(StringType* t)
{
  result_ = PyObject_CallMethod(idltype_, (char*)"stringType",
				(char*)"i", t->bound());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitWStringType(WStringType* t)
{
  result_ = PyObject_CallMethod(idltype_, (char*)"wstringType",
				(char*)"i", t->bound());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitSequenceType(SequenceType* t)
{
  t->seqType()->accept(*this);
  result_ = PyObject_CallMethod(idltype_, (char*)"sequenceType", (char*)"Nii",
				result_, t->bound(), (int)t->local());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitFixedType(FixedType* t)
{
  result_ = PyObject_CallMethod(idltype_, (char*)"fixedType", (char*)"ii",
				t->digits(), t->scale());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitDeclaredType(DeclaredType* t)
{
  if (t->decl()) {
    result_ =
      PyObject_CallMethod(idltype_, (char*)"declaredType", (char*)"NNii",
			  findPyDecl(t->declRepoId()->scopedName()),
			  scopedNameToList(t->declRepoId()->scopedName()),
			  (int)t->kind(), (int)t->local());
  }
  else {
    if (t->kind() == IdlType::tk_objref) {
      PyObject* pysn   = Py_BuildValue((char*)"[ss]", (char*)"CORBA",
				       (char*)"Object");

      PyObject* pydecl = PyObject_CallMethod(idlast_, (char*)"findDecl",
					     (char*)"O", pysn);

      result_          = PyObject_CallMethod(idltype_, (char*)"declaredType",
					     (char*)"NNii", pydecl, pysn,
					     (int)t->kind(), (int)t->local());
    }
    else if (t->kind() == IdlType::tk_value) {
      PyObject* pysn   = Py_BuildValue((char*)"[ss]", (char*)"CORBA",
				       (char*)"ValueBase");

      PyObject* pydecl = PyObject_CallMethod(idlast_, (char*)"findDecl",
					     (char*)"O", pysn);

      result_          = PyObject_CallMethod(idltype_, (char*)"declaredType",
					     (char*)"NNii", pydecl, pysn,
					     (int)t->kind(), (int)t->local());
    }
    else abort();
  }
  ASSERT_RESULT;
}

extern "C" {
  static PyObject* IdlPyCompile(PyObject* self, PyObject* args)
  {
    PyObject*   arg;
    const char* name;
    FILE*       file;
    IDL_Boolean to_close = 0;

    if (!PyArg_ParseTuple(args, (char*)"Os", &arg, &name))
      return 0;

    if (PyString_Check(arg)) {
      name = PyString_AsString(arg);
      file = fopen(name, "r");
      if (!file) {
	PyErr_SetString(PyExc_IOError,
			(char*)"Cannot open file");
	return 0;
      }
      to_close = 1;
    }

#ifndef OMNIIDL_PY3

    else if (PyFile_Check(arg)) {
      PyObject* pyname = PyFile_Name(arg);
      file = PyFile_AsFile(arg);
    }
    else {
      PyErr_SetString(PyExc_TypeError,
		      (char*)"First argument must be a file or filename");
      return 0;
    }
#else
    else {
      int fd = PyObject_AsFileDescriptor(arg);
      if (fd == -1)
	return 0;

      file = fdopen(fd, "r");
      if (!file) {
	PyErr_SetString(PyExc_IOError,
			(char*)"Cannot open file descriptor");
	return 0;
      }
    }
#endif

    IDL_Boolean success = AST::process(file, name);

    if (to_close)
      fclose(file);

    if (success) {
      PythonVisitor v;
      AST::tree()->accept(v);
      return v.result();
    }
    else {
      AST::clear();
      Py_INCREF(Py_None);
      return Py_None;
    }
  }

  static PyObject* IdlPyClear(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)""))
      return 0;

    AST::clear();
    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject* IdlPyDump(PyObject* self, PyObject* args)
  {
    PyObject*   arg;
    const char* name;
    FILE*       file;
    IDL_Boolean to_close = 0;

    if (!PyArg_ParseTuple(args, (char*)"Os", &arg, &name))
      return 0;

    if (PyString_Check(arg)) {
      name = PyString_AsString(arg);
      file = fopen(name, "r");
      if (!file) {
	PyErr_SetString(PyExc_IOError,
			(char*)"Cannot open file");
	return 0;
      }
      to_close = 1;
    }

#ifndef OMNIIDL_PY3

    else if (PyFile_Check(arg)) {
      PyObject* pyname = PyFile_Name(arg);
      file = PyFile_AsFile(arg);
      name = PyString_AsString(pyname);
    }
    else {
      PyErr_SetString(PyExc_TypeError,
		      (char*)"Argument must be a file or filename");
      return 0;
    }
#else
    else {
      int fd = PyObject_AsFileDescriptor(arg);
      if (fd == -1)
	return 0;

      file = fdopen(fd, "r");
      if (!file) {
	PyErr_SetString(PyExc_IOError,
			(char*)"Cannot open file descriptor");
	return 0;
      }
    }
#endif

    IDL_Boolean success = AST::process(file, name);

    if (to_close)
      fclose(file);

    if (success) {
      DumpVisitor v;
      AST::tree()->accept(v);
    }
    AST::clear();

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject* IdlPyQuiet(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)"")) return 0;
    Config::quiet = 1;
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject* IdlPyNoForwardWarning(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)"")) return 0;
    Config::forwardWarning = 0;
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject* IdlPyKeepComments(PyObject* self, PyObject* args)
  {
    int first;
    if (!PyArg_ParseTuple(args, (char*)"i", &first)) return 0;
    Config::keepComments  = 1;
    Config::commentsFirst = first;
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject* IdlPyRelativeScopedName(PyObject* self, PyObject* args)
  {
    PyObject *pyfrom, *pyto;
    if (!PyArg_ParseTuple(args, (char*)"OO", &pyfrom, &pyto)) return 0;

    if (!PySequence_Check(pyfrom) || !PySequence_Check(pyto)) {
      PyErr_SetString(PyExc_TypeError,
		      (char*)"Both arguments must be sequences of strings");
      return 0;
    }

    if (PyObject_Length(pyto) == 0) {
      PyErr_SetString(PyExc_TypeError,
		      (char*)"Argument 2 must be a non-empty sequence");
      return 0;
    }

    ScopedName* from = 0;
    ScopedName* to   = 0;

    int i;
    // Convert lists to absolute ScopedNames
    for (i=0; i < PyObject_Length(pyfrom); i++) {
      PyObject* tmp = PySequence_GetItem(pyfrom, i);

      if (!PyString_Check(tmp)) {
	if (from) delete from;
	PyErr_SetString(PyExc_TypeError,
			(char*)"Both arguments must be sequences of strings");
	return 0;
      }
      if (from)
	from->append(PyString_AsString(tmp));
      else
	from = new ScopedName(PyString_AsString(tmp), 1);
    }

    for (i=0; i < PyObject_Length(pyto); i++) {
      PyObject* tmp = PySequence_GetItem(pyto, i);

      if (!PyString_Check(tmp)) {
	if (from) delete from;
	if (to)   delete to;
	PyErr_SetString(PyExc_TypeError,
			(char*)"Both arguments must be sequences of strings");
	return 0;
      }
      if (to)
	to->append(PyString_AsString(tmp));
      else
	to = new ScopedName(PyString_AsString(tmp), 1);
    }

    ScopedName* result = Scope::relativeScopedName(from, to);

    if (from) delete from;
    delete to;

    if (result) {
      PyObject* pyresult = PythonVisitor::scopedNameToList(result);
      if (result->absolute())
	PyList_Insert(pyresult, 0, Py_None);
      delete result;
      return pyresult;
    }
    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject* IdlPyRunInteractiveLoop(PyObject* self, PyObject* args)
  {
    PyRun_InteractiveLoop(stdin, (char*)"<stdin>");
    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject* IdlPyCaseSensitive(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)"")) return 0;
    Config::caseSensitive = 1;
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject* IdlPyPlatformDefines(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)"")) return 0;
    PyObject* l = PyList_New(0);
#ifdef HAS_LongLong
    PyList_Append(l, PyString_FromString("-DHAS_LongLong"));
#endif
#ifdef HAS_LongDouble
    PyList_Append(l, PyString_FromString("-DHAS_LongDouble"));
#endif
    return l;
  }

  static PyObject* IdlPyAlwaysTempFile(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)"")) return 0;
#if defined (_MSC_VER) && _MSC_VER > 1200 || defined(__DMC__)
    return PyInt_FromLong(1);
#else
    return PyInt_FromLong(0);
#endif
  }

  static PyMethodDef omniidl_methods[] = {
    {(char*)"compile",            IdlPyCompile,            METH_VARARGS},
    {(char*)"clear",              IdlPyClear,              METH_VARARGS},
    {(char*)"dump",               IdlPyDump,               METH_VARARGS},
    {(char*)"quiet",              IdlPyQuiet,              METH_VARARGS},
    {(char*)"noForwardWarning",   IdlPyNoForwardWarning,   METH_VARARGS},
    {(char*)"keepComments",       IdlPyKeepComments,       METH_VARARGS},
    {(char*)"relativeScopedName", IdlPyRelativeScopedName, METH_VARARGS},
    {(char*)"runInteractiveLoop", IdlPyRunInteractiveLoop, METH_VARARGS},
    {(char*)"caseSensitive",      IdlPyCaseSensitive,      METH_VARARGS},
    {(char*)"platformDefines",    IdlPyPlatformDefines,    METH_VARARGS},
    {(char*)"alwaysTempFile",     IdlPyAlwaysTempFile,     METH_VARARGS},
    {NULL, NULL}
  };

#ifndef OMNIIDL_PY3

  void DLL_EXPORT init_omniidl()
  {
    PyObject* m = Py_InitModule((char*)"_omniidl", omniidl_methods);
    PyObject_SetAttrString(m, (char*)"version",
			   PyString_FromString(IDLMODULE_VERSION));
  }

#else

  static struct PyModuleDef omniidlmodule = {
    PyModuleDef_HEAD_INIT,
    "_omniidl",
    "omniidl front-end",
    -1,
    omniidl_methods,
    NULL,
    NULL,
    NULL,
    NULL
  };

  PyMODINIT_FUNC
  PyInit__omniidl(void)
  {
    PyObject* m = PyModule_Create(&omniidlmodule);
    if (!m)
      return 0;

    PyObject_SetAttrString(m, (char*)"version",
			   PyString_FromString(IDLMODULE_VERSION));
    return m;
  }

#endif

}

#ifdef OMNIIDL_EXECUTABLE

// It's awkward to make a command named 'omniidl' on NT which runs
// Python, so we make the front-end a Python executable which always
// runs omniidl.main.
#ifdef __VMS
#ifdef PYTHON_1
extern "C" int PyVMS_init(int* pvi_argc, char*** pvi_argv);
#endif
#endif

#if defined(__WIN32__) && defined(OMNIIDL_PY3)
int
wmain(int argc, wchar_t** argv)
#else
int
main(int argc, char** argv)
#endif
{
  const char* omniidl_string =
"import sys, os, os.path\n"
"\n"
"pylibdir   = None\n"
"binarchdir = os.path.abspath(os.path.dirname(sys.executable))\n"
"\n"
"if binarchdir != '':\n"
"    sys.path.insert(0, binarchdir)\n"
"    bindir, archname = os.path.split(binarchdir)\n"
"    treedir, bin     = os.path.split(bindir)\n"
"    if bin.lower() == 'bin':\n"
"        pylibdir = os.path.join(treedir, 'lib', 'python')\n"
"\n"
"        if os.path.isdir(pylibdir):\n"
"            sys.path.insert(0, pylibdir)\n"
"else:\n"
"    sys.stderr.write('''can't parse %s's path name!''' % sys.executable)\n"
"\n"
"try:\n"
"    import omniidl.main\n"
#ifdef OMNIIDL_PY3
"except ImportError as msg:\n"
#else
"except ImportError, msg:\n"
#endif
"    sys.stderr.write('\\n\\n')\n"
"    sys.stderr.write('omniidl: ERROR!\\n\\n')\n"
"    sys.stderr.write('omniidl: Could not open Python files for IDL compiler\\n')\n"
"    sys.stderr.write('omniidl: Please put them in directory ' + \\\n"
"                     (pylibdir or binarchdir) + '\\n')\n"
"    sys.stderr.write('omniidl: (or set the PYTHONPATH environment variable)\\n')\n"
"    sys.stderr.write('\\n')\n"
"    sys.stderr.write('omniidl: (The error was \\'' + str(msg) + '\\')\\n')\n"
"    sys.stderr.write('\\n\\n')\n"
"    sys.stderr.flush()\n"
"    sys.exit(1)\n"
"\n"
"omniidl.main.main()\n";

#ifdef __VMS
#ifdef PYTHON_1
  PyVMS_init(&argc, &argv);
#endif
  Py_SetProgramName(argv[0]);
#endif

#ifdef OMNIIDL_PY3
  PyImport_AppendInittab("_omniidl", &PyInit__omniidl);
#endif

  Py_Initialize();
  PySys_SetArgv(argc, argv);

#ifndef OMNIIDL_PY3
  init_omniidl();
#endif

  return PyRun_SimpleString((char*)omniidl_string);
}

#endif
