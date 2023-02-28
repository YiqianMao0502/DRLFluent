// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniInternal.cc            Created on: 25/2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2002-2012 Apasphere Ltd
//    Copyright (C) 1996,1999 AT&T Research Cambridge
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
//    Implementation of methods defined in class omni.
//      

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/proxyFactory.h>
#include <omniORB4/omniServant.h>
#include <objectTable.h>
#include <inProcessIdentity.h>
#include <remoteIdentity.h>
#include <objectAdapter.h>
#include <anonObject.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <omniORB4/omniInterceptors.h>
#include <interceptors.h>
#include <giopRope.h>
#include <invoker.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <omniORB4/objTracker.h>
#include <corbaOrb.h>

#if defined(__WIN32__) && !defined(__GNUC__)
#  define WIN32_EXCEPTION_HANDLING
#endif

#ifdef WIN32_EXCEPTION_HANDLING
#  include <eh.h>
#endif

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
CORBA::ULong  omniORB::traceLevel = 1;
//    level 0 - critical errors only
//    level 1 - informational messages only
//    level 2 - configuration information and warnings
//    level 5 - the above plus report server thread creation and
//              communication socket shutdown
//    level 10 - the above plus execution trace messages
//    level 25 - output trace message per send or receive giop message
//    level 30 - dump up to 128 bytes of a giop message
//    level 40 - dump the complete giop message
//
//    Valid values = (n >= 0)

CORBA::Boolean  omniORB::traceExceptions = 0;
//    If true, then system exceptions are logged when they are thrown.
//
//    Valid values = 0 or 1

CORBA::Boolean  omniORB::traceInvocations = 0;
//    If true, then each local and remote invocation will generate a trace 
//    message.
//
//    Valid values = 0 or 1

CORBA::Boolean  omniORB::traceInvocationReturns = 0;
//    If true, then each local and remote invocation will generate a trace 
//    message when it returns.
//
//    Valid values = 0 or 1

CORBA::Boolean  omniORB::traceThreadId = 1;
//    If true, then the log messages will contain the thread id.
//
//    Valid values = 0 or 1

CORBA::Boolean  omniORB::traceTime = 1;
//    If true, then the log messages will contain the time.
//
//    Valid values = 0 or 1

CORBA::Boolean  omniORB::traceLocking = 0;
//    If true, trace lock operations.
//
//    Valid values = 0 or 1



const CORBA::Char                omni::myByteOrder = _OMNIORB_HOST_BYTE_ORDER_;
omni_tracedmutex*                omni::internalLock = 0;
omni_tracedmutex*                omni::poRcLock = 0;
_CORBA_Unbounded_Sequence_Octet  omni::myPrincipalID;
const omni::alignment_t          omni::max_alignment = omni::ALIGN_8;

int                              omni::remoteInvocationCount = 0;
int                              omni::localInvocationCount = 0;
int                              omni::mainThreadId = 0;

omni_tracedmutex*                omni::objref_rc_lock = 0;
// Protects omniObjRef reference counting and linked list.

OMNI_USING_NAMESPACE(omni)

CORBA::ULong orbParameters::objectTableSize = 0;
//  Hash table size of the Active Object Map. If this is zero, the ORB
//  uses a dynamically resized open hash table. This is normally the
//  best option, but it leads to less predictable performance since
//  any operation which adds or removes a table entry may trigger a
//  resize. If you set this to a non-zero value, the hash table has
//  the specified number of entries, and is never resized. Note that
//  the hash table is open, so this does not limit the number of
//  active objects, just how efficiently they can be located.
//
//  Valid values = (n >= 0)
//                 0 --> use a dynamically resized table.

CORBA::Boolean orbParameters::abortOnInternalError = 0;
//  If the value of this variable is TRUE then the ORB will abort
//  instead of throwing an exception when a fatal internal error is
//  detected. This is useful for debuging the ORB -- as the stack will
//  not be unwound by the exception handler, so a stack trace can be
//  obtained.
//  It is hoped that this will not often be needed by users of omniORB!
//
//  Valid values = 0 or 1

static CORBA::Boolean abortOnNativeException = 0;
//  On Windows, "native" exceptions such as segmentation faults and
//  divide by zero appear as C++ exceptions that can be caught with
//  catch (...). Setting this parameter to true causes such exceptions
//  to abort the process instead.
//
//  Valid values = 0 or 1

CORBA::Boolean orbParameters::throwTransientOnTimeOut = 0;
//  If true, CORBA::TRANSIENT is thrown when a timeout occurs. If
//  false (the default), CORBA::TIMEOUT is thrown.
//  
//  Valid values = 0 or 1


////////////////////////////////////////////////////////////////////////////

#if defined(HAS_Cplusplus_Namespace)
#  ifndef __DMC__
using omniORB::operator==;
#  endif
#endif


OMNI_NAMESPACE_BEGIN(omni)

// The local object table.  This is a dynamically resized
// open hash table.
static omniObjTableEntry**       objectTable = 0;
static _CORBA_ULong              objectTableSize = 0;
static int                       objectTableSizeI = 0;
static _CORBA_ULong              numObjectsInTable = 0;
static _CORBA_ULong              maxNumObjects = 0;
static _CORBA_ULong              minNumObjects = 0;

// Some sort of magic numbers that are supposed
// to be good for hash tables...
static int objTblSizes[] = {
  128 + 3,              // 2^7
  1024 + 9,             // 2^10
  8192 + 27,            // 2^13
  32768 + 3,            // 2^15
  65536 + 45,           // 2^16
  131072 + 9,
  262144 + 39,
  524288 + 39,
  1048576 + 9,          // 2^20
  2097152 + 5,
  4194304 + 3,
  8388608 + 33,
  16777216 + 27,
  33554432 + 9,         // 2^25
  67108864 + 71,
  134217728 + 39,
  268435456 + 9,
  536870912 + 5,
  1073741824 + 83,      // 2^30 -- I'd be suprised if this is exceeded!
  -1                    // Sentinel to detect the end, just to be paranoid.
};

OMNI_NAMESPACE_END(omni)


void
omniObjTable::resize()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  OMNIORB_ASSERT((numObjectsInTable > maxNumObjects) ||
		 (numObjectsInTable < minNumObjects && objectTableSizeI > 0));

  if (numObjectsInTable > maxNumObjects) {
    ++objectTableSizeI;
  }
  else if (numObjectsInTable < minNumObjects && objectTableSizeI > 0) {
    --objectTableSizeI;
  }
  else
    return;

  int newsizei = objTblSizes[objectTableSizeI];

  if (newsizei == -1) {
    // Wow, we fell off the bottom of the table!  If this happens,
    // I'll eat my hat...
    if (omniORB::trace(5)) {
      omniORB::logger l;
      l << "More than " << maxNumObjects << " active objects!  "
	<< "Consider extending the available object table sizes in "
	<< __FILE__ << ".\n";
    }
    objectTableSizeI--;
    maxNumObjects = 1ul << 31;
    return;
  }
  CORBA::ULong newsize = newsizei;

  if (omniORB::trace(15)) {
    omniORB::logger l;
    l << "Object table resizing from " << objectTableSize
      << " to " << newsize << "\n";
  }

  // Create and initialise new object table.
  omniObjTableEntry** newtable = new omniObjTableEntry* [newsize];
  CORBA::ULong i;
  for( i = 0; i < newsize; i++ )  newtable[i] = 0;

  // Move the objects across...
  for( i = 0; i < objectTableSize; i++ ) {
    omniObjTableEntry* id = objectTable[i];

    while( id ) {
      omniObjTableEntry* next = id->nextInObjectTable();

      _CORBA_ULong j = omni::hash(id->key(), id->keysize()) % newsize;
      *(id->addrOfNextInObjectTable()) = newtable[j];
      newtable[j] = id;

      id = next;
    }
  }

  // Replace the old table.
  delete[] objectTable;
  objectTable = newtable;
  objectTableSize = newsize;
  maxNumObjects = objectTableSize * 2 / 3;
  minNumObjects =
    objectTableSizeI ? (objTblSizes[objectTableSizeI - 1] / 3) : 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////// omni ////////////////////////////////
//////////////////////////////////////////////////////////////////////

omni_tracedmutex&
omni::nilRefLock()
{
  // We are safe just testing this here, as we guarentee that
  // it will be initialised during the static initialisation.
  // (Which is single-threaded).  If not by this method, then
  // by the static initialiser below.

  static omni_tracedmutex* nil_ref_lock = 0;

  if( !nil_ref_lock )  nil_ref_lock = new omni_tracedmutex("nil_ref_lock");
  return *nil_ref_lock;
}


void
omni::duplicateObjRef(omniObjRef* objref)
{
  OMNIORB_ASSERT(objref);

  objref_rc_lock->lock();
  objref->pd_refCount++;
  objref_rc_lock->unlock();
}


void
omni::releaseObjRef(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 0);
  OMNIORB_ASSERT(objref);

  objref_rc_lock->lock();
  int rc = --objref->pd_refCount;
  objref_rc_lock->unlock();

  if( rc > 0 )  return;

  if( rc < 0 ) {
    omniORB::logs(1,
      "Error: trying to release an object with reference count <= 0. "
      "CORBA::release() may have been called too many times on an object "
      "reference.");
    return;
  }

  // Clear objref's identity
  {
    omni_tracedmutex_lock sync(*internalLock);
    objref->_setIdentity(0);
  }

  if( omniORB::trace(15) ) {
    omniORB::logger l;
    l << "ObjRef(" << objref->_mostDerivedRepoId() << ") -- deleted.\n";
  }

  // Destroy the reference.
  delete objref;
}


omniObjTableEntry*
omniObjTable::locateActive(const _CORBA_Octet* key, int keysize,
			   _CORBA_ULong hashv, _CORBA_Boolean wait)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

 again:
  omniObjTableEntry** head  = objectTable + hashv % objectTableSize;
  omniObjTableEntry*  entry = *head;

  while (entry) {
    if (entry->is_equal(key, keysize)) break;

    entry = entry->nextInObjectTable();
  }

  omniObjTableEntry::State state;

  if (entry) {
    if (wait) {
      while (entry->state() == omniObjTableEntry::ACTIVATING) {

	state = entry->wait(omniObjTableEntry::ACTIVE |
			    omniObjTableEntry::DEACTIVATING |
			    omniObjTableEntry::ETHEREALISING);

	if (state == omniObjTableEntry::DEAD) {
	  // The entry has been removed from the object table. Have to
	  // start from scratch in case a new entry has been created.
	  goto again;
	}
      }
    }
    if (entry->state() & (omniObjTableEntry::ACTIVE |
			  omniObjTableEntry::DEACTIVATING))
      return entry;
  }
  return 0;
}


omniObjTableEntry*
omniObjTable::locate(const _CORBA_Octet* key, int keysize,
		     _CORBA_ULong hashv, _CORBA_ULong set)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

 again:
  omniObjTableEntry** head  = objectTable + hashv % objectTableSize;
  omniObjTableEntry*  entry = *head;

  while (entry) {
    if (entry->is_equal(key, keysize)) break;

    entry = entry->nextInObjectTable();
  }

  if (entry) {
    while (!(entry->state() & set)) {
      if (omniORB::trace(15)) {
	omniORB::logger l;
	l << "Waiting for object table entry " << entry << "\n";
      }
      if (entry->wait(set) == omniObjTableEntry::DEAD) {
	// The entry has been removed from the object table. Have to
	// start from scratch in case a new entry has been created.
	goto again;
      }
    }
    return entry;
  }
  return 0;
}

omniObjTableEntry*
omniObjTable::newEntry(omniObjKey& key)
{
  CORBA::ULong hashv = omni::hash(key.key(), key.size());
  return newEntry(key, hashv);
}

omniObjTableEntry*
omniObjTable::newEntry(omniObjKey& key, _CORBA_ULong hashv)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniObjTableEntry** head  = objectTable + hashv % objectTableSize;
  omniObjTableEntry*  entry = *head;

  while (entry) {
    if (entry->is_equal(key.key(), key.size())) return 0;
    entry = entry->nextInObjectTable();
  }

  if( ++numObjectsInTable > maxNumObjects ) {
    omniObjTable::resize();
    head = objectTable + hashv % objectTableSize;
  }

  entry = new omniObjTableEntry(key);
  
  *(entry->addrOfNextInObjectTable()) = *head;
  *head = entry;

  if (omniORB::trace(10)) {
    omniORB::logger l;
    l << "Adding " << entry << " to object table.\n";
  }
  return entry;
}


omniObjTableEntry::~omniObjTableEntry()
{
  if (pd_cond) delete pd_cond;
  if (omniORB::trace(15)) {
    omniORB::logger l;
    l << "Object table entry " << this << " deleted.\n";
  }
}


void
omniObjTableEntry::setActive(omniServant* servant, omniObjAdapter* adapter)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(pd_state == ACTIVATING);

  setServant(servant, adapter);
  servant->_addActivation(this);

  if( omniORB::trace(15) ) {
    omniORB::logger l;
    l << "State " << this << " -> active\n";
  }

  pd_state = ACTIVE;

  if (pd_waiters)
    pd_cond->broadcast();
}

void
omniObjTableEntry::setDeactivating()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(pd_state == ACTIVE);
  OMNIORB_ASSERT(pd_nInvocations > 0);

  if( omniORB::trace(15) ) {
    omniORB::logger l;
    l << "State " << this << " -> deactivating\n";
  }

  pd_state = DEACTIVATING;
  --pd_nInvocations;

  if (pd_waiters)
    pd_cond->broadcast();
}

void
omniObjTableEntry::setDeactivatingOA()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(pd_state == ACTIVE);
  OMNIORB_ASSERT(pd_nInvocations > 0);

  if( omniORB::trace(15) ) {
    omniORB::logger l;
    l << "State " << this << " -> deactivating (OA destruction)\n";
  }

  pd_state = DEACTIVATING_OA;
  --pd_nInvocations;

  if (pd_waiters)
    pd_cond->broadcast();
}

void
omniObjTableEntry::setEtherealising()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(pd_state & DEACTIVATING);

  pd_servant->_removeActivation(this);

  if( omniORB::trace(15) ) {
    omniORB::logger l;
    l << "State " << this << " -> etherealising\n";
  }

  pd_state       = ETHEREALISING;
  pd_deactivated = 1;

  if (pd_waiters)
    pd_cond->broadcast();
}

void
omniObjTableEntry::setDead()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniObjTableEntry** pid =
    objectTable + omni::hash(key(), keysize()) % objectTableSize;

  while (*pid) {
    if (*pid == this) break;
    pid = (*pid)->addrOfNextInObjectTable();
  }
  OMNIORB_ASSERT(*pid);

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "Removing " << this << " from object table\n";
  }
  *pid = nextInObjectTable();

  if( --numObjectsInTable < minNumObjects )
    omniObjTable::resize();

  if (pd_state != ETHEREALISING && pd_servant) {
    pd_servant->_removeActivation(this);
    pd_deactivated = 1;
  }
  pd_state = DEAD;

  if (pd_waiters)
    pd_cond->broadcast();

  loseRef(); // Drop the object table's entry to ourselves
}


omniObjTableEntry::State
omniObjTableEntry::wait(_CORBA_ULong set)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if (pd_state & set) return pd_state;

  if (!pd_cond)
    pd_cond = new omni_tracedcondition(omni::internalLock,
				       "omniObjTableEntry::pd_cond");

  gainRef();
  ++pd_waiters;
  if (omniORB::trace(15)) {
    omniORB::logger l;
    l << "Waiting for " << this << "\n";
  }
  while ((pd_state != DEAD) && !(pd_state & set)) pd_cond->wait();
  --pd_waiters;

  State ret = pd_state;
  // If the state is now DEAD, the call to loseRef() below might
  // delete this object, so we must save the return state here.

  loseRef();

  return ret;
}


void
omniObjTableEntry::gainRef(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  ++pd_refCount;
  if (objref)
    pd_objRefs.push_back(objref);
}

void
omniObjTableEntry::loseRef(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if (objref) {
    CORBA::Boolean reference_found = 0;

    omnivector<omniObjRef*>::iterator i    = pd_objRefs.begin();
    omnivector<omniObjRef*>::iterator last = pd_objRefs.end();

    for (; i != last; i++) {
      if (*i == objref) {
	pd_objRefs.erase(i);
	reference_found = 1;
	break;
      }
    }
    OMNIORB_ASSERT(reference_found);
  }
  
  if (--pd_refCount > 0) return;

  OMNIORB_ASSERT(pd_refCount == 0);
  // < 0 means someone has released too many references.

  OMNIORB_ASSERT(pd_waiters  == 0);
  // Waiting threads hold a reference to us, so pd_waiters should be
  // zero by the time we get here.

  OMNIORB_ASSERT(pd_objRefs.empty());
  // If this fails, an object reference released its reference to us
  // without passing the objref pointer.

  delete this;
}


void*
omniObjTableEntry::ptrToClass(int* cptr)
{
  if (cptr == &omniObjTableEntry::_classid) return (omniObjTableEntry*)this;
  if (cptr == &omniLocalIdentity::_classid) return (omniLocalIdentity*)this;
  if (cptr == &omniIdentity     ::_classid) return (omniIdentity*)     this;
  return 0;
}

int omniObjTableEntry::_classid;



omniIdentity*
omni::createIdentity(omniIOR* ior, const char* target, CORBA::Boolean locked)
{
  omniIOR_var holder(ior); // Place the ior inside a var. If ever
                           // any function we called results in an
                           // exception being thrown, the ior is released
                           // by the var, hence fullfilling the semantics
                           // of this function.
                           // If this function completes normally, make
                           // sure that _retn() is called on the var so
                           // that the ior is not released incorrectly!

  // Try an interceptor
  omniIdentity* result = 0;

  if (omniInterceptorP::createIdentity) {
    omniInterceptors::createIdentity_T::info_T info(ior,target,result,locked);
    omniInterceptorP::visit(info);

    if (result) {
      holder._retn();

      omni_optional_lock sync(*internalLock,locked,locked);
      result->gainRef();

      return result;
    }
  }

  // Decode the profiles
  const IOP::TaggedProfileList& profiles = ior->iopProfiles();

  if (ior->addr_selected_profile_index() < 0) {

    // Pick the first TAG_INTERNET_IOP profile

    CORBA::ULong total = profiles.length();
    CORBA::ULong index;
    for (index = 0; index < total; index++) {
      if ( profiles[index].tag == IOP::TAG_INTERNET_IOP ) break;
    }
    if (index < total)
      ior->addr_selected_profile_index(index);
    else
      omniORB::logs(25, "createIdentity for IOR with no IIOP profiles.");
  }

  omniIOR::IORInfo* info = ior->getIORInfo();
  // getIORInfo() has the side effect of decoding the selected
  // TAG_INTERNET_IOP profile and any IOP::TAG_MULTIPLE_COMPONENTS
  // if that has not been done already.
  //
  // We use this function to trigger the decoding, instead of say
  // calling omniIOR::decodeIOPprofile() first, because createIdentity
  // may be called with the same ior more than once. It is highly
  // undesirable if the IOP profile is decoded in each of these calls.
  // Not only is this inefficent but doing so would create a thread
  // safety nightmare.

  CORBA::Boolean is_local = 0;
  Rope* rope;

  if (giopRope::selectRope(info->addresses(), info, rope, is_local) == 0) {
    return 0;
  }

  _CORBA_Unbounded_Sequence_Octet object_key;

  if (ior->addr_selected_profile_index() >= 0)
    IIOP::unmarshalObjectKey(profiles[ior->addr_selected_profile_index()],
			     object_key);

  if (is_local) {

    CORBA::ULong hashv = hash(object_key.get_buffer(), object_key.length());
    omni_optional_lock sync(*internalLock,locked,locked);

    omniObjTableEntry* entry =
      omniObjTable::locateActive(object_key.get_buffer(),
				 object_key.length(), hashv, 0);

    if (entry && entry->servant()->_ptrToInterface(target)) {
      // Compatible activated object
      entry->gainRef();
      return entry;
    }
    else {
      // Not active or servant incompatible with target
      result = createInProcessIdentity(object_key.get_buffer(),
				       object_key.length());
      result->gainRef();
      return result;
    }
  }
  else {
    // Remote
    holder._retn();
    omni_optional_lock sync(*internalLock,locked,locked);
    result = new omniRemoteIdentity(ior,
				    object_key.get_buffer(),
				    object_key.length(),
				    rope);
    result->gainRef();
    return result;
  }
}


omniIdentity*
omni::createInProcessIdentity(const _CORBA_Octet* key, int keysize) {
  return new omniInProcessIdentity(key,keysize);
}


omniObjRef*
omni::createObjRef(const char* targetRepoId,
		   omniIOR* ior,
		   CORBA::Boolean locked,
		   omniIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, locked);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(ior);

  CORBA::Boolean called_create = 0;

  if (id) {
    omniLocalIdentity* lid = omniLocalIdentity::downcast(id);

    if (lid && (!lid->servant() ||
		!lid->servant()->_ptrToInterface(targetRepoId))) {
      // Local id can't be used by the objref
      omni_optional_lock sync(*internalLock,locked,locked);
      id = createInProcessIdentity(lid->key(), lid->keysize());
    }
  }
  else {
    ior->duplicate();  // consumed by createIdentity
    id = omni::createIdentity(ior, targetRepoId, locked);
    called_create = 1;
    if ( !id ) {
      ior->release();
      return 0;
    }
  }

  proxyObjectFactory* pof = proxyObjectFactory::lookup(ior->repositoryID());

  if( pof && !pof->is_a(targetRepoId) &&
      !omni::ptrStrMatch(targetRepoId, CORBA::Object::_PD_repoId) ) {

    // We know that <mostDerivedRepoId> is not derived from
    // <targetRepoId>. 

    pof = 0;
  }

  // Once we reach here:
  // if (pof != 0)
  //    pof points to the proxy factory that is an exact match to
  //    the interface identified by the ior's repository ID and it has been
  //    verified that the interface identified by <targetRepoId> is
  //    equal to or is a base class of the ior's repository ID.
  // else
  //    there is no proxy factory linked into this executable that
  //    matches the interface identified by the ior's repository ID.
  // or
  //    there _is_ a proxy factory for ior's repository ID, but we
  //    know that it is not derived from <targetRepoId>. We must
  //    contact the object, in case it actually supports an interface
  //    derived from both the ior's type and <targetRepoId>.

  int target_intf_not_confirmed = 0;

  if( !pof ) {
    pof = proxyObjectFactory::lookup(targetRepoId);
    OMNIORB_ASSERT(pof);
    // The assertion above will fail if your application attempts to
    // create an object reference while another thread is shutting
    // down the ORB.

    if( !omni::ptrStrMatch(targetRepoId, CORBA::Object::_PD_repoId) )
      target_intf_not_confirmed = 1;
  }

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "Creating ref to ";
    if      (omniLocalIdentity    ::downcast(id)) l << "local";
    else if (omniInProcessIdentity::downcast(id)) l << "in process";
    else if (omniRemoteIdentity   ::downcast(id)) l << "remote";
    else                                          l << "unknown";

    l << ": " << id << "\n"
      " target id      : " << targetRepoId << "\n"
      " most derived id: " << (const char*)ior->repositoryID() << "\n";
  }

  // Now create the object reference itself.

  omniObjRef* objref = pof->newObjRef(ior, id);
  if( target_intf_not_confirmed )  objref->pd_flags.type_verified = 0;

  {
    omni_optional_lock sync(*internalLock, locked, locked);
    id->gainRef(objref);

    if (called_create)
      id->loseRef();
  }

  if (orbParameters::persistentId.length()) {
    // Check to see if we need to re-write the IOR.

    omniIOR::IORExtraInfoList& extra = ior->getIORInfo()->extraInfo();

    for (CORBA::ULong index = 0; index < extra.length(); index++) {

      if (extra[index]->compid == IOP::TAG_OMNIORB_PERSISTENT_ID) {

	if (!id->inThisAddressSpace()) {

	  omniORB::logs(15, "Re-write local persistent object reference.");

	  omniObjRef* new_objref;
	  omniIORHints hints(0);
	  {
	    omni_optional_lock sync(*internalLock, locked, locked);

	    omniIOR* new_ior = new omniIOR(ior->repositoryID(),
					   id->key(), id->keysize(), hints);

	    new_objref = createObjRef(targetRepoId, new_ior, 1, 0);
	  }
	  releaseObjRef(objref);
	  objref = new_objref;
	}
	break;
      }
    }
  }
  return objref;
}

omniObjRef*
omni::createLocalObjRef(const char* mostDerivedRepoId,
			const char* targetRepoId,
			omniObjTableEntry* entry,
			const omniIORHints& hints)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 1);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(entry);

  // See if a suitable reference exists in the local ref list.
  // Suitable means having the same most-derived-intf-repo-id, and
  // also supporting the <targetRepoId>.
  {
    omniObjRef* objref;

    omnivector<omniObjRef*>::iterator i    = entry->objRefs().begin();
    omnivector<omniObjRef*>::iterator last = entry->objRefs().end();

    for (; i != last; i++) {
      objref = *i;

      if( omni::ptrStrMatch(mostDerivedRepoId, objref->_mostDerivedRepoId()) &&
	  objref->_ptrToObjRef(targetRepoId) ) {

	// We just need to check that the ref count is not zero here,
	// 'cos if it is then the objref is about to be deleted!
	// See omni::releaseObjRef().

	objref_rc_lock->lock();
	int dying = objref->pd_refCount == 0;
	if( !dying )  objref->pd_refCount++;
	objref_rc_lock->unlock();

	if( !dying ) {
	  omniORB::logs(15, "createLocalObjRef -- reusing reference from local"
			" ref list.");
	  return objref;
	}
      }
    }
  }
  // Reach here if we have to create a new objref.
  omniIOR* ior = new omniIOR(mostDerivedRepoId,
			     entry->key(), entry->keysize(), hints);

  return createObjRef(targetRepoId,ior,1,entry);
}

omniObjRef*
omni::createLocalObjRef(const char* mostDerivedRepoId,
			const char* targetRepoId,
			const _CORBA_Octet* key, int keysize,
			const omniIORHints& hints)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 1);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(key && keysize);

  // See if there's a suitable entry in the object table
  CORBA::ULong hashv = hash(key, keysize);

  omniObjTableEntry* entry = omniObjTable::locateActive(key, keysize,
							hashv, 0);

  if (entry)
    return createLocalObjRef(mostDerivedRepoId, targetRepoId, entry, hints);

  omniIOR* ior = new omniIOR(mostDerivedRepoId, key, keysize, hints);

  return createObjRef(targetRepoId,ior,1,entry);
}



void
omni::revertToOriginalProfile(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 0);
  OMNIORB_ASSERT(objref);

  omniORB::logs(10, "Reverting object reference to original profile");

  omniIOR_var ior = objref->_getIOR();

  omni_tracedmutex_lock sync(*internalLock);

  // We might have already been reverted... We check here
  // rather than sooner, so as to avoid holding <internalLock>
  // longer than necessary.
  if( !objref->pd_flags.forward_location ) {
    return;
  }

  ior->duplicate(); // consumed by createIdentity
  omniIdentity* id = omni::createIdentity(ior, 
					  objref->_localServantTarget(), 1);
  if( !id ) {
    OMNIORB_THROW(INV_OBJREF,INV_OBJREF_CorruptedObjRef, CORBA::COMPLETED_NO);
  }

  // For efficiency lets just assume that it exists.  We are
  // about to retry anyway -- so we'll soon find out!
  objref->pd_flags.forward_location = 0;
  objref->pd_flags.type_verified = 1;
  objref->pd_flags.object_exists = 1;

  objref->_setIdentity(id);
  id->loseRef(); // Drop the extra reference held by createIdentity.
}


void
omni::locationForward(omniObjRef* objref, omniObjRef* new_location,
		      CORBA::Boolean permanent)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 0);
  OMNIORB_ASSERT(objref);
  OMNIORB_ASSERT(new_location);

  omniORB::logs(10, "GIOP::LOCATION_FORWARD -- retry request.");

  // I suppose it is possible that a local servant was stupid
  // enough to re-direct us to itself!  If this happened it is
  // just possible that new_location == objref (if the most-
  // derived and interface types were the same).

  if( new_location == objref ) {
    releaseObjRef(new_location);
    return;
  }

  {
    omni_tracedmutex_lock sync(*internalLock);

    // We assume that the new object exists, and that it supports
    // the correct interface.  If it doesn't exist we'll get an
    // OBJECT_NOT_EXIST exception, revert to the original and try
    // again.  At worst we will keep trying again and again with
    // exponential backoff.
    //  If it supports the wrong interface then this is not our
    // fault.  It should show up as a BAD_OPERATION or something.

    objref->pd_flags.forward_location = 1;
    objref->pd_flags.object_exists = 1;
    objref->pd_flags.type_verified = 1;

    // <new_location>'s identity may well be sufficient
    // for our needs -- but if local we do need to check
    // that the servant supports the correct c++ type
    // interface.

    omniIdentity* new_id = new_location->_identity();

    omniLocalIdentity* new_lid = omniLocalIdentity::downcast(new_id);

    if (new_lid) {

      if (new_lid->deactivated() ||
	  !new_lid->servant()->_ptrToInterface(objref->_localServantTarget())){
	// Identity in new location is either dead or incompatible
	// with this object reference. Use an inProcessIdentity.

	new_id = createInProcessIdentity(new_lid->key(), new_lid->keysize());
	objref->pd_flags.type_verified = 0;
      }
    }
    objref->_setIdentity(new_id);

    if (permanent) {
      // This location forwarding is permanent, replace the IOR of this
      // object reference with the new one. If this object reference is
      // later passed to another server, the new IOR will be transferred.
      omni_tracedmutex_lock sync(*omniIOR::lock);

      new_location->pd_ior->duplicateNoLock();
      objref->pd_ior->releaseNoLock();
      objref->pd_ior = new_location->pd_ior;
      objref->pd_flags.forward_location = 0;
    }
  }
  releaseObjRef(new_location);
}


void
omni::assertFail(const char* file, int line, const char* expr)
{
  if( omniORB::trace(1) ) {
    omniORB::logger l;
    l << "Assertion failed.  This indicates a bug in the application\n"
      "using omniORB, or maybe in omniORB itself.\n"
      " file: " << file << "\n"
      " line: " << line << "\n"
      " info: " << expr << "\n";
  }
  throw omniORB::fatalException(file, line, expr);
}


void
omni::ucheckFail(const char* file, int line, const char* expr)
{
  if( omniORB::trace(1) ) {
    omniORB::logger l;
    l << "Application check failed. This indicates a bug in the application\n"
      " using omniORB.  See the comment in the source code for more info.\n"
      " file: " << file << "\n"
      " line: " << line << "\n"
      " info: " << expr << "\n";
  }
}

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class traceLevelHandler : public orbOptions::Handler {
public:

  traceLevelHandler() : 
    orbOptions::Handler("traceLevel",
			"traceLevel = n >= 0",
			1,
			"-ORBtraceLevel < n >= 0 >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_ulong_msg);
    }
    omniORB::traceLevel = v;
    if (v >= 10)
      omniORB::traceExceptions = 1;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),omniORB::traceLevel,
			   result);
  }

};

static traceLevelHandler traceLevelHandler_;

/////////////////////////////////////////////////////////////////////////////
class traceExceptionsHandler : public orbOptions::Handler {
public:

  traceExceptionsHandler() : 
    orbOptions::Handler("traceExceptions",
			"traceExceptions = 0 or 1",
			1,
			"-ORBtraceExceptions < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    omniORB::traceExceptions = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),omniORB::traceExceptions,
			     result);
  }
};

static traceExceptionsHandler traceExceptionsHandler_;

/////////////////////////////////////////////////////////////////////////////
class traceInvocationsHandler : public orbOptions::Handler {
public:

  traceInvocationsHandler() : 
    orbOptions::Handler("traceInvocations",
			"traceInvocations = 0 or 1",
			1,
			"-ORBtraceInvocations < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    omniORB::traceInvocations = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),omniORB::traceInvocations,
			     result);
  }
};

static traceInvocationsHandler traceInvocationsHandler_;

/////////////////////////////////////////////////////////////////////////////
class traceInvocationReturnsHandler : public orbOptions::Handler {
public:

  traceInvocationReturnsHandler() : 
    orbOptions::Handler("traceInvocationReturns",
			"traceInvocationReturns = 0 or 1",
			1,
			"-ORBtraceInvocationReturns < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    omniORB::traceInvocationReturns = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),omniORB::traceInvocationReturns,
			     result);
  }
};

static traceInvocationReturnsHandler traceInvocationReturnsHandler_;

/////////////////////////////////////////////////////////////////////////////
class traceThreadIdHandler : public orbOptions::Handler {
public:

  traceThreadIdHandler() : 
    orbOptions::Handler("traceThreadId",
			"traceThreadId = 0 or 1",
			1,
			"-ORBtraceThreadId < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    omniORB::traceThreadId = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),omniORB::traceThreadId,
			     result);
  }
};

static traceThreadIdHandler traceThreadIdHandler_;

/////////////////////////////////////////////////////////////////////////////
class traceTimeHandler : public orbOptions::Handler {
public:

  traceTimeHandler() : 
    orbOptions::Handler("traceTime",
			"traceTime = 0 or 1",
			1,
			"-ORBtraceTime < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    omniORB::traceTime = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),omniORB::traceTime,
			     result);
  }
};

static traceTimeHandler traceTimeHandler_;

/////////////////////////////////////////////////////////////////////////////
class traceLockingHandler : public orbOptions::Handler {
public:

  traceLockingHandler() : 
    orbOptions::Handler("traceLocking",
			"traceLocking = 0 or 1",
			1,
			"-ORBtraceLocking < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    omniORB::traceLocking = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),omniORB::traceLocking,
			     result);
  }
};

static traceLockingHandler traceLockingHandler_;

/////////////////////////////////////////////////////////////////////////////
class traceFileHandler : public orbOptions::Handler {
public:

  traceFileHandler() :
    orbOptions::Handler("traceFile",
			"traceFile = <filename>",
			1,
			"-ORBtraceFile <filename>") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    omniORB::setLogFilename(value);
  }

  void dump(orbOptions::sequenceString& result) {
    const char* n = omniORB::getLogFilename();
    orbOptions::addKVString(key(), n ? n : "[stderr]", result);
  }
};

static traceFileHandler traceFileHandler_;

/////////////////////////////////////////////////////////////////////////////
class objectTableSizeHandler : public orbOptions::Handler {
public:

  objectTableSizeHandler() : 
    orbOptions::Handler("objectTableSize",
			"objectTableSize = n >= 0",
			1,
			"-ORBobjectTableSize < n >= 0 >") {}

  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,orbOptions::expect_ulong_msg);
    }
    orbParameters::objectTableSize = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::objectTableSize,
			   result);
  }

};

static objectTableSizeHandler objectTableSizeHandler_;

/////////////////////////////////////////////////////////////////////////////
class abortOnInternalErrorHandler : public orbOptions::Handler {
public:

  abortOnInternalErrorHandler() : 
    orbOptions::Handler("abortOnInternalError",
			"abortOnInternalError = 0 or 1",
			1,
			"-ORBabortOnInternalError < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::abortOnInternalError = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::abortOnInternalError,
			     result);
  }
};

static abortOnInternalErrorHandler abortOnInternalErrorHandler_;


/////////////////////////////////////////////////////////////////////////////
class abortOnNativeExceptionHandler : public orbOptions::Handler {
public:

  abortOnNativeExceptionHandler() : 
    orbOptions::Handler("abortOnNativeException",
			"abortOnNativeException = 0 or 1",
			1,
			"-ORBabortOnNativeException < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    abortOnNativeException = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),abortOnNativeException,result);
  }
};

static abortOnNativeExceptionHandler abortOnNativeExceptionHandler_;


#ifdef WIN32_EXCEPTION_HANDLING
extern "C" void omniORB_rethrow_exception(unsigned, EXCEPTION_POINTERS*)
{
  throw;
}
static void abortOnNativeExceptionInterceptor(omniInterceptors::
					      createThread_T::info_T& info)
{
  _set_se_translator(omniORB_rethrow_exception);
  info.run();
}
#endif

/////////////////////////////////////////////////////////////////////////////
class throwTransientOnTimeOutHandler : public orbOptions::Handler {
public:

  throwTransientOnTimeOutHandler() : 
    orbOptions::Handler("throwTransientOnTimeOut",
			"throwTransientOnTimeOut = 0 or 1",
			1,
			"-ORBthrowTransientOnTimeOut < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::throwTransientOnTimeOut = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::throwTransientOnTimeOut,
			     result);
  }
};

static throwTransientOnTimeOutHandler throwTransientOnTimeOutHandler_;


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

OMNI_NAMESPACE_BEGIN(omni)

class omni_omniInternal_initialiser : public omniInitialiser {
public:

  omni_omniInternal_initialiser() {
    orbOptions::singleton().registerHandler(traceLevelHandler_);
    orbOptions::singleton().registerHandler(traceExceptionsHandler_);
    orbOptions::singleton().registerHandler(traceInvocationsHandler_);
    orbOptions::singleton().registerHandler(traceInvocationReturnsHandler_);
    orbOptions::singleton().registerHandler(traceThreadIdHandler_);
    orbOptions::singleton().registerHandler(traceTimeHandler_);
    orbOptions::singleton().registerHandler(traceLockingHandler_);
    orbOptions::singleton().registerHandler(traceFileHandler_);
    orbOptions::singleton().registerHandler(objectTableSizeHandler_);
    orbOptions::singleton().registerHandler(abortOnInternalErrorHandler_);
    orbOptions::singleton().registerHandler(abortOnNativeExceptionHandler_);
    orbOptions::singleton().registerHandler(throwTransientOnTimeOutHandler_);
  }

  void attach() {
    if (!omni::internalLock)
      omni::internalLock   = new omni_tracedmutex("omni::internalLock");

    if (!omni::poRcLock)
      omni::poRcLock       = new omni_tracedmutex("omni::poRcLock");

    if (!omni::objref_rc_lock)
      omni::objref_rc_lock = new omni_tracedmutex("omni::objref_rc_lock");

    numObjectsInTable = 0;
    minNumObjects = 0;

    if( orbParameters::objectTableSize ) {
      objectTableSize = orbParameters::objectTableSize;
      maxNumObjects = 1ul << 31;
    }
    else {
      objectTableSizeI = 0;
      objectTableSize = objTblSizes[objectTableSizeI];
      maxNumObjects = objectTableSize * 2 / 3;
    }

    objectTable = new omniObjTableEntry* [objectTableSize];
    for( CORBA::ULong i = 0; i < objectTableSize; i++ )  objectTable[i] = 0;

#ifdef WIN32_EXCEPTION_HANDLING
    if (abortOnNativeException) {
      omniInterceptors* interceptors = omniORB::getInterceptors();
      interceptors->createThread.add(abortOnNativeExceptionInterceptor);
    }
#endif
  }

  void detach() {
    if (numObjectsInTable && omniORB::trace(1)) {
      omniORB::logger l;
      l << "Error: the object table still contains "
	<< numObjectsInTable << " entr"
	<< (numObjectsInTable == 1 ? "y" : "ies")
	<< " at ORB shutdown time.\n";
    }
    OMNIORB_ASSERT(numObjectsInTable == 0);
    delete [] objectTable;
    objectTable = 0;
  }
};

static omni_omniInternal_initialiser initialiser;

omniInitialiser& omni_omniInternal_initialiser_ = initialiser;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class static_initialiser {
public:
  inline static_initialiser() {
    // Ensure that nil_ref_lock is initialised during
    // static initialisation.
    omni::nilRefLock();
  }
  static static_initialiser the_instance;
};

OMNI_NAMESPACE_END(omni)


/////////////////////////////////////////////////////////////////////////////
//            Nil object reference list                                    //
/////////////////////////////////////////////////////////////////////////////

static omnivector<CORBA::Object_ptr>*& nilObjectList() {
  static omnivector<CORBA::Object_ptr>* the_list = 0;
  if (!the_list) the_list = new omnivector<CORBA::Object_ptr>;
  return the_list;
}

static omnivector<omniTrackedObject*>*& trackedList() {
  static omnivector<omniTrackedObject*>* the_list = 0;
  if (!the_list) the_list = new omnivector<omniTrackedObject*>;
  return the_list;
}

OMNI_NAMESPACE_BEGIN(omni)

void registerNilCorbaObject(CORBA::Object_ptr obj)
{
  nilObjectList()->push_back(obj);
}

void registerTrackedObject(omniTrackedObject* obj)
{
  trackedList()->push_back(obj);
}

OMNI_NAMESPACE_END(omni)


/////////////////////////////////////////////////////////////////////////////
//            Final clean-up                                               //
/////////////////////////////////////////////////////////////////////////////

static int& count() {
  static int the_count = 0;
  return the_count;
}

_omniFinalCleanup::_omniFinalCleanup() {
  ++count();
}

_omniFinalCleanup::~_omniFinalCleanup()
{
  if (--count() != 0)
    return;

  if (!omniOrbORB::all_destroyed()) {
    omniORB::logs(15, "ORB not destroyed; no final clean-up.");
    return;
  }
  omniORB::logs(15, "Final clean-up");
  int nils = 0;
  omnivector<CORBA::Object_ptr>::iterator i = nilObjectList()->begin();
  for (; i != nilObjectList()->end(); i++, nils++)
    delete *i;

  delete nilObjectList();
  nilObjectList() = 0;

  int tracked = 0;
  omnivector<omniTrackedObject*>::iterator j = trackedList()->begin();
  for (; j != trackedList()->end(); j++, tracked++)
    delete *j;

  delete trackedList();
  trackedList() = 0;

  if (omniORB::trace(15)) {
    omniORB::logger l;
    l << "Deleted " << nils << " nil object reference"
      << (nils == 1 ? "" : "s" )
      << " and " << tracked << " other tracked object"
      << (tracked == 1 ? "" : "s" )
      << ".\n";
  }

  // Remove list of proxyObjectFactories
  proxyObjectFactory::shutdown();

  // Delete mutexes
  delete &omni::nilRefLock();
  if (omni::internalLock)   delete omni::internalLock;
  if (omni::objref_rc_lock) delete omni::objref_rc_lock;
  if (omni::poRcLock)       delete omni::poRcLock;
  if (omniTransportLock)    delete omniTransportLock;
  if (omniIOR::lock)        delete omniIOR::lock;

  omni::internalLock   = 0;
  omni::objref_rc_lock = 0;
  omni::poRcLock       = 0;
  omniTransportLock    = 0;
  omniIOR::lock        = 0;

  omniORB::logs(10, "Final clean-up completed.");
}
