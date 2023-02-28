// -*- Mode: C++; -*-
//                            Package   : omniORB2
// proxyFactory.h             Created on: 13/6/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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
//	*** PROPRIETARY INTERFACE ***
//

#ifndef __OMNI_PROXYFACTORY_H__
#define __OMNI_PROXYFACTORY_H__


class omniObjRef;

OMNI_NAMESPACE_BEGIN(omni)



class proxyObjectFactory {
public:
  virtual ~proxyObjectFactory();
  proxyObjectFactory(const char* repoId);
  // This constructor inserts this object into the list
  // of object factories.
  //  Assumes that <repoId> will remain valid for the
  // lifetime of this object.

  static void shutdown();
  // Frees resources -- only called on ORB shutdown.  Does not
  // release the pof's themselves, the stubs take care of that.

  static proxyObjectFactory* lookup(const char* repoId);

  inline const char* irRepoId() const { return pd_repoId; }
  // Returns the Interface Repository ID for proxies this
  // factory can instantiate.

  virtual omniObjRef* newObjRef(omniIOR* ior, omniIdentity* id) = 0;
  // Returns a new object reference. Consumes <profiles>.

  virtual _CORBA_Boolean is_a(const char* base_repoId) const = 0;
  // Must return true if <base_repoId> is the interface
  // repository ID of a base interface. Need not recognise
  // CORBA::Object as a base interface.

private:
  const char* pd_repoId;
};

OMNI_NAMESPACE_END(omni)


#endif // __OMNI_PROXYFACTORY_H__
