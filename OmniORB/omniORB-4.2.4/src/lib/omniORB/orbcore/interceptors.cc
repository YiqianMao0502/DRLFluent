// -*- Mode: C++; -*-
//                            Package   : omniORB
// interceptors.cc            Created on: 22/09/2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories, Cambridge
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

#include <omniORB4/CORBA.h>
#include <omniORB4/IOP_S.h>
#include <omniORB4/IOP_C.h>
#include <omniORB4/omniServer.h>
#include <omniORB4/omniInterceptors.h>
#include <omniORB4/callDescriptor.h>
#include <interceptors.h>
#include <exceptiondefs.h>
#include <initialiser.h>
#include <giopStrand.h>
#include <giopRope.h>
#include <giopStream.h>
#include <GIOP_C.h>
#include <GIOP_S.h>


OMNI_NAMESPACE_BEGIN(omni)

omniInterceptors::omniInterceptors() {}
omniInterceptors::~omniInterceptors() {}


//
// Interceptors following the usual pattern

omniInterceptorP::elmT* omniInterceptorP::encodeIOR              = 0;
omniInterceptorP::elmT* omniInterceptorP::decodeIOR              = 0;
omniInterceptorP::elmT* omniInterceptorP::clientOpenConnection   = 0;
omniInterceptorP::elmT* omniInterceptorP::clientSendRequest      = 0;
omniInterceptorP::elmT* omniInterceptorP::clientReceiveReply     = 0;
omniInterceptorP::elmT* omniInterceptorP::serverAcceptConnection = 0;
omniInterceptorP::elmT* omniInterceptorP::serverReceiveRequest   = 0;
omniInterceptorP::elmT* omniInterceptorP::serverSendReply        = 0;
omniInterceptorP::elmT* omniInterceptorP::serverSendException    = 0;
omniInterceptorP::elmT* omniInterceptorP::createRope             = 0;
omniInterceptorP::elmT* omniInterceptorP::createIdentity         = 0;
omniInterceptorP::elmT* omniInterceptorP::createORBServer        = 0;
omniInterceptorP::elmT* omniInterceptorP::createPolicy           = 0;
omniInterceptorP::elmT* omniInterceptorP::createThread           = 0;
omniInterceptorP::elmT* omniInterceptorP::assignUpcallThread     = 0;
omniInterceptorP::elmT* omniInterceptorP::assignAMIThread        = 0;


static void list_add(omniInterceptorP::elmT** ep, void* func)
{
  while (*ep) {
    if ((*ep)->func == func) return;
    ep = &((*ep)->next);
  }
    
  omniInterceptorP::elmT* np = new omniInterceptorP::elmT();
  np->func = func;
  np->next = *ep;
  *ep = np;
}

static void list_remove(omniInterceptorP::elmT** ep, void* func)
{
  while (*ep) {
    if ((*ep)->func == func) {
      omniInterceptorP::elmT* p = *ep;
      *ep = p->next;
      delete p;
      return;
    }
    ep = &((*ep)->next);
  }
}

static void list_del(omniInterceptorP::elmT** ep)
{
  while (*ep) {
    omniInterceptorP::elmT* p = *ep;
    *ep = p->next;
    delete p;
  }
}

#define INTERCEPTOR_IMPLEMENTATION(interceptor) \
void omniInterceptors::interceptor##_T::add(\
           omniInterceptors::interceptor##_T::interceptFunc f) { \
  list_add(&omniInterceptorP::interceptor, (void*)f); \
} \
\
void omniInterceptors::interceptor##_T::remove(\
            omniInterceptors::interceptor##_T::interceptFunc f) { \
  list_remove(&omniInterceptorP::interceptor, (void*)f); \
}

INTERCEPTOR_IMPLEMENTATION(encodeIOR)
INTERCEPTOR_IMPLEMENTATION(decodeIOR)
INTERCEPTOR_IMPLEMENTATION(clientOpenConnection)
INTERCEPTOR_IMPLEMENTATION(clientSendRequest)
INTERCEPTOR_IMPLEMENTATION(clientReceiveReply)
INTERCEPTOR_IMPLEMENTATION(serverAcceptConnection)
INTERCEPTOR_IMPLEMENTATION(serverReceiveRequest)
INTERCEPTOR_IMPLEMENTATION(serverSendReply)
INTERCEPTOR_IMPLEMENTATION(serverSendException)
INTERCEPTOR_IMPLEMENTATION(createRope)
INTERCEPTOR_IMPLEMENTATION(createIdentity)
INTERCEPTOR_IMPLEMENTATION(createORBServer)
INTERCEPTOR_IMPLEMENTATION(createPolicy)
INTERCEPTOR_IMPLEMENTATION(createThread)
INTERCEPTOR_IMPLEMENTATION(assignUpcallThread)
INTERCEPTOR_IMPLEMENTATION(assignAMIThread)

#undef INTERCEPTOR_IMPLEMENTATION


//
// invokeLocalCall is handled by the omniCallDescriptor class

void omniInterceptors::invokeLocalCall_T::add(
         omniInterceptors::invokeLocalCall_T::interceptFunc f)
{
  omniCallDescriptor::addInterceptor(f);
}

void omniInterceptors::invokeLocalCall_T::remove(
         omniInterceptors::invokeLocalCall_T::interceptFunc f)
{
  omniCallDescriptor::removeInterceptor(f);
}


//
// Convenience methods to access operation name

#define OPERATION_ACCESSOR(interceptor, g) \
\
const char* \
omniInterceptors::interceptor##_T::info_T::operation() \
{ \
  return g.operation(); \
}

OPERATION_ACCESSOR(clientOpenConnection, giop_c)
OPERATION_ACCESSOR(clientSendRequest,    giop_c)
OPERATION_ACCESSOR(clientReceiveReply,   giop_c)
OPERATION_ACCESSOR(serverReceiveRequest, giop_s)
OPERATION_ACCESSOR(serverSendReply,      giop_s)
OPERATION_ACCESSOR(serverSendException,  giop_s)


//
// Convenience methods to access connection details

#define CONNECTION_ACCESSORS(interceptor,strand) \
\
const char* \
omniInterceptors::interceptor##_T::info_T::myaddress() \
{ \
  giopConnection* connection = strand.connection; \
  return connection ? connection->myaddress() : 0; \
}\
\
const char* \
omniInterceptors::interceptor##_T::info_T::peeraddress() \
{ \
  giopConnection* connection = strand.connection; \
  return connection ? connection->peeraddress() : 0; \
}\
\
const char* \
omniInterceptors::interceptor##_T::info_T::peeridentity() \
{ \
  giopConnection* connection = strand.connection; \
  return connection ? connection->peeridentity() : 0; \
}\
\
void* \
omniInterceptors::interceptor##_T::info_T::peerdetails() \
{ \
  giopConnection* connection = strand.connection; \
  return connection ? connection->peerdetails() : 0; \
}

CONNECTION_ACCESSORS(clientOpenConnection,   giop_c.strand())
CONNECTION_ACCESSORS(clientSendRequest,      giop_c.strand())
CONNECTION_ACCESSORS(clientReceiveReply,     giop_c.strand())
CONNECTION_ACCESSORS(serverAcceptConnection, strand)
CONNECTION_ACCESSORS(serverReceiveRequest,   giop_s.strand())
CONNECTION_ACCESSORS(serverSendReply,        giop_s.strand())
CONNECTION_ACCESSORS(serverSendException,    giop_s.strand())

#undef CONNECTION_ACCESSORS


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_interceptor_initialiser : public omniInitialiser {
public:

  omni_interceptor_initialiser() : pd_interceptors(0) {}

  void attach() {
    if (!pd_interceptors) pd_interceptors = new omniInterceptors();
  }

  void detach() {
    if (pd_interceptors) {
      delete pd_interceptors;
      pd_interceptors = 0;
      list_del(&omniInterceptorP::encodeIOR);
      list_del(&omniInterceptorP::decodeIOR);
      list_del(&omniInterceptorP::clientOpenConnection);
      list_del(&omniInterceptorP::clientSendRequest);
      list_del(&omniInterceptorP::clientReceiveReply);
      list_del(&omniInterceptorP::serverAcceptConnection);
      list_del(&omniInterceptorP::serverReceiveRequest);
      list_del(&omniInterceptorP::serverSendReply);
      list_del(&omniInterceptorP::serverSendException);
      list_del(&omniInterceptorP::createRope);
      list_del(&omniInterceptorP::createIdentity);
      list_del(&omniInterceptorP::createORBServer);
      list_del(&omniInterceptorP::createPolicy);
      list_del(&omniInterceptorP::createThread);
      list_del(&omniInterceptorP::assignUpcallThread);
      list_del(&omniInterceptorP::assignAMIThread);
    }
  }

  omniInterceptors* pd_interceptors;
};

static omni_interceptor_initialiser initialiser;

omniInitialiser& omni_interceptor_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)
/////////////////////////////////////////////////////////////////////////////
omniInterceptors*
omniORB::getInterceptors() {
  if (!initialiser.pd_interceptors) 
    OMNIORB_THROW(INITIALIZE,INITIALIZE_FailedLoadLibrary,
		  CORBA::COMPLETED_NO);

  return initialiser.pd_interceptors;
}
