// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniORB.cc                 Created on: 15/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2002-2011 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//

#include <stdlib.h>
#include <omniORB4/CORBA.h>
#include <orbParameters.h>
#include <omniCurrent.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exceptiondefs.h>

OMNI_USING_NAMESPACE(omni)


//////////////////////////////////////////////////////////////////////
/////////////////////// omniORB::version... //////////////////////////
//////////////////////////////////////////////////////////////////////

const char*
omniORB::versionString()
{
  return OMNIORB_VERSION_STRING;
}

_CORBA_ULong
omniORB::versionHex()
{
  return OMNIORB_VERSION_HEX;
}


//////////////////////////////////////////////////////////////////////
/////////////////////// omniORB::fatalException //////////////////////
//////////////////////////////////////////////////////////////////////

omniORB::fatalException::fatalException(const char* file, int line,
					const char* errmsg)
  : pd_file(file), pd_line(line), pd_errmsg(errmsg)
{
  if( orbParameters::abortOnInternalError )  abort();
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void
omniORB::setMainThread()
{
  omni_thread* self = omni_thread::self();
  if (!self)
    OMNIORB_THROW(INITIALIZE, INITIALIZE_NotOmniThread, CORBA::COMPLETED_NO);

  omni::mainThreadId = self->id();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void
omniORB::setClientCallTimeout(CORBA::ULong v)
{
  orbParameters::clientCallTimeOutPeriod.assign(v / 1000,
						(v % 1000) * 1000000);
}

void
omniORB::setClientCallTimeout(CORBA::Object_ptr obj, CORBA::ULong v)
{
  omniObjRef* oo = obj->_PR_getobj();
  if (!oo)
    OMNIORB_THROW(INV_OBJREF, INV_OBJREF_InvokeOnNilObjRef,
		  CORBA::COMPLETED_NO);

  oo->_setTimeout(v / 1000, (v % 1000) * 1000000);
}

void
omniORB::setClientThreadCallTimeout(CORBA::ULong v)
{
  omniCurrent* current = omniCurrent::get();
  if (!current)
    OMNIORB_THROW(INITIALIZE, INITIALIZE_NotOmniThread, CORBA::COMPLETED_NO);

  current->setTimeout(v / 1000, (v % 1000) * 1000000);
}

void
omniORB::setClientThreadCallDeadline(unsigned long secs, unsigned long ns)
{
  omniCurrent* current = omniCurrent::get();
  if (!current)
    OMNIORB_THROW(INITIALIZE, INITIALIZE_NotOmniThread, CORBA::COMPLETED_NO);

  current->setDeadline(secs, ns);
}

void
omniORB::setClientConnectTimeout(CORBA::ULong v)
{
  orbParameters::clientConnectTimeOutPeriod.assign(v / 1000,
						   (v % 1000) * 1000000);
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
CORBA::ULong
omniORB::giopMaxMsgSize() {
  return orbParameters::giopMaxMsgSize;
}


#if defined(__DMC__) && defined(_WINDLL)
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  return TRUE;
}
#endif
