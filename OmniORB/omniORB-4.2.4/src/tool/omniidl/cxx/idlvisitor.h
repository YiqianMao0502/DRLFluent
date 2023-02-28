// -*- c++ -*-
//                          Package   : omniidl
// idlvisitor.h             Created on: 1999/10/11
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
//   Visitor base class

#ifndef _idlvisitor_h_
#define _idlvisitor_h_

//
// Visitor for AST
//

class AST;
class Module;
class Interface;
class Forward;
class Const;
class ArraySize;
class TypedefDeclarator;
class Typedef;
class Declarator;
class Member;
class Struct;
class StructForward;
class Exception;
class CaseLabel;
class UnionCase;
class Union;
class UnionForward;
class Enumerator;
class Enum;
class Attribute;
class Parameter;
class Operation;
class Native;
class StateMember;
class Factory;
class ValueForward;
class ValueBox;
class ValueInheritSpec;
class ValueAbs;
class Value;


class AstVisitor {
public:
  AstVisitor() {}
  virtual ~AstVisitor() {}

  virtual void visitAST              (AST*)               { }
  virtual void visitModule           (Module*)            { }
  virtual void visitInterface        (Interface*)         { }
  virtual void visitForward          (Forward*)           { }
  virtual void visitConst            (Const*)             { }
  virtual void visitDeclarator       (Declarator*)        { }
  virtual void visitTypedef          (Typedef*)           { }
  virtual void visitMember           (Member*)            { }
  virtual void visitStruct           (Struct*)            { }
  virtual void visitStructForward    (StructForward*)     { }
  virtual void visitException        (Exception*)         { }
  virtual void visitCaseLabel        (CaseLabel*)         { }
  virtual void visitUnionCase        (UnionCase*)         { }
  virtual void visitUnion            (Union*)             { }
  virtual void visitUnionForward     (UnionForward*)      { }
  virtual void visitEnumerator       (Enumerator*)        { }
  virtual void visitEnum             (Enum*)              { }
  virtual void visitAttribute        (Attribute*)         { }
  virtual void visitParameter        (Parameter*)         { }
  virtual void visitOperation        (Operation*)         { }
  virtual void visitNative           (Native*)            { }
  virtual void visitStateMember      (StateMember*)       { }
  virtual void visitFactory          (Factory*)           { }
  virtual void visitValueForward     (ValueForward*)      { }
  virtual void visitValueBox         (ValueBox*)          { }
  virtual void visitValueAbs         (ValueAbs*)          { }
  virtual void visitValue            (Value*)             { }
};


//
// Visitor for types
//

class BaseType;
class StringType;
class WStringType;
class SequenceType;
class FixedType;
class DeclaredType;

class TypeVisitor{
public:
  TypeVisitor() {}
  virtual ~TypeVisitor() {}

  virtual void visitBaseType    (BaseType*)     { }
  virtual void visitStringType  (StringType*)   { }
  virtual void visitWStringType (WStringType*)  { }
  virtual void visitSequenceType(SequenceType*) { }
  virtual void visitFixedType   (FixedType*)    { }
  virtual void visitDeclaredType(DeclaredType*) { }
};


#endif // _idlvisitor_h_
