// -*- c++ -*-
//                          Package   : omniidl
// idlast.h                 Created on: 1999/10/07
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2008 Apasphere Ltd
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
//   Definitions for abstract syntax tree classes

#ifndef _idlast_h_
#define _idlast_h_

#include <idlutil.h>
#include <idltype.h>
#include <idlexpr.h>
#include <idlscope.h>
#include <idlvisitor.h>

#include <stdio.h>

extern "C" int yyparse();
class Decl;

// Pragma class stores a list of pragmas:
class Pragma {
public:
  Pragma(const char* pragmaText, const char* file, int line)
    : pragmaText_(idl_strdup(pragmaText)),
      file_(idl_strdup(file)), line_(line), next_(0) {}

  ~Pragma() {
    delete [] pragmaText_;
    delete [] file_;
    if (next_) delete next_;
  }

  const char* pragmaText() const { return pragmaText_; }
  const char* file()       const { return file_; }
  int         line()       const { return line_; }
  Pragma*     next()       const { return next_; }

  static void add(const char* pragmaText, const char* file, int line);

private:
  char*   pragmaText_;
  char*   file_;
  int     line_;
  Pragma* next_;

  friend class AST;
  friend class Decl;
};

// Comment class stores a list of comment strings:
class Comment {
public:
  Comment(const char* commentText, const char* file, int line)
    : commentText_(idl_strdup(commentText)),
      file_(idl_strdup(file)), line_(line), next_(0) {
    mostRecent_ = this;
  }

  ~Comment() {
    delete [] commentText_;
    delete [] file_;
    if (next_) delete next_;
  }

  const char* commentText() const { return commentText_; }
  const char* file()        const { return file_; }
  int         line()        const { return line_; }
  Comment*    next()        const { return next_; }

  static void add   (const char* commentText, const char* file, int line);
  static void append(const char* commentText);
  static void clear() { mostRecent_ = 0; }

  static Comment* grabSaved();
  // Return any saved comments, and clear the saved comment list

private:
  char*           commentText_;
  char*           file_;
  int             line_;
  Comment*        next_;
  static Comment* mostRecent_;
  static Comment* saved_;

  friend class AST;
  friend class Decl;
};



// AST class represents the whole IDL definition
class AST {
public:
  AST();
  ~AST();
  static AST*        tree();
  static IDL_Boolean process(FILE* f, const char* name);
  static void        clear();

  Decl*       declarations()              { return declarations_; }
  const char* file()                      { return file_; }
  Pragma*     pragmas()                   { return pragmas_; }
  Comment*    comments()                  { return comments_; }

  void        accept(AstVisitor& visitor) { visitor.visitAST(this); }

  void        setFile(const char* f);
  void        addPragma(const char* pragmaText, const char* file, int line);
  void        addComment(const char* commentText, const char* file, int line);

private:
  void        setDeclarations(Decl* d);

  Decl*       declarations_;
  char*       file_;
  static AST* tree_;
  Pragma*     pragmas_;
  Pragma*     lastPragma_;
  Comment*    comments_;
  Comment*    lastComment_;
  friend int  yyparse();
};


// Base declaration abstract class
class Decl {
public:
  // Declaration kinds
  enum Kind {
    D_MODULE, D_INTERFACE, D_FORWARD, D_CONST, D_DECLARATOR,
    D_TYPEDEF, D_MEMBER, D_STRUCT, D_STRUCTFORWARD, D_EXCEPTION,
    D_CASELABEL, D_UNIONCASE, D_UNION, D_UNIONFORWARD,
    D_ENUMERATOR, D_ENUM, D_ATTRIBUTE, D_PARAMETER, D_OPERATION,
    D_NATIVE, D_STATEMEMBER, D_FACTORY, D_VALUEFORWARD, D_VALUEBOX,
    D_VALUEABS, D_VALUE
  };

  Decl(Kind kind, const char* file, int line, IDL_Boolean mainFile);
  virtual ~Decl();

  // Declaration kind
  Kind                kind()         const { return kind_; }
  virtual const char* kindAsString() const = 0;

  // Query interface
  const char*       file()       const { return file_; }
  int               line()       const { return line_; }
  IDL_Boolean       mainFile()   const { return mainFile_; }
  const Scope*      inScope()    const { return inScope_; }
  const Pragma*     pragmas()    const { return pragmas_; }
  const Comment*    comments()   const { return comments_; }

  // Linked list
  Decl* next() { return next_; }

  void append(Decl* d) {
    if (d) {
      last_->next_ = d;
      last_        = d;
    }
  }

  // Find a decl given a name. Does not mark the name used.
  static Decl* scopedNameToDecl(const char* file, int line,
				const ScopedName* sn);

  static Decl* mostRecent() { return mostRecent_; }
  static void  clear()      { mostRecent_ = 0; }

  // Visitor pattern accept(). The visitor is responsible for
  // recursively visiting children if it needs to
  virtual void accept(AstVisitor& visitor) = 0;

  void addPragma(const char* pragmaText, const char* file, int line);
  void addComment(const char* commentText, const char* file, int line);

private:
  Kind              kind_;
  char*             file_;
  int               line_;
  IDL_Boolean       mainFile_;
  const Scope*      inScope_;
  Pragma*           pragmas_;
  Pragma*           lastPragma_;
  Comment*          comments_;
  Comment*          lastComment_;

protected:
  static Decl*      mostRecent_;

  Decl* next_;
  Decl* last_;
};


// Mixin class for Decls with a RepoId
class DeclRepoId {
public:
  DeclRepoId(const char* identifier);
  ~DeclRepoId();

  // eidentifier() returns the identifier with _ escapes intact
  const char*       identifier()  const { return identifier_; }
  const char*       eidentifier() const { return eidentifier_; }
  const ScopedName* scopedName()  const { return scopedName_; }
  const char*       repoId()      const { return repoId_; }
  const char*       prefix()      const { return prefix_; }

  void setRepoId(const char* repoId, const char* file, int line);
  void setVersion(IDL_Short maj, IDL_Short min,
		  const char* file, int line);

  // Static set functions taking a Decl as an argument
  static void setRepoId(Decl* d, const char* repoId,
			const char* file, int line);
  static void setVersion(Decl* d, IDL_Short maj, IDL_Short min,
			 const char* file, int line);

  IDL_Boolean repoIdSet() const { return set_; }
  const char* rifile()    const { return rifile_; }
  int         riline()    const { return riline_; }
  IDL_Short   rimaj()     const { return maj_; }
  IDL_Short   rimin()     const { return min_; }

private:
  void genRepoId();

  char*          identifier_;
  char*          eidentifier_;
  ScopedName*    scopedName_;
  char*          repoId_;
  char*          prefix_; // Prefix in force at time of declaration
  IDL_Boolean    set_;    // True if repoId or version has been manually set
  char*          rifile_; // File where repoId or version was set
  int            riline_; // Line where repoId or version was set
  IDL_Short      maj_;
  IDL_Short      min_;
};


// Module
class Module : public Decl, public DeclRepoId {
public:
  Module(const char* file, int line, IDL_Boolean mainFile,
	 const char* identifier);

  virtual ~Module();

  const char* kindAsString() const { return "module"; }

  // Query interface
  Decl* definitions() const { return definitions_; }
  
  void accept(AstVisitor& visitor) { visitor.visitModule(this); }

  void finishConstruction(Decl* defs);

private:
  Decl*  definitions_;
};


// List of inherited interfaces
class InheritSpec {
public:
  InheritSpec(const ScopedName* sn, const char* file, int line);

  ~InheritSpec() { if (next_) delete next_; }

  // The ScopedName used in an inheritance specification may be a
  // typedef. In that case, decl() returns the Typedef declarator
  // object and interface() returns the actual Interface object.
  // Otherwise, both functions return the same Interface pointer.

  Interface*   interface() const { return interface_; }
  Decl*        decl()      const { return decl_; }
  const Scope* scope()     const { return scope_; }
  InheritSpec* next()      const { return next_; }

  void append(InheritSpec* is, const char* file, int line);

private:
  Interface*   interface_;
  Decl*        decl_;
  const Scope* scope_;

protected:
  InheritSpec* next_;
};



// Interface
class Interface : public Decl, public DeclRepoId {
public:
  Interface(const char* file, int line, IDL_Boolean mainFile,
	    const char* identifier, IDL_Boolean abstract,
	    IDL_Boolean local, InheritSpec* inherits);

  virtual ~Interface();

  const char* kindAsString() const { return "interface"; }

  // Queries
  IDL_Boolean    abstract() const { return abstract_; }
  IDL_Boolean    local()    const { return local_;    }
  InheritSpec*   inherits() const { return inherits_; }
  Decl*          contents() const { return contents_; }
  Scope*         scope()    const { return scope_;    }
  IdlType*       thisType() const { return thisType_; }

  IDL_Boolean    isDerived(const Interface* base) const;

  void accept(AstVisitor& visitor) { visitor.visitInterface(this); }

  void finishConstruction(Decl* decls);

private:
  IDL_Boolean    abstract_;
  IDL_Boolean    local_;
  InheritSpec*   inherits_;
  Decl*          contents_;
  Scope*         scope_;
  IdlType*       thisType_;
};


// Forward-declared interface
class Forward : public Decl, public DeclRepoId {
public:
  Forward(const char* file, int line, IDL_Boolean mainFile,
	  const char* identifier, IDL_Boolean abstract, IDL_Boolean local);

  virtual ~Forward();

  const char* kindAsString() const { return "forward interface"; }

  // Query interface
  IDL_Boolean    abstract()   const { return abstract_; }
  IDL_Boolean    local()      const { return local_;    }
  Interface*     definition() const;
  IDL_Boolean    isFirst()    const { return !firstForward_; }
  IdlType*       thisType()   const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitForward(this); }

  void setDefinition(Interface* defn);

private:
  IDL_Boolean    abstract_;
  IDL_Boolean    local_;
  Interface*     definition_;
  Forward*       firstForward_;
  IdlType*       thisType_;
};


// Constant
class Const : public Decl, public DeclRepoId {
public:
  Const(const char* file, int line, IDL_Boolean mainFile,
	IdlType* constType, const char* identifier, IdlExpr* expr);

  virtual ~Const();

  const char* kindAsString() const { return "constant"; }

  // Queries
  IdlType*      constType() const { return constType_; }
  IdlType::Kind constKind() const { return constKind_; }

  IDL_Short        constAsShort()      const;
  IDL_Long         constAsLong()       const;
  IDL_UShort       constAsUShort()     const;
  IDL_ULong        constAsULong()      const;
  IDL_Float        constAsFloat()      const;
  IDL_Double       constAsDouble()     const;
  IDL_Boolean      constAsBoolean()    const;
  IDL_Char         constAsChar()       const;
  IDL_Octet        constAsOctet()      const;
  const char*      constAsString()     const;
#ifdef HAS_LongLong
  IDL_LongLong     constAsLongLong()   const;
  IDL_ULongLong    constAsULongLong()  const;
#endif
#ifdef HAS_LongDouble
  IDL_LongDouble   constAsLongDouble() const;
#endif
  IDL_WChar        constAsWChar()      const;
  const IDL_WChar* constAsWString()    const;
  IDL_Fixed*       constAsFixed()      const;
  Enumerator*      constAsEnumerator() const;

  void accept(AstVisitor& visitor) { visitor.visitConst(this); }

private:
  IdlType*       constType_;
  IDL_Boolean    delType_;
  IdlType::Kind  constKind_;
  union {
    IDL_Short        short_;
    IDL_Long         long_;
    IDL_UShort       ushort_;
    IDL_ULong        ulong_;
#ifndef __VMS
    IDL_Float        float_;
    IDL_Double       double_;
#else
    float            float_;
    double           double_;
#endif
    IDL_Boolean      boolean_;
    IDL_Char         char_;
    IDL_Octet        octet_;
    char*            string_;
#ifdef HAS_LongLong
    IDL_LongLong     longlong_;
    IDL_ULongLong    ulonglong_;
#endif
#ifdef HAS_LongDouble
    IDL_LongDouble   longdouble_;
#endif
    IDL_WChar        wchar_;
    IDL_WChar*       wstring_;
    IDL_Fixed*       fixed_;
    Enumerator*      enumerator_;
  } v_;
};


// Typedef

class ArraySize {
public:
  ArraySize(int size) : size_(size), next_(0), last_(0) {}

  ~ArraySize() { if (next_) delete next_; }

  int size()        const { return size_; }
  ArraySize* next() const { return next_; }

  void append(ArraySize* as) {
    if (last_) last_->next_ = as;
    else       next_ = as;
    last_ = as;
  }

private:
  int size_;

protected:
  ArraySize* next_;
  ArraySize* last_;
};


class Typedef;
class Attribute;

class Declarator : public Decl, public DeclRepoId {
public:
  Declarator(const char* file, int line, IDL_Boolean mainFile,
	     const char* identifier, ArraySize* sizes);

  virtual ~Declarator();

  const char* kindAsString() const;

  // Queries
  ArraySize* sizes()    const { return sizes_; }
				// Null if a simple declarator

  // Only for typedef declarators
  IdlType*   thisType()  const { return thisType_; }
  Typedef*   alias()     const { return alias_; } 
  Attribute* attribute() const { return attribute_; }

  void accept(AstVisitor& visitor) { visitor.visitDeclarator(this); }

  void setAlias    (Typedef*   td);
  void setAttribute(Attribute* at);

private:
  ArraySize*  sizes_;
  IdlType*    thisType_;
  Typedef*    alias_;
  Attribute*  attribute_;
};


class Typedef : public Decl {
public:
  Typedef(const char* file, int line, IDL_Boolean mainFile,
	  IdlType* aliasType, IDL_Boolean constrType,
	  Declarator* declarators);

  virtual ~Typedef();

  const char* kindAsString() const { return "typedef"; }

  // Queries
  IdlType*       aliasType()   const { return aliasType_; }
  IDL_Boolean    constrType()  const { return constrType_; }
  Declarator*    declarators() const { return declarators_; }

  void accept(AstVisitor& visitor) { visitor.visitTypedef(this); }

private:
  IdlType*       aliasType_;
  IDL_Boolean    delType_;
  IDL_Boolean    constrType_;
  Declarator*    declarators_;
};


// Struct member
class Member : public Decl {
public:
  Member(const char* file, int line, IDL_Boolean mainFile,
	 IdlType* memberType, IDL_Boolean constrType,
	 Declarator* declarators);
  virtual ~Member();

  const char* kindAsString() const { return "member"; }

  // Queries
  IdlType*       memberType()  const { return memberType_; }
  IDL_Boolean    constrType()  const { return constrType_; }
  Declarator*    declarators() const { return declarators_; }

  void accept(AstVisitor& visitor) { visitor.visitMember(this); }

private:
  IdlType*       memberType_;
  IDL_Boolean    delType_;
  IDL_Boolean    constrType_;
  Declarator*    declarators_;
};



// Struct
class Struct : public Decl, public DeclRepoId {
public:
  Struct(const char* file, int line, IDL_Boolean mainFile,
	 const char* identifier);
  virtual ~Struct();

  const char* kindAsString() const { return "struct"; }

  // Queries
  Member*        members()   const { return members_; }
  IdlType*       thisType()  const { return thisType_; }
  IDL_Boolean    recursive() const { return recursive_; }
  IDL_Boolean    finished()  const { return finished_; }

  void accept(AstVisitor& visitor) { visitor.visitStruct(this); }

  void finishConstruction(Member* members);

  void setRecursive() { recursive_ = 1; }

private:
  Member*        members_;
  IdlType*       thisType_;
  IDL_Boolean    recursive_;
  IDL_Boolean    finished_;
};

// Forward-declared struct
class StructForward : public Decl, public DeclRepoId {
public:
  StructForward(const char* file, int line, IDL_Boolean mainFile,
		const char* identifier);
  virtual ~StructForward();

  const char* kindAsString()  const { return "forward struct"; }

  // Queries
  Struct*        definition() const;
  IDL_Boolean    isFirst()    const { return !firstForward_; }
  IdlType*       thisType()   const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitStructForward(this); }

  void setDefinition(Struct* defn);

private:
  Struct*        definition_;
  StructForward* firstForward_;
  IdlType*       thisType_;
};


// Exception
class Exception : public Decl, public DeclRepoId {
public:
  Exception(const char* file, int line, IDL_Boolean mainFile,
	    const char* identifier);
  virtual ~Exception();

  const char* kindAsString() const { return "exception"; }

  // Queries
  Member*        members()  const { return members_; }
  IDL_Boolean    local()    const { return local_; }

  void accept(AstVisitor& visitor) { visitor.visitException(this); }

  void finishConstruction(Member* members);

private:
  Member*        members_;
  IDL_Boolean    local_;
};


// Union case label
class CaseLabel : public Decl {
public:
  CaseLabel(const char* file, int line, IDL_Boolean mainFile,
	    IdlExpr* value);
  virtual ~CaseLabel();

  const char* kindAsString() const { return "case label"; }

  IDL_Short        labelAsShort()      const;
  IDL_Long         labelAsLong()       const;
  IDL_UShort       labelAsUShort()     const;
  IDL_ULong        labelAsULong()      const;
  IDL_Boolean      labelAsBoolean()    const;
  IDL_Char         labelAsChar()       const;
#ifdef HAS_LongLong
  IDL_LongLong     labelAsLongLong()   const;
  IDL_ULongLong    labelAsULongLong()  const;
#endif
  IDL_WChar        labelAsWChar()      const;
  Enumerator*      labelAsEnumerator() const;

  inline IDL_Boolean isDefault() const { return isDefault_; }
  IdlType::Kind      labelKind() const { return labelKind_; }

  void accept(AstVisitor& visitor) { visitor.visitCaseLabel(this); }

  void setType(IdlType* type);
  void setDefaultShort     (IDL_Short     v) { v_.short_      = v; }
  void setDefaultLong      (IDL_Long      v) { v_.long_       = v; }
  void setDefaultUShort    (IDL_UShort    v) { v_.ushort_     = v; }
  void setDefaultULong     (IDL_ULong     v) { v_.ulong_      = v; }
  void setDefaultBoolean   (IDL_Boolean   v) { v_.boolean_    = v; }
  void setDefaultChar      (IDL_Char      v) { v_.char_       = v; }
#ifdef HAS_LongLong
  void setDefaultLongLong  (IDL_LongLong  v) { v_.longlong_   = v; }
  void setDefaultULongLong (IDL_ULongLong v) { v_.ulonglong_  = v; }
#endif
  void setDefaultWChar     (IDL_WChar     v) { v_.wchar_      = v; }
  void setDefaultEnumerator(Enumerator*   v) { v_.enumerator_ = v; }

private:
  IdlExpr*       value_;
  IDL_Boolean isDefault_;
  IdlType::Kind  labelKind_;
  union {
    IDL_Short        short_;
    IDL_Long         long_;
    IDL_UShort       ushort_;
    IDL_ULong        ulong_;
    IDL_Boolean      boolean_;
    IDL_Char         char_;
#ifdef HAS_LongLong
    IDL_LongLong     longlong_;
    IDL_ULongLong    ulonglong_;
#endif
    IDL_WChar        wchar_;
    Enumerator*      enumerator_;
  } v_;
};


// Union case
class UnionCase : public Decl {
public:
  UnionCase(const char* file, int line, IDL_Boolean mainFile,
	    IdlType* caseType, IDL_Boolean constrType,
	    Declarator* declarator);
  virtual ~UnionCase();

  const char* kindAsString() const { return "case"; }

  // Queries
  CaseLabel*     labels()     const { return labels_; }
  IdlType*       caseType()   const { return caseType_; }
  IDL_Boolean    constrType() const { return constrType_; }
  Declarator*    declarator() const { return declarator_; }

  void accept(AstVisitor& visitor) { visitor.visitUnionCase(this); }

  void finishConstruction(CaseLabel* labels);

private:
  CaseLabel*     labels_;
  IdlType*       caseType_;
  IDL_Boolean    delType_;
  IDL_Boolean    constrType_;
  Declarator*    declarator_;
};


// Union
class Union : public Decl, public DeclRepoId {
public:
  Union(const char* file, int line, IDL_Boolean mainFile,
	const char* identifier);
  virtual ~Union();

  const char* kindAsString() const { return "union"; }

  // Queries
  IdlType*       switchType() const { return switchType_; }
  IDL_Boolean    constrType() const { return constrType_; }
  UnionCase*     cases()      const { return cases_; }
  IdlType*       thisType()   const { return thisType_; }
  IDL_Boolean    recursive()  const { return recursive_; }
  IDL_Boolean    finished()   const { return finished_; }

  void accept(AstVisitor& visitor) { visitor.visitUnion(this); }

  void finishConstruction(IdlType* switchType, IDL_Boolean constrType,
			  UnionCase* cases);
  void setRecursive() { recursive_ = 1; }

private:
  IdlType*       switchType_;
  IDL_Boolean    constrType_;
  UnionCase*     cases_;
  IdlType*       thisType_;
  IDL_Boolean    recursive_;
  IDL_Boolean    finished_;
};

class UnionForward : public Decl, public DeclRepoId {
public:
  UnionForward(const char* file, int line, IDL_Boolean mainFile,
	       const char* identifier);
  virtual ~UnionForward();

  const char* kindAsString() const { return "forward union"; }

  // Queries
  Union*      definition() const;
  IDL_Boolean isFirst()    const { return !firstForward_; }
  IdlType*    thisType()   const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitUnionForward(this); }

  void setDefinition(Union* defn);

private:
  Union*        definition_;
  UnionForward* firstForward_;
  IdlType*      thisType_;
};


// Enumerator
class Enum;

class Enumerator : public Decl, public DeclRepoId {
public:
  Enumerator(const char* file, int line, IDL_Boolean mainFile,
	     const char* identifier);
  virtual ~Enumerator();

  const char* kindAsString() const { return "enumerator"; }

  // Queries
  Enum*       container()  const { return container_; }
  IDL_ULong   value()      const { return value_; }

  void accept(AstVisitor& visitor) { visitor.visitEnumerator(this); }

  void finishConstruction(Enum* container, IDL_ULong value);

private:
  const char* identifier_;
  Enum*       container_;
  IDL_ULong   value_;
};


// Enum
class Enum : public Decl, public DeclRepoId {
public:
  Enum(const char* file, int line, IDL_Boolean mainFile,
       const char* identifier);
  virtual ~Enum();

  const char* kindAsString() const { return "enum"; }

  // Queries
  Enumerator* enumerators() const { return enumerators_; }
  IdlType*    thisType()    const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitEnum(this); }

  void finishConstruction(Enumerator* enumerators);

private:
  Enumerator* enumerators_;
  IdlType*    thisType_;
};



// Attribute
class Attribute : public Decl {
public:
  Attribute(const char* file, int line, IDL_Boolean mainFile,
	    IDL_Boolean readonly, IdlType* attrType,
	    Declarator* declarators);
  virtual ~Attribute();

  const char* kindAsString() const { return "attribute"; }

  // Queries
  IDL_Boolean    readonly()    const { return readonly_; }
  IdlType*       attrType()    const { return attrType_; }
  Declarator*    declarators() const { return declarators_; }

  void accept(AstVisitor& visitor) { visitor.visitAttribute(this); }

private:
  IDL_Boolean    readonly_;
  IdlType*       attrType_;
  IDL_Boolean    delType_;
  Declarator*    declarators_;
};


// Parameter
class Parameter : public Decl {
public:
  Parameter(const char* file, int line, IDL_Boolean mainFile,
	    int direction, IdlType* paramType, const char* identifier);
  virtual ~Parameter();

  const char* kindAsString() const { return "parameter"; }

  // Queries
  int         direction()  const { return direction_; }
				// 0: in, 1: out, 2: inout
  IdlType*    paramType()  const { return paramType_; }
  const char* identifier() const { return identifier_; }

  void accept(AstVisitor& visitor) { visitor.visitParameter(this); }

private:
  int            direction_;
  IdlType*       paramType_;
  IDL_Boolean    delType_;
  char*          identifier_;
};


// List of exceptions
class RaisesSpec {
public:
  RaisesSpec(const ScopedName* sn, const char* file, int line);
  ~RaisesSpec();

  Exception*  exception() const { return exception_; }
  RaisesSpec* next()      const { return next_; }

  void append(RaisesSpec* rs) {
    if (rs) {
      last_->next_ = rs;
      last_ = rs;
    }
  }
private:
  Exception*  exception_;

protected:
  RaisesSpec* next_;
  RaisesSpec* last_;
};

// List of contexts
class ContextSpec {
public:
  ContextSpec(const char* c, const char* file, int line);
  ~ContextSpec();

  const char*  context() const { return context_; }
  ContextSpec* next()    const { return next_; }

  void append(ContextSpec* rs) {
    last_->next_ = rs;
    last_ = rs;
  }

private:
  char*  context_;

protected:
  ContextSpec* next_;
  ContextSpec* last_;
};


// Operation
class Operation : public Decl, public DeclRepoId {
public:
  Operation(const char* file, int line, IDL_Boolean mainFile,
	    IDL_Boolean oneway, IdlType* return_type,
	    const char* identifier);
  virtual ~Operation();

  const char* kindAsString() const { return "operation"; }

  // Queries
  IDL_Boolean    oneway()     const { return oneway_; }
  IdlType*       returnType() const { return returnType_; }
  Parameter*     parameters() const { return parameters_; }
  RaisesSpec*    raises()     const { return raises_; }
  ContextSpec*   contexts()   const { return contexts_; }

  void accept(AstVisitor& visitor) { visitor.visitOperation(this); }

  void closeParens();
  void finishConstruction(Parameter* parameters, RaisesSpec* raises,
			  ContextSpec* contexts);

private:
  IDL_Boolean    oneway_;
  IdlType*       returnType_;
  IDL_Boolean    delType_;
  Parameter*     parameters_;
  RaisesSpec*    raises_;
  ContextSpec*   contexts_;
};


// Native
class Native : public Decl, public DeclRepoId {
public:
  Native(const char* file, int line, IDL_Boolean mainFile,
	 const char* identifier, IdlType* type=0);
  virtual ~Native();

  const char* kindAsString() const { return "native"; }

  void accept(AstVisitor& visitor) { visitor.visitNative(this); }
};


// Things for valuetype

class StateMember : public Decl {
public:
  StateMember(const char* file, int line, IDL_Boolean mainFile,
	      int memberAccess, IdlType* memberType,
	      IDL_Boolean constrType, Declarator* declarators);
  virtual ~StateMember();

  const char* kindAsString() const { return "state member"; }

  // Queries
  int            memberAccess() const { return memberAccess_; }
				// 0: public, 1: private
  IdlType*       memberType()   const { return memberType_; }
  IDL_Boolean    constrType()   const { return constrType_; }
  Declarator*    declarators()  const { return declarators_; }

  void accept(AstVisitor& visitor) { visitor.visitStateMember(this); }

private:
  int            memberAccess_;
  IdlType*       memberType_;
  IDL_Boolean    delType_;
  IDL_Boolean    constrType_;
  Declarator*    declarators_;
};

class Factory : public Decl {
public:
  Factory(const char* file, int line, IDL_Boolean mainFile,
	  const char* identifier);
  ~Factory();

  const char* kindAsString() const { return "initializer"; }

  // Queries
  const char* identifier() const { return identifier_; }
  Parameter*  parameters() const { return parameters_; }
  RaisesSpec* raises()     const { return raises_; }

  void accept(AstVisitor& visitor) { visitor.visitFactory(this); }

  void closeParens();
  void finishConstruction(Parameter* parameters, RaisesSpec* raises);

private:
  char*       identifier_;
  Parameter*  parameters_;
  RaisesSpec* raises_;
};


// Base class for all the multifarious value types
class ValueBase : public Decl, public DeclRepoId {
public:
  ValueBase(Decl::Kind k, const char* file, int line, IDL_Boolean mainFile,
	    const char* identifier);
  virtual ~ValueBase();
};


// Forward declared value
class ValueForward : public ValueBase {
public:
  ValueForward(const char* file, int line, IDL_Boolean mainFile,
	       IDL_Boolean abstract, const char* identifier);
  virtual ~ValueForward();

  const char* kindAsString() const { return "forward value"; }

  // Queries
  IDL_Boolean    abstract()   const { return abstract_; }
  ValueBase*     definition() const;
  IDL_Boolean    isFirst()    const { return !firstForward_; }
  IdlType*       thisType()   const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitValueForward(this); }

  void setDefinition(ValueBase* defn);

private:
  IDL_Boolean    abstract_;
  ValueBase*     definition_;
  ValueForward*  firstForward_;
  IdlType*       thisType_;
};


class ValueBox : public ValueBase {
public:
  ValueBox(const char* file, int line, IDL_Boolean mainFile,
	   const char* identifier, IdlType* boxedType,
	   IDL_Boolean constrType);
  virtual ~ValueBox();

  const char* kindAsString() const { return "value box"; }

  // Queries
  IdlType*       boxedType()  const { return boxedType_; }
  IDL_Boolean    constrType() const { return constrType_; }
  IdlType*       thisType()   const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitValueBox(this); }

private:
  IdlType*       boxedType_;
  IDL_Boolean    constrType_;
  IDL_Boolean    delType_;
  IdlType*       thisType_;
};


class ValueInheritSpec {
public:
  ValueInheritSpec(ScopedName* sn, const char* file, int line);

  virtual ~ValueInheritSpec() { if (next_) delete next_; }

  ValueBase*        value()       const { return value_; }
  Decl*             decl()        const { return decl_; }
  const Scope*      scope()       const { return scope_; }
  ValueInheritSpec* next()        const { return next_; }
  IDL_Boolean       truncatable() const { return truncatable_; }

  void append(ValueInheritSpec* is, const char* file, int line);
  void setTruncatable() { truncatable_ = 1; }

private:
  ValueBase*   value_;
  Decl*        decl_;
  const Scope* scope_;

protected:
  ValueInheritSpec* next_;
  IDL_Boolean       truncatable_;
};


class ValueInheritSupportSpec {
public:
  ValueInheritSupportSpec(ValueInheritSpec* inherits,
			  InheritSpec*      supports) :
    inherits_(inherits), supports_(supports) {}

  ~ValueInheritSupportSpec() {}

  ValueInheritSpec* inherits() const { return inherits_; }
  InheritSpec*      supports() const { return supports_; }

private:
  ValueInheritSpec* inherits_;
  InheritSpec*      supports_;
};


class ValueAbs : public ValueBase {
public:
  ValueAbs(const char* file, int line, IDL_Boolean mainFile,
	   const char* identifier, ValueInheritSpec* inherits,
	   InheritSpec* supports);
  virtual ~ValueAbs();

  const char* kindAsString() const { return "abstract valuetype"; }

  // Queries
  ValueInheritSpec* inherits() const { return inherits_; }
  InheritSpec*      supports() const { return supports_; }
  Decl*             contents() const { return contents_; }
  Scope*            scope()    const { return scope_;    }
  IdlType*          thisType() const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitValueAbs(this); }

  void finishConstruction(Decl* contents);

private:
  ValueInheritSpec* inherits_;
  InheritSpec*      supports_;
  Decl*             contents_;
  Scope*            scope_;
  IdlType*          thisType_;
};


class Value : public ValueBase {
public:
  Value(const char* file, int line, IDL_Boolean mainFile,
	IDL_Boolean custom, const char* identifier,
	ValueInheritSpec* inherits, InheritSpec* supports);
  virtual ~Value();

  const char* kindAsString() const { return "valuetype"; }

  // Queries
  IDL_Boolean       custom()   const { return custom_; }
  ValueInheritSpec* inherits() const { return inherits_; }
  InheritSpec*      supports() const { return supports_; }
  Decl*             contents() const { return contents_; }
  Scope*            scope()    const { return scope_;    }
  IdlType*          thisType() const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitValue(this); }

  void finishConstruction(Decl* contents);

private:
  IDL_Boolean       custom_;
  ValueInheritSpec* inherits_;
  InheritSpec*      supports_;
  Decl*             contents_;
  Scope*            scope_;
  IdlType*          thisType_;
};


#endif // _idlast_h_
