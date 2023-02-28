// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopMonitor.h              Created on: 23 July 2001
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

#ifndef __GIOPMONITOR_H__
#define __GIOPMONITOR_H__

OMNI_NAMESPACE_BEGIN(omni)

class giopMonitor : public omniTask, public giopServer::Link {
public:
  giopMonitor(giopActiveCollection* c, giopServer* s) : 
    omniTask(omniTask::ImmediateDispatch),
    pd_collection(c),
    pd_server(s) { }

  void execute();

  void deactivate() { pd_collection->deactivate(); }

  giopActiveCollection* collection() const { return pd_collection; }

  static void notifyReadable(void*,giopConnection*);

private:
  giopActiveCollection*  pd_collection;
  giopServer*            pd_server;

  giopMonitor();
  giopMonitor(const giopMonitor&);
  giopMonitor& operator=(const giopMonitor&);
};

OMNI_NAMESPACE_END(omni)

#endif // __GIOPMONITOR_H__
