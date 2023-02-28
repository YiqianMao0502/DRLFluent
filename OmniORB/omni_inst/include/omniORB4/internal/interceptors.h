// -*- Mode: C++; -*-
//                            Package   : omniORB
// interceptors.h             Created on: 2002/03/21
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2006-2013 Apasphere Ltd
//    Copyright (C) 2002 AT&T Laboratories, Cambridge
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

#ifndef __INTERCEPTORS_H__
#define __INTERCEPTORS_H__

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif


OMNI_NAMESPACE_BEGIN(omni)

class omniInterceptorP {
public:
  struct elmT {
    void* func;
    elmT* next;
  };

  static _core_attr elmT* encodeIOR;
  static _core_attr elmT* decodeIOR;
  static _core_attr elmT* clientOpenConnection;
  static _core_attr elmT* clientSendRequest;
  static _core_attr elmT* clientReceiveReply;
  static _core_attr elmT* serverAcceptConnection;
  static _core_attr elmT* serverReceiveRequest;
  static _core_attr elmT* serverSendReply;
  static _core_attr elmT* serverSendException;
  static _core_attr elmT* createRope;
  static _core_attr elmT* createIdentity;
  static _core_attr elmT* createORBServer;
  static _core_attr elmT* createPolicy;
  static _core_attr elmT* createThread;
  static _core_attr elmT* assignUpcallThread;
  static _core_attr elmT* assignAMIThread;

#define VISIT_FUNCTION(name) \
  static inline void visit(omniInterceptors::name##_T::info_T& info) { \
    for (elmT* e = name; e; e = e->next) { \
      if (!(*((omniInterceptors::name##_T::interceptFunc)(e->func)))(info)) \
	return; \
    } \
  }

  VISIT_FUNCTION(encodeIOR)
  VISIT_FUNCTION(decodeIOR)
  VISIT_FUNCTION(clientOpenConnection)
  VISIT_FUNCTION(clientSendRequest)
  VISIT_FUNCTION(clientReceiveReply)
  VISIT_FUNCTION(serverAcceptConnection)
  VISIT_FUNCTION(serverReceiveRequest)
  VISIT_FUNCTION(serverSendReply)
  VISIT_FUNCTION(serverSendException)
  VISIT_FUNCTION(createRope)
  VISIT_FUNCTION(createIdentity)
  VISIT_FUNCTION(createORBServer)
  VISIT_FUNCTION(createPolicy)

#undef VISIT_FUNCTION
};


OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __INTERCEPTORS_H__
