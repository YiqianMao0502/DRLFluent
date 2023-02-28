// -*- Mode: C++; -*-
//                            Package   : omniORB
// corbaOrb.cc                Created on: 6/2/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2013 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//      Implementation of the ORB interface
//

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <corbaOrb.h>
#include <initRefs.h>
#include <omniORB4/omniObjRef.h>
#include <poaimpl.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/objTracker.h>
#include <giopStreamImpl.h>
#include <invoker.h>
#include <omniCurrent.h>
#include <omniORB4/distdate.hh>
#include <orbOptions.h>
#include <orbParameters.h>
#include <omniIdentity.h>
#include <SocketCollection.h>

#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#  include <errno.h>
#endif
#include <stdio.h>
#include <stdlib.h>

OMNI_USING_NAMESPACE(omni)

#define ORB_ID_STRING "omniORB4"

static const char* orb_ids[] = { ORB_ID_STRING,
				 "omniORB3",
				 "omniORB2",
				 0 };

static omniOrbORB*          the_orb                   = 0;
static int                  orb_count                 = 0;
static omni_tracedmutex     orb_lock("orb_lock");
static omni_tracedcondition orb_signal(&orb_lock, "orb_signal");
static volatile int         orb_n_blocked_in_run      = 0;

#ifdef __SINIX__
// Why haven't we got this signature from signal.h? - sll
//
extern "C" int sigaction(int, const struct sigaction *, struct sigaction *);
#endif

// Config file
#if defined(NTArchitecture) && !defined(__ETS_KERNEL__)
static const char* config_fname = 0;
#else
static const char* config_fname = CONFIG_DEFAULT_LOCATION;
#endif



///////////////////////////////////////////////////////////////////////
//          Per module initialisers.
//
OMNI_NAMESPACE_BEGIN(omni)

omniAsyncInvoker* orbAsyncInvoker = 0;

extern omniInitialiser& omni_omniIOR_initialiser_;
extern omniInitialiser& omni_corbaOrb_initialiser_;
extern omniInitialiser& omni_omniInternal_initialiser_;
extern omniInitialiser& omni_initRefs_initialiser_;
extern omniInitialiser& omni_hooked_initialiser_;
extern omniInitialiser& omni_interceptor_initialiser_;
extern omniInitialiser& omni_ior_initialiser_;
extern omniInitialiser& omni_codeSet_initialiser_;
extern omniInitialiser& omni_cdrStream_initialiser_;
extern omniInitialiser& omni_giopStrand_initialiser_;
extern omniInitialiser& omni_giopStreamImpl_initialiser_;
extern omniInitialiser& omni_giopRope_initialiser_;
extern omniInitialiser& omni_giopserver_initialiser_;
extern omniInitialiser& omni_giopbidir_initialiser_;
extern omniInitialiser& omni_omniTransport_initialiser_;
extern omniInitialiser& omni_omniCurrent_initialiser_;
extern omniInitialiser& omni_dynamiclib_initialiser_;
extern omniInitialiser& omni_objadpt_initialiser_;
extern omniInitialiser& omni_giopEndpoint_initialiser_;
extern omniInitialiser& omni_transportRules_initialiser_;
extern omniInitialiser& omni_ObjRef_initialiser_;
extern omniInitialiser& omni_orbOptions_initialiser_;
extern omniInitialiser& omni_poa_initialiser_;
extern omniInitialiser& omni_uri_initialiser_;
extern omniInitialiser& omni_invoker_initialiser_;

OMNI_NAMESPACE_END(omni)


//////////////////////////////////////////////////////////////////////
///////////////////////////// CORBA::ORB /////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ORB::~ORB()  {}


CORBA::ORB_ptr
CORBA::ORB::_duplicate(CORBA::ORB_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


CORBA::ORB_ptr
CORBA::ORB::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) || !obj->_NP_is_pseudo() )  return _nil();

  ORB_ptr p = (ORB_ptr) obj->_ptrToObjRef(_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}


CORBA::ORB_ptr
CORBA::ORB::_nil()
{
  static omniOrbORB* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniOrbORB(1 /* is nil */);
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


const char*
CORBA::ORB::_PD_repoId = "IDL:omg.org/CORBA/ORB:1.0";

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x500

#include <exception.h>
static void omni_abort()
{
  abort();
}

#endif
#endif

static void enableLcdMode() {
  orbParameters::strictIIOP = 0;
  orbParameters::tcAliasExpand = 1;
  orbParameters::scanGranularity = 0;
  orbParameters::outConScanPeriod = 0;
  orbParameters::inConScanPeriod = 0;
  orbParameters::useTypeCodeIndirections = 0;
  orbParameters::verifyObjectExistsAndType = 0;
  orbParameters::acceptMisalignedTcIndirections = 1;
}

CORBA::ORB_ptr
CORBA::ORB_init(int& argc, char** argv, const char* orb_identifier,
		const char* options[][2])
{
  omni_tracedmutex_lock sync(orb_lock);

  if( the_orb ) {
    the_orb->_NP_incrRefCount();
    return the_orb;
  }

  const char* option_src_1  = "configuration file";
  const char* option_src_2  = "environment variable";
  const char* option_src_3  = "argument";
  const char* option_src_4  = "option list";
  const char* option_src_5  = "-ORB arguments";
  const char* option_source = 0;
  try {

    orbOptions::singleton().reset();

    // Look for -ORBtraceLevel arg first
    option_source = option_src_5;
    orbOptions::singleton().getTraceLevel(argc,argv);

    {
      const char* f = getenv(CONFIG_ENV);
      if (f) config_fname = f;
    }
    // Configuration file name can be overriden by command line.
    config_fname = orbOptions::singleton().getConfigFileName(argc, argv,
							     config_fname);

    // Parse configuration file
    option_source = option_src_1;

    if (config_fname) {
      orbOptions::singleton().importFromFile(config_fname);
    }
#if defined(NTArchitecture) && !defined(__ETS_KERNEL__)
    else {
      // Parse configuration from registry on NT if no configuration
      // file is specified.
      if (!orbOptions::singleton().importFromRegistry()) {
	// Failed to read from the registry. Try the default file location.
	config_fname = CONFIG_DEFAULT_LOCATION;
	orbOptions::singleton().importFromFile(config_fname);
      }
    }
#endif

    // Parse configuration from environment variables
    option_source = option_src_2;
    orbOptions::singleton().importFromEnv();


    if ( orb_identifier && strlen(orb_identifier) ) {
      option_source = option_src_3;
      orbOptions::singleton().addOption("id",orb_identifier);
    }

    // Parse configuration from argument <options>
    if (options) {
      option_source = option_src_4;
      orbOptions::singleton().addOptions(options);
    }

    // Parse configurations from argv
    option_source = option_src_5;
    orbOptions::singleton().extractInitOptions(argc,argv);

  }
  catch (const orbOptions::Unknown& ex) {
    if ( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "ORB_init failed: unknown option ("
	<< ex.key << ") in " << option_source << "\n";
    }
    OMNIORB_THROW(INITIALIZE,INITIALIZE_InvalidORBInitArgs,
		  CORBA::COMPLETED_NO);
  }
  catch (const orbOptions::BadParam& ex) {
    if ( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "ORB_init failed: Bad parameter (" << ex.value
	<< ") for option "
	<< ((option_source == option_src_5) ? "-ORB" : "")
	<< ex.key << " in " <<  option_source << ", reason: "
	<< ex.why << "\n";
    }
    OMNIORB_THROW(INITIALIZE,INITIALIZE_InvalidORBInitArgs,
		  CORBA::COMPLETED_NO);
  }

  try {
    orbOptions::singleton().visit();
  }
  catch (const orbOptions::BadParam& ex) {
    if ( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "ORB_init failed: Bad parameter (" << ex.value
	<< ") for ORB configuration option " << ex.key
	<< ", reason: " << ex.why << "\n";
    }
    OMNIORB_THROW(INITIALIZE,INITIALIZE_InvalidORBInitArgs,
		  CORBA::COMPLETED_NO);
  }

  omniORB::logs(2, "Version: " OMNIORB_VERSION_STRING);
  omniORB::logs(2, "Distribution date: " OMNIORB_DIST_DATE);

  try {
    // Call attach method of each initialiser object.
    // The order of these calls must take into account of the dependency
    // among the modules.
    omni_giopEndpoint_initialiser_.attach();
    omni_transportRules_initialiser_.attach();
    omni_interceptor_initialiser_.attach();
    omni_omniInternal_initialiser_.attach();
    omni_corbaOrb_initialiser_.attach();
    omni_objadpt_initialiser_.attach();
    omni_giopStreamImpl_initialiser_.attach();
    omni_omniIOR_initialiser_.attach();
    omni_ior_initialiser_.attach();
    omni_codeSet_initialiser_.attach();
    omni_cdrStream_initialiser_.attach();
    omni_omniTransport_initialiser_.attach();
    omni_giopRope_initialiser_.attach();
    omni_giopserver_initialiser_.attach();
    omni_giopbidir_initialiser_.attach();
    omni_giopStrand_initialiser_.attach();
    omni_omniCurrent_initialiser_.attach();
    omni_dynamiclib_initialiser_.attach();
    omni_ObjRef_initialiser_.attach();
    omni_initRefs_initialiser_.attach();
    omni_orbOptions_initialiser_.attach();
    omni_poa_initialiser_.attach();
    omni_uri_initialiser_.attach();
    omni_invoker_initialiser_.attach();
    omni_hooked_initialiser_.attach();

    if (orbParameters::lcdMode) {
      enableLcdMode();
    }

    if (omniORB::trace(20) || orbParameters::dumpConfiguration) {
      orbOptions::sequenceString_var currentSet;
      currentSet = orbOptions::singleton().dumpCurrentSet();
      omniORB::logger l;
      l << "Current configuration is as follows:\n";
      for (CORBA::ULong i = 0; i < currentSet->length(); i++)
	l << "omniORB:   " << (const char*)currentSet[i] << "\n";
    }
  }
  catch (CORBA::INITIALIZE&) {
    throw;
  }
  catch (...) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_FailedORBInit,CORBA::COMPLETED_NO);
  }

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x500
  // Sun C++ 5.0 or Forte C++ 6.0 generated code will segv occasionally
  // when concurrent threads throw an exception. The stack trace points
  // to a problem in the exception unwinding. The workaround seems to be
  // to install explicitly an uncaught exception handler, which is what
  // we do here.
  set_terminate(omni_abort);
#endif
#endif

  the_orb = new omniOrbORB(0);
  the_orb->_NP_incrRefCount();
  orb_count++;
  return the_orb;
}

//////////////////////////////////////////////////////////////////////
///////////////////////////// omniOrbORB /////////////////////////////
//////////////////////////////////////////////////////////////////////

#define CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED()  \
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();  \
  if( pd_destroyed )  OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_NoMatch, CORBA::COMPLETED_NO);  \
  if( pd_shutdown  )  OMNIORB_THROW(BAD_INV_ORDER, \
                                    BAD_INV_ORDER_ORBHasShutdown, \
                                    CORBA::COMPLETED_NO);  \

CORBA::Boolean
omniOrbORB::all_destroyed()
{
  return orb_count == 0;
}


omniOrbORB::~omniOrbORB()  {}


omniOrbORB::omniOrbORB(int nil)
  : OMNIORB_BASE_CTOR(CORBA::)ORB(nil),
    pd_refCount(1),
    pd_destroyed(0),
    pd_shutdown(0),
    pd_shutdown_in_progress(0)
{
}

char*
omniOrbORB::id()
{
  return CORBA::string_dup("");
}


char*
omniOrbORB::object_to_string(CORBA::Object_ptr obj)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();
  return omniURI::objectToString(obj);
}


CORBA::Object_ptr
omniOrbORB::string_to_object(const char* uri)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();
  return omniURI::stringToObject(uri);
}


CORBA::ORB::ObjectIdList*
omniOrbORB::list_initial_services()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  CORBA::ORB::ObjectIdList* ids = omniInitialReferences::list();
  CORBA::ORB::ObjectIdList& idl = *ids;

  CORBA::ULong len = idl.length();
  idl.length(len + 2);
  idl[len++] = CORBA::string_dup("RootPOA");
  idl[len++] = CORBA::string_dup("POACurrent");

  return ids;
}


CORBA::Object_ptr
omniOrbORB::resolve_initial_references(const char* id)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();
  return omniInitialReferences::resolve(id);
}

CORBA::Boolean
omniOrbORB::work_pending()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  omni_thread* self = omni_thread::self();

  if (self && self->id() == omni::mainThreadId)
    return orbAsyncInvoker->work_pending();

  return 0;
}


void
omniOrbORB::perform_work()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  omni_thread* self = omni_thread::self();

  if (self && self->id() == omni::mainThreadId) {
    unsigned long s, ns;
    omni_thread::get_time(&s, &ns);
    orbAsyncInvoker->perform(s, ns);
  }
}


void
omniOrbORB::run()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  omni_thread* self = omni_thread::self();

  if (self && self->id() == omni::mainThreadId) {
    orbAsyncInvoker->perform();
  }
  else {
    omni_tracedmutex_lock l(orb_lock);

    orb_n_blocked_in_run++;

    while (!pd_shutdown)
      orb_signal.wait();

    orb_n_blocked_in_run--;
  }
}


CORBA::Boolean
omniOrbORB::run_timeout(unsigned long secs, unsigned long nanosecs)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  omni_thread* self = omni_thread::self();

  if (self && self->id() == omni::mainThreadId) {
    orbAsyncInvoker->perform(secs, nanosecs);
  }
  else {
    omni_tracedmutex_lock l(orb_lock);

    orb_n_blocked_in_run++;

    if (!pd_shutdown)
      orb_signal.timedwait(secs, nanosecs);

    orb_n_blocked_in_run--;
  }
  return pd_shutdown;
}



void
omniOrbORB::shutdown(CORBA::Boolean wait_for_completion)
{
  omni_tracedmutex_lock sync(orb_lock);

  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  if( wait_for_completion ) {
    // Complain if in the context of an operation invocation
    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }
  }
  do_shutdown(wait_for_completion);
}


void
omniOrbORB::destroy()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();

  omniOrbORB* orb;
  {
    omni_tracedmutex_lock sync(orb_lock);

    if( pd_destroyed )  OMNIORB_THROW(BAD_INV_ORDER,
				      BAD_INV_ORDER_ORBHasShutdown,
				      CORBA::COMPLETED_NO);

    // Complain if in the context of an operation invocation
    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }

    if( !pd_shutdown )  do_shutdown(1);

    if( pd_destroyed ) {
      omniORB::logs(15, "ORB destroyed by another thread.");
      return;
    }

    omniORB::logs(5, "Destroy ORB...");

    // Call detach method of the initialisers in reverse order.
    omni_hooked_initialiser_.detach();
    omni_invoker_initialiser_.detach();
    omni_uri_initialiser_.detach();
    omni_poa_initialiser_.detach();
    omni_orbOptions_initialiser_.detach();
    omni_initRefs_initialiser_.detach();
    omni_ObjRef_initialiser_.detach();
    omni_dynamiclib_initialiser_.detach();
    omni_omniCurrent_initialiser_.detach();
    omni_giopStrand_initialiser_.detach();
    omni_giopbidir_initialiser_.detach();
    omni_giopserver_initialiser_.detach();
    omni_giopRope_initialiser_.detach();
    omni_omniTransport_initialiser_.detach();
    omni_cdrStream_initialiser_.detach();
    omni_codeSet_initialiser_.detach();
    omni_ior_initialiser_.detach();
    omni_omniIOR_initialiser_.detach();
    omni_giopStreamImpl_initialiser_.detach();
    omni_objadpt_initialiser_.detach();
    omni_corbaOrb_initialiser_.detach();
    omni_omniInternal_initialiser_.detach();
    omni_interceptor_initialiser_.detach();
    omni_transportRules_initialiser_.detach();
    omni_giopEndpoint_initialiser_.detach();

    pd_destroyed = 1;
    orb = the_orb;
    the_orb = 0;
    orb_count--;
  }
  CORBA::release(orb);
  omniORB::logs(5, "ORB destroyed.");
}


void
omniOrbORB::register_initial_reference(const char* id, CORBA::Object_ptr obj)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  omniInitialReferences::setFromORB(id, obj);
}



CORBA::Boolean
omniOrbORB::_non_existent()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  orb_lock.lock();
  CORBA::Boolean ret = pd_destroyed ? 1 : 0;
  orb_lock.unlock();

  return ret;
}


void*
omniOrbORB::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, CORBA::ORB::_PD_repoId) )
    return (CORBA::ORB_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


void
omniOrbORB::_NP_incrRefCount()
{
  omni::poRcLock->lock();
  pd_refCount++;
  omni::poRcLock->unlock();
}


void
omniOrbORB::_NP_decrRefCount()
{
  omni::poRcLock->lock();
  int done = --pd_refCount > 0;
  omni::poRcLock->unlock();
  if( done )  return;

  OMNIORB_USER_CHECK(pd_destroyed);
  OMNIORB_USER_CHECK(pd_refCount == 0);
  // If either of these fails then the application has released the
  // ORB reference too many times.

  omniORB::logs(15, "No more references to the ORB -- deleted.");

  delete this;
}


void
omniOrbORB::actual_shutdown()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(orb_lock, 1);
  OMNIORB_ASSERT(pd_shutdown_in_progress);

  // Release lock while shutting down subsystems
  orb_lock.unlock();

  CORBA::Boolean failure = 0;

  // Shutdown object adapters.  When this returns all
  // outstanding requests have completed.
  try {
    omniOrbPOA::shutdown();
  }
  catch (...) {
    omniORB::logs(1, "Unexpected exception shutting down POAs.");
    failure = 1;
  }

  // Shutdown incoming connections.
  try {
    omniObjAdapter::shutdown();
  }
  catch (...) {
    omniORB::logs(1, "Unexpected exception shutting down connections.");
    failure = 1;
  }

  // Disable object references
  try {
    omniObjRef::_shutdown();
  }
  catch (...) {
    omniORB::logs(1, "Unexpected exception disabling object references.");
    failure = 1;
  }

  if (!failure) {
    try {
      // Wait for all client requests to complete
      omniIdentity::waitForLastIdentity();
      omniORB::logs(10, "ORB shutdown is complete.");
    }
    catch (...) {
      omniORB::logs(1, "Unexpected exception waiting for "
                    "client requests to complete.");
      failure = 1;
    }
  }

  if (failure) {
    omniORB::logs(1, "ORB shutdown completed with errors.");
  }

  orb_lock.lock();
  pd_shutdown = 1;

  // Wake up threads stuck in run().
  orb_signal.broadcast();

  // Wake up main thread if there is one running
  orbAsyncInvoker->shutdown();
}


static void
shutdown_thread_fn(void* arg)
{
  OMNIORB_ASSERT(arg);

  omniORB::logs(15, "ORB shutdown thread started.");

  omni_tracedmutex_lock sync(orb_lock);
  ((omniOrbORB*) arg)->actual_shutdown();
}


void
omniOrbORB::do_shutdown(CORBA::Boolean wait_for_completion)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(orb_lock, 1);

  if( pd_shutdown )  return;

  if( pd_shutdown_in_progress ) {
    if( wait_for_completion ) {
      omniORB::logs(15, "ORB shutdown already in progress -- waiting.");
      orb_n_blocked_in_run++;
      while( !pd_shutdown )  orb_signal.wait();
      orb_n_blocked_in_run--;
      omniORB::logs(15, "ORB shutdown complete -- finished waiting.");
    }
    else {
      omniORB::logs(15, "ORB shutdown already in progress -- nothing to do.");
    }
    return;
  }

  omniORB::logs(10, "Preparing to shutdown ORB.");

  pd_shutdown_in_progress = 1;

  if( wait_for_completion ) {
    actual_shutdown();
  }
  else {
    // If wait_for_completion is zero we need to pass this to another
    // thread. This is needed to support shutting down the orb from
    // a method invocation -- otherwise we would deadlock waiting for
    // the method invocation to complete.

    omniORB::logs(15, "Starting an ORB shutdown thread.");

    try {
      (new omni_thread(shutdown_thread_fn, (omniOrbORB*) this))->start();
    }
    catch (const omni_thread_fatal& ex) {
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Unable to start ORB shutdown thread (error "
	    << ex.error << ").\n";
      }
      OMNIORB_THROW(NO_RESOURCES,
		    NO_RESOURCES_UnableToStartThread,
		    CORBA::COMPLETED_NO);
    }
  }
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static
CORBA::Boolean
isValidId(const char* id) {
  const char** p = orb_ids;
  while (*p) {
    if (strcmp(*p,id) == 0) return 1;
    p++;
  }
  return 0;
}

static
const char*
myOrbId() {
  return orb_ids[0];
}

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////////
//            Hooked initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

static omnivector<omniInitialiser*>*& the_hooked_list()
{
  static omnivector<omniInitialiser*>* the_list = 0;
  if (!the_list) the_list = new omnivector<omniInitialiser*>;
  return the_list;
}

class omni_hooked_initialiser : public omniInitialiser {
public:
  void attach() {
    omnivector<omniInitialiser*>::iterator i    = the_hooked_list()->begin();
    omnivector<omniInitialiser*>::iterator last = the_hooked_list()->end();

    for (; i != last; i++) {
      (*i)->attach();
    }
  }

  void detach() {
    omnivector<omniInitialiser*>::iterator i    = the_hooked_list()->begin();
    omnivector<omniInitialiser*>::iterator last = the_hooked_list()->end();

    for (; i != last; i++) {
      (*i)->detach();
    }
  }

  virtual ~omni_hooked_initialiser() {
    omnivector<omniInitialiser*>*& the_list = the_hooked_list();
    delete the_list;
    the_list = 0;
  }
};

static omni_hooked_initialiser hinitialiser;
omniInitialiser& omni_hooked_initialiser_ = hinitialiser;

void
omniInitialiser::
install(omniInitialiser* init) {
  the_hooked_list()->push_back(init);
}


////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
CORBA::Boolean   orbParameters::dumpConfiguration = 0;
//  Set to 1 to cause the ORB to dump the current set of configuration
//  parameters.
//
//  Valid values = 0 or 1

CORBA::Boolean   orbParameters::lcdMode = 0;
//  Set to 1 to enable 'Lowest Common Denominator' Mode.
//  This will disable various features of IIOP and GIOP which are
//  poorly supported by some ORBs, and disable warnings/errors when
//  certain types of erroneous message are received on the wire.
//
//  Valid values = 0 or 1


/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class helpHandler : public orbOptions::Handler {
public:

  helpHandler() :
    orbOptions::Handler("help",
			0,
			1,
			"-ORBhelp ",
			1) {}


  void visit(const char*,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    orbOptions::sequenceString_var usage;
    usage = orbOptions::singleton().usageArgv();

    omniORB::logger l;
    l << "Valid -ORB<options> are:\n";
    for (CORBA::ULong i = 0; i < usage->length(); i++)
      l << "  " << usage[i] << "\n";

  }

  void dump(orbOptions::sequenceString& result) {
    return;
  }
};

static helpHandler helpHandler_;

/////////////////////////////////////////////////////////////////////////////
class idHandler : public orbOptions::Handler {
public:

  idHandler() :
    orbOptions::Handler("id",
			"id = " ORB_ID_STRING,
			1,
			"-ORBid " ORB_ID_STRING " (standard option)") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    if (!isValidId(value)) {
      throw orbOptions::BadParam(key(),value,"id is not " ORB_ID_STRING);
    }
    if( strcmp(value, myOrbId()) ) {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "Warning: using ORBid " << value
	  << " (should be " << ORB_ID_STRING << ")." << "\n";
      }
    }
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVString(key(),ORB_ID_STRING,result);
  }
};

static idHandler idHandler_;

/////////////////////////////////////////////////////////////////////////////
class dumpConfigurationHandler : public orbOptions::Handler {
public:

  dumpConfigurationHandler() :
    orbOptions::Handler("dumpConfiguration",
			"dumpConfiguration = 0 or 1",
			1,
			"-ORBdumpConfiguration < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::dumpConfiguration = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::dumpConfiguration,
			     result);
  }
};

static dumpConfigurationHandler dumpConfigurationHandler_;

/////////////////////////////////////////////////////////////////////////////
class lcdModeHandler : public orbOptions::Handler {
public:

  lcdModeHandler() :
    orbOptions::Handler("lcdMode",
			"lcdMode = 0 or 1",
			1,
			"-ORBlcdMode < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::lcdMode = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::lcdMode,
			     result);
  }
};

static lcdModeHandler lcdModeHandler_;


/////////////////////////////////////////////////////////////////////////////
class principalHandler : public orbOptions::Handler {
public:

  principalHandler() :
    orbOptions::Handler("principal",
			"principal = <GIOP 1.0 principal string>",
			1,
			"-ORBprincipal <GIOP 1.0 principal string>") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong l = (CORBA::ULong)strlen(value) + 1;
    omni::myPrincipalID.length(l);
    for (CORBA::ULong i = 0; i < l; i++)
      omni::myPrincipalID[i] = value[i];
  }

  void dump(orbOptions::sequenceString& result) {
    if (omni::myPrincipalID.length() == 0)
      orbOptions::addKVString(key(),"[Null]",result);
    else {
      CORBA::String_var s(CORBA::string_alloc(omni::myPrincipalID.length()+1));
      CORBA::ULong i;
      for (i=0; i<omni::myPrincipalID.length(); i++) {
	((char*)s)[i] = omni::myPrincipalID[i];
      }
      ((char*)s)[i] = '\0';
      orbOptions::addKVString(key(),s,result);
    }
  }
};

static principalHandler principalHandler_;

/////////////////////////////////////////////////////////////////////////////
class configFileHandler : public orbOptions::Handler {
public:

  configFileHandler() :
    orbOptions::Handler("configFile",
			"configFile = <filename>",
			1,
			"-ORBconfigFile <filename>") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    // Do nothing -- already handled before normal arguments are processed
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVString(key(), config_fname ? config_fname : "[none]",
			    result);
  }
};

static configFileHandler configFileHandler_;


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_corbaOrb_initialiser : public omniInitialiser {
public:

  omni_corbaOrb_initialiser() {
    orbOptions::singleton().registerHandler(helpHandler_);
    orbOptions::singleton().registerHandler(idHandler_);
    orbOptions::singleton().registerHandler(dumpConfigurationHandler_);
    orbOptions::singleton().registerHandler(lcdModeHandler_);
    orbOptions::singleton().registerHandler(principalHandler_);
    orbOptions::singleton().registerHandler(configFileHandler_);
  }

  void attach() {

#if !defined(__CIAO__)
# if defined(HAVE_SIGACTION)

    struct sigaction act;
    sigemptyset(&act.sa_mask);
#  ifdef HAVE_SIG_IGN
    act.sa_handler = SIG_IGN;
#  else
    act.sa_handler = (void (*)())0;
#  endif
    act.sa_flags = 0;
    if (sigaction(SIGPIPE,&act,0) < 0) {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "Warning: ORB_init() cannot install the "
	  "SIG_IGN handler for signal SIGPIPE. (errno = " << errno << ")\n";
      }
    }
# elif defined(HAVE_SIGVEC)
    struct sigvec act;
    act.sv_mask = 0;
    act.sv_handler = SIG_IGN;
    act.sv_flags = 0;
    if (sigvec(SIGPIPE,&act,0) < 0) {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "Warning: ORB_init() cannot install the "
	  "SIG_IGN handler for signal SIGPIPE. (errno = " << errno << ")\n";
      }
    }
# endif // HAVE_SIGACTION
#endif // __CIAO__

    orbAsyncInvoker = new omniAsyncInvoker();
  }

  void detach() {
    if (orbAsyncInvoker) {
      delete orbAsyncInvoker;
      orbAsyncInvoker = 0;
    }
#ifdef __WIN32__
    (void) WSACleanup();
#endif
  }
};

static omni_corbaOrb_initialiser initialiser;

omniInitialiser& omni_corbaOrb_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
