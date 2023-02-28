// -*- Mode: C++; -*-
//                            Package   : omniORB
// poamanager.h               Created on: 12/5/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
//    Internal implementation of the PortableServer::POAManager.
//

#ifndef __POAMANAGER_H__
#define __POAMANAGER_H__

OMNI_NAMESPACE_BEGIN(omni)

class omniOrbPOA;


class omniOrbPOAManager : public PortableServer::POAManager {
public:
  virtual ~omniOrbPOAManager();
  inline omniOrbPOAManager(int is_nil = 0)
    : OMNIORB_BASE_CTOR(PortableServer::)POAManager(is_nil),
      pd_refCount(1),
      pd_state(HOLDING),
      pd_deactivated(0)
    {}

  ////////////////////////////////
  // PortableServer::POAManager //
  ////////////////////////////////
  virtual void activate();
  virtual void hold_requests(CORBA::Boolean wait_for_completion);
  virtual void discard_requests(CORBA::Boolean wait_for_completion);
  virtual void deactivate(CORBA::Boolean etherealize_objects,
			  CORBA::Boolean wait_for_completion);
  virtual State get_state();

  ////////////////////////////
  // Override CORBA::Object //
  ////////////////////////////
  virtual void* _ptrToObjRef(const char* repoId);
  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();

  //////////////
  // Internal //
  //////////////
  void gain_poa(omniOrbPOA* poa);
  // Adds <poa> to our list of POAs, and if our state is not
  // HOLDING, updates the POAs state to match our own.

  void lose_poa(omniOrbPOA* poa);


  typedef _CORBA_PseudoValue_Sequence<omniOrbPOA*> POASeq;


private:
  POASeq pd_poas;
  // We don't hold a reference to each POA - since they each
  // hold references to us.

  int    pd_refCount;
  State  pd_state;
  int    pd_deactivated; // Becomes true when deactivation has finished
};

OMNI_NAMESPACE_END(omni)

#endif  // __POAMANAGER_H__
