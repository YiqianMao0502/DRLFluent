// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynamicLib.h               Created on: 15/9/99
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
//    Hook to allow orbcore to access dynamic library when linked.
//

#ifndef __DYNAMICLIB_H__
#define __DYNAMICLIB_H__


#if defined(_OMNIORB_LIBRARY)
# define _core_attr
#elif defined(_OMNIORB_DYNAMIC_LIBRARY)
# define _core_attr _OMNIORB_NTDLL_IMPORT
#else
# error "Neither _OMNIORB_LIBRARY nor _OMNIORB_DYNAMIC_LIBRARY defined"
#endif

class omniCallDescriptor;

OMNI_NAMESPACE_BEGIN(omni)


class omniDynamicLib {
public:

  // This points to the implementation of dynamic library functions
  // that the orbcore library should use.  If the dynamic library
  // is linked then the real functions are called.  Otherwise
  // stub versions are called which do nothing, or deal with the
  // error appropriately.
  static _core_attr omniDynamicLib* ops;

  // This points to the dynamic library's version of the functions
  // if the dynamic is linked, or is zero otherwise.
  static _core_attr omniDynamicLib* hook;

  ////////////////////////////////////////////////
  // Operations exported by the dynamic library //
  ////////////////////////////////////////////////

  // Initialisation
  void (*init)();
  void (*deinit)();

  // Local call call-back function for CORBA::Repository::lookup_id().
  // This is needed to support the server side of _get_interface(),
  // in the case that the interface repository is co-located!
  void (*lookup_id_lcfn)(omniCallDescriptor*, omniServant*);
};


#undef _core_attr

OMNI_NAMESPACE_END(omni)

#endif  // __DYNAMICLIB_H__
