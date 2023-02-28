// -*- c++ -*-
//                          Package   : omniidl
// idlvalidate.h            Created on: 1999/10/26
//			    Author    : Duncan Grisby (dpg1)
//
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

#ifndef _idlvalidate_h_
#define _idlvalidate_h_

#include <idlvisitor.h>

// AstValidateVisitor currently just issues warnings if interfaces or
// values are forward declared but never fully declared

class AstValidateVisitor : public AstVisitor {
public:
  AstValidateVisitor() {}
  virtual ~AstValidateVisitor() {}

  void visitAST              (AST*           a);
  void visitModule           (Module*        m);
  void visitInterface        (Interface*     i);
  void visitForward          (Forward*       f);
  void visitValueForward     (ValueForward*  f);
  void visitStructForward    (StructForward* f);
  void visitUnionForward     (UnionForward*  f);
};




#endif // _idlvalidate_h_
