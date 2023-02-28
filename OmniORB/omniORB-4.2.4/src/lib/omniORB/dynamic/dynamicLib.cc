// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynamicLib.cc              Created on: 15/9/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2003 Apasphere Ltd
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
//    Implementation of dynamic library 'hook' functions.
//

#define ENABLE_CLIENT_IR_SUPPORT
#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <dynamicLib.h>
#include <context.h>
#include <initialiser.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(dynamicLib);

OMNI_NAMESPACE_BEGIN(omni)

//
// Module initialisers

extern omniInitialiser& omni_valueFactory_initialiser_;


static void init();
static void deinit();
static void lookup_id_lcfn(omniCallDescriptor* cd, omniServant* svnt);


static omniDynamicLib dynamic_ops = {
  init,
  deinit,
  lookup_id_lcfn
};

// Static constructor to initialise omniDynamicLib::hook.
struct omniDynamicLib_initialiser {
  inline omniDynamicLib_initialiser() {
    omniDynamicLib::hook = &dynamic_ops;
  }
  static omniDynamicLib_initialiser instance;
};
omniDynamicLib_initialiser omniDynamicLib_initialiser::instance;


static void
init()
{
  omniORB::logs(5, "Initialising omniDynamic library.");
  omni_valueFactory_initialiser_.attach();
}


static void
deinit()
{
  omniORB::logs(5, "Deinitialising omniDynamic library.");
  ContextImpl::releaseDefault();
  omni_valueFactory_initialiser_.detach();
}


static void
lookup_id_lcfn(omniCallDescriptor* cd, omniServant* svnt)
{
  omniStdCallDesc::_cCORBA_mObject_i_cstring* tcd = (omniStdCallDesc::_cCORBA_mObject_i_cstring*) cd;
  CORBA::_impl_Repository* impl = (CORBA::_impl_Repository*) svnt->_ptrToInterface(CORBA::Repository::_PD_repoId);
  tcd->pd_result = impl->lookup_id(tcd->arg_0);
}

OMNI_NAMESPACE_END(omni)


#if defined(__DMC__) && defined(_WINDLL)
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  return TRUE;
}
#endif
