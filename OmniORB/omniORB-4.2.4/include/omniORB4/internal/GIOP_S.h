// -*- Mode: C++; -*-
//                            Package   : omniORB
// GIOP_S.h                   Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2005-2013 Apasphere Ltd
//    Copyright (C) 2001      AT&T Laboratories Cambridge
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

#ifndef __GIOP_S_H__
#define __GIOP_S_H__

#include <omniORB4/IOP_S.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

class omniCallDescriptor;

OMNI_NAMESPACE_BEGIN(omni)

class giopWorker;

class GIOP_S : public IOP_S, public giopStream, public giopStreamList {
 public:


  GIOP_S(giopStrand*);
  GIOP_S(const GIOP_S&);
  ~GIOP_S();

  virtual void* ptrToClass(int* cptr);
  static inline GIOP_S* downcast(cdrStream* s) {
    return (GIOP_S*)s->ptrToClass(&_classid);
  }
  static _core_attr int _classid;

  CORBA::Boolean dispatcher();
  // This function do not raise an exception.
  // Returns TRUE(1) if the dispatch has been successful.
  // Returns FALSE(0) if a terminate error was encountered and no
  // more dispatch should be done.
  // XXX remember to catch all the system exceptions and
  //     giopStream::CommFailure.


  void ReceiveRequest(omniCallDescriptor&);
  // override IOP_S

  void SkipRequestBody();
  // override IOP_S

  void SendReply();
  // override IOP_S

  void SendException(CORBA::Exception*);
  // override IOP_S

  void notifyCommFailure(CORBA::Boolean heldlock,
			 CORBA::ULong& minor,
			 CORBA::Boolean& retry);
  // override giopStream member

  _CORBA_ULong completion();
  // override cdrStream member.

  cdrStream& getStream() { return *this; }

  //////////////////////////////////////////////////////////////////
  IOP_S::State state() const { return pd_state; }
  void state(IOP_S::State s) { pd_state = s; }

  //////////////////////////////////////////////////////////////////
  omniCallDescriptor* calldescriptor() { return pd_calldescriptor; }
  void calldescriptor(omniCallDescriptor* c) { pd_calldescriptor = c; }

  //////////////////////////////////////////////////////////////////
  GIOP::MsgType requestType() const { return pd_requestType; }
  void requestType(GIOP::MsgType m) { pd_requestType = m; }


  //////////////////////////////////////////////////////////////////
  inline CORBA::Boolean response_expected() const { 
    return pd_response_expected; 
  }
  inline void response_expected(CORBA::Boolean v) {
    pd_response_expected = v;
  }

  //////////////////////////////////////////////////////////////////
  inline void result_expected(CORBA::Boolean v) { 
      pd_result_expected = v;
  }
  inline CORBA::Boolean result_expected() const {
    return pd_result_expected;
  }

  //////////////////////////////////////////////////////////////////
  inline int keysize() const { return pd_key.size(); }
  inline CORBA::Octet* key() const { return (CORBA::Octet*)pd_key.key(); }
  inline void keysize(int sz) { pd_key.set_size((int)sz); }
  inline void resetKey() { pd_key.set_size(-1); }
  inline omniObjKey& keyobj() { return pd_key; }

  //////////////////////////////////////////////////////////////////
  inline GIOP::IORAddressingInfo& targetAddress() { 
      return pd_target_address;
  }
  void unmarshalIORAddressingInfo();


# define GIOP_S_INLINE_BUF_SIZE 32

  //////////////////////////////////////////////////////////////////
  inline char* operation() const { return pd_operation; }
  inline void  set_operation_size(CORBA::ULong sz) {
    if (pd_operation != pd_op_buffer) {
      delete [] pd_operation;
      pd_operation = pd_op_buffer;
    }
    if (sz > GIOP_S_INLINE_BUF_SIZE) {
      pd_operation = new char[sz];
    }
  }

  const char* operation_name() const;

  //////////////////////////////////////////////////////////////////
  inline CORBA::Octet* principal() const { return pd_principal; }
  inline CORBA::ULong principal_size() const { return pd_principal_len; }
  inline void set_principal_size(CORBA::ULong sz) {
    if (pd_principal != pd_pr_buffer) {
      delete [] pd_principal;
      pd_principal = pd_pr_buffer;
    }
    if (sz > GIOP_S_INLINE_BUF_SIZE) {
      pd_principal = new _CORBA_Octet[sz];
    }
    pd_principal_len = sz;
  }

  //////////////////////////////////////////////////////////////////
  inline IOP::ServiceContextList& service_contexts() {
    return pd_service_contexts;
  }

  //////////////////////////////////////////////////////////////////
  // terminateProcessing may be thrown by the GIOP implementations in
  // response to a CancelRequest message.
  class terminateProcessing {
  public:
    terminateProcessing() {}
    ~terminateProcessing() {}
  };

  inline void worker(giopWorker* w) { pd_worker = w; }
  inline giopWorker* worker() { return pd_worker; }

private:
  IOP_S::State             pd_state;
  giopWorker*              pd_worker;
  omniCallDescriptor*      pd_calldescriptor;
  const char* const*       pd_user_excns;
  int                      pd_n_user_excns;
  GIOP::MsgType            pd_requestType;

  omniObjKey               pd_key;
  // If pd_key.size() < 0, pd_target_address contains the
  // full IOR that the client has sent. Only used in GIOP 1.2.
  GIOP::IORAddressingInfo  pd_target_address;

  char*                    pd_operation;
  char                     pd_op_buffer[GIOP_S_INLINE_BUF_SIZE];

  CORBA::Octet*            pd_principal;
  CORBA::Octet             pd_pr_buffer[GIOP_S_INLINE_BUF_SIZE];
  CORBA::ULong             pd_principal_len;

  CORBA::Boolean           pd_response_expected;
  CORBA::Boolean           pd_result_expected;

  IOP::ServiceContextList  pd_service_contexts;

  CORBA::Boolean handleRequest();
  CORBA::Boolean handleLocateRequest();

public:
  CORBA::Boolean handleCancelRequest();

private:
  GIOP_S();
  GIOP_S& operator=(const GIOP_S&);

# undef GIOP_S_INLINE_BUF_SIZE

};


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class GIOP_S_Holder {
public:
  GIOP_S_Holder(giopStrand*, giopWorker*);
  ~GIOP_S_Holder();

  GIOP_S* operator->() { return pd_iop_s; }

  operator GIOP_S& () { return *pd_iop_s; }

private:
  giopStrand*  pd_strand;
  GIOP_S*      pd_iop_s;
  
};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __GIOP_S_H__
