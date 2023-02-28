// -*- Mode: C++; -*-
//                            Package   : omniORB
// GIOP_C.h                   Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
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
//	*** PROPRIETARY INTERFACE ***
//	

#ifndef __GIOP_C_H__
#define __GIOP_C_H__

#include <omniORB4/IOP_C.h>
#include <omniORB4/callDescriptor.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

class GIOP_C : public IOP_C, public giopStream, public giopStreamList {
 public:

  GIOP_C(giopRope*,giopStrand*);
  ~GIOP_C();

  virtual void* ptrToClass(int* cptr);
  static inline GIOP_C* downcast(cdrStream* s) {
    return (GIOP_C*)s->ptrToClass(&_classid);
  }
  static _core_attr int _classid;

  void InitialiseRequest();

  GIOP::ReplyStatusType ReceiveReply();

  void RequestCompleted(CORBA::Boolean skip=0);

  GIOP::LocateStatusType IssueLocateRequest();

  void notifyCommFailure(CORBA::Boolean heldlock,
			 CORBA::ULong& minor,
			 CORBA::Boolean& retry);
  // override giopStream member

  _CORBA_ULong completion();
  // override cdrStream member.

  cdrStream& getStream() { return *this; }

  void initialise(const omniIOR*,
		  const CORBA::Octet* key,
		  int keysize,
		  omniCallDescriptor*);

  void cleanup();

  inline IOP_C::State state() const { return pd_state; }
  inline void state(IOP_C::State s) { pd_state = s; }

  inline omniCallDescriptor* calldescriptor() { return pd_calldescriptor; }
  inline void calldescriptor(omniCallDescriptor* c) { pd_calldescriptor = c; }

  inline const char* operation() { return pd_calldescriptor->op(); }

  inline const CORBA::Octet* key() const  { return pd_key;    }
  inline void key(const CORBA::Octet* k)  { pd_key = k; }

  inline CORBA::ULong keysize() const  { return pd_keysize; }
  inline void keysize(CORBA::ULong sz) { pd_keysize = sz; }

  inline const omniIOR* ior() const { return pd_ior; }
  inline void ior(const omniIOR* c) { pd_ior = c; }

  inline GIOP::ReplyStatusType replyStatus() const { return pd_replyStatus; }
  inline void replyStatus(GIOP::ReplyStatusType rc) { pd_replyStatus = rc; }

  inline GIOP::LocateStatusType locateStatus() const { return pd_locateStatus; }
  inline void locateStatus(GIOP::LocateStatusType rc) { pd_locateStatus = rc; }

  inline CORBA::ULong  replyId() const { return pd_reply_id; }
  inline void replyId(CORBA::ULong v) { pd_reply_id = v; }

  inline giopRope* rope() const { return pd_rope; }

private:
  IOP_C::State            pd_state;
  omniCallDescriptor*     pd_calldescriptor;
  const omniIOR*          pd_ior;
  const CORBA::Octet*     pd_key;
  CORBA::ULong            pd_keysize;
  giopRope*               pd_rope;
  GIOP::ReplyStatusType   pd_replyStatus;
  GIOP::LocateStatusType  pd_locateStatus;
  CORBA::ULong            pd_reply_id;

  void UnMarshallSystemException();

  GIOP_C();
  GIOP_C(const GIOP_C&);
  GIOP_C& operator=(const GIOP_C&);
};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __GIOP_C_H__
