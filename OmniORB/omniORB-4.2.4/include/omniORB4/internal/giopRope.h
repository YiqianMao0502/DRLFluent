// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopRope.h                 Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2013 Apasphere Ltd
//    Copyright (C) 2001      AT&T Laboratories Cambridge
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

#ifndef __GIOPROPE_H__
#define __GIOPROPE_H__

#include <omniORB4/omniTransport.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

class omniIOR;

OMNI_NAMESPACE_BEGIN(omni)

class giopStream;
class giopStrand;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class giopRope : public Rope, public RopeLink {
public:

  static int selectRope(const giopAddressList& addrlist,
			omniIOR::IORInfo*      info,
			Rope*&                 rope,
			CORBA::Boolean&        is_local);
  // Given an address list, return a rope that can be used to talk to
  // remote objects in that address space. If the address list is in fact
  // pointing to ourselves, set <is_local> to true.
  // Returns true if either the addresses are local or the rope is found.
  // Returns false if no suitable rope is found.
  // If a rope is returned, the reference count on the rope is already
  // incremented by 1.
  //
  // This is normally the entry function that causes a rope to be created
  // in the first place.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  giopRope(const giopAddressList& addrlist, omniIOR::IORInfo* info);
  // <addrlist> is copied.
  // Reference count is initialised to 0.
  // No thread safety precondition

  giopRope(giopAddress* addr);
  // Construct with a single address. Used for server-side bidir.
  // <addr> is consumed by this instance.
  // No thread safety precondition

  virtual ~giopRope();
  // No thread safety precondition


  virtual IOP_C* acquireClient(const omniIOR*      ior,
			       const CORBA::Octet* key,
			       CORBA::ULong        keysize,
			       omniCallDescriptor* cd);
  // Acquire a GIOP_C from this rope.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  void releaseClient(IOP_C*);
  // Release the GIOP_C back to this rope. The GIOP_C must have been acquired
  // previously through acquireClient from this rope. Passing in a GIOP_C
  // from a different rope would result in undefined behaviour.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.


  void incrRefCount();
  // Increment the reference count by 1.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  virtual void decrRefCount();
  // Decrement the reference count by 1. If the reference count becomes
  // 0, the rope will be deleted at the earliest convenient time.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  void disconnect();
  // Forcibly disconnect all strands.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  CORBA::Boolean hasAddress(const giopAddress*);
  // Returns true if the address is in this rope's address list; false
  // otherwise.
  //
  // Thread Safety preconditions:
  //    None: the list of addresses is constant once set.

  virtual const giopAddress* notifyCommFailure(const giopAddress*,
					       CORBA::Boolean heldlock);
  // Caller detects an error in sending or receiving data with this address.
  // It calls this function to indicate to the rope that the address is bad.
  // If the rope has other alternative addresses, it should select another
  // address next time acquireClient is called.
  //
  // Returns the next address the rope will try. In the case when there is
  // only 1 address, the return value will be the same as the argument.
  // When there are more than 1 addresses and the caller decides to retry
  // an invocation on all of these addresses, the caller can use the return
  // value to decide when all the addresses have been tried. This
  // is done by comparing the return value with the address in use when
  // the first call is made.
  //
  // Thread Safety preconditions:
  //    Internally, omniTransportLock is used for synchronisation, if
  //    <heldlock> is true, the caller already holds the lock.

  void resetAddressOrder(CORBA::Boolean heldlock, giopStrand* strand);
  // If the retainAddressOrder parameter is not set true, reset the
  // address order to ensure the next connection attempt uses the
  // highest priority address. If names were resolved to addresses,
  // clears the resolved names so they are re-resolved next call.
  //
  // strand is a pointer to the strand that encountered an error
  // leading to this call, or null in the case of an idle rope.
  //
  // Thread Safety preconditions:
  //    Internally, omniTransportLock is used for synchronisation, if
  //    <heldlock> is true, the caller already holds the lock.


  static void resetIdleRopeAddresses();
  // If the retainAddressOrder parameter is not set true, reset the
  // address order in any idle ropes, to ensure the next connection
  // attempt uses the highest priority address. If names were resolved
  // to addresses, clears the resolved names so they are re-resolved
  // next call.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock.
  
  
  // Access functions to change the rope parameters. Notice that these
  // functions does not perform any mutual exclusion internally. It is
  // however safe to change the parameters while the rope is in use.  The
  // new parameter value will take effect as soon as appropriate.  For
  // instance, decrease the max. number of strand will not have any effect
  // on these strands that have already been created.

  CORBA::Boolean oneCallPerConnection() {
    // No thread safety precondition, use with extreme care
    // return True(1) if only one call can be in progress at any time on
    // each strand.
    // return False(0) if the same strand can be used to carry multiple
    // concurrent calls.
    // The default is True.
    return pd_oneCallPerConnection;
  }

  void oneCallPerConnection(CORBA::Boolean yes) {
    // No thread safety precondition, use with extreme care
    pd_oneCallPerConnection = yes;
  }

  CORBA::ULong maxStrands() {
    // No thread safety precondition, use with extreme care
    // Return the maximum no. of strands that can be opened.
    // The default is omniORB::maxTcpConnectionPerServer.
    return pd_maxStrands;
  }

  void maxStrands(CORBA::ULong max) {
    // No thread safety precondition, use with extreme care
    if (max == 0) max = 1;
    pd_maxStrands = max;
  }

  friend class giopStream;
  friend class giopStrand;
  friend class omni_giopRope_initialiser;
  friend class omni_giopbidir_initialiser;

 protected:
  int                      pd_refcount;
  giopAddressList          pd_addresses;     // Addresses of the remote server
  size_t                   pd_ior_addr_size; // Number of addresses in IOR
  omnivector<CORBA::ULong> pd_addresses_order;
  size_t                   pd_address_in_use;
  CORBA::ULong             pd_maxStrands;
  CORBA::Boolean           pd_oneCallPerConnection;
  int                      pd_nwaiting;
  omni_tracedcondition     pd_cond;
  CORBA::ULong             pd_flags;      // Selected flags in use
  CORBA::ULong             pd_ior_flags;  // Flags requested in IOR
  CORBA::Boolean           pd_offerBiDir; // State of orbParameters::
                                          // offerBiDir... at time of creation.
  CORBA::Boolean           pd_addrs_filtered;
  CORBA::Boolean           pd_filtering;

  static _core_attr RopeLink ropes;
  // All ropes created by selectRope are linked together by this list.

  virtual void realIncrRefCount();
  // Really increment the reference count.
  //
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock.

  virtual CORBA::Boolean match(const giopAddressList&,
                               omniIOR::IORInfo* info) const;
  // Return true if the address list matches EXACTLY those of this
  // rope, and the IORInfo policies match.
  // No thread safety precondition

  virtual void filterAndSortAddressList();
  // Use the clientTransportRules to filter and sort the address list
  // in pd_addresses, storing the resulting order in
  // pd_addresses_order, and set pd_flags appropriately. Any names in
  // pd_addresses are resolved and the resulting addresses are added
  // to pd_addresses.
  //
  // Caller holds omniTransportLock.

 private:
  giopRope();
  giopRope(const giopRope&);
  giopRope& operator=(const giopRope&);
};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __GIOPROPE_H__
