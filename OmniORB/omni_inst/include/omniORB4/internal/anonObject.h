// -*- Mode: C++; -*-
//                            Package   : omniORB
// anonObject.h               Created on: 26/2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996, 1999 AT&T Research Cambridge
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
//    Implementation of an anonymous object (no compile-time knowledge
//    of the interface).
//      
 
#ifndef __ANONOBJECT_H__
#define __ANONOBJECT_H__

//
// An omniAnonObjRef is used as a proxyObject when no proxyObjectFactory
// class for a give interface repository ID is found.
//  Of course, one can only use such an object as a CORBA::Object_ptr and
// pass it around as the type "Object" in IDL operations and attributes.
// See also the comments in omni::createObjRef().
//

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniAnonObjRef ///////////////////////////
//////////////////////////////////////////////////////////////////////

class omniAnonObjRef : public virtual omniObjRef,
		       public virtual CORBA::Object
{
public:
  inline omniAnonObjRef(omniIOR* ior, omniIdentity* id)
    : omniObjRef(CORBA::Object::_PD_repoId, ior, id, 1)
    { _PR_setobj(this); }

protected:
  virtual void* _ptrToObjRef(const char* repoId);
  virtual ~omniAnonObjRef();

private:
  omniAnonObjRef(const omniAnonObjRef&);
  omniAnonObjRef& operator = (const omniAnonObjRef&);
};

//////////////////////////////////////////////////////////////////////
///////////////////////// omniAnonObjRef_pof /////////////////////////
//////////////////////////////////////////////////////////////////////

class omniAnonObjRef_pof : public proxyObjectFactory {
public:
  virtual ~omniAnonObjRef_pof();
  inline omniAnonObjRef_pof()
    : proxyObjectFactory(CORBA::Object::_PD_repoId) {}

  virtual omniObjRef* newObjRef(omniIOR* ior, omniIdentity* id);
  virtual CORBA::Boolean is_a(const char* base_repoId) const;
};


OMNI_NAMESPACE_END(omni)

#endif
