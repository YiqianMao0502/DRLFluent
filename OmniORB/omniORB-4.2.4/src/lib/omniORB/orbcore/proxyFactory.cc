// -*- Mode: C++; -*-
//                            Package   : omniORB
// proxyFactory.cc            Created on: 24/2/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2006 Apasphere Ltd
//    Copyright (C) 1996,1997 Olivetti & Oracle Research Laboratory
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
//    Implementation of proxyObjectFactory.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/proxyFactory.h>

OMNI_NAMESPACE_BEGIN(omni)

static proxyObjectFactory** ofl       = 0;
static int                  ofl_size  = 0;
static int                  ofl_len   = 0;
static omni_tracedmutex*    ofl_mutex = 0;


proxyObjectFactory::proxyObjectFactory(const char* repoId)
  : pd_repoId(repoId)
{
  OMNIORB_ASSERT(repoId);

  if( !ofl ) {
    ofl_size  = 5;
    ofl       = new proxyObjectFactory* [ofl_size];
    ofl_len   = 0;
    ofl_mutex = new omni_tracedmutex("proxyObjectFactory::ofl_mutex");
  }

  omni_tracedmutex_lock sync(*ofl_mutex);

  if( ofl_len == ofl_size ) {
    int new_ofl_size = ofl_size * 2;
    proxyObjectFactory** new_ofl = new proxyObjectFactory* [new_ofl_size];
    for( int i = 0; i < ofl_size; i++ )  new_ofl[i] = ofl[i];
    delete[] ofl;
    ofl = new_ofl;
    ofl_size = new_ofl_size;
  }

  // Binary search to determine the insertion point.

  int bottom = 0;
  int top = ofl_len;

  while( bottom < top ) {

    int middle = (bottom + top) / 2;

    int cmp = strcmp(repoId, ofl[middle]->pd_repoId);

    if( cmp < 0 )       top = middle;
    else if( cmp > 0 )  bottom = middle + 1;
    else {
      ofl[middle] = this;
      if(omniORB::trace(15)) {
	omniORB::logger l;
	l << "Replaced proxyObjectFactory for " << repoId << ".\n";
      }
      return;
    }
  }

  OMNIORB_ASSERT(top == bottom);

  for( int i = ofl_len; i > bottom; i-- )
    ofl[i] = ofl[i - 1];

  ofl[bottom] = this;
  ofl_len++;
}


proxyObjectFactory::~proxyObjectFactory()
{
  // As we reach here, the list should still exist, since each
  // compilation unit with a proxyObjectFactory should be holding a
  // reference to the final cleanup object. Just to be safe, we check
  // the list still exists, and do nothing if it has already been
  // deleted.

  // This code used to do "if (!ofl) return;", but that triggered a
  // bug in SGI's compiler.

  if (ofl) {
    OMNIORB_ASSERT(pd_repoId);

    omni_tracedmutex_lock sync(*ofl_mutex);

    // Binary search to find the factory.

    int bottom = 0;
    int top = ofl_len;
    int pos = -1;
    while( bottom < top ) {

      int middle = (bottom + top) / 2;

      int cmp = strcmp(pd_repoId, ofl[middle]->pd_repoId);

      if( cmp < 0 )       top = middle;
      else if( cmp > 0 )  bottom = middle + 1;
      else                { pos = middle; break; }
    }

    // sanity check
    if (pos == -1) {
      if( omniORB::trace(2) ) {
	omniORB::logger l;
	l << "Could not find proxyObjectFactory " << pd_repoId
	  << " within its destructor at "
	  << __FILE__ << ": line " << __LINE__ << "\n";
      }
    }
    else {
      // remove it by shifting all pointers
      ofl_len--;
      for (int i=pos; i < ofl_len; i++)
	ofl[i] = ofl[i+1];
    }
  }
}


void
proxyObjectFactory::shutdown()
{
  ofl_mutex->lock(); 
  ofl_size = 0;
  ofl_len = 0;
  delete[] ofl;
  ofl = 0;
  ofl_mutex->unlock();
  delete ofl_mutex;
  ofl_mutex = 0;
}


proxyObjectFactory*
proxyObjectFactory::lookup(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  omni_tracedmutex_lock sync(*ofl_mutex);

  // Binary search to find the factory.

  int bottom = 0;
  int top = ofl_len;

  while( bottom < top ) {

    int middle = (bottom + top) / 2;

    int cmp = strcmp(repoId, ofl[middle]->pd_repoId);

    if( cmp < 0 )       top = middle;
    else if( cmp > 0 )  bottom = middle + 1;
    else                return ofl[middle];
  }

  return 0;
}

OMNI_NAMESPACE_END(omni)
