// -*- c++ -*-
//                          Package   : omniidl
// idlvalidate.cc           Created on: 1999/10/26
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2006 Apasphere Ltd
//    Copyright (C) 1999 AT&T Laboratories Cambridge
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
//   Visitor object to validate the tree

#include <idlvalidate.h>
#include <idlerr.h>
#include <idlast.h>
#include <idlconfig.h>

#include <string.h>

void
AstValidateVisitor::
visitAST(AST* a)
{
  for (Decl* d = a->declarations(); d; d = d->next())
    d->accept(*this);
}

void
AstValidateVisitor::
visitModule(Module* m)
{
  for (Decl* d = m->definitions(); d; d = d->next())
    d->accept(*this);
}

void
AstValidateVisitor::
visitInterface(Interface* i)
{
  for (Decl* d = i->contents(); d; d = d->next())
    d->accept(*this);
}

void
AstValidateVisitor::
visitForward(Forward* f)
{
  if (Config::forwardWarning) {
    if (f->isFirst() && !f->definition() &&
        strcmp(f->scopedName()->scopeList()->identifier(), "CORBA")) {

      char* ssn = f->scopedName()->toString();
      IdlWarning(f->file(), f->line(),
		 "Forward declared interface '%s' was never fully defined",
		 ssn);
      delete [] ssn;
    }
  }
}

void
AstValidateVisitor::
visitValueForward(ValueForward* f)
{
  if (Config::forwardWarning) {
    if (f->isFirst() && !f->definition()) {
      char* ssn = f->scopedName()->toString();
      IdlWarning(f->file(), f->line(),
		 "Forward declared valuetype '%s' was never fully defined",
		 ssn);
      delete [] ssn;
    }
  }
}

void
AstValidateVisitor::
visitStructForward(StructForward* f)
{
  if (f->isFirst() && !f->definition()) {
    char* ssn = f->scopedName()->toString();
    IdlError(f->file(), f->line(),
	     "Forward declared struct '%s' was never fully defined", ssn);
    delete [] ssn;
  }
}

void
AstValidateVisitor::
visitUnionForward(UnionForward* f)
{
  if (f->isFirst() && !f->definition()) {
    char* ssn = f->scopedName()->toString();
    IdlError(f->file(), f->line(),
	     "Forward declared union '%s' was never fully defined", ssn);
    delete [] ssn;
  }
}
