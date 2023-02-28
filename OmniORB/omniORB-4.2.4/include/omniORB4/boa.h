// -*- Mode: C++; -*-
//                            Package   : omniORB2
// boa.h                      Created on: 27/7/99
//                            Author    : David Riddoch (djr)
//
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

#ifndef __OMNIORB_BOA_H__
#define __OMNIORB_BOA_H__


//////////////////////////////////////////////////////////////////////
////////////////////////// omniOrbBoaServant /////////////////////////
//////////////////////////////////////////////////////////////////////

class omniOrbBoaServant : public virtual omniServant {
public:
  virtual ~omniOrbBoaServant();

  omniOrbBoaServant();
  inline omniOrbBoaServant(const omniOrbBoaKey& k)
    : pd_key(k) {}

  void _dispose();


  //////////////////////
  // omniORB Internal //
  //////////////////////

  void _obj_is_ready();
  // Implementation of _obj_is_ready(BOA_ptr);

  void* _this(const char* repoId);
  // If activated, return a <foo_ptr> (where foo is given in the
  // <repoId>).  Otherwise throws BAD_INV_ORDER.

  inline const omniOrbBoaKey& _key() const { return pd_key; }
  inline void _obj_is_ready(CORBA::BOA_ptr) { _obj_is_ready(); }
  inline CORBA::BOA_ptr _boa() { return CORBA::BOA::getBOA(); }

private:
  virtual omniObjRef* _do_get_interface();
  // Overrides omniServant.

protected:
  omniOrbBoaKey pd_key;
  // This is used only for construction/initialisation.
  // When the object is activated, the key in the id is
  // what counts.
  //  Immutable.
};


#endif  // __OMNIORB_BOA_H__
