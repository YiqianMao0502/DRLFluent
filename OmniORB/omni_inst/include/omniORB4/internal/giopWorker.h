// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopWorker.h               Created on: 20 Dec 2000
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

#ifndef __GIOPWORKER_H__
#define __GIOPWORKER_H__

OMNI_NAMESPACE_BEGIN(omni)

class giopWorker : public omniTask, public giopServer::Link {
public:
  giopWorker(giopStrand* strand, giopServer* server, 
	     CORBA::Boolean singleshot=0);

  void execute();

  void terminate();
  // finish off this task, call by another thread.

  giopServer* server() const { return pd_server; }
  giopStrand* strand() const { return pd_strand; }
  CORBA::Boolean singleshot() const { return pd_singleshot; }

private:
  giopStrand*          pd_strand;
  giopServer*          pd_server;
  const CORBA::Boolean pd_singleshot;

  giopWorker();
  giopWorker(const giopWorker&);
  giopWorker& operator=(const giopWorker&);
};

OMNI_NAMESPACE_END(omni)

#endif // __GIOPWORKER_H__
