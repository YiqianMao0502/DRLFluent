// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopRendezvouser.h           Created on: 20 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
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

#ifndef __GIOPRENDEZVOUSER_H__
#define __GIOPRENDEZVOUSER_H__

OMNI_NAMESPACE_BEGIN(omni)

class giopRendezvouser : public omniTask, public giopServer::Link {
public:
  giopRendezvouser(giopEndpoint* e, giopServer* s, CORBA::Boolean h=0) : 
    omniTask(omniTask::ImmediateDispatch),
    pd_endpoint(e),
    pd_server(s),
    pd_singleshot(h) { }

  void execute();

  void terminate();
  // finish off this task, call by another thread

  giopEndpoint* endpoint() const { return pd_endpoint; }

  static void notifyReadable(void*,giopConnection*);

private:
  giopEndpoint*          pd_endpoint;
  giopServer*            pd_server;
  const CORBA::Boolean   pd_singleshot;

  giopRendezvouser();
  giopRendezvouser(const giopRendezvouser&);
  giopRendezvouser& operator=(const giopRendezvouser&);
};

OMNI_NAMESPACE_END(omni)

#endif // __GIOPRENDEZVOUSER_H__
