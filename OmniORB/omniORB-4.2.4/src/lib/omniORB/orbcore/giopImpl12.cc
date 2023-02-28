// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopImpl12.cc              Created on: 14/02/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories, Cambridge
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
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <giopStrand.h>
#include <giopRope.h>
#include <giopServer.h>
#include <GIOP_S.h>
#include <GIOP_C.h>
#include <omniORB4/minorCode.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/omniInterceptors.h>
#include <interceptors.h>
#include <orbParameters.h>

OMNI_NAMESPACE_BEGIN(omni)

class nonexistence;

class giopImpl12 {
public:
  static void outputMessageBegin(giopStream*,
				 void (*marshalHeader)(giopStream*));
  static void outputMessageEnd(giopStream*);
  static void inputMessageBegin(giopStream*,
				void (*unmarshalHeader)(giopStream*));
  static void inputMessageEnd(giopStream*,CORBA::Boolean disgard = 0);
  static void sendMsgErrorMessage(giopStream*, const CORBA::SystemException*);
  static void marshalRequestHeader(giopStream*);
  static void sendLocateRequest(giopStream*);
  static void unmarshalReplyHeader(giopStream*);
  static void unmarshalLocateReply(giopStream*);
  static void unmarshalWildCardRequestHeader(giopStream*);
  static void unmarshalRequestHeader(giopStream*);
  static void unmarshalLocateRequest(giopStream*);
  static void marshalReplyHeader(giopStream*);
  static void sendSystemException(giopStream*,const CORBA::SystemException&);
  static void sendUserException(giopStream*,const CORBA::UserException&);
  static void sendLocationForwardReply(giopStream*,CORBA::Object_ptr,
				       CORBA::Boolean=0);
  static void sendLocateReply(giopStream*,GIOP::LocateStatusType,
			      CORBA::Object_ptr,CORBA::SystemException* p = 0);
  static size_t inputRemaining(giopStream*);
  static void getInputData(giopStream*,omni::alignment_t,size_t);
  static void skipInputData(giopStream*,size_t);
  static void copyInputData(giopStream*,void*, size_t,omni::alignment_t);
  static size_t outputRemaining(const giopStream*);
  static void getReserveSpace(giopStream*,omni::alignment_t,size_t);
  static void copyOutputData(giopStream*,void*, size_t,omni::alignment_t);
  static CORBA::ULong currentInputPtr(const giopStream*);
  static CORBA::ULong currentOutputPtr(const giopStream*);

  friend class nonexistence;  // Just to make gcc shut up.

  static void inputNewFragment(giopStream* g);

  static void inputNewServerMessage(giopStream* g);

  static void inputQueueMessage(giopStream*,giopStream_Buffer*);

  static void inputReplyBegin(giopStream*, 
			      void (*unmarshalHeader)(giopStream*));

  static void inputSkipWholeMessage(giopStream* g);

  static void inputTerminalProtocolError(giopStream* g,
					 const char* file, int line,
					 const char* message);
  // Helper function.  Call this function to indicate that a protocol
  // voilation was detected.  This function *always* raise a
  // giopStream::CommFailure exception.  Therefore the caller should not
  // expect this function to return.

  static void inputRaiseCommFailure(giopStream* g, const char* message);

  static void outputNewMessage(giopStream* g);

  static void outputFlush(giopStream* g,CORBA::Boolean knownFragmentSize=0);

  static void outputSetFragmentSize(giopStream*,CORBA::ULong);

  static inline CORBA::Boolean outputHasReachedLimit(giopStream* g) {
    return g->outEnd() == g->outputBufferStart() + 12;
  }

private:
  giopImpl12();
  giopImpl12(const giopImpl12&);
  giopImpl12& operator=(const giopImpl12&);
};


////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputQueueMessage(giopStream* g, giopStream_Buffer* b) {

  // On entry, this function owns the giopStream_Buffer. On exit, the
  // buffer has either been assigned to another owner or deleted.

  unsigned char* hdr = (unsigned char*)b + b->start;
  GIOP::MsgType mtype = (GIOP::MsgType)hdr[7];

  if (hdr[4] != 1 || hdr[5] != 2 || mtype > GIOP::Fragment) {

    // Notice that we only accept GIOP 1.2 packets here. That is, any
    // GIOP 1.0 or 1.1. messages will be rejected.
    // While the spec. say that a GIOP 1.2 message can interleave with
    // another 1.2 message. It does not say if a GIOP 1.0 or 1.1 message
    // can interleave with a GIOP 1.2 message. Our interpretation is to
    // disallow this.
    //
    // *** HERE: when we support GIOP > 1.2, we can get here (or
    // rather the equivalent place in GIOP 1.3+) due to receiving a
    // reply message on the receiver thread of a bidirectional
    // connection.

    // We accept a CloseConnection message with any GIOP version.
    if ((GIOP::MsgType)hdr[7] != GIOP::CloseConnection) {
      giopStream_Buffer::deleteBuffer(b);
      inputTerminalProtocolError(g, __FILE__, __LINE__,
				 "Invalid GIOP message version");
      // never reach here
    }
  }

  CORBA::ULong reqid;

  if (mtype != GIOP::CloseConnection && mtype != GIOP::MessageError) {

    // unmarshal request id.
    reqid = *((CORBA::ULong*)(hdr + 12));
    if ((hdr[6] & 0x1) != _OMNIORB_HOST_BYTE_ORDER_) {
      CORBA::ULong v = reqid;
      reqid = ((((v) & 0xff000000) >> 24) |
	       (((v) & 0x00ff0000) >> 8)  |
	       (((v) & 0x0000ff00) << 8)  |
	       (((v) & 0x000000ff) << 24));
    }
  }
  else if (mtype == GIOP::MessageError) {
    giopStream_Buffer::deleteBuffer(b);
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Received a MessageError message");
    // never reach here
    reqid = 0; // avoid compiler warning
  }
  else if (g->pd_strand->isClient() || g->pd_strand->isBiDir()) {
    // orderly shutdown.
    CORBA::ULong   minor;
    CORBA::Boolean retry;

    giopStream_Buffer::deleteBuffer(b);
    g->pd_strand->orderly_closed = 1;
    g->notifyCommFailure(0,minor,retry);
    g->pd_strand->state(giopStrand::DYING);
    giopStream::CommFailure::_raise(minor,
				    CORBA::COMPLETED_NO,
				    retry,__FILE__,__LINE__,
				    "Orderly connection shutdown",
				    g->pd_strand);
    // never reach here
    reqid = 0; // avoid compiler warning
  }
  else {
    giopStream_Buffer::deleteBuffer(b);
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Orderly connection shutdown on server");
    reqid = 0; // avoid compiler warning
    // never reach here
  }

  giopStream*    matched_target = 0;
  CORBA::Boolean matched_target_is_client = 0;

  omniTransportLock->lock();

  // Look at clients
  switch (mtype) {
  case GIOP::Reply:
  case GIOP::LocateReply:
    if (!(g->pd_strand->isClient() || g->pd_strand->isBiDir())) {
      omniTransportLock->unlock();
      giopStream_Buffer::deleteBuffer(b);
      inputTerminalProtocolError(g, __FILE__, __LINE__,
				 "Server received an invalid reply message");
      // never reach here
    }
    // falls through
  case GIOP::Fragment:
    {
      // search for a matching id in clients.

      giopStreamList* gp = g->pd_strand->clients.next;
      for (; gp != &g->pd_strand->clients; gp = gp->next) {

	GIOP_C* target = (GIOP_C*)gp;

	if (target->state() != IOP_C::UnUsed && 
	    target->requestId() == reqid) {

	  // sanity check
	  if (target->inputFullyBuffered()) {
	    // a reply has already been received!
	    omniTransportLock->unlock();
	    giopStream_Buffer::deleteBuffer(b);
	    inputTerminalProtocolError(g, __FILE__, __LINE__,
				       "Message claims to belong to a "
				       "completed reply");
	    // never reach here
	  }

	  if (target->inputMatchedId()) {
	    if (mtype != GIOP::Fragment) {
	      // already got the header
	      omniTransportLock->unlock();
	      giopStream_Buffer::deleteBuffer(b);
	      inputTerminalProtocolError(g, __FILE__, __LINE__,
					 "Reply header has already been "
					 "received");
	      // never reach here
	    }
	  }
	  else if (mtype == GIOP::Fragment) {
	    // receive body before the header
	    omniTransportLock->unlock();
	    giopStream_Buffer::deleteBuffer(b);
	    inputTerminalProtocolError(g, __FILE__, __LINE__,
				       "Reply header not seen for message "
				       "fragment");
	    // never reach here
	  }
	  else {
	    target->inputMatchedId(1);
	  }
	  matched_target = (giopStream*)target;
	  matched_target_is_client = 1;
	  break;
	}
      }
    }
  default:
    break;
  }

  if (!matched_target) {

    // look at servers
    switch (mtype) {
    case GIOP::Request:
    case GIOP::LocateRequest:
      {
	if (g->pd_strand->isClient() && !g->pd_strand->isBiDir()) {
	  omniTransportLock->unlock();
	  giopStream_Buffer::deleteBuffer(b);
	  inputTerminalProtocolError(g, __FILE__, __LINE__,
				     "Request message received by client");
	  // never reach here
	}
	// Make sure this request id has not been seen before
	GIOP_S* unused = 0;
	giopStreamList* gp = g->pd_strand->servers.next;
	for (; gp != &g->pd_strand->servers; gp = gp->next) {

	  GIOP_S* target = (GIOP_S*)gp;
	  if (target->state() == IOP_S::UnUsed) {
	    unused = target;
	  }
	  else if (target->requestId() == reqid) {
	    // already have a request with the same id.
	    omniTransportLock->unlock();
	    giopStream_Buffer::deleteBuffer(b);
	    inputTerminalProtocolError(g, __FILE__, __LINE__,
				       "Duplicate request id");
	    // never reach here
	  }
	}
	if (!unused) {
	  OMNIORB_ASSERT(g->pd_strand->servers.next != &g->pd_strand->servers);
	  GIOP_S* copy = (GIOP_S*)g->pd_strand->servers.next;
	  unused = new GIOP_S(*copy);
	  unused->giopStreamList::insert(g->pd_strand->servers);
	}
	unused->state(IOP_S::InputPartiallyBuffered);
	unused->requestId(reqid);
      }
      // falls through
    case GIOP::CancelRequest:
      if (g->pd_strand->isClient() && !g->pd_strand->isBiDir()) {
	omniTransportLock->unlock();
	giopStream_Buffer::deleteBuffer(b);
	inputTerminalProtocolError(g, __FILE__, __LINE__,
				   "Client received a CancelRequest message");
	// never reach here
      }
      // falls through
    case GIOP::Fragment:
      {
	// search for a matching id in servers.
	
	giopStreamList* gp = g->pd_strand->servers.next;
	for (; gp != &g->pd_strand->servers; gp = gp->next) {

	  GIOP_S* target = (GIOP_S*)gp;

	  if (target->state() != IOP_S::UnUsed && 
	      target->requestId() == reqid) {
	    
	    // sanity check
	    if (target->inputFullyBuffered()) {
	      // a reply has already been received!
	      omniTransportLock->unlock();
	      giopStream_Buffer::deleteBuffer(b);
	      inputTerminalProtocolError(g, __FILE__, __LINE__,
					 "Message claims to belong to a "
					 "completed request");
	      // never reach here
	    }

	    matched_target = (giopStream*)target;
	    break;
	  }
	}
      }
    default:
      break;
    }
  }
  omniTransportLock->unlock();

  if (matched_target) {

    if (matched_target == g) {
      OMNIORB_ASSERT(g->pd_input == 0);
      if (g->pd_currentInputBuffer) {
	g->releaseInputBuffer(g->pd_currentInputBuffer);
	g->pd_currentInputBuffer = 0;
      }
      g->pd_input = b;
      return;
    }

    OMNIORB_ASSERT(matched_target->pd_currentInputBuffer == 0);
    giopStream_Buffer** pp = &matched_target->pd_input;
    while (*pp) {
      pp = &((*pp)->next);
    }
    *pp = b;

    CORBA::ULong fetchsz = b->size - b->dataSize();
    while (fetchsz) {
      // fetch the rest of the message;
      giopStream_Buffer* p = g->inputChunk(fetchsz);
      pp = &((*pp)->next);
      *pp = p;
      fetchsz -= p->dataSize();
    }
      
    CORBA::Boolean isfull = ((hdr[6] & 0x2) ? 0 : 1);

    if (mtype == GIOP::CancelRequest) {
      if (!matched_target_is_client)
	((GIOP_S*)matched_target)->handleCancelRequest();
    }
    else if (isfull) {
      {
	omni_tracedmutex_lock sync(*omniTransportLock);
	matched_target->inputFullyBuffered(isfull);

	if (!matched_target_is_client)
	  ((GIOP_S*)matched_target)->state(IOP_S::InputFullyBuffered);

	giopStream::wakeUpRdLock(g->pd_strand);
      }
      if (!matched_target_is_client) {
	omniORB::logs(25, "Changed GIOP_S to InputFullyBuffered");
	if (!g->pd_strand->isClient()) {
	  g->pd_strand->server->notifyCallFullyBuffered(g->pd_strand->connection);
	}
      }
    }
    return;
  }

  // reach here if we have no match.
  {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Discarding message for non-existent request id "
	  << reqid << ".\n";
    }
    CORBA::ULong fetchsz = b->size - b->dataSize();
    giopStream_Buffer::deleteBuffer(b);
    while (fetchsz) {
      // fetch the rest of the message;
      b = g->inputChunk(fetchsz);
      fetchsz -= b->dataSize();
      giopStream_Buffer::deleteBuffer(b);
    }
  }
}


////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputNewServerMessage(giopStream* g) {

  OMNIORB_ASSERT(g->pd_currentInputBuffer == 0);

  g->pd_currentInputBuffer = g->inputMessage();

  unsigned char* hdr = (unsigned char*)g->inputBufferStart();

  if (hdr[4] != 1 || hdr[5] > 2 || hdr[7] > (unsigned char) GIOP::Fragment) {
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Invalid message version/type");
    // never reach here
  }

  switch ((GIOP::MsgType)hdr[7]) {
  case GIOP::Request:
  case GIOP::LocateRequest:
  case GIOP::MessageError:
  case GIOP::CloseConnection:
    return;
  case GIOP::Reply:
  case GIOP::LocateReply:
    if (g->pd_strand->isBiDir()) {
      break;
    }
    else {
      inputTerminalProtocolError(g, __FILE__, __LINE__,
				 "Server received an invalid reply message");
      // Never reach here.
    }
  case GIOP::Fragment:
  case GIOP::CancelRequest:
    break;
  }

  // reach here if the message is destined to some other call in progress.
  giopStream_Buffer* p = g->pd_currentInputBuffer;
  g->pd_currentInputBuffer = 0;
  inputQueueMessage(g,p);
}


////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputNewFragment(giopStream* g) {

  if (g->pd_currentInputBuffer) {
    g->releaseInputBuffer(g->pd_currentInputBuffer);
    g->pd_currentInputBuffer = 0;
  }

 again:
  if (!g->pd_input) {
    giopStream_Buffer* p = g->inputMessage();
    inputQueueMessage(g,p);
    goto again;
  }
  else {
    g->pd_currentInputBuffer       = g->pd_input;
    g->pd_input                    = g->pd_currentInputBuffer->next;
    g->pd_currentInputBuffer->next = 0;
  }

  char* hdr = (char*)g->inputBufferStart();

  if (hdr[7] == GIOP::CancelRequest) {
    if (g->pd_strand->isBiDir() || !g->pd_strand->isClient()) {
      throw GIOP_S::terminateProcessing();
    }
    else {
      inputTerminalProtocolError(g, __FILE__, __LINE__,
				 "Client received a CancelRequest message");
      // never reach here.
    }
  }

  CORBA::Boolean bswap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_)
			  ? 0 : 1 );

  if (bswap != g->pd_unmarshal_byte_swap) {
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Fragment has different byte ordering to "
			       "initial message");
    // never reach here
  }

  g->pd_inb_mkr = (void*)(hdr + 16);
  g->pd_inb_end = (void*)g->inputBufferLast();

  g->inputExpectAnotherFragment(((hdr[6] & 0x2) ? 1 : 0));

  g->inputMessageSize(g->inputMessageSize() + 
		      g->inputBufferMsgSize() - 16);

  g->inputFragmentToCome(g->inputBufferMsgSize() - g->inputBufferDataSize());
}


////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputReplyBegin(giopStream* g, 
			    void (*unmarshalHeader)(giopStream*)) {

  {
    omni_tracedmutex_lock sync(*omniTransportLock);

    if (!g->pd_strand->isBiDir()) {

      while (!(g->inputFullyBuffered() || g->pd_rdlocked)) {
	if (!g->rdLockNonBlocking()) {
	  g->sleepOnRdLock();
	}
      }
    }
    else {
      // If the strand is used for bidirectional GIOP, we
      // let the server thread to read the incoming message and to
      // demultiplex any reply back to us. Therefore, we do not
      // try to acquire the read lock but just sleep on the read
      // lock instead.
      while (!(g->pd_strand->state() == giopStrand::DYING ||
	       g->inputFullyBuffered() ) ) {
	OMNIORB_ASSERT(g->pd_rdlocked == 0);
	g->sleepOnRdLockAlways();
      }
      if (g->pd_strand->state() == giopStrand::DYING) {
	CORBA::ULong   minor;
	CORBA::Boolean retry;

	g->notifyCommFailure(1,minor,retry);

	CORBA::CompletionStatus status;
	if (g->pd_strand->orderly_closed) {
	  status = CORBA::COMPLETED_NO;
	}
	else {
	  status = (CORBA::CompletionStatus)g->completion();
	}
	giopStream::CommFailure::_raise(minor,
					status,
					retry,
					__FILE__,__LINE__,
					"Connection closed while waiting for "
					"a reply on a bidirectional "
					"connection", g->pd_strand);
	// never reaches here.
      }
    }
  }

  if (!g->pd_currentInputBuffer) {
  again:
    if (!g->pd_input) {
      giopStream_Buffer* p = g->inputMessage();
      inputQueueMessage(g,p);
      goto again;
    }
    else {
      g->pd_currentInputBuffer       = g->pd_input;
      g->pd_input                    = g->pd_input->next;
      g->pd_currentInputBuffer->next = 0;
    }
  }

  char* hdr = (char*)g->inputBufferStart();

  g->pd_unmarshal_byte_swap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_)
			       ? 0 : 1 );

  g->pd_inb_mkr = (void*)(hdr + 16);
  g->pd_inb_end = (void*)g->inputBufferLast();

  g->inputExpectAnotherFragment(((hdr[6] & 0x2) ? 1 : 0));

  g->inputMessageSize(g->inputBufferMsgSize());
  g->inputFragmentToCome(g->inputBufferMsgSize() - g->inputBufferDataSize());

  unmarshalHeader(g);

  if (g->inputMessageSize() > orbParameters::giopMaxMsgSize) {
    OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnClient,
		  CORBA::COMPLETED_YES);
  }
}


////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputMessageBegin(giopStream* g,
			      void (*unmarshalHeader)(giopStream*)) {

  if (unmarshalHeader != unmarshalWildCardRequestHeader) {
    inputReplyBegin(g,unmarshalHeader);
    return;
  }

  {
    omni_tracedmutex_lock sync(*omniTransportLock);

    if (!(g->inputFullyBuffered() || g->pd_rdlocked)) {
      g->rdLock();
    }
  }
 again:
  if (!g->pd_currentInputBuffer) {

    if (g->pd_input) {
      g->pd_currentInputBuffer       = g->pd_input;
      g->pd_input                    = g->pd_input->next;
      g->pd_currentInputBuffer->next = 0;
    }
    else {
      inputNewServerMessage(g);
      goto again;
    }
  }

  char* hdr = (char*)g->inputBufferStart();

  if (hdr[5] <= 1) {
    // This is a GIOP 1.0 or 1.1 message, switch to the implementation
    // and dispatch again.
    GIOP::Version v;
    v.major = 1;
    v.minor = hdr[5];
    g->strand().version = v;
    g->impl(giopStreamImpl::matchVersion(v));
    OMNIORB_ASSERT(g->impl());
    g->impl()->inputMessageBegin(g,g->impl()->unmarshalWildCardRequestHeader);
    return;
  }

  g->pd_unmarshal_byte_swap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_)
			       ? 0 : 1 );

  g->pd_inb_mkr = (void*)(hdr + 12);
  g->pd_inb_end = (void*)g->inputBufferLast();

  g->inputExpectAnotherFragment(((hdr[6] & 0x2) ? 1 : 0));

  g->inputMessageSize(g->inputBufferMsgSize());
  g->inputFragmentToCome(g->inputBufferMsgSize() - g->inputBufferDataSize());

  unmarshalHeader(g);

  if (g->inputMessageSize() > orbParameters::giopMaxMsgSize) {
    OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnServer,
		  CORBA::COMPLETED_NO);
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputSkipWholeMessage(giopStream* g) {

  do {
    if (g->pd_currentInputBuffer) {
      giopStream_Buffer::deleteBuffer(g->pd_currentInputBuffer);
      g->pd_currentInputBuffer = 0;
    }
    if (g->inputFragmentToCome()) {

      if (!g->pd_input) {
	g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome());
      }
      else {
	g->pd_currentInputBuffer       = g->pd_input;
	g->pd_input                    = g->pd_currentInputBuffer->next;
	g->pd_currentInputBuffer->next = 0;
      }
      g->pd_inb_mkr = (void*)g->inputBufferStart();
      g->pd_inb_end = (void*)g->inputBufferLast();
      g->inputFragmentToCome(g->inputFragmentToCome() -
                             g->inputBufferDataSize());
    }
    else if (g->inputExpectAnotherFragment()) {
      inputNewFragment(g);
    }
    else {
      break;
    }
  } while (1);

  g->pd_inb_mkr = g->pd_inb_end;
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputMessageEnd(giopStream* g,CORBA::Boolean disgard) {

  if ( g->pd_strand->state() != giopStrand::DYING ) {

    while (g->inputExpectAnotherFragment() &&
	   g->inputFragmentToCome() == 0   && 
	   g->pd_inb_end == g->pd_inb_mkr     ) {

      // If there are more fragments to come and we do not have any
      // data left in our buffer, we keep fetching the next
      // fragment until one of the conditions is false.
      // This will cater for the case where the remote end is sending
      // the last fragment(s) with 0 body size to indicate the end of
      // a message.
      inputNewFragment(g);
    }

    if (!disgard && inputRemaining(g)) {
      if (omniORB::trace(15)) {
	omniORB::logger l;
	l << "Garbage left at the end of input message from "
	  << g->pd_strand->connection->peeraddress() << "\n";
      }
      if (!orbParameters::strictIIOP) {
	disgard = 1;
      }
      else {
	inputTerminalProtocolError(g, __FILE__, __LINE__,
				   "Garbage left at end of input message");
	// never reach here.
      }
    }

    if (disgard)
      inputSkipWholeMessage(g);

    if (g->pd_currentInputBuffer) {
      g->releaseInputBuffer(g->pd_currentInputBuffer);
      g->pd_currentInputBuffer = 0;
    }
  }

  if (g->pd_rdlocked) {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->rdUnLock();
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::unmarshalReplyHeader(giopStream* g) {

  char* hdr = (char*)g->inputBufferStart();

  if ((GIOP::MsgType)hdr[7] != GIOP::Reply) {
    // Unexpected reply. The other end is terribly confused. Drop the
    // connection and died.
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Incorrect message type when expecting a "
			       "Reply");
    // Never reach here.
  }

  GIOP_C&    giop_c = *((GIOP_C*) g);
  cdrStream& s      = *((cdrStream*)g);

  // We have already verified the request id in the header and the stream
  // have been setup to go pass it

  CORBA::ULong v;
  v <<= s;

  switch (v) {
  case GIOP::SYSTEM_EXCEPTION:
  case GIOP::NO_EXCEPTION:
  case GIOP::USER_EXCEPTION:
  case GIOP::LOCATION_FORWARD:
  case GIOP::LOCATION_FORWARD_PERM:
  case GIOP::NEEDS_ADDRESSING_MODE:
    break;
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Invalid status in Reply");
    // Never reach here.
  }

  giop_c.replyStatus((GIOP::ReplyStatusType)v);

  if (omniInterceptorP::clientReceiveReply) {
    IOP::ServiceContextList sctxts;
    sctxts <<= s;
    omniInterceptors::clientReceiveReply_T::info_T info(giop_c, sctxts);
    omniInterceptorP::visit(info);
  }
  else {
    // Skip service context
    CORBA::ULong svcccount;
    CORBA::ULong svcctag;
    CORBA::ULong svcctxtsize;
    svcccount <<= s;
    while (svcccount-- > 0) {
      svcctag <<= s;
      svcctxtsize <<= s;
      s.skipInput(svcctxtsize);
    }
  }
  if (inputRemaining(g))
    s.alignInput(omni::ALIGN_8);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::unmarshalLocateReply(giopStream* g) {

  char* hdr = (char*)g->inputBufferStart();

  if ((GIOP::MsgType)hdr[7] != GIOP::LocateReply) {
    // Unexpected reply. The other end is terribly confused. Drop the
    // connection and died.
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Incorrect message type when expecting a "
			       "LocateReply");
    // Never reach here.
  }

  GIOP_C&    giop_c = *((GIOP_C*) g);
  cdrStream& s      = *((cdrStream*)g);

  // We have already verified the request id in the header and the stream
  // have been setup to go pass it

  CORBA::ULong v;
  v <<= s;

  switch (v) {
  case GIOP::UNKNOWN_OBJECT:
  case GIOP::OBJECT_HERE:
  case GIOP::OBJECT_FORWARD:
  case GIOP::OBJECT_FORWARD_PERM:
  case GIOP::LOC_SYSTEM_EXCEPTION:
  case GIOP::LOC_NEEDS_ADDRESSING_MODE:
    break;
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Invalid status in LocateReply");
    // never reach here.
  }
  giop_c.locateStatus((GIOP::LocateStatusType)v);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::unmarshalWildCardRequestHeader(giopStream* g) {

  g->inputMatchedId(1);

  char* hdr = (char*)g->inputBufferStart();

  ((GIOP_S*)g)->requestType((GIOP::MsgType)hdr[7]);
  switch (((GIOP_S*)g)->requestType()) {

  case GIOP::Request:
  case GIOP::LocateRequest:
  case GIOP::CancelRequest:
    CORBA::ULong reqid;
    reqid <<= (cdrStream&)(*g);
    ((GIOP_S*)g)->requestId(reqid);
    // HERE: should check if request id has been seen before
    break;

  case GIOP::CloseConnection:
    if (g->pd_strand->isBiDir() && g->pd_strand->isClient()) {
      // proper shutdown of a connection.
      if (omniORB::trace(30)) {
	omniORB::logger l;
	l << "Server has closed a bi-directional connection on strand "
	  << (void*)g->pd_strand << ". Will scavenge it.\n";
      }
      {
	omni_tracedmutex_lock sync(*omniTransportLock);
	g->pd_strand->startIdleCounter();
      }
    }
    inputRaiseCommFailure(g, "Orderly connection shutdown");
    break;

  default:
    inputTerminalProtocolError(g, __FILE__, __LINE__,
			       "Unknown GIOP message type");
    // Never reach here.
    break;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::unmarshalRequestHeader(giopStream* g) {

  GIOP_S&    giop_s = *((GIOP_S*) g);
  cdrStream& s      = *((cdrStream*)g);

  // We have already verified the request id in the header and the stream
  // have been setup to go pass it

  CORBA::ULong vl;
  CORBA::Octet vb;

  // response flag
  vb = s.unmarshalOctet();
  giop_s.response_expected((vb & 0x1));
  giop_s.result_expected((vb & 0x2) ? 1 : 0);

  // padding
  vb = s.unmarshalOctet();
  vb = s.unmarshalOctet();
  vb = s.unmarshalOctet();

  // object key
  giop_s.unmarshalIORAddressingInfo();

  // operation
  vl <<= s;
  if (!vl || !s.checkInputOverrun(1,vl)) {
    OMNIORB_THROW(MARSHAL,MARSHAL_InvalidVariableLenComponentSize,
		  CORBA::COMPLETED_NO);
  }
  giop_s.set_operation_size(vl);
  char* op = giop_s.operation();
  s.get_octet_array((CORBA::Octet*)op,vl);
  op[vl-1] = '\0';

  // Service context
  giop_s.service_contexts() <<= s;

  if (inputRemaining(g))
    s.alignInput(omni::ALIGN_8);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::unmarshalLocateRequest(giopStream* g) {

  GIOP_S&    giop_s = *((GIOP_S*) g);
  cdrStream& s      = *((cdrStream*)g);

  // We have already verified the request id in the header and the stream
  // have been setup to go pass it

  giop_s.unmarshalIORAddressingInfo();
}

////////////////////////////////////////////////////////////////////////
size_t
giopImpl12::inputRemaining(giopStream* g) {

  if (g->inputExpectAnotherFragment()) {
    return orbParameters::giopMaxMsgSize - currentInputPtr(g);
  }
  else {
    return (g->inputFragmentToCome() + (g->inEnd() - g->inMkr()));
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::getInputData(giopStream* g,omni::alignment_t align,size_t sz) {

 again:
  omni::ptr_arith_t last = g->inMkr(align);
  omni::ptr_arith_t end  = last + sz;

  if (end <= g->inEnd()) {
    return;
  }
  else {
    // Invariant check 
    // Either the full message is already in the buffer or the part that is
    // in the buffer always ends at an 8 byte aligned boundary.
    // Also remember that sz is <= 8 always!
    size_t extra = end - g->inEnd();

    if (extra != sz) {
      if (omniORB::trace(25)) {
        omniORB::logger log;
        log << "getInputData requires invalid size " << (unsigned long)extra
            << " bytes to satisfy input size " << (unsigned long)sz << ".\n";
      }
      if (!(g->inputFragmentToCome() || g->inputExpectAnotherFragment()) ) {
	// The full message is already in the buffer. The unmarshalling
	// code is asking for more. This is an error caused by the received
	// data. We'll let the code below to raise a MARSHAL exception
	sz = extra;            // in case sz == 0
      }
      else {
	if (g->inputExpectAnotherFragment()) {
	  // The incoming message is fragmented at the wrong boundary!
	  inputTerminalProtocolError(g, __FILE__, __LINE__,
				     "Message fragmented at incorrect "
				     "boundary");
	  // never reach here
	}
	// Very bad. Should never happen given our invariant.
	{
	  if (omniORB::trace(1)) {
	    omniORB::logger l;
	    l << "Fatal error in unmarshalling message from "
	      << g->pd_strand->connection->peeraddress()
	      << ", invariant was violated at "
	      << __FILE__ 	
	      << ":" << __LINE__ << '\n';
	  }
	}
	OMNIORB_ASSERT(0);
	// never reach here.
      }
    }
  }

  if (g->inputFragmentToCome() < sz && !g->inputExpectAnotherFragment()) {
    OMNIORB_THROW(MARSHAL,MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)g->completion());
  }

  if (!g->inputFragmentToCome()) {
    inputNewFragment(g);
    if (g->inputMessageSize() > orbParameters::giopMaxMsgSize) {
      OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimit,
		    (CORBA::CompletionStatus)g->completion());
    }
    goto again;
  }

  // Reach here if we have some bytes to fetch for the current fragment
  if (g->pd_currentInputBuffer) {
    g->releaseInputBuffer(g->pd_currentInputBuffer);
    g->pd_currentInputBuffer = 0;
  }

  if (!g->pd_input) {
    g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome());
  }
  else {
    g->pd_currentInputBuffer       = g->pd_input;
    g->pd_input                    = g->pd_currentInputBuffer->next;
    g->pd_currentInputBuffer->next = 0;
  }

  g->pd_inb_mkr = (void*)g->inputBufferStart();
  g->pd_inb_end = (void*)g->inputBufferLast();
  g->inputFragmentToCome(g->inputFragmentToCome() -
                         g->inputBufferDataSize());
}


////////////////////////////////////////////////////////////////////////
void
giopImpl12::skipInputData(giopStream* g, size_t sz) {
  copyInputData(g, 0, sz, omni::ALIGN_1);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::copyInputData(giopStream* g, void* b, size_t sz,
			  omni::alignment_t align) {

  // If b == 0, we don't actually copy the data but just skip <sz> bytes.

  omni::ptr_arith_t last = g->inMkr(align);

  if (last > g->inEnd()) {
    // Invariant check 
    // Either the full message is already in the buffer or the part that is
    // in the buffer always ends at an 8 byte aligned boundary.

    if (!(g->inputFragmentToCome() || g->inputExpectAnotherFragment())) {
      // The full message is already in the buffer. The unmarshalling
      // code is asking for more. This is an error caused by the received
      // data.
      OMNIORB_THROW(MARSHAL,MARSHAL_PassEndOfMessage,
		    (CORBA::CompletionStatus)g->completion());
    }
    else {
      if (g->inputExpectAnotherFragment()) {
	// The incoming message is fragmented at the wrong boundary!
	inputTerminalProtocolError(g, __FILE__, __LINE__,
				   "Message fragmented at incorrect "
				   "boundary (bulk copy)");
	// never reach here
      }
      // Very bad. Should never happen given our invariant.
      {
	if (omniORB::trace(1)) {
	  omniORB::logger l;
	  l << "Fatal error in unmarshalling message from "
	    << g->pd_strand->connection->peeraddress()
	    << ", invariant was violated at "
	    << __FILE__ 	
	    << ":" << __LINE__ << '\n';
	}
      }
      OMNIORB_ASSERT(0);
      // never reach here.
    }
  }

  g->pd_inb_mkr = (void*) last;

  while (sz) {
    size_t avail = g->inEnd() - g->inMkr();

    if (avail >= sz)
      avail = sz;

    if (avail) {
      if (b)
        memcpy(b,g->pd_inb_mkr,avail);

      g->pd_inb_mkr = (void*)(g->inMkr() + avail);
    }
    sz -= avail;

    if (!sz)
      break;

    if (b)
      b = (void*)((omni::ptr_arith_t)b + avail);

    if (g->pd_inb_mkr == g->pd_inb_end) {

      if (g->inputFragmentToCome() < sz && !g->inputExpectAnotherFragment()) {
	OMNIORB_THROW(MARSHAL,MARSHAL_PassEndOfMessage,
		      (CORBA::CompletionStatus)g->completion());
	// never reach here
      }

      if (!g->inputFragmentToCome()) {
	inputNewFragment(g);
	if (g->inputMessageSize() > orbParameters::giopMaxMsgSize) {
	  OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimit,
			(CORBA::CompletionStatus)g->completion());
	}
	continue;
      }

      if (g->pd_currentInputBuffer) {
	g->releaseInputBuffer(g->pd_currentInputBuffer);
	g->pd_currentInputBuffer = 0;
      }
      if (g->pd_input) {
	g->pd_currentInputBuffer       = g->pd_input;
	g->pd_input                    = g->pd_currentInputBuffer->next;
	g->pd_currentInputBuffer->next = 0;
      }
      else {
	if (b && sz >= giopStream::directReceiveCutOff) {

	  CORBA::ULong transz = g->inputFragmentToCome();
	  if (transz > sz)
            transz = sz;

	  transz = (transz >> 3) << 3;

	  g->inputCopyChunk(b, transz);

	  sz -= transz;
	  b = (void*)((omni::ptr_arith_t)b + transz);
	  g->inputFragmentToCome(g->inputFragmentToCome() - transz);
	  continue;
	}
	else {
	  g->pd_currentInputBuffer = g->inputChunk(g->inputFragmentToCome());
	}
      }

      g->pd_inb_mkr = (void*)g->inputBufferStart();
      g->pd_inb_end = (void*)g->inputBufferLast();
      g->inputFragmentToCome(g->inputFragmentToCome() -
                             g->inputBufferDataSize());
    }
  }	
}

////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopImpl12::currentInputPtr(const giopStream* g) {

  return  g->inputMessageSize() - 
          g->inputFragmentToCome() -
         ((omni::ptr_arith_t) g->pd_inb_end - 
	  (omni::ptr_arith_t) g->pd_inb_mkr);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputTerminalProtocolError(giopStream* g,
				       const char* file, int line,
				       const char* message)
{
  // XXX We may choose to send a message error to the other end.
  if (omniORB::trace(1)) {
    omniORB::logger l;
    l << "From endpoint: " << g->pd_strand->connection->peeraddress()
      << ". Detected GIOP 1.2 protocol error in input message. "
      << omniExHelper::strip(file) << ":" << line
      << ". Connection is closed.\n";
  }

  inputRaiseCommFailure(g, message);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::inputRaiseCommFailure(giopStream* g, const char* message) {

  CORBA::ULong minor;
  CORBA::Boolean retry;
  g->notifyCommFailure(0,minor,retry);
  g->pd_strand->state(giopStrand::DYING);

  giopStream::CommFailure::_raise(minor,
				  (CORBA::CompletionStatus)g->completion(),
				  0,__FILE__,__LINE__,message,g->pd_strand);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::outputNewMessage(giopStream* g) {

  if (!g->pd_wrlocked) {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->wrLock();
  }

  if (!g->pd_currentOutputBuffer) {
    g->pd_currentOutputBuffer = giopStream_Buffer::newBuffer();
  }
  g->pd_currentOutputBuffer->alignStart(omni::ALIGN_8);

  char* hdr = (char*)g->outputBufferStart();

  hdr[0] = 'G'; hdr[1] = 'I'; hdr[2] = 'O'; hdr[3] = 'P';
  hdr[4] = 1;   hdr[5] = 2;   hdr[6] = _OMNIORB_HOST_BYTE_ORDER_;

  g->pd_outb_mkr = (void*)(hdr + 12);
  g->pd_outb_end = (void*)g->outputBufferEnd();
  g->outputFragmentSize(0);
  g->outputMessageSize(0);
  *((CORBA::ULong*)(hdr + 8)) = 0xffffffff;
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::outputMessageBegin(giopStream* g,
			       void (*marshalHeader)(giopStream*)) {

  outputNewMessage(g);
  marshalHeader(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::outputMessageEnd(giopStream* g) {

  if (g->pd_currentOutputBuffer) {

    omni::ptr_arith_t outbuf_begin = g->outputBufferStart();

    if (outbuf_begin != g->outMkr()) {

      if (!g->outputFragmentSize()) {

	CORBA::ULong sz = g->bufferedOutputSize() - 12;

	if (!g->outputMessageSize()) {
	  // The whole message is still in the buffer, we have to check
	  // if any arguments have been marshalled after the header.
	  // If there is none, we have to remove the paddings at the end
	  // of the header.
	  // The header's size is stored in the buffer
	  CORBA::ULong hdrsz = *((CORBA::ULong*)(outbuf_begin + 8));
	  OMNIORB_ASSERT(hdrsz != 0xffffffff);

	  if (omni::align_to(hdrsz,omni::ALIGN_8) ==
              (omni::ptr_arith_t)(sz+12)) {

	    // No argument has been marshalled, remove the padding
	    sz = hdrsz - 12;
	    g->pd_outb_mkr = (void*)(outbuf_begin + hdrsz);
	  }
	}
	*((CORBA::ULong*)(outbuf_begin + 8)) = sz;
      }

      g->setOutputLastOffset();
      g->sendChunk(g->pd_currentOutputBuffer);
    }
    // Notice that we do not release the buffer. Next time this giopStream
    // is re-used, the buffer will be reused as well.
  }

  {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->wrUnLock();
  }

}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendMsgErrorMessage(giopStream* g,
				const CORBA::SystemException* ex) {

  if (!g->pd_wrlocked) {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->wrLock();
  }


  if (omniORB::trace(1)) {
    omniORB::logger l;

    l << "To endpoint: " << g->pd_strand->connection->peeraddress() << ". ";

    if (ex) {
      l << "System exception " << *ex << " while (un)marshalling. "
	<< "Send GIOP 1.2 MessageError.\n";
    }
    else {
      l << "Send GIOP 1.2 MessageError because a protocol error has "
	<< "been detected. Connection is closed.\n";
    }
  }

  if (!g->pd_currentOutputBuffer) {
    g->pd_currentOutputBuffer = giopStream_Buffer::newBuffer();
  }
  g->pd_currentOutputBuffer->alignStart(omni::ALIGN_8);

  char* hdr = (char*)g->outputBufferStart();

  hdr[0] = 'G'; hdr[1] = 'I'; hdr[2] = 'O'; hdr[3] = 'P';
  hdr[4] = 1;   hdr[5] = 2;   hdr[6] = _OMNIORB_HOST_BYTE_ORDER_;

  g->pd_outb_mkr = (void*)(hdr + 12);
  g->pd_outb_end = (void*)g->outputBufferEnd();
  g->outputFragmentSize(0);
  g->outputMessageSize(0);

  hdr[7] = (char)GIOP::MessageError;
  hdr[8] = hdr[9] = hdr[10] = hdr[11] = 0;

  g->pd_strand->connection->Send(hdr, 12, g->getDeadline());

  g->pd_strand->state(giopStrand::DYING);

  {
    omni_tracedmutex_lock sync(*omniTransportLock);
    g->wrUnLock();
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::marshalRequestHeader(giopStream* g) {

  char* hdr = (char*)g->outputBufferStart();

  hdr[7] = (char) GIOP::Request;

  GIOP_C&    giop_c = *(GIOP_C*)g;
  cdrStream& s      = (cdrStream&) *g;

  omniCallDescriptor& calldesc          = *giop_c.calldescriptor();
  CORBA::Boolean      response_expected = !calldesc.is_oneway();
  
  omniInterceptors::clientSendRequest_T::info_T info(giop_c);
  omniInterceptorP::visit(info);

  CORBA::Octet v = 0;
  CORBA::ULong sc_len = info.service_contexts.length();

  if (sc_len == 0 && giop_c.ior()->addr_mode() == GIOP::KeyAddr) {

    // Simple fast case to calculate request header size
    //
    // GIOP header (12) + req_id (4) + flags (1) + pad(3) +
    // KeyAddr(2) + pad(2) + key_len (4) + key (n) + pad (<=3) +
    // op_len (4) + op (n) + pad (<=3) + service_ctx list len (4)

    CORBA::ULong total;
    total = 12 + 4 + 1 + 3 + 2 + 2 + 4 + giop_c.keysize();
    total = (total + 3) & ~((CORBA::ULong)3);
    total = (total + 4 + calldesc.op_len() + 3) & ~((CORBA::ULong)3);
    total = total + 4;

    *((CORBA::ULong*)(hdr+8)) = total;
  }
  else {
    // Calculate the request header size
    cdrCountingStream cs(g->TCS_C(), g->TCS_W(), 12 + 4 + 1 + 3);

    if (giop_c.ior()->addr_mode() == GIOP::KeyAddr) {
      GIOP::KeyAddr >>= cs;
      giop_c.keysize() >>= cs;
      cs.put_octet_array(giop_c.key(),giop_c.keysize());
    }
    else {
      giop_c.ior()->marshalIORAddressingInfo(cs);
    }

    operator>>= ((CORBA::ULong) calldesc.op_len(),cs);
    cs.put_octet_array((CORBA::Octet*) calldesc.op(), calldesc.op_len());
    info.service_contexts >>= cs;

    *((CORBA::ULong*)(hdr+8)) = cs.total();
  }

  // request id
  giop_c.requestId() >>= s;

  // response flags
  v = ((response_expected << 1) + response_expected);
  s.marshalOctet(v);

  // reserved[3]
  v = 0;
  s.marshalOctet(v);
  s.marshalOctet(v);
  s.marshalOctet(v);

  // Target address
  if (giop_c.ior()->addr_mode() == GIOP::KeyAddr) {
    GIOP::KeyAddr >>= s;
    giop_c.keysize() >>= s;
    s.put_small_octet_array(giop_c.key(),giop_c.keysize());
  }
  else {
    giop_c.ior()->marshalIORAddressingInfo(s);
  }

  // operation
  operator>>= ((CORBA::ULong) calldesc.op_len(),s);
  s.put_small_octet_array((CORBA::Octet*) calldesc.op(), calldesc.op_len());

  // service context
  if (sc_len)
    info.service_contexts >>= s;
  else
    CORBA::ULong(0) >>= s;

  s.alignOutput(omni::ALIGN_8);
}


////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendLocateRequest(giopStream* g) {

  outputNewMessage(g);

  char* hdr = (char*)g->outputBufferStart();

  hdr[7] = (char)GIOP::LocateRequest;

  GIOP_C& giop_c = *(GIOP_C*)g;
  cdrStream& s = (cdrStream&) *g;

  // Compute and initialise the message size field
  {
    cdrCountingStream cs(g->TCS_C(), g->TCS_W(), 12);
    operator>>= ((CORBA::ULong)0,cs);
    giop_c.ior()->marshalIORAddressingInfo(cs);
    outputSetFragmentSize(g,cs.total()-12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }

  // request id
  giop_c.requestId() >>= s;
  
  // object key
  giop_c.ior()->marshalIORAddressingInfo(s);

  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::marshalReplyHeader(giopStream* g) {

  char* hdr = (char*)g->outputBufferStart();

  hdr[7] = (char) GIOP::Reply;

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  CORBA::ULong sc_len = giop_s.service_contexts().length();
  CORBA::ULong rc = GIOP::NO_EXCEPTION;
  if (sc_len == 0) {
    CORBA::ULong total = 12 + 4 + 4 + 4;
    *((CORBA::ULong*)(hdr+8)) = total;
  }
  else {
    // calculate the reply header size
    cdrCountingStream cs(g->TCS_C(), g->TCS_W(), 12);
    operator>>= ((CORBA::ULong)0, cs);
    rc >>= cs;
    giop_s.service_contexts() >>= cs;
    *((CORBA::ULong*)(hdr+8)) = cs.total();
  }

  // request id
  giop_s.requestId() >>= s;

  // reply status
  rc >>= s;

  // Service context
  if (sc_len)
    giop_s.service_contexts() >>= s;
  else
    CORBA::ULong(0) >>= s;

  s.alignOutput(omni::ALIGN_8);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendSystemException(giopStream* g,const CORBA::SystemException& ex)
{
  GIOP_S&    giop_s = *(GIOP_S*)g;
  cdrStream& s      = (cdrStream&) *g;

  if (giop_s.state() == GIOP_S::ReplyIsBeingComposed) {
    // This system exception is raised during the marshalling of the reply.
    // We cannot marshal the exception. Can only indicate that something
    // fatal about this request.
    sendMsgErrorMessage(g, &ex);

    CORBA::ULong minor;
    CORBA::Boolean retry;
    giop_s.notifyCommFailure(0,minor,retry);
    giopStream::CommFailure::_raise(minor,(CORBA::CompletionStatus)
				    giop_s.completion(),
				    retry,__FILE__,__LINE__,
				    "System Exception occurred while "
				    "marshalling reply. Sending a "
				    "MessageError", g->pd_strand);
  }

  giop_s.state(GIOP_S::ReplyIsBeingComposed);

  int repoid_size;
  const char* repoid = ex._NP_repoId(&repoid_size);

  outputNewMessage(g);

  char* hdr = (char*)g->outputBufferStart();

  hdr[7] = (char) GIOP::Reply;

  giop_s.service_contexts().length(0);

  if (omniInterceptorP::serverSendException) {
    omniInterceptors::serverSendException_T::info_T info(giop_s, &ex);
    omniInterceptorP::visit(info);

    if (giop_s.service_contexts().length() > 0) {
      // Compute and initialise the message size field. Only necessary
      // if there are service contexts, since we know a message without
      // service contexts will fit in a single buffer.

      cdrCountingStream cs(g->TCS_C(), g->TCS_W(), 12);
      operator>>= ((CORBA::ULong)0,cs);
      operator>>= ((CORBA::ULong)0,cs);
      giop_s.service_contexts() >>= cs;
      cs.alignOutput(omni::ALIGN_8);

      CORBA::ULong(repoid_size) >>= cs;
      cs.put_octet_array((const CORBA::Octet*) repoid, repoid_size);
      ex.minor() >>= cs;
      operator>>= ((CORBA::ULong)0,cs);

      outputSetFragmentSize(g, cs.total() - 12);
      *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
    }
  }

  // request id
  giop_s.requestId() >>= s;

  // reply status
  CORBA::ULong rc = GIOP::SYSTEM_EXCEPTION;
  rc >>= s;

  // service contexts
  giop_s.service_contexts() >>= s;

  // End of reply header

  if (giop_s.service_contexts().length() == 0){
    // Set header size in buffer
    CORBA::ULong sz = ((omni::ptr_arith_t)g->pd_outb_mkr - 
		       (omni::ptr_arith_t)hdr - 12);
    *((CORBA::ULong*)(hdr + 8)) = sz;
  }

  s.alignOutput(omni::ALIGN_8);

  // RepoId
  CORBA::ULong(repoid_size) >>= s;
  s.put_small_octet_array((const CORBA::Octet*) repoid, repoid_size);

  // system exception value
  ex.minor() >>= s;

  CORBA::ULong(ex.completed()) >>= s;

  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendUserException(giopStream* g,const CORBA::UserException& ex) {

  GIOP_S&    giop_s = *(GIOP_S*)g;
  cdrStream& s      = (cdrStream&) *g;

  giop_s.state(GIOP_S::ReplyIsBeingComposed);

  int repoid_size;
  const char* repoid = ex._NP_repoId(&repoid_size);

  outputNewMessage(g);

  char* hdr = (char*)g->outputBufferStart();

  hdr[7] = (char)GIOP::Reply;

  giop_s.service_contexts().length(0);

  if (omniInterceptorP::serverSendException) {
    omniInterceptors::serverSendException_T::info_T info(giop_s, &ex);
    omniInterceptorP::visit(info);
  }

  // user exception value

  // Compute and initialise the message size field
  {
    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),12);
    operator>>= ((CORBA::ULong)0,cs);
    operator>>= ((CORBA::ULong)0,cs);
    giop_s.service_contexts() >>= cs;
    cs.alignOutput(omni::ALIGN_8);

    CORBA::ULong(repoid_size) >>= cs;
    cs.put_octet_array((const CORBA::Octet*) repoid, repoid_size);
    ex._NP_marshal(cs);

    outputSetFragmentSize(g, cs.total() - 12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }

  // request id
  giop_s.requestId() >>= s;

  // reply status
  CORBA::ULong rc = GIOP::USER_EXCEPTION;
  rc >>= s;

  // Service context
  giop_s.service_contexts() >>= s;

  s.alignOutput(omni::ALIGN_8);

  // RepoId
  CORBA::ULong(repoid_size) >>= s;
  s.put_small_octet_array((const CORBA::Octet*) repoid, repoid_size);

  // Exception value
  ex._NP_marshal(s);

  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendLocationForwardReply(giopStream* g,CORBA::Object_ptr obj,
				     CORBA::Boolean permanent) {

  outputNewMessage(g);
  
  char* hdr = (char*)g->outputBufferStart();

  hdr[7] = (char)GIOP::Reply;

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  giop_s.state(GIOP_S::ReplyIsBeingComposed);

  // Compute and initialise the message size field
  {
    cdrCountingStream cs(g->TCS_C(),g->TCS_W(),12);
    operator>>= ((CORBA::ULong)0,cs);
    operator>>= ((CORBA::ULong)0,cs);
    operator>>= ((CORBA::ULong)0,cs);
    CORBA::Object::_marshalObjRef(obj,cs);
    outputSetFragmentSize(g,cs.total()-12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }

  // request id
  giop_s.requestId() >>= s;

  // reply status
  CORBA::ULong rc = (permanent ? GIOP::LOCATION_FORWARD_PERM : 
                                 GIOP::LOCATION_FORWARD);
  rc >>= s;

  // Service context
  operator>>= ((CORBA::ULong)0,s);


  // object reference
  CORBA::Object::_marshalObjRef(obj,s);
  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::sendLocateReply(giopStream* g,GIOP::LocateStatusType rc,
			    CORBA::Object_ptr obj,CORBA::SystemException* p) {

  outputNewMessage(g);

  char* hdr = (char*)g->outputBufferStart();

  hdr[7] = (char)GIOP::LocateReply;

  GIOP_S& giop_s = *(GIOP_S*)g;
  cdrStream& s = (cdrStream&) *g;

  giop_s.state(GIOP_S::ReplyIsBeingComposed);

  CORBA::Object_ptr extra = CORBA::Object::_nil();

  // reply status
  switch (rc) {
  case GIOP::OBJECT_FORWARD:
  case GIOP::OBJECT_FORWARD_PERM:
    extra = obj;
    break;
  case GIOP::LOC_SYSTEM_EXCEPTION:
  case GIOP::UNKNOWN_OBJECT:
  case GIOP::OBJECT_HERE:
    break;
  case GIOP::LOC_NEEDS_ADDRESSING_MODE:
    // We never use this!
  default:
    // Never reach here.
    OMNIORB_ASSERT(0);
    break;
  }

  int         repoid_size = 0;
  const char* repoid      = 0;
  
  // Compute and initialise the message size field
  {
    cdrCountingStream cs(g->TCS_C(), g->TCS_W(), 12);
    operator>>= ((CORBA::ULong)0,cs);
    operator>>= ((CORBA::ULong)0,cs);
    if (!CORBA::is_nil(extra)) {
      CORBA::Object::_marshalObjRef(extra,cs);
    }
    else if (rc == GIOP::LOC_SYSTEM_EXCEPTION) {
      repoid = p->_NP_repoId(&repoid_size);
      operator>>= ((CORBA::ULong)0,cs);
      cs.put_octet_array((const CORBA::Octet*) repoid, repoid_size);
      operator>>= ((CORBA::ULong)0,cs);
      operator>>= ((CORBA::ULong)0,cs);
    }

    outputSetFragmentSize(g, cs.total() - 12);
    *((CORBA::ULong*)(hdr + 8)) = cs.total() - 12;
  }

  // request id
  giop_s.requestId() >>= s;

  operator>>= ((CORBA::ULong)rc,s);

  if (!CORBA::is_nil(extra)) {
    CORBA::Object::_marshalObjRef(extra,s);
  }
  else if (rc == GIOP::LOC_SYSTEM_EXCEPTION) {
    CORBA::ULong(repoid_size) >>= s;
    s.put_small_octet_array((const CORBA::Octet*) repoid, repoid_size);
    p->minor() >>= s;
    operator>>= ((CORBA::ULong)p->completed(),s);
  }

  outputMessageEnd(g);
}

////////////////////////////////////////////////////////////////////////
size_t
giopImpl12::outputRemaining(const giopStream* g) {

  CORBA::ULong total = g->outputFragmentSize();
  if (!total) {
    CORBA::ULong avail = (CORBA::Long)orbParameters::giopMaxMsgSize -
                         (CORBA::Long)currentOutputPtr(g);
    
    // Adjust avail to exactly the same value as calculated in outputFlush().
    // See the comment in outputFlush() for the reason why.
    avail = ((avail + 4) >> 3) << 3;
    avail = (avail ? avail - 4 : 0);
    return avail;
  }
  else {
    return total - currentOutputPtr(g);
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::outputFlush(giopStream* g, CORBA::Boolean knownFragmentSize) {

  // Note: g->outputFragmentSize() != 0 implies that the full message
  //       size has been pre-calculated and no GIOP Fragment should be
  //       sent! This also means that the message size limit has been
  //       checked and there is no need to check against
  //       orbParameters::giopMaxMsgSize.

  omni::ptr_arith_t outbuf_begin = g->outputBufferStart();

  CORBA::ULong fsz = g->outMkr() - outbuf_begin;

  if (!g->outputFragmentSize()) {
    ((char*)outbuf_begin)[6] |= 0x2;
    CORBA::ULong sz;
    if (!knownFragmentSize) {
      sz = fsz - 12;
      *((CORBA::ULong*)((omni::ptr_arith_t)outbuf_begin + 8)) = sz;
    }
    else {
      sz = *((CORBA::ULong*)((omni::ptr_arith_t)outbuf_begin + 8));
    }
    CORBA::Long msz = g->outputMessageSize();
    if (msz) {
      // Message has increased by 4 bytes less than the fragment size,
      // due to the message id at the start of the fragment.
      g->outputMessageSize(msz + sz - 4);
    }
    else {
      g->outputMessageSize(sz);
    }
    if (g->outputMessageSize() > orbParameters::giopMaxMsgSize) {
      OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnClient,
		    (CORBA::CompletionStatus)g->completion());
    }
  }
  else {
    CORBA::Long msz = g->outputMessageSize();
    if (msz) {
      g->outputMessageSize(msz+fsz-16);
    }
    else {
      g->outputMessageSize(fsz-12);
    }
  }

  g->pd_currentOutputBuffer->last = g->pd_currentOutputBuffer->start + fsz;
  g->sendChunk(g->pd_currentOutputBuffer);

  if (outbuf_begin & 0x7) {
    // start has previously been changed to non 8-bytes aligned
    g->pd_currentOutputBuffer->alignStart(omni::ALIGN_8);
    outbuf_begin = g->outputBufferStart();
  }

  g->pd_outb_mkr = (void*)outbuf_begin;
  g->pd_outb_end = (void*)g->outputBufferEnd();

  g->pd_currentOutputBuffer->last = g->pd_currentOutputBuffer->start;

  if (!g->outputFragmentSize()) {
    char* hdr = (char*) outbuf_begin;

    hdr[0] = 'G'; hdr[1] = 'I'; hdr[2] = 'O'; hdr[3] = 'P';
    hdr[4] = 1;   hdr[5] = 2;   hdr[6] = _OMNIORB_HOST_BYTE_ORDER_;
    hdr[7] = (char)GIOP::Fragment;
    *((CORBA::ULong*)(hdr + 12)) = g->requestId();
    g->pd_outb_mkr = (void*)(outbuf_begin + 16);

    // Now determine how much space we have left.
    // If the message size has already reached orbParameters::giopMaxMsgSize,
    // outputHasReachedLimit() will return TRUE.
    CORBA::ULong avail = orbParameters::giopMaxMsgSize - g->outputMessageSize();

    // Adjust avail to make sure that it a multiple of 8.
    // This preserves our invariant: g->pd_outb_end always align on 8 bytes
    // boundary.
    avail = ((avail + 7) >> 3) << 3;

    omni::ptr_arith_t newmkr = g->outMkr() + avail;

    // If the new position is inside the buffer, set the end pointer.
    // Note that if avail is very large, newmkr may wrap around and be
    // < pd_outb_mkr.

    if ((newmkr >= g->outMkr() && newmkr < g->outEnd())) {
      g->pd_outb_end = (void*) newmkr;
    }
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::getReserveSpace(giopStream* g,omni::alignment_t align,size_t sz) {

  // The caller has already checked that align == sz, or sz == 0.

  if (outputHasReachedLimit(g)) {
    // Already reached the message size limit
    goto overflow;
  }

  g->pd_outb_mkr = (void*)g->outMkr(align);

  if (sz == 0)
    return;

  if (g->outMkr() < g->outEnd()) {
    omni::ptr_arith_t newmkr = g->outMkr() + sz;
    if (newmkr <= g->outEnd())
      return;
    // Should never happen!!
    OMNIORB_ASSERT(0);
  }

  // Reach here only if the buffer has been filled up completely.

  outputFlush(g);
  if (!outputHasReachedLimit(g))
    return;

 overflow:
    OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnClient,
		  (CORBA::CompletionStatus)g->completion());
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::copyOutputData(giopStream* g, void* b, size_t sz,
			   omni::alignment_t align) {
  
  if (outputHasReachedLimit(g)) {
    // Already reached the message size limit
    OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnClient,
		  (CORBA::CompletionStatus)g->completion());
  }

  omni::ptr_arith_t newmkr = g->outMkr(align);
  OMNIORB_ASSERT(newmkr <= g->outEnd());
  
  g->pd_outb_mkr = (void*)newmkr;

  if (sz >= giopStream::directSendCutOff && !g->pd_strand->compressor) {

    omni::ptr_arith_t outbuf_begin = g->outputBufferStart();

    if (newmkr - outbuf_begin < giopStream::minChunkBeforeDirectSend) {
      // Copy some of the data into the buffer, to prevent
      // transmission of a small chunk.
      size_t current = newmkr - outbuf_begin;
      size_t avail   = g->outEnd() - newmkr;
      size_t filler  = giopStream::minChunkBeforeDirectSend - current;
      if (filler > avail)
        filler = avail;

      memcpy(g->pd_outb_mkr, b, filler);
      sz -= filler;
      g->pd_outb_mkr = (void*)(g->outMkr() + filler);
      b = (void*)((omni::ptr_arith_t)b + filler);
      newmkr = g->outMkr();
    }

    // The fragment including this vector of bytes must end on a 8 byte
    // boundary. Therefore we may have to leave behind 0-7 bytes in the
    // next fragment.

    size_t leftover = (newmkr + sz) & 0x7;

    if (!g->outputFragmentSize()) {
      CORBA::ULong fsz = g->outMkr() - outbuf_begin + sz - leftover - 12;
      *((CORBA::ULong*)((omni::ptr_arith_t)outbuf_begin + 8)) = fsz;
    }

    outputFlush(g,1);

    g->sendCopyChunk(b, sz - leftover);

    if (leftover) {
      if (outputHasReachedLimit(g)) {
	// Already reached the message size limit
	OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnClient,
		      (CORBA::CompletionStatus)g->completion());
      }
      b = (void*) ((omni::ptr_arith_t)b + sz - leftover);
    }
    sz = leftover;
  }

  while (sz) {
    size_t avail = g->outEnd() - g->outMkr();

    if (avail > sz)
      avail = sz;

    memcpy(g->pd_outb_mkr, b, avail);
    sz -= avail;

    g->pd_outb_mkr = (void*)(g->outMkr() + avail);

    b = (void*)((omni::ptr_arith_t) b + avail);

    if (g->pd_outb_mkr == g->pd_outb_end) {
      outputFlush(g);
      if (sz && outputHasReachedLimit(g)) {
	// Already reached the message size limit
	OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnClient,
		      (CORBA::CompletionStatus)g->completion());
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////
CORBA::ULong
giopImpl12::currentOutputPtr(const giopStream* g) {

  CORBA::ULong fsz = (omni::ptr_arith_t) g->pd_outb_mkr - 
                     ((omni::ptr_arith_t) g->pd_currentOutputBuffer + 
		      g->pd_currentOutputBuffer->start);

  // Output offset is the sent message so far, plus the current buffer
  // minus 12 byte header.
  CORBA::Long msz = g->outputMessageSize();
  if (msz) {
    // At least one fragment has been output already; message has
    // been extended by the current fragment size minus its 16 byte
    // header (12 for GIOP header, plus 4 for request id).
    return msz + fsz - 16;
  }
  else {
    // This is the first fragment -- message size is the fragment
    // size minus 12 byte GIOP header.
    return fsz - 12;
  }
}

////////////////////////////////////////////////////////////////////////
void
giopImpl12::outputSetFragmentSize(giopStream* g,CORBA::ULong msz) {

  if (msz > orbParameters::giopMaxMsgSize) {
    OMNIORB_THROW(MARSHAL,MARSHAL_MessageSizeExceedLimitOnClient,
		  (CORBA::CompletionStatus)g->completion());
  }
  g->outputFragmentSize(msz);
}

////////////////////////////////////////////////////////////////////////
static giopStreamImpl* giop_1_2_singleton = 0;

class omni_giopImpl12_initialiser : public omniInitialiser {
public:

  void attach() {
    if (!giop_1_2_singleton) {
      GIOP::Version ver = { 1, 2};
      giopStreamImpl* p;
      giop_1_2_singleton = p = new giopStreamImpl(ver);
      
      // Shared by the client and server side
      // Process message header
      p->outputMessageBegin             = giopImpl12::outputMessageBegin;
      p->outputMessageEnd               = giopImpl12::outputMessageEnd;
      p->inputMessageBegin              = giopImpl12::inputMessageBegin;
      p->inputMessageEnd                = giopImpl12::inputMessageEnd;
      p->sendMsgErrorMessage            = giopImpl12::sendMsgErrorMessage;

      // Client side
      // Process message header
      p->marshalRequestHeader           = giopImpl12::marshalRequestHeader;
      p->sendLocateRequest              = giopImpl12::sendLocateRequest;
      p->unmarshalReplyHeader           = giopImpl12::unmarshalReplyHeader;
      p->unmarshalLocateReply           = giopImpl12::unmarshalLocateReply;

      // Server side
      // Process message header
      p->unmarshalWildCardRequestHeader = giopImpl12::unmarshalWildCardRequestHeader;
      p->unmarshalRequestHeader         = giopImpl12::unmarshalRequestHeader;
      p->unmarshalLocateRequest         = giopImpl12::unmarshalLocateRequest;
      p->marshalReplyHeader             = giopImpl12::marshalReplyHeader;
      p->sendSystemException            = giopImpl12::sendSystemException;
      p->sendUserException              = giopImpl12::sendUserException;
      p->sendLocationForwardReply       = giopImpl12::sendLocationForwardReply;
      p->sendLocateReply                = giopImpl12::sendLocateReply;


      // Shared by the client and the server side
      // Process message body
      p->inputRemaining                 = giopImpl12::inputRemaining;
      p->getInputData                   = giopImpl12::getInputData;
      p->skipInputData                  = giopImpl12::skipInputData;
      p->copyInputData                  = giopImpl12::copyInputData;
      p->outputRemaining                = giopImpl12::outputRemaining;
      p->getReserveSpace                = giopImpl12::getReserveSpace;
      p->copyOutputData                 = giopImpl12::copyOutputData;
      p->currentInputPtr                = giopImpl12::currentInputPtr;
      p->currentOutputPtr               = giopImpl12::currentOutputPtr;

      giopStreamImpl::registerImpl(giop_1_2_singleton);
    }
  }

  void detach() { 
    if (giop_1_2_singleton) {
      delete giop_1_2_singleton;
      giop_1_2_singleton = 0;
    }
  }
};

static omni_giopImpl12_initialiser initialiser;

omniInitialiser& omni_giopImpl12_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
