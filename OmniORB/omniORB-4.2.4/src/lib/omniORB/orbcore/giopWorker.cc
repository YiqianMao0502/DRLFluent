// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopWorker.cc              Created on: 20 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
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
#include <omniORB4/omniInterceptors.h>
#include <interceptors.h>
#include <invoker.h>
#include <giopServer.h>
#include <giopWorker.h>
#include <giopStrand.h>
#include <giopStream.h>
#include <GIOP_S.h>
#include <transportRules.h>

OMNI_NAMESPACE_BEGIN(omni)


giopWorker::giopWorker(giopStrand* r, giopServer* s, CORBA::Boolean h)
  : omniTask(((h) ? omniTask::AnyTime : omniTask::ImmediateDispatch),
             omniTask::ServerUpcall),
    pd_strand(r),
    pd_server(s),
    pd_singleshot(h) {}

void
giopWorker::execute()
{
  omniORB::logs(25, "giopWorker task execute.");

  if (!pd_strand->gatekeeper_checked) {

    if (!pd_strand->connection->gatekeeperCheck(pd_strand)) {
      {
	omni_tracedmutex_lock sync(*omniTransportLock);
	pd_strand->safeDelete();
      }
      pd_server->notifyWkDone(this, 1);
      return;
    }
    pd_strand->gatekeeper_checked = 1;
  }

  CORBA::Boolean exit_on_error;
  CORBA::Boolean go = 1;

  do {
    {
      GIOP_S_Holder iops_holder(pd_strand, this);

      GIOP_S* iop_s = iops_holder.operator->();
      if (iop_s) {
	exit_on_error = !iop_s->dispatcher();
      }
      else {
	exit_on_error = 1;
      }
    }
    go = pd_server->notifyWkDone(this, exit_on_error);

  } while(go && !exit_on_error);

}

void
giopWorker::terminate() {
  pd_strand->connection->Shutdown();
}

OMNI_NAMESPACE_END(omni)
