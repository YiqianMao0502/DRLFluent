// -*- Mode: C++; -*-
//                            Package   : omniORB
// callHandle.h               Created on: 16/05/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2006-2013 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//   Call handle used during remote or in-process operation dispatch.

#ifndef __OMNIORB_CALLHANDLE_H__
#define __OMNIORB_CALLHANDLE_H__

#include <omniORB4/callDescriptor.h>
#include <omniORB4/IOP_S.h>

OMNI_NAMESPACE_BEGIN(omni)
class omniOrbPOA;
class giopConnection;
OMNI_NAMESPACE_END(omni)

class omniLocalIdentity;

class omniCallHandle {
public:

  inline omniCallHandle(_OMNI_NS(IOP_S)* iop_s_, omni_thread* self_thread)
    : pd_iop_s(iop_s_),
      pd_call_desc(0),
      pd_op(iop_s_->operation_name()),
      pd_postinvoke_hook(0),
      pd_poa(0),
      pd_localId(0),
      pd_mainthread_mu(0),
      pd_self_thread(self_thread),
      pd_try_direct(0)
  {}

  inline omniCallHandle(omniCallDescriptor* call_desc_,
			_CORBA_Boolean try_direct_)
    : pd_iop_s(0),
      pd_call_desc(call_desc_),
      pd_op(call_desc_->op()),
      pd_postinvoke_hook(0),
      pd_poa(0),
      pd_localId(0),
      pd_mainthread_mu(0),
      pd_self_thread(0),
      pd_try_direct(try_direct_)
  {}

  inline const char*         operation_name() const { return pd_op; }
  inline _OMNI_NS(IOP_S*)    iop_s()          const { return pd_iop_s; }
  inline omniCallDescriptor* call_desc()      const { return pd_call_desc; }
  inline _CORBA_Boolean      try_direct()     const { return pd_try_direct; }

  // Accessors for connection details. Return zero in the case of an
  // in-process call.
  _OMNI_NS(giopConnection)* connection();
  const char*               myaddress();
  const char*               peeraddress();
  const char*               peeridentity();
  void*                     peerdetails();

  // Call entry-point
  void upcall(omniServant* servant, omniCallDescriptor& desc);

  // Class PostInvokeHook is used to insert extra processing after the
  // upcall is made but before the results are marshalled. It is used
  // in ServantLocator dispatch.
  class PostInvokeHook {
  public:
    virtual void postinvoke() = 0;
    virtual ~PostInvokeHook();
  };

  inline void postinvoke_hook(PostInvokeHook* hook) {
    pd_postinvoke_hook = hook;
  }

  void SkipRequestBody();
  // SkipRequestBody is called if an exception occurs while
  // unmarshalling arguments. It skips any remaining input data so the
  // exception may be returned as soon as possible.

  inline void poa(_OMNI_NS(omniOrbPOA)* poa_) { pd_poa = poa_; }
  inline void localId(omniLocalIdentity* id)  { pd_localId = id; }

  inline void mainThread(omni_tracedmutex* mu, omni_tracedcondition* cond) {
    pd_mainthread_mu   = mu;
    pd_mainthread_cond = cond;
  }

  // Accessors
  inline PostInvokeHook*       postinvoke_hook() { return pd_postinvoke_hook; }
  inline _OMNI_NS(omniOrbPOA)* poa()             { return pd_poa; }
  inline omniLocalIdentity*    localId()         { return pd_localId; }
  inline omni_tracedmutex*     mainthread_mu()   { return pd_mainthread_mu; }
  inline omni_tracedcondition* mainthread_cond() { return pd_mainthread_cond; }

private:
  _OMNI_NS(IOP_S)*      pd_iop_s;
  omniCallDescriptor*   pd_call_desc;
  const char*           pd_op;
  PostInvokeHook*       pd_postinvoke_hook;
  _OMNI_NS(omniOrbPOA)* pd_poa;
  omniLocalIdentity*    pd_localId;
  omni_tracedmutex*     pd_mainthread_mu;
  omni_tracedcondition* pd_mainthread_cond;
  omni_thread*          pd_self_thread;
  _CORBA_Boolean        pd_try_direct;

private:
  // Not implemented
  omniCallHandle(const omniCallHandle&);
  omniCallHandle& operator=(const omniCallHandle&);
};



#endif // __OMNIORB_CALLHANDLE_H__
