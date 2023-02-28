// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopRendezvouser.cc        Created on: 20 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2005 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
#include <invoker.h>
#include <giopServer.h>
#include <giopRendezvouser.h>

OMNI_NAMESPACE_BEGIN(omni)

void
giopRendezvouser::notifyReadable(void* this_,giopConnection* conn) {

  giopRendezvouser* r = (giopRendezvouser*)this_;
  r->pd_server->notifyRzReadable(conn);
}

void
giopRendezvouser::execute()
{
  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "giopRendezvouser task execute for "
      << pd_endpoint->address() << "\n";
  }

  CORBA::Boolean exit_on_error;

  do {
    exit_on_error = 0;
    giopConnection* newconn = 0;
    try {
      newconn = pd_endpoint->AcceptAndMonitor(notifyReadable,this);
      if (newconn) {
	pd_server->notifyRzNewConnection(this,newconn);
      }
      else {
	exit_on_error = 1;
	break;
      }
    }
    catch(const giopServer::outOfResource&) {
      // giopServer has consumed the connection.
    }
    catch(const giopServer::Terminate&) {
      newconn->decrRefCount(1);
      break;
    }
    catch(...) {
      // Catch all unexpected error conditions. Reach here means that we
      // should not continue!
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "Unexpected exception caught by giopRendezvouser\n";
      }
      if (newconn) {
	newconn->decrRefCount(1);
      }
      exit_on_error = 1;
      break;
    }
  } while(!pd_singleshot);

  pd_server->notifyRzDone(this,exit_on_error);
}

void
giopRendezvouser::terminate() {
  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "giopRendezvouser for " << pd_endpoint->address()
      << " terminate...\n";
  }
  pd_endpoint->Poke();
}


OMNI_NAMESPACE_END(omni)
