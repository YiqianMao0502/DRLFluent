// -*- Mode: C++; -*-
//                            Package   : omniORB2
// IOP_S.h                    Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
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

#ifndef __IOP_S_H__
#define __IOP_S_H__

class omniCallDescriptor;

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class IOP_S {
public:

  IOP_S() {}
  virtual ~IOP_S() {}

  virtual cdrStream& getStream() = 0;

  virtual const char* operation_name() const = 0;

  enum State { UnUsed,
	       Idle,
	       Zombie,
	       WaitForRequestHeader,
	       RequestHeaderIsBeingProcessed,
	       RequestIsBeingProcessed,
	       WaitingForReply,
	       ReplyIsBeingComposed,
	       ReplyCompleted,
	       InputFullyBuffered,
	       InputPartiallyBuffered
  };

  // A IOP_S instance operates in one of the following states:
  //
  //      Idle                        - no request has been received
  //      RequestIsBeingProcessed     - a request is being processed, arguments
  //                                    can be unmarshalled from the
  //                                    cdrStream
  //      WaitingForReply             - An upcall has been dispatched into
  //                                    the object implementation
  //      ReplyIsBeingComposed        - a reply is being composed, results
  //                                    can be marshalled into the
  //                                    cdrStream
  //      Zombie                      - the destructor has been called
  //
  // Calling the member functions RequestReceived(), InitialiseReply() and
  // ReplyCompleted() cause the IOP_S to change state. The state transitions
  // are as follows:
  //
  //					+------------+
  //					|	     |  ~IOP_S()
  //				   +----+---+	     |
  //				   | Zombie |<-------+
  //				   +--------+
  //                                    ^
  //   	       	       	       	       	| ~IOP_S()
  //	      pd_response_expected &&   |
  //          ReplyCompleted()     +----+---+  	   <dispatcher() got a valid
  //             +---------------->|  Idle  |---------------+  request>
  //             |                 +--------+               |
  //             |                      ^                   V
  //  +-----------------------+         |         +--------------------------+
  //  | ReplyIsBeingComposed  |         |         |  RequestIsBeingProcessed |
  //  +-----------------------+         |         +-----------------+--------+
  //		 ^                      | !pd_response_expected &&  |
  //             |                      | ReplyCompleted()          |
  //             |         +--------------------+                   |
  //             +---------|  WaitingForReply   |<------------------+
  //     InitialiseReply() +--------------------+     RequestReceived()
  //
  //
  // The dispatcher() encapsulates the upcall to the stub level dispatcher
  // with a try block that catches all system exceptions. The exception
  // handlers deal with an exception according to the state of the IOP_S
  // at the time:
  //
  //    1. If a system exception is caught when arguments are unmarshalled or
  //       results are marshalled, i.e. the IOP_S is either in
  //         RequestIsBeingProcessed or ReplyIsBeingComposed,
  //       something is seriously wrong in the marshalling or unmarshalling
  //       and cannot recover without destroying the strand. The destructor
  //       is called with the state unchanged.
  //
  //    2. If a system exception is caught while the strand is in
  //       WaitingForReply, the dispatcher() would completed internally the
  //       reply sequence and propagate the exception back to the client.
  //
  // The destructor examines the state, if it is *not* in Idle, there is
  // something seriously wrong with the cdrStream. Also the state
  // of the strand, on which the cdrStream is built, is unknown.
  // For this reason, the strand is marked as dying by the destructor. The
  // destructor may be called multiple times. It simply returns if the instance
  // is already in Zombie state.
  //
  // Calling the member functions RequestReceived(), InitialiseReply() and
  // ReplyCompleted() while the IOP_S is not in the appropriate state as
  // indicated above would cause a CORBA::INTERNAL() exception to be raised.
  //

  virtual void ReceiveRequest(omniCallDescriptor&) = 0;

  virtual void SkipRequestBody() = 0;

  virtual void SendReply() = 0;

  virtual void SendException(CORBA::Exception* value) = 0;

private:
  IOP_S(const IOP_S&);
  IOP_S& operator=(const IOP_S&);

  operator cdrStream& ();
  // This is to make sure that we do not have any code that blindly cast
  // a reference to this object to a cdrStream&. All code should use
  // getStream() instead.
};

OMNI_NAMESPACE_END(omni)

#endif // __IOP_S_H__
