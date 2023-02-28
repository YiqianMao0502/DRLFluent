// -*- Mode: C++; -*-
//                            Package   : omniORB
// userexception.h            Created on: 1999
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2003-2011 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library. If not, see http://www.gnu.org/licenses/
//
//
// Description:
//

#ifndef __OMNIORB_USEREXCEPTION_H__
#define __OMNIORB_USEREXCEPTION_H__


#define OMNIORB_DECLARE_USER_EXCEPTION(name, attr)  \
  \
class name : public CORBA::UserException {  \
public:  \
  inline name() {  \
    pd_insertToAnyFn    = insertToAnyFn;  \
    pd_insertToAnyFnNCP = insertToAnyFnNCP;  \
  }  \
  inline name(const name& _ex) :  \
    OMNIORB_BASE_CTOR(CORBA::)UserException(_ex) {}  \
  inline name& operator=(const name& _ex) {  \
    * (CORBA::UserException*) this = _ex;  return *this;  \
  }  \
  virtual ~name();  \
  virtual void _raise() const;  \
  static name* _downcast(CORBA::Exception*);  \
  static const name* _downcast(const CORBA::Exception*);  \
  static inline name* _narrow(CORBA::Exception* _ex) {  \
    return _downcast(_ex);  \
  }  \
  \
  inline void operator>>=(cdrStream&) const {}  \
  inline void operator<<=(cdrStream&) {}  \
  \
  static attr insertExceptionToAny    insertToAnyFn;  \
  static attr insertExceptionToAnyNCP insertToAnyFnNCP;  \
  \
  static attr const char* _PD_repoId; \
  \
private:  \
  virtual CORBA::Exception* _NP_duplicate() const;  \
  virtual const char* _NP_typeId() const;  \
  virtual const char* _NP_repoId(int* size) const;  \
  virtual void _NP_marshal(cdrStream&) const;  \
};


#define OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(name, attr)  \
  \
class name : public UserException {  \
public:  \
  inline name() {  \
    pd_insertToAnyFn    = insertToAnyFn;  \
    pd_insertToAnyFnNCP = insertToAnyFnNCP;  \
  }  \
  inline name(const name& _ex) : UserException(_ex) {}  \
  inline name& operator=(const name& _ex) {  \
    * (UserException*) this = _ex;  return *this;  \
  }  \
  virtual ~name();  \
  virtual void _raise() const;  \
  static name* _downcast(Exception*);  \
  static const name* _downcast(const Exception*);  \
  static inline name* _narrow(Exception* _ex) {  \
    return _downcast(_ex);  \
  }  \
  \
  inline void operator>>=(cdrStream&) const {}  \
  inline void operator<<=(cdrStream&) {}  \
  \
  static attr insertExceptionToAny    insertToAnyFn;  \
  static attr insertExceptionToAnyNCP insertToAnyFnNCP;  \
  \
  static attr const char* _PD_repoId; \
  \
private:  \
  virtual Exception* _NP_duplicate() const;  \
  virtual const char* _NP_typeId() const;  \
  virtual const char* _NP_repoId(int* size) const;  \
  virtual void _NP_marshal(cdrStream&) const;  \
};


// This macro applies its argument to the name of each
// of the system exceptions.  It is expected that the
// argument <doit> will be another macro.

#define OMNIORB_FOR_EACH_SYS_EXCEPTION(doit) \
 \
doit (UNKNOWN) \
doit (BAD_PARAM) \
doit (NO_MEMORY) \
doit (IMP_LIMIT) \
doit (COMM_FAILURE) \
doit (INV_OBJREF) \
doit (NO_PERMISSION) \
doit (INTERNAL) \
doit (MARSHAL) \
doit (INITIALIZE) \
doit (NO_IMPLEMENT) \
doit (BAD_TYPECODE) \
doit (BAD_OPERATION) \
doit (NO_RESOURCES) \
doit (NO_RESPONSE) \
doit (PERSIST_STORE) \
doit (BAD_INV_ORDER) \
doit (TRANSIENT) \
doit (FREE_MEM) \
doit (INV_IDENT) \
doit (INV_FLAG) \
doit (INTF_REPOS) \
doit (BAD_CONTEXT) \
doit (OBJ_ADAPTER) \
doit (DATA_CONVERSION) \
doit (OBJECT_NOT_EXIST) \
doit (TRANSACTION_REQUIRED) \
doit (TRANSACTION_ROLLEDBACK) \
doit (INVALID_TRANSACTION) \
doit (INV_POLICY) \
doit (CODESET_INCOMPATIBLE) \
doit (REBIND) \
doit (TIMEOUT) \
doit (TRANSACTION_UNAVAILABLE) \
doit (TRANSACTION_MODE) \
doit (BAD_QOS)

#endif // __OMNIORB_USEREXCEPTION_H__
