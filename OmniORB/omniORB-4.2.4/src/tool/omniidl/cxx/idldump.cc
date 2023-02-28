// -*- c++ -*-
//                          Package   : omniidl
// idldump.cc               Created on: 1999/10/26
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2005 Apasphere Ltd
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
//   Visitor object to dump the tree

#include <idldump.h>
#include <idlutil.h>
#include <idlast.h>
#include <idltype.h>

#include <stdio.h>
#include <ctype.h>

DumpVisitor::
DumpVisitor()
  : indent_(0)
{
}

DumpVisitor::
~DumpVisitor()
{
}

void
DumpVisitor::
printIndent()
{
  for (int i=0; i<indent_; ++i)
    printf("  ");
}

void
DumpVisitor::
printScopedName(const ScopedName* sn)
{
  char* ssn = sn->toString();
  printf("%s", ssn);
  delete [] ssn;
}

void
DumpVisitor::
printString(const char* str)
{
  const char* c;
  for (c=str; *c; c++) {
    if (*c == '\\')
      printf("\\\\");
    else if (isprint(*c))
      putchar(*c);
    else
      printf("\\%03o", (int)(unsigned char)*c);
  }
}

void
DumpVisitor::
printChar(const char c)
{
  if (c == '\\')
    printf("\\\\");
  else if (isprint(c))
    putchar(c);
  else
    printf("\\%03o", (int)(unsigned char)c);
}


void
DumpVisitor::
visitAST(AST* a)
{
  printf("\n");
  for (Decl* d = a->declarations(); d; d = d->next()) {
    d->accept(*this);
    printf(";\n\n");
  }
}

void
DumpVisitor::
visitModule(Module* m)
{
  printf("module %s { // RepoId = %s, file = %s, line = %d, %s\n",
	 m->identifier(), m->repoId(), m->file(), m->line(),
	 m->mainFile() ? "in main file" : "not in main file");
  ++indent_;
  for (Decl* d = m->definitions(); d; d = d->next()) {
    printIndent();
    d->accept(*this);
    printf(";\n");
  }
  --indent_;
  printIndent();
  printf("}");
}

void
DumpVisitor::
visitInterface(Interface* i)
{
  if (i->abstract()) printf("abstract ");
  if (i->local())    printf("local ");
  printf("interface %s ", i->identifier());

  if (i->inherits()) {
    printf(": ");
    char* ssn;
    for (InheritSpec* is = i->inherits(); is; is = is->next()) {
      ssn = is->interface()->scopedName()->toString();
      printf("%s%s ", ssn, is->next() ? "," : "");
      delete [] ssn;
    }
  }
  printf("{ // RepoId = %s\n", i->repoId());

  ++indent_;
  for (Decl* d = i->contents(); d; d = d->next()) {
    printIndent();
    d->accept(*this);
    printf(";\n");
  }
  --indent_;
  printIndent();
  printf("}");
}

void
DumpVisitor::
visitForward(Forward* f)
{
  if (f->abstract()) printf("abstract ");
  if (f->local())    printf("local ");
  printf("interface %s; // RepoId = %s", f->identifier(), f->repoId());
}

static void
printdouble(IDL_Double d)
{
  // .17g guarantees an IEEE 754 double can be exactly reconstructed,
  // but it prints integers without a trailing .0, so we must put it
  // back on in that case.
  char buffer[1024];
  sprintf(buffer, "%.17g", d);
  char* c = buffer;
  if (*c == '-') ++c;
  for (; *c; c++) {
    if (!isdigit(*c))
      break;
  }
  if (*c == '\0') {
    *c++ = '.';
    *c++ = '0';
    *c++ = '\0';
  }
  printf("%s", buffer);
}

#ifdef HAS_LongDouble
static void
printlongdouble(IDL_LongDouble d)
{
  // Can't find a reference for how many digits it enough for long
  // double. 40 is probably enough.
  char buffer[1024];
  sprintf(buffer, "%.40Lg", d);
  char* c = buffer;
  if (*c == '-') ++c;
  for (; *c; c++) {
    if (!isdigit(*c))
      break;
  }
  if (*c == '\0') {
    *c++ = '.';
    *c++ = '0';
    *c++ = '\0';
  }
  printf("%s", buffer);
}
#endif


static void
printwchar(IDL_WChar wc)
{
  if (wc == '\\')
    printf("L'\\\\'");
  else if (wc < 0xff && isprint(wc))
    printf("L'%c'", (char)wc);
  else
    printf("L'\\u%04x", (int)wc);
}

static void
printwstring(const IDL_WChar* ws)
{
  printf("L\"");
  int i, wc;
  for (i=0; ws[i]; ++i) {
    wc = ws[i];
    if (wc == '\\')
      printf("\\\\");
    else if (wc < 0xff && isprint(wc))
      putchar(wc);
    else
      printf("\\u%04x", (int)wc);
  }
  putchar('"');
}


void
DumpVisitor::
visitConst(Const* c)
{
  printf("const ");

  c->constType()->accept(*this);
  printf(" %s = ", c->identifier());

  switch(c->constKind()) {
  case IdlType::tk_short:   printf("%hd", c->constAsShort());           break;
  case IdlType::tk_long:    printf("%ld", (long)c->constAsLong());      break;
  case IdlType::tk_ushort:  printf("%hu", c->constAsUShort());          break;
  case IdlType::tk_ulong:   printf("%lu", (unsigned long)c->constAsULong());
                                                                        break;
  case IdlType::tk_float:   printdouble(c->constAsFloat());             break;
  case IdlType::tk_double:  printdouble(c->constAsDouble());            break;
  case IdlType::tk_boolean:
    printf("%s", c->constAsBoolean() ? "TRUE" : "FALSE");
    break;
  case IdlType::tk_char:
    printf("'");
    printChar(c->constAsChar());
    printf("'");
    break;
  case IdlType::tk_octet:   printf("%d",     (int)c->constAsOctet());  break;
  case IdlType::tk_string:
    printf("\"");
    printString(c->constAsString());
    printf("\"");
    break;
#ifdef HAS_LongLong

#  if defined(SIZEOF_LONG) && (SIZEOF_LONG == 8)
  case IdlType::tk_longlong:  printf("%ld", c->constAsLongLong());     break;
  case IdlType::tk_ulonglong: printf("%lu", c->constAsULongLong());    break;
#  else
  case IdlType::tk_longlong:  printf("%Ld", c->constAsLongLong());     break;
  case IdlType::tk_ulonglong: printf("%Lu", c->constAsULongLong());    break;
#  endif

#endif
#ifdef HAS_LongDouble
  case IdlType::tk_longdouble:printlongdouble(c->constAsLongDouble()); break;
#endif
  case IdlType::tk_wchar:     printwchar(c->constAsWChar());           break;
  case IdlType::tk_wstring:   printwstring(c->constAsWString());       break;

  case IdlType::tk_fixed:
    {
      IDL_Fixed* fv = c->constAsFixed();
      char*      fs = fv->asString();
      printf("%sd", fs);
      delete [] fs;
      delete    fv;
    }
    break;

  case IdlType::tk_enum:      c->constAsEnumerator()->accept(*this);   break;
  default:
    assert(0);
  }
}

void
DumpVisitor::
visitDeclarator(Declarator* d)
{
  printf("%s", d->identifier());
  for (ArraySize* s = d->sizes(); s; s = s->next())
    printf("[%d]", s->size());
}

void
DumpVisitor::
visitTypedef(Typedef* t)
{
  printf("typedef ");

  if (t->constrType()) { 
    assert(t->aliasType()->kind() == IdlType::tk_struct ||
	   t->aliasType()->kind() == IdlType::tk_union  ||
	   t->aliasType()->kind() == IdlType::tk_enum);

    ((DeclaredType*)t->aliasType())->decl()->accept(*this);
  }
  else
    t->aliasType()->accept(*this);

  printf(" ");
  for (Declarator* d = t->declarators(); d; d = (Declarator*)d->next()) {
    d->accept(*this);
    if (d->next()) printf(", ");
  }
}

void
DumpVisitor::
visitMember(Member* m)
{
  if (m->constrType()) {
    assert(m->memberType()->kind() == IdlType::tk_struct ||
	   m->memberType()->kind() == IdlType::tk_union  ||
	   m->memberType()->kind() == IdlType::tk_enum);

    ((DeclaredType*)m->memberType())->decl()->accept(*this);
  }
  else
    m->memberType()->accept(*this);

  printf(" ");
  for (Declarator* d = m->declarators(); d; d = (Declarator*)d->next()) {
    d->accept(*this);
    if (d->next())
      printf(", ");
  }
}

void
DumpVisitor::
visitStruct(Struct* s)
{
  printf("struct %s { // RepoId = %s%s\n", s->identifier(), s->repoId(),
	 s->recursive() ? " recursive" : "");

  ++indent_;
  for (Member* m = s->members(); m; m = (Member*)m->next()) {
    printIndent();
    m->accept(*this);
    printf(";\n");
  }
  --indent_;
  printIndent();
  printf("}");
}

void
DumpVisitor::
visitStructForward(StructForward* s)
{
  printf("struct %s", s->identifier());
}


void
DumpVisitor::
visitException(Exception* e)
{
  printf("exception %s {\n", e->identifier());

  ++indent_;
  for (Member* m = e->members(); m; m = (Member*)m->next()) {
    printIndent();
    m->accept(*this);
    printf(";\n");
  }
  --indent_;
  printIndent();
  printf("}");
}

void
DumpVisitor::
visitCaseLabel(CaseLabel* l)
{
  if (l->isDefault())
    printf("default /* ");
  else
    printf("case ");

  switch(l->labelKind()) {
  case IdlType::tk_short:  printf("%hd", l->labelAsShort());        break;
  case IdlType::tk_long:   printf("%ld", (long)l->labelAsLong());   break;
  case IdlType::tk_ushort: printf("%hu", l->labelAsUShort());       break;
  case IdlType::tk_ulong:  printf("%lu", (unsigned long)l->labelAsULong());
                                                                    break;
  case IdlType::tk_boolean:
    printf("%s", l->labelAsBoolean() ? "TRUE" : "FALSE");
    break;
  case IdlType::tk_char:
    printf("'");
    printChar(l->labelAsChar());
    printf("'");
    break;
#ifdef HAS_LongLong

#  if defined(SIZEOF_LONG) && (SIZEOF_LONG == 8)
  case IdlType::tk_longlong:  printf("%ld", l->labelAsLongLong());  break;
  case IdlType::tk_ulonglong: printf("%lu", l->labelAsULongLong()); break;
#  else
  case IdlType::tk_longlong:  printf("%Ld", l->labelAsLongLong());  break;
  case IdlType::tk_ulonglong: printf("%Lu", l->labelAsULongLong()); break;
#  endif

#endif
  case IdlType::tk_wchar:     printf("'\\u%hx", l->labelAsWChar()); break;
  case IdlType::tk_enum: l->labelAsEnumerator()->accept(*this);     break;
  default:
    assert(0);
  }
  if (l->isDefault())
    printf(" */:");
  else
    printf(":");
}

void
DumpVisitor::
visitUnionCase(UnionCase* c)
{
  for (CaseLabel* l = c->labels(); l; l = (CaseLabel*)l->next()) {
    l->accept(*this);
    if (l->next()) printf(" ");
  }
  printf("\n");
  ++indent_;
  printIndent();

  if (c->constrType()) {
    assert(c->caseType()->kind() == IdlType::tk_struct ||
	   c->caseType()->kind() == IdlType::tk_union  ||
	   c->caseType()->kind() == IdlType::tk_enum);

    ((DeclaredType*)c->caseType())->decl()->accept(*this);
  }
  else
    c->caseType()->accept(*this);

  printf(" %s", c->declarator()->identifier());
  --indent_;
}

void
DumpVisitor::
visitUnion(Union* u)
{
  printf("union %s switch (", u->identifier());

  if (u->constrType())
    ((DeclaredType*)u->switchType())->decl()->accept(*this);
  else
    u->switchType()->accept(*this);
  printf(") { // RepoId = %s%s\n", u->repoId(),
	 u->recursive() ? " recursive" : "");
  ++indent_;
  for (UnionCase* c = u->cases(); c; c = (UnionCase*)c->next()) {
    printIndent();
    c->accept(*this);
    printf(";\n");
  }
  --indent_;
  printIndent();
  printf("}");
}

void
DumpVisitor::
visitUnionForward(UnionForward* u)
{
  printf("union %s", u->identifier());
}

void
DumpVisitor::
visitEnumerator(Enumerator* e)
{
  char* ssn = e->scopedName()->toString();
  printf("%s", ssn);
  delete [] ssn;
}

void
DumpVisitor::
visitEnum(Enum* e)
{
  printf("enum %s { // RepoId = %s\n", e->identifier(), e->repoId());
  ++indent_;
  for (Enumerator* n = e->enumerators(); n; n = (Enumerator*)n->next()) {
    printIndent();
    printf("%s%s\n", n->identifier(), n->next() ? "," : "");
  }
  --indent_;
  printIndent();
  printf("}");
}

void
DumpVisitor::
visitAttribute(Attribute* a)
{
  if (a->readonly()) printf("readonly ");
  printf("attribute ");
  a->attrType()->accept(*this);
  printf(" ");
  for (Declarator* d = a->declarators(); d; d = (Declarator*)d->next()) {
    d->accept(*this);
    if (d->next()) printf(", ");
  }
}

void
DumpVisitor::
visitParameter(Parameter* p)
{
  switch (p->direction()) {
  case 0: printf("in ");    break;
  case 1: printf("out ");   break;
  case 2: printf("inout "); break;
  }
  p->paramType()->accept(*this);
  printf(" %s", p->identifier());
}

void
DumpVisitor::
visitOperation(Operation* o)
{
  if (o->oneway()) printf("oneway ");
  o->returnType()->accept(*this);
  printf(" %s(", o->identifier());

  for (Parameter* p = o->parameters(); p; p = (Parameter*)p->next()) {
    p->accept(*this);
    if (p->next()) printf(", ");
  }
  printf(")");

  if (o->raises()) {
    printf(" raises (");
    char* ssn;
    for (RaisesSpec* r = o->raises(); r; r = r->next()) {
      ssn = r->exception()->scopedName()->toString();
      printf("%s", ssn);
      delete [] ssn;
      if (r->next()) printf(", ");
    }
    printf(")");
  }
  if (o->contexts()) {
    printf(" context (");
    for (ContextSpec* c = o->contexts(); c; c = c->next()) {
      printf("\"%s\"", c->context());
      if (c->next()) printf(", ");
    }
    printf(")");
  }
}

void
DumpVisitor::
visitNative(Native* n)
{
  printf("native %s", n->identifier());
}

void
DumpVisitor::
visitStateMember(StateMember* s)
{
  switch(s->memberAccess()) {
  case 0: printf("public ");  break;
  case 1: printf("private "); break;
  }

  if (s->constrType()) { 
    assert(s->memberType()->kind() == IdlType::tk_struct ||
	   s->memberType()->kind() == IdlType::tk_union  ||
	   s->memberType()->kind() == IdlType::tk_enum);

    ((DeclaredType*)s->memberType())->decl()->accept(*this);
  }
  else
    s->memberType()->accept(*this);

  printf(" ");

  for (Declarator* d = s->declarators(); d; d = (Declarator*)d->next()) {
    d->accept(*this);
    if (d->next()) printf(", ");
  }
}

void
DumpVisitor::
visitFactory(Factory* f)
{
  printf("factory %s(", f->identifier());
  for (Parameter* p = f->parameters(); p; p = (Parameter*)p->next()) {
    p->accept(*this);
    if (p->next()) printf(", ");
  }
  printf(")");
  if (f->raises()) {
    printf(" raises (");
    char* ssn;
    for (RaisesSpec* r = f->raises(); r; r = r->next()) {
      ssn = r->exception()->scopedName()->toString();
      printf("%s", ssn);
      delete [] ssn;
      if (r->next()) printf(", ");
    }
    printf(")");
  }
}

void
DumpVisitor::
visitValueForward(ValueForward* f)
{
  if (f->abstract()) printf("abstract ");
  printf("valuetype %s", f->identifier());
}

void
DumpVisitor::
visitValueBox(ValueBox* b)
{
  printf("valuetype %s ", b->identifier());

  if (b->constrType()) { 
    assert(b->boxedType()->kind() == IdlType::tk_struct ||
	   b->boxedType()->kind() == IdlType::tk_union  ||
	   b->boxedType()->kind() == IdlType::tk_enum);

    ((DeclaredType*)b->boxedType())->decl()->accept(*this);
  }
  else
    b->boxedType()->accept(*this);
}

void
DumpVisitor::
visitValueAbs(ValueAbs* v)
{
  printf("abstract valuetype %s ", v->identifier());

  if (v->inherits()) {
    printf(": ");
    char* ssn;
    for (ValueInheritSpec* vis = v->inherits(); vis; vis = vis->next()) {
      ssn = vis->value()->scopedName()->toString();
      printf("%s%s%s ",
	     vis->truncatable() ? "truncatable " : "",
	     ssn, vis->next() ? "," : "");
      delete [] ssn;
    }
  }
  if (v->supports()) {
    printf("supports ");
    char* ssn;
    for (InheritSpec* is = v->supports(); is; is = is->next()) {
      ssn = is->interface()->scopedName()->toString();
      printf("%s%s ", ssn, is->next() ? "," : "");
      delete [] ssn;
    }
  }
  printf("{\n");

  ++indent_;
  for (Decl* d = v->contents(); d; d = d->next()) {
    printIndent();
    d->accept(*this);
    printf(";\n");
  }
  --indent_;
  printIndent();
  printf("}");
}

void
DumpVisitor::
visitValue(Value* v)
{
  if (v->custom()) printf("custom ");
  printf("valuetype %s ", v->identifier());

  if (v->inherits()) {
    printf(": ");
    char* ssn;
    for (ValueInheritSpec* vis = v->inherits(); vis; vis = vis->next()) {
      ssn = vis->value()->scopedName()->toString();
      printf("%s%s%s ",
	     vis->truncatable() ? "truncatable " : "",
	     ssn, vis->next() ? "," : "");
      delete [] ssn;
    }
  }
  if (v->supports()) {
    printf("supports ");
    char* ssn;
    for (InheritSpec* is = v->supports(); is; is = is->next()) {
      ssn = is->interface()->scopedName()->toString();
      printf("%s%s ", ssn, is->next() ? "," : "");
      delete [] ssn;
    }
  }
  printf("{\n");

  ++indent_;
  for (Decl* d = v->contents(); d; d = d->next()) {
    printIndent();
    d->accept(*this);
    printf(";\n");
  }
  --indent_;
  printIndent();
  printf("}");
}


// Types

void
DumpVisitor::
visitBaseType(BaseType* t)
{
  printf("%s", t->kindAsString());
}

void
DumpVisitor::
visitStringType(StringType* t)
{
  if (t->bound())
    printf("string<%ld>", (long)t->bound());
  else
    printf("string");
}

void
DumpVisitor::
visitWStringType(WStringType* t)
{
  if (t->bound())
    printf("wstring<%ld>", (long)t->bound());
  else
    printf("wstring");
}

void
DumpVisitor::
visitSequenceType(SequenceType* t)
{
  printf("sequence<");
  t->seqType()->accept(*this);

  if (t->bound())
    printf(", %ld>", (long)t->bound());
  else
    printf(">");
}

void
DumpVisitor::
visitFixedType(FixedType* t)
{
  if (t->digits() > 0)
    printf("fixed<%hu,%hd>", t->digits(), t->scale());
  else
    printf("fixed");
}

#define DTCASE(tk, dt, dk) \
  case IdlType::tk: \
    { \
      dt* d = (dt*)t->decl(); \
      assert(d->kind() == Decl::dk); \
      printScopedName(d->scopedName()); \
      break; \
    }

void
DumpVisitor::
visitDeclaredType(DeclaredType* t)
{
  switch(t->kind()) {
  case IdlType::tk_objref:
  case IdlType::tk_abstract_interface:
  case IdlType::tk_local_interface:
    {
      if (t->decl()) {
	if (t->decl()->kind() == Decl::D_INTERFACE) {
	  Interface* i = (Interface*)t->decl();
	  printScopedName(i->scopedName());
	}
	else {
	  assert(t->decl()->kind() == Decl::D_FORWARD);
	  Forward* f = (Forward*)t->decl();
	  printScopedName(f->scopedName());
	}
      }
      else
	printf("Object");
      break;
    }
  case IdlType::tk_value:
    {
      if (t->decl()) {
	if (t->decl()->kind() == Decl::D_VALUE) {
	  Value* v = (Value*)t->decl();
	  printScopedName(v->scopedName());
	}
	else {
	  assert(t->decl()->kind() == Decl::D_VALUEFORWARD);
	  ValueForward* f = (ValueForward*)t->decl();
	  printScopedName(f->scopedName());
	}
      }
      else
	printf("Object");
      break;
    }
  DTCASE(tk_struct,        Struct,            D_STRUCT)
  DTCASE(ot_structforward, StructForward,     D_STRUCTFORWARD)
  DTCASE(tk_union,  	   Union,      	      D_UNION)
  DTCASE(ot_unionforward,  UnionForward,      D_UNIONFORWARD)
  DTCASE(tk_enum,   	   Enum,       	      D_ENUM)
  DTCASE(tk_alias,  	   Declarator, 	      D_DECLARATOR)
  DTCASE(tk_native, 	   Native,     	      D_NATIVE)
  DTCASE(tk_value_box,     ValueBox,          D_VALUEBOX)
  default:
    printf("%s", t->kindAsString());
  }
}
