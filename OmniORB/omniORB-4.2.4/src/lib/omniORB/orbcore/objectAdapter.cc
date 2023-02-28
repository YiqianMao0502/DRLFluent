// -*- Mode: C++; -*-
//                            Package   : omniORB
// objectAdapter.cc           Created on: 5/3/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2002-2012 Apasphere Ltd
//    Copyright (C) 1996,1999 AT&T Research Cambridge
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

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <objectAdapter.h>
#include <localIdentity.h>
#include <initRefs.h>
#include <poaimpl.h>
#include <corbaBoa.h>
#include <exceptiondefs.h>
#include <giopServer.h>
#include <giopRope.h>
#include <omniORB4/omniInterceptors.h>
#include <omniORB4/omniURI.h>
#include <interceptors.h>
#include <initialiser.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <libcWrapper.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

OMNI_NAMESPACE_BEGIN(omni)

static char                             initialised = 0;
static int                              num_active_oas = 0;
static omni_tracedmutex                 oa_lock("oa_lock");
static omnivector<_OMNI_NS(orbServer)*> oa_servers;
static orbServer::EndpointList          oa_endpoints;

omni_tracedmutex omniObjAdapter::sd_detachedObjectLock(
  "omniObjAdapter::sd_detachedObjectLock");

omni_tracedcondition omniObjAdapter::sd_detachedObjectSignal(
  &omniObjAdapter::sd_detachedObjectLock,
  "omniObjAdapter::sd_detachedObjectSignal");

omniObjAdapter::Options omniObjAdapter::options;


//////////////////////////////////////////////////////////////////////
omniObjAdapter::~omniObjAdapter()
{
  OMNIORB_ASSERT(pd_signal == 0);
}


//////////////////////////////////////////////////////////////////////
omniObjAdapter*
omniObjAdapter::getAdapter(const _CORBA_Octet* key, int keysize)
{
  omniObjAdapter* adapter;

  adapter = omniOrbPOA::getAdapter(key, keysize);
  if( adapter )  return adapter;

  if( keysize == sizeof(omniOrbBoaKey) )
    return omniOrbBOA::theBOA();

  return 0;
}


//////////////////////////////////////////////////////////////////////
_CORBA_Boolean
omniObjAdapter::isInitialised()
{
  omni_tracedmutex_lock sync(oa_lock);

  return initialised;
}


//////////////////////////////////////////////////////////////////////
_CORBA_Boolean
omniObjAdapter::isDeactivating()
{
  return !num_active_oas;
}


//////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
instantiate_endpoint(const char*              uri,
		     CORBA::Boolean           no_publish,
		     orbServer::EndpointList& listening_endpoints)
{
  if (omniORB::trace(20)) {
    omniORB::logger l;
    l << "Instantiate endpoint '" << uri << "'"
      << (no_publish ? " (no publish)" : "") << "\n";
  }
  omnivector<orbServer*>::iterator j,last;
  CORBA::Boolean ok = 0;
  j = oa_servers.begin();
  last = oa_servers.end();
  for ( ; j != last; j++ ) {
    ok = (*j)->instantiate(uri, no_publish, listening_endpoints);
    if (ok) break;
  }
  return ok;
}

//////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
publish_endpoints(const char* publish_spec,
		  orbServer::EndpointList& published_endpoints)
{
  if (omniORB::trace(15)) {
    omniORB::logger l;
    l << "Publish specification: '" << publish_spec << "'\n";
  }

  char* buffer = CORBA::string_alloc(strlen(publish_spec));
  CORBA::String_var buffer_var(buffer);

  CORBA::Boolean result = 0;
  CORBA::Boolean ok;

  const char* pc = publish_spec;
  char* bc = buffer;

  orbServer::PublishSpecs pspecs;
  CORBA::ULong psi = 0;

  CORBA::Boolean all_specs = 0;
  CORBA::Boolean all_eps   = 0;
  CORBA::Boolean all_open  = 0;

  while (1) {
    if (!strncasecmp(pc, "all(", 4)) {
      all_open = 1;
      pc += 4;
      continue;
    }
    if (*pc == '|') {
      if (all_specs || bc == buffer)
	OMNIORB_THROW(INITIALIZE,
		      INITIALIZE_EndpointPublishFailure,
		      CORBA::COMPLETED_NO);
      *bc = '\0';
      if (psi == pspecs.length())
	pspecs.length((psi + 1) * 2);
      
      pspecs[psi++] = CORBA::string_dup(buffer);
      bc = buffer;
    }
    else if (*pc == ')') {
      if (!all_open)
	OMNIORB_THROW(INITIALIZE,
		      INITIALIZE_EndpointPublishFailure,
		      CORBA::COMPLETED_NO);

      if (pc[1] != ',' && pc[1] != '\0')
	OMNIORB_THROW(INITIALIZE,
		      INITIALIZE_EndpointPublishFailure,
		      CORBA::COMPLETED_NO);
      all_open = 0;
      all_eps  = 1;
    }
    else if (*pc == ',' || *pc == '\0') {
      if (bc == buffer)
	OMNIORB_THROW(INITIALIZE,
		      INITIALIZE_EndpointPublishFailure,
		      CORBA::COMPLETED_NO);

      if (all_open) {
	if ((!all_specs && pspecs.length()))
	  OMNIORB_THROW(INITIALIZE,
			INITIALIZE_EndpointPublishFailure,
			CORBA::COMPLETED_NO);
	all_specs = 1;
	*bc = '\0';
	if (psi == pspecs.length())
	  pspecs.length((psi + 1) * 2);
      
	pspecs[psi++] = CORBA::string_dup(buffer);
	bc = buffer;
      }
      else {
	*bc = '\0';
	if (psi == pspecs.length())
	  pspecs.length(psi + 1);
      
	pspecs[psi++] = CORBA::string_dup(buffer);
	bc = buffer;

	pspecs.length(psi);

	// Time to actually publish something
	omnivector<orbServer*>::iterator j,last;
	j = oa_servers.begin();
	last = oa_servers.end();
	for (; j != last; ++j) {
	  ok = (*j)->publish(pspecs, all_specs, all_eps, published_endpoints);
	  result |= ok;
	}

	psi = 0;
	all_specs = 0;
	all_eps   = 0;
      }
    }
    else if (isspace(*pc)) {
      // Do nothing
    }
    else {
      *bc++ = tolower(*pc);
    }
    if (*pc == '\0') break;

    ++pc;
  }
  return result;
}



//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::initialise()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( initialised )  return;

  omniORB::logs(10, "Initialising incoming endpoints.");

  try {

    orbServer::EndpointList listening_endpoints;
    orbServer::EndpointList published_endpoints;

    if ( oa_servers.empty() ) {
      omniInterceptors::createORBServer_T::info_T info(oa_servers);
      omniInterceptorP::visit(info);
    }

    if ( !options.endpoints.empty() ) {

      Options::EndpointURIList::iterator i = options.endpoints.begin();
      for ( ; i != options.endpoints.end(); i++ ) {

	CORBA::Boolean ok = instantiate_endpoint((*i)->uri,
						 (*i)->no_publish,
						 listening_endpoints);
	if (!ok) {
	  if (omniORB::trace(1)) {
	    omniORB::logger log;
	    log << "Error: Unable to create an endpoint of this description: "
		<< (const char*)(*i)->uri
		<< "\n";
	  }
	  OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,
			CORBA::COMPLETED_NO);
	}
      }
    }
    else {
      // instantiate a default tcp port.
      const char* hostname = getenv(OMNIORB_USEHOSTNAME_VAR);
      if( !hostname )  hostname = "";

      CORBA::String_var estr = omniURI::buildURI("giop:tcp", hostname, 0);

      CORBA::Boolean ok = instantiate_endpoint(estr, 0, listening_endpoints);

      if (!ok) {
	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  log << "Error: Unable to create an endpoint of this description: "
	      << (const char*)estr
	      << "\n";
	}
	OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,
		      CORBA::COMPLETED_NO);
      }
    }

    // Handle the publish specification
    if (!publish_endpoints(options.publish, published_endpoints)) {
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Error: endPointPublish specification '"
	    << options.publish << "' did not publish any endpoints.\n";
      }
      OMNIORB_THROW(INITIALIZE,
		    INITIALIZE_EndpointPublishFailure,
		    CORBA::COMPLETED_NO);
    }

    // Build master list of all endpoints that relate to this process
    {
      oa_endpoints = listening_endpoints;
      CORBA::ULong i;
      CORBA::ULong j = oa_endpoints.length();
      CORBA::ULong l = published_endpoints.length();
      for (i=0; i<l; ++i) {
	if (!endpointInList(published_endpoints[i], oa_endpoints)) {
	  oa_endpoints.length(j+1);
	  oa_endpoints[j++] = published_endpoints[i];
	}
      }
    }

    // Bootstrap agent?
    if( orbParameters::supportBootstrapAgent )
      omniInitialReferences::initialise_bootstrap_agentImpl();
  }
  catch (const CORBA::INITIALIZE&) {
    throw;
  }
  catch (omniORB::fatalException&) {
    throw;
  }
  catch (...) {
    OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_POANotInitialised,
		  CORBA::COMPLETED_NO);
  }

  initialised = 1;
}


//////////////////////////////////////////////////////////////////////
CORBA::Boolean
omniObjAdapter::endpointInList(const char* ep,
			       const orbServer::EndpointList& eps)
{
  CORBA::ULong i;
  CORBA::ULong l = eps.length();

  for (i=0; i < l; ++i) {
    if (omni::strMatch(ep, eps[i]))
      return 1;
  }
  return 0;
}


//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::shutdown()
{
  omni_tracedmutex_lock sync(oa_lock);

  omniORB::logs(10, "Shutting-down all incoming endpoints.");

  if (num_active_oas != 0 && omniORB::trace(1)) {
    omniORB::logger log;
    log << "Warning: " << num_active_oas
        << " active object adapters at endpoint shutdown time.\n";
  }

  if ( !oa_servers.empty() ) {

    omnivector<orbServer*>::iterator j,last;
    j = oa_servers.begin();
    last = oa_servers.end();
    for ( ; j != last; j++ ) {
      (*j)->remove();
    }
    oa_servers.erase(oa_servers.begin(),oa_servers.end());
  }

  oa_endpoints.length(0);

  initialised = 0;
}


//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::adapterActive()
{
  omni_tracedmutex_lock sync(oa_lock);

  OMNIORB_ASSERT(initialised);

  if( pd_isActive )  return;

  if( num_active_oas++ == 0 ) {
    omniORB::logs(10, "Starting serving incoming endpoints.");

    if ( !oa_servers.empty() ) {

      omnivector<orbServer*>::iterator j,last;
      j = oa_servers.begin();
      last = oa_servers.end();
      for ( ; j != last; j++ ) {
	(*j)->start();
      }
    }
  }

  pd_isActive = 1;
}


//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::adapterInactive()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( !pd_isActive )  return;

  if( --num_active_oas == 0 ) {
    omniORB::logs(10, "All object adapters inactive. "
		  "Stopping serving incoming endpoints.");

    if ( !oa_servers.empty() ) {

      omnivector<orbServer*>::iterator j,last;
      j = oa_servers.begin();
      last = oa_servers.end();
      for ( ; j != last; j++ ) {
	(*j)->stop();
      }
    }
  }

  pd_isActive = 0;
}


//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::waitForActiveRequestsToComplete(int locked)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, locked);

  if( !locked )  omni::internalLock->lock();

  OMNIORB_ASSERT(pd_nReqActive >= 0);

  pd_signalOnZeroInvocations++;
  while( pd_nReqActive )  pd_signal->wait();
  pd_signalOnZeroInvocations--;

  if( !locked )  omni::internalLock->unlock();
}


//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::waitForAllRequestsToComplete(int locked)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, locked);

  if( !locked )  omni::internalLock->lock();

  OMNIORB_ASSERT(pd_nReqInThis >= 0);

  pd_signalOnZeroInvocations++;
  while( pd_nReqInThis )  pd_signal->wait();
  pd_signalOnZeroInvocations--;

  if( !locked )  omni::internalLock->unlock();
}


//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::met_detached_object()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  sd_detachedObjectLock.lock();

  OMNIORB_ASSERT(pd_nDetachedObjects > 0);

  int do_signal = --pd_nDetachedObjects == 0 && pd_signalOnZeroDetachedObjects;

  if (omniORB::trace(20)) {
    omniORB::logger log;
    log << "Met detached object. " << pd_nDetachedObjects << " remaining.";
    if (do_signal)
      log << " Signalling.";
    log << "\n";
  }

  sd_detachedObjectLock.unlock();

  if( do_signal )  sd_detachedObjectSignal.broadcast();
}

//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::wait_for_detached_objects()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  sd_detachedObjectLock.lock();
  pd_signalOnZeroDetachedObjects++;

  if (omniORB::trace(20)) {
    omniORB::logger log;
    log << "Wait for " << pd_nDetachedObjects << " detached objects.\n";
  }

  OMNIORB_ASSERT(pd_nDetachedObjects >= 0);

  while( pd_nDetachedObjects )  sd_detachedObjectSignal.wait();

  pd_signalOnZeroDetachedObjects--;
  sd_detachedObjectLock.unlock();
}

//////////////////////////////////////////////////////////////////////
CORBA::Boolean
omniObjAdapter::matchMyEndpoints(const char* addr)
{
  CORBA::ULong i;
  CORBA::ULong l = oa_endpoints.length();

  for (i=0; i < l; ++i) {
    if (omni::strMatch(addr, oa_endpoints[i]))
      return 1;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////
const orbServer::EndpointList&
omniObjAdapter::listMyEndpoints()
{
  return oa_endpoints;
}

//////////////////////////////////////////////////////////////////////
omniObjAdapter::omniObjAdapter(int nil)
  : pd_nReqInThis(0),
    pd_nReqActive(0),
    pd_signalOnZeroInvocations(0),
    pd_signal(0),
    pd_nDetachedObjects(0),
    pd_signalOnZeroDetachedObjects(0),
    pd_isActive(0)
{
  if (!nil) pd_signal = new omni_tracedcondition(omni::internalLock,
						 "omniObjAdapter::pd_signal");
}

//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::adapterDestroyed()
{
  OMNIORB_ASSERT(pd_signal);
  delete pd_signal;
  pd_signal = 0;
}

//////////////////////////////////////////////////////////////////////
omniObjAdapter::
Options::~Options() {
  for( EndpointURIList::iterator i = options.endpoints.begin();
       i != options.endpoints.end(); i++ ) {
    delete (*i);
  }
}

//////////////////////////////////////////////////////////////////////
void*
omniObjAdapter::
_ptrToClass(int* cptr)
{
  if (cptr == &omniObjAdapter::_classid) return (omniObjAdapter*)this;
  return 0;
}
int omniObjAdapter::_classid;


/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class endpointHandler : public orbOptions::Handler {
public:

  endpointHandler() : 
    orbOptions::Handler("endPoint",
			"endPoint = <endpoint uri>",
			1,
                        "-ORBendPoint = <endpoint uri>\n"
"          <endpoint uri> = \"giop:tcp:<host>:<port>\" |\n"
"                          *\"giop:ssl:<host>:<port>\" |\n"
"                          *\"giop:unix:<filename>\"   |\n"
"                          *\"giop:fd:<no.>\"          |\n"
"                          *\"<other protocol>:<network protocol>:<options>\"\n"
"                          * may not be supported on the platform.\n") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    omniObjAdapter::Options::EndpointURI* opt;
    opt = new omniObjAdapter::Options::EndpointURI();
    opt->no_publish = 0;
    opt->uri = value;
    omniObjAdapter::options.endpoints.push_back(opt);
  }

  void dump(orbOptions::sequenceString& result) {

    omniObjAdapter::Options::EndpointURIList::iterator last, i;
    i = omniObjAdapter::options.endpoints.begin();
    last = omniObjAdapter::options.endpoints.end();

    if (i == last) {
      // none specified, output the default
      orbOptions::addKVString(key(),"giop:tcp::",result);
      return;
    }

    for (; i != last; i++) {
      if (!(*i)->no_publish) {
	orbOptions::addKVString(key(),(*i)->uri,result);
      }
    }
  }
};

static endpointHandler endpointHandler_;

/////////////////////////////////////////////////////////////////////////////
class endpointNoPublishHandler : public orbOptions::Handler {
public:

  endpointNoPublishHandler() : 
    orbOptions::Handler("endPointNoPublish",
			"endPointNoPublish = <endpoint uri>",
			1,
			"-ORBendPointNoPublish <endpoint uri>") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    omniObjAdapter::Options::EndpointURI* opt;
    opt = new omniObjAdapter::Options::EndpointURI();
    opt->no_publish = 1;
    opt->uri = value;
    omniObjAdapter::options.endpoints.push_back(opt);
  }

  void dump(orbOptions::sequenceString& result) {

    omniObjAdapter::Options::EndpointURIList::iterator last, i;
    i = omniObjAdapter::options.endpoints.begin();
    last = omniObjAdapter::options.endpoints.end();
    for (; i != last; i++) {
      if ( (*i)->no_publish ) {
	orbOptions::addKVString(key(),(*i)->uri,result);
      }
    }
  }
};

static endpointNoPublishHandler endpointNoPublishHandler_;

/////////////////////////////////////////////////////////////////////////////

class endpointPublishHandler : public orbOptions::Handler {
public:

  endpointPublishHandler() : 
    orbOptions::Handler("endPointPublish",
			"endPointPublish = <publish options>",
			1,
			"-ORBendPointPublish <publish options>") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    omniObjAdapter::options.publish = value;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVString(key(), omniObjAdapter::options.publish, result);
  }
};

static endpointPublishHandler endpointPublishHandler_;


/////////////////////////////////////////////////////////////////////////////
class endpointNoListenHandler : public orbOptions::Handler {
public:

  endpointNoListenHandler() : 
    orbOptions::Handler("endPointNoListen",
			"endPointNoListen = <endpoint uri>",
			1,
			"-ORBendPointNoListen <endpoint uri>") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    omniObjAdapter::Options::EndpointURI* opt;
    opt = new omniObjAdapter::Options::EndpointURI();
    opt->no_publish = 0;
    opt->uri = value;
    omniObjAdapter::options.no_listen.push_back(opt);
  }

  void dump(orbOptions::sequenceString& result) {

    omniObjAdapter::Options::EndpointURIList::iterator last, i;
    i = omniObjAdapter::options.no_listen.begin();
    last = omniObjAdapter::options.no_listen.end();
    for (; i != last; i++) {
      orbOptions::addKVString(key(),(*i)->uri,result);
    }
  }
};

static endpointNoListenHandler endpointNoListenHandler_;

/////////////////////////////////////////////////////////////////////////////
class endpointPublishAllIFsHandler : public orbOptions::Handler {
public:

  endpointPublishAllIFsHandler() : 
    orbOptions::Handler("endPointPublishAllIFs",
			"endPointPublishAllIFs = 0 or 1",
			1,
			"-ORBendPointPublishAllIFs < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    omniObjAdapter::options.publish_all = v;
  }

  void dump(orbOptions::sequenceString& result) {

    // Deprecated, so only dump if set.
    if (omniObjAdapter::options.publish_all)
      orbOptions::addKVBoolean(key(),omniObjAdapter::options.publish_all,
			       result);
  }
};

static endpointPublishAllIFsHandler endpointPublishAllIFsHandler_;

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
class omni_objadpt_initialiser : public omniInitialiser {
public:

  omni_objadpt_initialiser() {
    orbOptions::singleton().registerHandler(endpointHandler_);
    orbOptions::singleton().registerHandler(endpointNoPublishHandler_);
    orbOptions::singleton().registerHandler(endpointPublishHandler_);
    orbOptions::singleton().registerHandler(endpointNoListenHandler_);
    orbOptions::singleton().registerHandler(endpointPublishAllIFsHandler_);
  }

  void attach() { 

    //
    // Initialise random seed
    {
      unsigned long s, ns;
      omni_thread::get_time(&s, &ns);
      LibcWrapper::SRand(s);
    }

    //
    // endPoint publishing options

    if ((const char*)omniObjAdapter::options.publish == 0 ||
	strlen(omniObjAdapter::options.publish) == 0) {

      omniObjAdapter::options.publish = (const char*)"addr";
    }

    if (omni::strMatch(omniObjAdapter::options.publish, "fail-if-multiple")) {
      // Backwards compatibility with 4.0.x.
      omniObjAdapter::options.publish = (const char*)"fail-if-multiple,addr";
    }

    // Handle deprecated endPointPublishAllIFs and endPointNoListen
    if (omniObjAdapter::options.publish_all) {
      omniORB::logs(1, "The endPointPublishAllIFs parameter is deprecated.");
      omniORB::logs(1, "Use an endPointPublish specification instead.");
      char* new_publish =
	CORBA::string_alloc(strlen(omniObjAdapter::options.publish) +
			    sizeof(",all(addr)"));
      strcpy(new_publish, omniObjAdapter::options.publish);
      strcat(new_publish, ",all(addr)");
      omniObjAdapter::options.publish = new_publish;
    }

    if (omniObjAdapter::options.no_listen.size()) {
      omniORB::logs(1, "The endPointNoListen parameter is deprecated.");
      omniORB::logs(1, "Use an endPointPublish specification instead.");
      omniObjAdapter::Options::EndpointURIList::iterator i, end;
      end = omniObjAdapter::options.no_listen.end();

      int extend = 0;
      for (i = omniObjAdapter::options.no_listen.begin(); i != end; ++i)
	extend += strlen((*i)->uri) + 1;
      
      char* new_publish =
	CORBA::string_alloc(strlen(omniObjAdapter::options.publish) + extend);

      strcpy(new_publish, omniObjAdapter::options.publish);
      
      for (i = omniObjAdapter::options.no_listen.begin(); i != end; ++i) {
	strcat(new_publish, ",");
	strcat(new_publish, (*i)->uri);
      }
      omniObjAdapter::options.publish = new_publish;
    }
  }
  void detach() {
    omniORB::logs(20, "Clear endPoint options.");
    omniObjAdapter::Options::EndpointURIList::iterator i;
    for (i = omniObjAdapter::options.endpoints.begin();
	 i != omniObjAdapter::options.endpoints.end(); i++) {
      delete (*i);
    }
    omniObjAdapter::options.endpoints.erase(
      omniObjAdapter::options.endpoints.begin(),
      omniObjAdapter::options.endpoints.end());

    omniObjAdapter::options.publish     = (char*)0;
    omniObjAdapter::options.publish_all = 0;
  }
};


static omni_objadpt_initialiser initialiser;

omniInitialiser& omni_objadpt_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
