// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopRope.cc                Created on: 16/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2018 Apasphere Ltd
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
//

#include <omniORB4/CORBA.h>
#include <omniORB4/IOP_C.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/omniInterceptors.h>
#include <giopRope.h>
#include <giopStream.h>
#include <giopStrand.h>
#include <giopStrandFlags.h>
#include <giopStreamImpl.h>
#include <giopBiDir.h>
#include <GIOP_C.h>
#include <objectAdapter.h>
#include <exceptiondefs.h>
#include <initialiser.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <transportRules.h>
#include <interceptors.h>
#include <libcWrapper.h>

#include <stdlib.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
CORBA::Boolean orbParameters::oneCallPerConnection = 1;
//  1 means only one call can be in progress at any time per connection.
//
//  Valid values = 0 or 1

CORBA::ULong orbParameters::maxGIOPConnectionPerServer = 5;
//  The ORB can open more than one connection to a server depending on
//  the number of concurrent invocations to the same server. This
//  variable decides the maximum number of connections to use per
//  server. If the number of concurrent invocations exceeds this
//  number, the extra invocations would be blocked until the the
//  outstanding ones return.
//
//  Valid values = (n >= 1) 

CORBA::Boolean orbParameters::immediateRopeSwitch = 0;
//  Switch rope to use a new address immediately, rather than
//  retrying with the existing one.
//
//  Valid values = 0 or 1

CORBA::Boolean orbParameters::resolveNamesForTransportRules = 1;
//  If true, names in IORs will be resolved when evaluating client
//  transport rules, and remembered from then on; if false, names
//  will not be resolved until connect time. Client transport rules
//  based on IP address will therefore not match, but some platforms
//  can use external knowledge to pick the best address to use if
//  given a name to connect to.
//
//  Valid values = 0 or 1

CORBA::Boolean orbParameters::retainAddressOrder = 1;
//  For IORs with multiple addresses, determines how the address to
//  connect to is chosen. When first estabilishing a connection, the
//  addresses are ordered according to the client transport rules
//  (after resolving names if resolveNamesForTransportRules is true),
//  and the addresses are tried in priority order until one connects
//  successfully. For as long as there is at least one connection
//  open to the address, new connections continue to use the same
//  address.
//
//  After a failure, or after all open connections have been
//  scavenged and closed, this parameter determines the address used
//  to reconnect on the next call. If this parameter is true, the
//  address order and chosen address within the order is remembered;
//  if false, a new connection attempt causes re-evaluation of the
//  order (in case name resolutions change), and the highest priority
//  address is tried first.
//
//  Valid values = 0 or 1


///////////////////////////////////////////////////////////////////////
RopeLink giopRope::ropes;

////////////////////////////////////////////////////////////////////////
giopRope::giopRope(const giopAddressList& addrlist, omniIOR::IORInfo* info) :
  pd_refcount(0),
  pd_address_in_use(0),
  pd_maxStrands(orbParameters::maxGIOPConnectionPerServer),
  pd_oneCallPerConnection(orbParameters::oneCallPerConnection),
  pd_nwaiting(0),
  pd_cond(omniTransportLock, "giopRope::pd_cond"),
  pd_flags(0),
  pd_ior_flags(info->flags()),
  pd_offerBiDir(orbParameters::offerBiDirectionalGIOP),
  pd_addrs_filtered(0),
  pd_filtering(0)
{
  {
    giopAddressList::const_iterator i, last;
    i    = addrlist.begin();
    last = addrlist.end();
    for (; i != last; i++) {
      giopAddress* a = (*i)->duplicate();
      pd_addresses.push_back(a);
    }
  }
  pd_ior_addr_size = pd_addresses.size();
}


////////////////////////////////////////////////////////////////////////
giopRope::giopRope(giopAddress* addr) :
  pd_refcount(0),
  pd_address_in_use(0),
  pd_maxStrands(orbParameters::maxGIOPConnectionPerServer),
  pd_oneCallPerConnection(orbParameters::oneCallPerConnection),
  pd_nwaiting(0),
  pd_cond(omniTransportLock, "giopRope::pd_cond"),
  pd_flags(0),
  pd_ior_flags(0),
  pd_offerBiDir(orbParameters::offerBiDirectionalGIOP),
  pd_addrs_filtered(1),
  pd_filtering(0)
{
  pd_addresses.push_back(addr);
  pd_addresses_order.push_back(0);
  pd_ior_addr_size = 1;
}

////////////////////////////////////////////////////////////////////////
giopRope::~giopRope() {
  OMNIORB_ASSERT(pd_nwaiting == 0);
  giopAddressList::iterator i, last;
  i    = pd_addresses.begin();
  last = pd_addresses.end();
  for (; i != last; i++) {
    delete (*i);
  }
}

////////////////////////////////////////////////////////////////////////
IOP_C*
giopRope::acquireClient(const omniIOR*      ior,
			const CORBA::Octet* key,
			CORBA::ULong        keysize,
			omniCallDescriptor* calldesc)
{
  GIOP::Version v = ior->getIORInfo()->version();
  giopStreamImpl* impl = giopStreamImpl::matchVersion(v);
  if (!impl) {
    impl = giopStreamImpl::maxVersion();
    v = impl->version();
  }

  omni_tracedmutex_lock sync(*omniTransportLock);

  while (!pd_addrs_filtered) {

    if (pd_filtering) {
      // Another thread is filtering. Wait until it is done
      while (pd_filtering)
        pd_cond.wait();

      // The thread doing the filtering will have filtered the
      // addresses and set pd_addrs_filtered to 1. However, by the
      // time this thread runs, it is possible for the addresses to
      // have been reset again by resetAddressOrder, so we loop to
      // check again.
    }
    else {
      pd_filtering = 1;
      filterAndSortAddressList();

      pd_filtering      = 0;
      pd_addrs_filtered = 1;

      pd_cond.broadcast();
      break;
    }
  }

 again:

  unsigned int nbusy     = 0;
  unsigned int ndying    = 0;
  unsigned int nwrongver = 0;
  CORBA::ULong max       = pd_maxStrands; // snap the value now as it may
                                          // change by the application any time.
  RopeLink* p = pd_strands.next;
  for (; p != &pd_strands; p = p->next) {
    giopStrand* s = (giopStrand*)p;
    switch (s->state()) {
    case giopStrand::DYING:
      {
	// Bidirectional strands do not count towards the total of
	// dying strands. This is because with a bidirectional rope,
	// the max number of strands is one. Below, if the number of
	// dying strands is > the max, we wait for the strands to die.
	// However, it is possible that we are the client keeping the
	// strand alive, leading to a deadlock. To avoid the
	// situation, we do not count dying bidir strands, allowing us
	// to create a new one, and release the one that is dying.
	if (!s->isBiDir())
	  ndying++;

	break;
      }
    case giopStrand::TIMEDOUT:
      {
	s->StrandList::remove();
	s->state(giopStrand::ACTIVE);
	s->StrandList::insert(giopStrand::active);
	// falls through
      }
    case giopStrand::ACTIVE:
      {
	if (s->version.major != v.major || s->version.minor != v.minor) {
	  // Wrong GIOP version. Each strand can only be used
	  // for one GIOP version.
	  // If ever we allow more than one GIOP version
	  // to use one strand, make sure the client side interceptor
	  // for codeset is updated to reflect this.
	  nwrongver++;
	}
	else {
	  GIOP_C* g;
	  if (!giopStreamList::is_empty(s->clients)) {
	    giopStreamList* gp = s->clients.next;
	    for (; gp != &s->clients; gp = gp->next) {
	      g = (GIOP_C*)gp;
	      if (g->state() == IOP_C::UnUsed) {
		g->initialise(ior,key,keysize,calldesc);
		return g;
	      }
	    }
	    nbusy++;
	  }
	  else {
	    g = new GIOP_C(this,s);
	    g->impl(s->giopImpl);
	    g->initialise(ior,key,keysize,calldesc);
	    g->giopStreamList::insert(s->clients);
	    return g;
	  }
	}
      }
    }
  }

  // Reach here if we haven't got a strand to grab a GIOP_C.
  if ((nbusy + ndying) < max) {
    // Create a new strand.
    // Notice that we can have up to
    //  pd_maxStrands * <no. of supported GIOP versions> strands created.
    //
    
    // Do a sanity check here. It could be the case that this rope has
    // no valid address to use. This can be the case if we have
    // unmarshalled an IOR which has no profiles we can support, or
    // the client transport rules have denied all the addresses.
    // Notice that we do not raise an exception at the time when the
    // IOR was unmarshalled because we would like to be able to
    // receive and pass along object references that we ourselves
    // cannot talk to.
    if (pd_addresses_order.empty()) {
      resetAddressOrder(1, 0);
      OMNIORB_THROW(TRANSIENT,TRANSIENT_NoUsableProfile,CORBA::COMPLETED_NO);
    }

    giopStrand* s = new giopStrand(pd_addresses[pd_addresses_order[pd_address_in_use]]);
    
    s->state(giopStrand::ACTIVE);
    s->RopeLink::insert(pd_strands);
    s->StrandList::insert(giopStrand::active);
    s->version  = v;
    s->giopImpl = impl;
    s->flags    = pd_flags;

    GIOP_C* g = new GIOP_C(this,s);
    g->impl(s->giopImpl);
    g->initialise(ior,key,keysize,calldesc);
    g->giopStreamList::insert(s->clients);
    return g;
  }
  else if (pd_oneCallPerConnection || ndying >= max) {
    // Wait for a strand to be unused.
    pd_nwaiting++;

    const omni_time_t& deadline = calldesc->getDeadline();
    if (deadline) {
      if (pd_cond.timedwait(deadline) == 0) {
	pd_nwaiting--;
	if (orbParameters::throwTransientOnTimeOut) {
	  OMNIORB_THROW(TRANSIENT,
			TRANSIENT_CallTimedout,
			CORBA::COMPLETED_NO);
	}
	else {
	  OMNIORB_THROW(TIMEOUT,
			TIMEOUT_CallTimedOutOnClient,
			CORBA::COMPLETED_NO);
	}
      }
    }
    else {
      pd_cond.wait();
    }
    pd_nwaiting--;
  }
  else {
    // Pick a random non-dying strand.
    OMNIORB_ASSERT(nbusy);  // There must be a non-dying strand that can
                            // serve this GIOP version
    int n = LibcWrapper::Rand() % nbusy;
    // Pick a random and non-dying strand
    RopeLink* p = pd_strands.next;
    giopStrand* q = 0;
    giopStrand* s = 0;
    while (n >=0 && p != &pd_strands) {
      s = (giopStrand*)p;
      if (s->state() == giopStrand::ACTIVE &&
	  s->version.major == v.major &&
	  s->version.minor == v.minor)
	{
	  n--;
	  if (!q) q = s;
	}
      else {
	s = 0;
      }
      p = p->next;
    }
    s = (s) ? s : q;
    // By the time we look for busy strands, it's possible that they
    // are all dying, in which case we have to start again.
    if (s) {
      GIOP_C* g = new GIOP_C(this,s);
      g->impl(s->giopImpl);
      g->initialise(ior,key,keysize,calldesc);
      g->giopStreamList::insert(s->clients);
      return g;
    }
  }
  goto again;
}

////////////////////////////////////////////////////////////////////////
void
giopRope::releaseClient(IOP_C* iop_c)
{
  omni_tracedmutex_lock sync(*omniTransportLock);

  GIOP_C* giop_c = (GIOP_C*) iop_c;

  giop_c->rdUnLock();
  giop_c->wrUnLock();

  // We decide in the following what to do with this GIOP_C and the strand
  // it is attached:
  //
  // 1. If the strand is used simultaneously for multiple calls, it will have
  //    multiple GIOP_Cs attached. We only want to keep at most 1 idle GIOP_C.
  //    In other words, if this is not the last GIOP_C attached to the strand
  //    we delete it. (Actually it does no harm to delete all GIOP_C
  //    irrespectively. It will be slower to do an invocation because a
  //    new GIOP_C has to be instantiated in every call.
  //
  // 2. If the strand is in the DYING state, we obviously should delete the
  //    GIOP_C. If this is also the last GIOP_C, we delete the strand as
  //    well. If the strand is used to support bidirectional GIOP, we
  //    also check to ensure that the GIOP_S list is empty.
  //

  giopStrand* s = &giop_c->strand();
  giop_c->giopStreamList::remove();

  CORBA::Boolean remove = 0;
  CORBA::Boolean avail = 1;

  if (giop_c->state() != IOP_C::Idle && s->state() != giopStrand::DYING) {
    if (omniORB::trace(30)) {
      omniORB::logger l;

      if (s->connection) {
	l << "Unexpected error encountered in talking to the server "
	  << s->connection->peeraddress()
	  << ". The connection is closed immediately. ";
      }
      else {
	OMNIORB_ASSERT(s->address);
	l << "Unexpected error encountered before talking to the server "
	  << s->address->address()
	  << ". No connection was opened.";
      }
      l << " GIOP_C state " << (int)giop_c->state()
        << ", strand state " << (int)s->state() << "\n";
    }
    s->state(giopStrand::DYING);
  }

  if (s->state() == giopStrand::DYING) {

    if (s->isBiDir() && s->isClient() && s->connection) {
      // This is the client side of a bidirectional connection. There
      // may be another thread using the strand on behalf of the
      // client, so we do not delete the strand yet. Instead we
      // re-insert the GIOP_C and start the strand's idle counter so
      // it can be scavenged.
      
      if (omniORB::trace(25)) {
        omniORB::logger log;
        log << "Strand " << (void*)s
            << " in bi-directional client rope is dying.\n";
      }
      giop_c->giopStreamList::insert(s->clients);
      s->startIdleCounter();
    }
    else {
      remove = 1;

      // If safeDelete() returns 1, this strand can be regarded as
      // deleted. Therefore, we flag avail to 1 to wake up any threads
      // waiting on the rope to have a chance to create another
      // strand.
      avail = s->safeDelete();
    }
  }
  else if ((s->isBiDir() && !s->isClient()) || 
	    !giopStreamList::is_empty(s->clients)) {
    // We do not cache the GIOP_C if this is server side bidirectional or
    // we already have other GIOP_Cs active or available.
    remove = 1;
    avail = 0;
  }
  else {
    OMNIORB_ASSERT(giop_c->state() == IOP_C::Idle);
    giop_c->giopStreamList::insert(s->clients);
    // The strand is definitely idle from this point onwards, we
    // reset the idle counter so that it will be retired at the right time.
    if (s->isClient() && !s->biDir_has_callbacks) 
      s->startIdleCounter();
  }

  if (remove) {
    delete giop_c;
  }
  else {
    giop_c->cleanup();
  }

  // If any thread is waiting for a strand to become available, we signal
  // it here.
  if (avail && pd_nwaiting) pd_cond.signal();
}

////////////////////////////////////////////////////////////////////////
void
giopRope::realIncrRefCount()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(pd_refcount >= 0);

  if (pd_refcount == 0 && !RopeLink::is_empty(pd_strands)) {
    // This Rope still has some strands in the giopStrand::active_timedout list
    // put there by decrRefCount() when the reference count goes to 0
    // previously. We move these stands back to the giopStrand::active list so
    // that they can be used straight away.
    RopeLink* p = pd_strands.next;
    for (; p != &pd_strands; p = p->next) {
      giopStrand* g = (giopStrand*)p;
      if (g->state() != giopStrand::DYING) {
	g->StrandList::remove();
	g->state(giopStrand::ACTIVE);
	g->StrandList::insert(giopStrand::active);
      }
    }
  }

  pd_refcount++;
}

////////////////////////////////////////////////////////////////////////
void
giopRope::incrRefCount()
{
  omni_tracedmutex_lock sync(*omniTransportLock);
  realIncrRefCount();
}

////////////////////////////////////////////////////////////////////////
void
giopRope::decrRefCount()
{
  omni_tracedmutex_lock sync(*omniTransportLock);

  pd_refcount--;
  OMNIORB_ASSERT(pd_refcount >=0);

  if (pd_refcount) return;

  // This Rope is not used by any object reference.
  //
  // If this Rope has no strand, we can remove this instance straight
  // away.
  //
  // Otherwise, we move all the strands from the giopStrand::active
  // list to the giopStrand::active_timedout list.  Eventually when
  // all the strands are retired by the scavenger, this instance will
  // also be deleted on the next call to selectRope(), or the module
  // detach() at shutdown.

  if (RopeLink::is_empty(pd_strands) && !pd_nwaiting) {
    RopeLink::remove();
    delete this;
  }
  else {
    RopeLink* p = pd_strands.next;
    for (; p != &pd_strands; p = p->next) {
      giopStrand* g = (giopStrand*)p;
      if (g->state() != giopStrand::DYING) {
	g->state(giopStrand::TIMEDOUT);
	// The strand may already be on the active_timedout list. However
	// it is OK to remove and reinsert again.
	g->StrandList::remove();
	g->StrandList::insert(giopStrand::active_timedout);
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////
void
giopRope::disconnect()
{
  omni_tracedmutex_lock sync(*omniTransportLock);

  RopeLink* p = pd_strands.next;
  for (; p != &pd_strands; p = p->next) {
    giopStrand* s = (giopStrand*)p;

    if (s->state() != giopStrand::DYING) {
      if (s->connection) {
        if (omniORB::trace(10)) {
          omniORB::logger log;
          log << "Force disconnect connection to "
              << s->connection->peeraddress() << "\n";
        }
        s->connection->Shutdown();
        s->state(giopStrand::DYING);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopRope::hasAddress(const giopAddress* addr)
{
  giopAddressList::const_iterator ai;
  for (ai = pd_addresses.begin(); ai != pd_addresses.end(); ++ai) {
    if (*ai == addr) {
      return 1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
const giopAddress*
giopRope::notifyCommFailure(const giopAddress* addr,
			    CORBA::Boolean heldlock)
{
  omni_optional_lock sync(*omniTransportLock, heldlock, heldlock);

  const giopAddress* addr_in_use;

  addr_in_use = pd_addresses[pd_addresses_order[pd_address_in_use]];
  if (addr == addr_in_use) {
    pd_address_in_use++;
    if (pd_address_in_use >= pd_addresses_order.size())
      pd_address_in_use = 0;
    addr_in_use = pd_addresses[pd_addresses_order[pd_address_in_use]];

    if (omniORB::trace(20)) {
      omniORB::logger l;
      l << "Switch rope to use address " << addr_in_use->address() << "\n";
    }
  }
  return addr_in_use;
}

////////////////////////////////////////////////////////////////////////
void
giopRope::resetAddressOrder(CORBA::Boolean heldlock, giopStrand* strand)
{
  if (orbParameters::retainAddressOrder)
    return;

  omni_optional_lock sync(*omniTransportLock, heldlock, heldlock);

  if (!pd_addrs_filtered || pd_filtering)
    return;

  CORBA::Boolean do_reset = 1;
  
  RopeLink* p = pd_strands.next;
  for (; p != &pd_strands; p = p->next) {
    giopStrand* s = (giopStrand*)p;
    if (s != strand) {
      // The rope contains a strand other than the triggering strand,
      // so we do not reset the addresses.
      do_reset = 0;
      break;
    }
  }

  if (omniORB::trace(25)) {
    omniORB::logger log;

    if (do_reset)
      log << "Reset rope addresses (";
    else
      log << "Rope not reset due to other active strands (";

    if (pd_addresses_order.size() > pd_address_in_use) {
      const giopAddress* addr =
        pd_addresses[pd_addresses_order[pd_address_in_use]];

      log << "current address " << addr->address();
    }
    else {
      log << "no current address";
    }
    log << ")\n";
  }

  if (!do_reset)
    return;
  
  // Names may have been resolved to addresses, so we remove the
  // resolved addresses from the end of pd_addresses.
  while (pd_addresses.size() > pd_ior_addr_size) {
    delete pd_addresses.back();
    pd_addresses.pop_back();
  }

  pd_addresses_order.clear();
  pd_addrs_filtered = 0;
  pd_address_in_use = 0;
}


////////////////////////////////////////////////////////////////////////
void
giopRope::resetIdleRopeAddresses()
{
  omni_tracedmutex_lock sync(*omniTransportLock);

  if (orbParameters::retainAddressOrder)
    return;

  RopeLink* p = giopRope::ropes.next;
  while (p != &giopRope::ropes) {
    giopRope* gr = (giopRope*)p;

    if (gr->pd_addrs_filtered && RopeLink::is_empty(gr->pd_strands))
      gr->resetAddressOrder(1, 0);

    p = p->next;
  }
}
    

////////////////////////////////////////////////////////////////////////
int
giopRope::selectRope(const giopAddressList& addrlist,
		     omniIOR::IORInfo*      info,
		     Rope*&                 rope,
                     CORBA::Boolean&        is_local)
{
  giopRope* gr;

  omni_tracedmutex_lock sync(*omniTransportLock);

  // Check if we have to use an existing bidirectional connection for
  // a callback
  if (orbParameters::acceptBiDirectionalGIOP &&
      BiDirServerRope::selectRope(addrlist, info, rope)) {
    is_local = 0;
    return 1;
  }

  // Check if these are our addresses
  giopAddressList::const_iterator i, last;
  i    = addrlist.begin();
  last = addrlist.end();
  for (; i != last; i++) {
    if (omniObjAdapter::matchMyEndpoints((*i)->address())) {
      rope = 0; is_local = 1;
      return 1;
    }
  }

  // Check if there already exists a rope that goes to the same
  // addresses and matches the IOR
  RopeLink* p = giopRope::ropes.next;
  while (p != &giopRope::ropes) {
    gr = (giopRope*)p;
    if (gr->match(addrlist, info)) {
      gr->realIncrRefCount();
      rope = (Rope*)gr; is_local = 0;
      return 1;
    }
    else if (gr->pd_refcount == 0 &&
             RopeLink::is_empty(gr->pd_strands) &&
             !gr->pd_nwaiting) {
      // garbage rope, remove it
      p = p->next;
      gr->RopeLink::remove();
      delete gr;
    }
    else {
      p = p->next;
    }
  }

  // Reach here because we cannot find an existing rope that matches,
  // must create a new one.

  gr = 0;

  if (omniInterceptorP::createRope) {
    omniInterceptors::createRope_T::info_T iinfo(addrlist, info, gr);
    omniInterceptorP::visit(iinfo);
  }

  if (!gr) {
    if (orbParameters::offerBiDirectionalGIOP) {
      gr = new BiDirClientRope(addrlist, info);
    }
    else {
      gr = new giopRope(addrlist, info);
    }
  }

  gr->RopeLink::insert(giopRope::ropes);
  gr->realIncrRefCount();
  rope = (Rope*)gr; is_local = 0;
  return 1;
}


////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopRope::match(const giopAddressList& addrlist, omniIOR::IORInfo* info) const
{
  if ((info->flags() != pd_ior_flags) ||
      (addrlist.size() != pd_ior_addr_size) ||
      (orbParameters::offerBiDirectionalGIOP != pd_offerBiDir)) {
    return 0;
  }

  giopAddressList::const_iterator i, last, j;
  i    = addrlist.begin();
  j    = pd_addresses.begin();
  last = addrlist.end();
  for (; i != last; i++, j++) {
    if (!omni::ptrStrMatch((*i)->address(),(*j)->address())) return 0;
  }
  return 1;
}

////////////////////////////////////////////////////////////////////////
void
giopRope::filterAndSortAddressList()
{
  // We consult the clientTransportRules to decide the preference
  // orders for the addresses. The rules may forbid the use of some of
  // the addresses and these will be filtered out. We then record the
  // order of the remaining addresses in pd_addresses_order.

  // First, resolve any names in pd_addresses and add their
  // resolutions to the end.

  CORBA::Boolean do_resolve = orbParameters::resolveNamesForTransportRules;
  
  if (do_resolve) {
    giopAddressList resolved;
    giopAddressList::const_iterator it;

    for (it = pd_addresses.begin(); it != pd_addresses.end(); ++it) {
      giopAddress* ga   = *it;
      const char*  host = ga->host();

      if (host && !LibcWrapper::isipaddr(host)) {

        // Unlock omniTransportLock while resolving the name.
        omni_tracedmutex_unlock ul(*omniTransportLock);

        if (omniORB::trace(25)) {
          omniORB::logger log;
          log << "Resolve name '" << host << "'...\n";
        }

        LibcWrapper::AddrInfo_var aiv;
        aiv = LibcWrapper::getAddrInfo(host, 0);

        LibcWrapper::AddrInfo* ai = aiv;

        if (ai == 0) {
          if (omniORB::trace(25)) {
            omniORB::logger log;
            log << "Unable to resolve '" << host << "'.\n";
          }
        }
        else {
          while (ai) {
            CORBA::String_var addr = ai->asString();

            if (omniORB::trace(25)) {
              omniORB::logger log;
              log << "Name '" << host << "' resolved to " << addr << "\n";
            }
            resolved.push_back(ga->duplicate(addr));
            ai = ai->next();
          }
        }
      }
    }

    if (!resolved.empty()) {
      for (it = resolved.begin(); it != resolved.end(); ++it) {
        pd_addresses.push_back(*it);
      }
    }
  }

  // For each address, find the rule that is applicable. Record the
  // rules priority in the priority list.
  omnivector<CORBA::ULong> priority_list;

  CORBA::ULong index;
  CORBA::ULong total = pd_addresses.size();

  for (index = 0; index < total; index++) {
    giopAddress* ga   = pd_addresses[index];
    const char*  host = ga->host();

    if (do_resolve && host && !LibcWrapper::isipaddr(host)) {
      // Skip name -- it has been resolved to an address above
      continue;
    }

    CORBA::StringSeq actions;
    CORBA::ULong     matchedRule;

    if (transportRules::clientRules().match(ga->address(),
                                            actions, matchedRule)) {

      const char* transport = strchr(ga->type(),':');
      OMNIORB_ASSERT(transport);
      transport++;
      
      CORBA::ULong   i;
      CORBA::Boolean matched  = 0;
      CORBA::ULong   flags    = 0;
      CORBA::ULong   priority;

      for (i = 0; i < actions.length(); i++) {
	size_t len = strlen(actions[i]);
	if (strncmp(actions[i],transport,len) == 0) {
	  priority = (matchedRule << 16) + i;
	  matched  = 1;
	}
	else if (strcmp(actions[i],"none") == 0) {
	  break;
	}
	else if (orbParameters::offerBiDirectionalGIOP &&
                 strcmp(actions[i],"bidir") == 0) {
          flags |= GIOPSTRAND_BIDIR;
	}
        else if (strcmp(actions[i],"ziop") == 0) {
          flags |= pd_ior_flags & GIOPSTRAND_COMPRESSION;
        }
      }
      if (matched) {
	pd_addresses_order.push_back(index);
	priority_list.push_back(priority);
        pd_flags |= flags;
      }
    }
  }

  // If we have more than 1 address to use, sort them according to
  // their value in prioritylist.

  if (pd_addresses_order.size() > 1) {
    // Won't it be nice to just use stl qsort? It is tempting to just
    // forget about old C++ compiler and use stl. Until the time has come
    // use shell sort to sort the addresses in order.

    int n = pd_addresses_order.size();
    for (int gap=n/2; gap > 0; gap=gap/2) {
      for (int i=gap; i < n; i++)
	for (int j = i-gap; j>=0; j=j-gap) {
	  if (priority_list[j] > priority_list[j+gap]) {
	    CORBA::ULong temp         = pd_addresses_order[j];
	    pd_addresses_order[j]     = pd_addresses_order[j+gap];
	    pd_addresses_order[j+gap] = temp;
	    temp                      = priority_list[j];
	    priority_list[j]          = priority_list[j+gap];
	    priority_list[j+gap]      = temp;
	  }
	}
    }
  }

#if 0
  {
    omniORB::logger log;
    log << "Sorted addresses are: \n";
    for (size_t i=0; i < pd_addresses_order.size(); i++) {
      log << pd_addresses[pd_addresses_order[i]]->address() << "\n";
    }
  }
#endif
}

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class oneCallPerConnectionHandler : public orbOptions::Handler {
public:

  oneCallPerConnectionHandler() : 
    orbOptions::Handler("oneCallPerConnection",
			"oneCallPerConnection = 0 or 1",
			1,
			"-ORBoneCallPerConnection < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::oneCallPerConnection = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::oneCallPerConnection,
			     result);
  }
};

static oneCallPerConnectionHandler oneCallPerConnectionHandler_;

/////////////////////////////////////////////////////////////////////////////
class maxGIOPConnectionPerServerHandler : public orbOptions::Handler {
public:

  maxGIOPConnectionPerServerHandler() : 
    orbOptions::Handler("maxGIOPConnectionPerServer",
			"maxGIOPConnectionPerServer = n > 0",
			1,
			"-ORBmaxGIOPConnectionPerServer < n > 0 >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 1) {
      throw orbOptions::BadParam(key(),value,
			 orbOptions::expect_greater_than_zero_ulong_msg);
    }
    orbParameters::maxGIOPConnectionPerServer = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::maxGIOPConnectionPerServer,
			   result);
  }

};

static maxGIOPConnectionPerServerHandler maxGIOPConnectionPerServerHandler_;


/////////////////////////////////////////////////////////////////////////////
class immediateRopeSwitchHandler : public orbOptions::Handler {
public:

  immediateRopeSwitchHandler() : 
    orbOptions::Handler("immediateAddressSwitch",
			"immediateAddressSwitch = 0 or 1",
			1,
			"-ORBimmediateAddressSwitch < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::immediateRopeSwitch = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::immediateRopeSwitch,
			     result);
  }
};

static immediateRopeSwitchHandler immediateRopeSwitchHandler_;


/////////////////////////////////////////////////////////////////////////////
class resolveNamesForTransportRulesHandler : public orbOptions::Handler {
public:

  resolveNamesForTransportRulesHandler() : 
    orbOptions::Handler("resolveNamesForTransportRules",
			"resolveNamesForTransportRules = 0 or 1",
			1,
			"-ORBresolveNamesForTransportRules < 0 | 1 >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::resolveNamesForTransportRules = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::resolveNamesForTransportRules,
			     result);
  }

};

static resolveNamesForTransportRulesHandler resolveNamesForTransportRulesHandler_;


/////////////////////////////////////////////////////////////////////////////
class retainAddressOrderHandler : public orbOptions::Handler {
public:

  retainAddressOrderHandler() : 
    orbOptions::Handler("retainAddressOrder",
			"retainAddressOrder = 0 or 1",
			1,
			"-ORBretainAddressOrder < 0 | 1 >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::retainAddressOrder = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::retainAddressOrder,
			     result);
  }

};

static retainAddressOrderHandler retainAddressOrderHandler_;


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_giopRope_initialiser : public omniInitialiser {
public:

  omni_giopRope_initialiser() {
    orbOptions::singleton().registerHandler(oneCallPerConnectionHandler_);
    orbOptions::singleton().registerHandler(maxGIOPConnectionPerServerHandler_);
    orbOptions::singleton().registerHandler(immediateRopeSwitchHandler_);
    orbOptions::singleton().registerHandler(resolveNamesForTransportRulesHandler_);
    orbOptions::singleton().registerHandler(retainAddressOrderHandler_);
  }

  void attach() {
  }
  void detach() {
    // Get rid of any remaining ropes. By now they should all be strand-less.
    omni_tracedmutex_lock sync(*omniTransportLock);

    RopeLink* p = giopRope::ropes.next;
    giopRope* gr;
    int i=0;

    while (p != &giopRope::ropes) {
      gr = (giopRope*)p;
      OMNIORB_ASSERT(gr->pd_refcount == 0 &&
		     RopeLink::is_empty(gr->pd_strands) &&
		     !gr->pd_nwaiting);
      p = p->next;
      gr->RopeLink::remove();
      delete gr;
      ++i;
    }
    if (omniORB::trace(15)) {
      omniORB::logger l;
      l << i << " remaining rope" << (i == 1 ? "" : "s") << " deleted.\n";
    }
  }
};


static omni_giopRope_initialiser initialiser;

omniInitialiser& omni_giopRope_initialiser_ = initialiser;


OMNI_NAMESPACE_END(omni)
