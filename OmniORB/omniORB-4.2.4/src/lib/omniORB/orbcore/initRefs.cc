// -*- Mode: C++; -*-
//                            Package   : omniORB
// initRefs.cc                Created on: 20/08/98
//                            Author    : Sai-Lai Lo
//
//    Copyright (C) 2002-2007 Apasphere Ltd
//    Copyright (C) 1996-2000 AT&T Laboratories Cambridge
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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <initRefs.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <poaimpl.h>
#include <poacurrentimpl.h>
#include <omniORB4/omniURI.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <stdio.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
CORBA::Boolean orbParameters::supportBootstrapAgent = 0;
// Applies to the server side. 1 means enable the support for Sun's
// bootstrap agent protocol.  This enables interoperability between omniORB
// servers and Sun's javaIDL clients. When this option is enabled, an
// omniORB server will response to a bootstrap agent request.

CORBA::String_var  orbParameters::bootstrapAgentHostname;
// Applies to the client side. Non-zero enables the use of Sun's bootstrap
// agent protocol to resolve initial references. The value is the host name
// where requests for initial references should be sent. Only uses this
// option to interoperate with Sun's javaIDL.

CORBA::UShort  orbParameters::bootstrapAgentPort = 900;
// Applies to the client side. Use this port no. to contact the bootstrap 
// agent.


////////////////////////////////////////////////////////////////////////////
static CORBA_InitialReferences_i*  the_bootagentImpl = 0;
static omni_tracedmutex ba_lock("ba_lock");


// When initial references are first set, they contain an id and a
// uri. The first time they are resolved, the uri is replaced with an
// object reference.
struct serviceRecord {
  CORBA::String_member id;
  CORBA::String_member uri;
  CORBA::Object_Member ref;
};

static _CORBA_PseudoValue_Sequence<serviceRecord> the_argsServiceList;
static _CORBA_PseudoValue_Sequence<serviceRecord> the_fileServiceList;
static char*                                      the_argsDefaultInitRef = 0;
static char*                                      the_fileDefaultInitRef = 0;
static CORBA_InitialReferences_ptr                the_bootagent          = 0;

static omni_tracedmutex sl_lock("sl_lock");


//////////////////////////////////////////////////////////////////////
////////////////////// CORBA_InitialReferences_i /////////////////////
//////////////////////////////////////////////////////////////////////

class CORBA_InitialReferences_i : public POA_CORBA_InitialReferences {
public:
  CORBA::Object_ptr get(const char* id);
  CORBA_InitialReferences::ObjIdList* list();
  inline CORBA_InitialReferences_i()   {}
  virtual ~CORBA_InitialReferences_i() {}

private:
  CORBA_InitialReferences_i(const CORBA_InitialReferences_i&);
  CORBA_InitialReferences_i& operator=(const CORBA_InitialReferences_i&);
};


CORBA::Object_ptr
CORBA_InitialReferences_i::get(const char* id)
{
  return omniInitialReferences::resolve(id);
  // *** What happens if resolve() returns a pseudo object?
}


CORBA_InitialReferences::ObjIdList*
CORBA_InitialReferences_i::list()
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA_InitialReferences::ObjIdList* result =
    new CORBA_InitialReferences::ObjIdList(the_argsServiceList.length() +
					   the_fileServiceList.length());
  CORBA_InitialReferences::ObjIdList& l = *result;

  l.length(the_argsServiceList.length() + the_fileServiceList.length());

  CORBA::ULong i, j;

  for (i=0,j=0; i < the_argsServiceList.length(); i++,j++) {
    l[j] = CORBA::string_dup(the_argsServiceList[i].id);
  }
  for (i=0; i < the_fileServiceList.length(); i++,j++) {
    l[j] = CORBA::string_dup(the_fileServiceList[i].id);
  }
  // XXX Should we go out to find the listing from the boot agent?

  return result;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Boolean
omniInitialReferences::setFromArgs(const char* identifier,
				   const char* uri)
{
  if (!omniURI::uriSyntaxIsValid(uri)) return 0;

  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong index;
  for (index=0; index < the_argsServiceList.length(); index++) {
    if (strcmp((const char*)the_argsServiceList[index].id,identifier) == 0)
      break;
  }
  if (index == the_argsServiceList.length()) {
    the_argsServiceList.length(index+1);
    the_argsServiceList[index].id = identifier;
  }
  the_argsServiceList[index].uri = uri;
  the_argsServiceList[index].ref = CORBA::Object::_nil();
  return 1;
}

static void
setFromArgs(const char* identifier, CORBA::Object_ptr obj)
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong index;
  for (index=0; index < the_argsServiceList.length(); index++) {
    if (strcmp((const char*)the_argsServiceList[index].id,identifier) == 0)
      break;
  }
  if (index == the_argsServiceList.length()) {
    the_argsServiceList.length(index+1);
    the_argsServiceList[index].id = identifier;
  }
  the_argsServiceList[index].uri = (char*)0;
  the_argsServiceList[index].ref = CORBA::Object::_duplicate(obj);
}

CORBA::Boolean
omniInitialReferences::setFromFile(const char* identifier,
				   const char* uri)
{
  if (!omniURI::uriSyntaxIsValid(uri)) return 0;

  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong index;
  for (index=0; index < the_fileServiceList.length(); index++) {
    if (strcmp((const char*)the_fileServiceList[index].id,identifier) == 0)
      break;
  }
  if (index == the_fileServiceList.length()) {
    the_fileServiceList.length(index+1);
    the_fileServiceList[index].id = identifier;
  }
  the_fileServiceList[index].uri = uri;
  the_fileServiceList[index].ref = CORBA::Object::_nil();
  return 1;
}

static void
setFromFile(const char* identifier, CORBA::Object_ptr obj)
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong index;
  for (index=0; index < the_fileServiceList.length(); index++) {
    if (strcmp((const char*)the_fileServiceList[index].id,identifier) == 0)
      break;
  }
  if (index == the_fileServiceList.length()) {
    the_fileServiceList.length(index+1);
    the_fileServiceList[index].id = identifier;
  }
  the_fileServiceList[index].uri = (char*)0;
  the_fileServiceList[index].ref = CORBA::Object::_duplicate(obj);
}

void
omniInitialReferences::setFromORB(const char* identifier,
				  CORBA::Object_ptr obj)
{
  if (!identifier || *identifier == '\0')
    throw CORBA::ORB::InvalidName();

  if (CORBA::is_nil(obj))
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_RegisterNilObject,
		    CORBA::COMPLETED_NO);

  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong index;
  for (index=0; index < the_fileServiceList.length(); index++) {
    if (strcmp((const char*)the_fileServiceList[index].id,identifier) == 0)
      throw CORBA::ORB::InvalidName();
  }
  for (index=0; index < the_argsServiceList.length(); index++) {
    if (strcmp((const char*)the_argsServiceList[index].id,identifier) == 0)
      throw CORBA::ORB::InvalidName();
  }
  OMNIORB_ASSERT(index == the_argsServiceList.length());

  the_argsServiceList.length(index+1);
  the_argsServiceList[index].id = identifier;
  the_argsServiceList[index].uri = (char*)0;
  the_argsServiceList[index].ref = CORBA::Object::_duplicate(obj);
}



void
omniInitialReferences::setDefaultInitRefFromArgs(const char* defInit)
{
  omni_tracedmutex_lock sync(sl_lock);
  if (the_argsDefaultInitRef) CORBA::string_free(the_argsDefaultInitRef);
  the_argsDefaultInitRef = CORBA::string_dup(defInit);
}

void
omniInitialReferences::setDefaultInitRefFromFile(const char* defInit)
{
  omni_tracedmutex_lock sync(sl_lock);
  if (the_fileDefaultInitRef) CORBA::string_free(the_fileDefaultInitRef);
  the_fileDefaultInitRef = CORBA::string_dup(defInit);
}


struct resolvePseudoEntry {
  const char* id;
  omniInitialReferences::pseudoObj_fn fn;

  resolvePseudoEntry(const char* i, omniInitialReferences::pseudoObj_fn f)
    : id(i), fn(f) {}

  resolvePseudoEntry() : id(0), fn(0) {}

  // Using default copy constructor
};

static omnivector<resolvePseudoEntry>*& thePseudoFnList()
{
  static omnivector<resolvePseudoEntry>* the_list = 0;
  if (the_list == 0) the_list = new omnivector<resolvePseudoEntry>;
  return the_list;
}


void
omniInitialReferences::registerPseudoObjFn(const char* identifier,
					   pseudoObj_fn fn)
{
  thePseudoFnList()->push_back(resolvePseudoEntry(identifier, fn));
}


static CORBA::Object_ptr
resolvePseudo(const char* id, unsigned int cycles)
{
  // Instantiate the pseudo objects on demand.
  // NB. No race condition problem here - these fns are thread safe.

  // We cannot insert the references into the initial references map,
  // since holding a reference there would prevent the objects from
  // being released properly when they have been destroyed.

  omnivector<resolvePseudoEntry>::iterator i    = thePseudoFnList()->begin();
  omnivector<resolvePseudoEntry>::iterator last = thePseudoFnList()->end();
  
  for (; i != last; i++) {
    if (!strcmp(id, (*i).id))
      return ((*i).fn)();
  }
  return 0;
}


static CORBA::Object_ptr
resolveArgs(const char* id, unsigned int cycles)
{
  CORBA::Object_ptr ref = 0;
  CORBA::String_var uri;
  CORBA::ULong      i;
  {
    omni_tracedmutex_lock sync(sl_lock);
    for (i=0; i < the_argsServiceList.length(); i++) {
      if (!strcmp((const char*)the_argsServiceList[i].id, id)) {

	if (!CORBA::is_nil(the_argsServiceList[i].ref)) {
	  ref = CORBA::Object::_duplicate(the_argsServiceList[i].ref);
	}
	else {
	  OMNIORB_ASSERT((char*)(the_argsServiceList[i].uri));
	  uri = CORBA::string_dup(the_argsServiceList[i].uri);
	}
	break;
      }
    }
  }
  if ((char*)uri) {
    ref = omniURI::stringToObject(uri, cycles);

    // Store the object reference in the list, unless another thread has
    // got there first
    if (!CORBA::is_nil(ref)) {
      omni_tracedmutex_lock sync(sl_lock);
      for (i=0; i < the_argsServiceList.length(); i++) {
	if (!strcmp((const char*)the_argsServiceList[i].id, id)) {

	  if (CORBA::is_nil(the_argsServiceList[i].ref)) {
	    the_argsServiceList[i].ref = CORBA::Object::_duplicate(ref);
	    the_argsServiceList[i].uri = (char*)0;
	  }
	  break;
	}
      }
    }
  }
  if (ref && omniORB::trace(10)) {
    omniORB::logger l;
    l << "Initial reference `" << id
      << "' resolved from -ORBInitRef argument / ORB registration.\n";
  }
  return ref;
}

static CORBA::Object_ptr
resolveFile(const char* id, unsigned int cycles)
{
  CORBA::Object_ptr ref = 0;
  CORBA::String_var uri;
  CORBA::ULong      i;
  {
    omni_tracedmutex_lock sync(sl_lock);
    for (i=0; i < the_fileServiceList.length(); i++) {
      if (!strcmp((const char*)the_fileServiceList[i].id, id)) {

	if (!CORBA::is_nil(the_fileServiceList[i].ref)) {
	  ref = CORBA::Object::_duplicate(the_fileServiceList[i].ref);
	}
	else {
	  OMNIORB_ASSERT((char*)(the_fileServiceList[i].uri));
	  uri = CORBA::string_dup(the_fileServiceList[i].uri);
	}
	break;
      }
    }
  }
  if ((char*)uri) {
    ref = omniURI::stringToObject(uri, cycles);

    // Store the object reference in the list, unless another thread has
    // got there first
    if (!CORBA::is_nil(ref)) {
      omni_tracedmutex_lock sync(sl_lock);
      for (i=0; i < the_fileServiceList.length(); i++) {
	if (!strcmp((const char*)the_fileServiceList[i].id, id)) {

	  if (CORBA::is_nil(the_fileServiceList[i].ref)) {
	    the_fileServiceList[i].ref = CORBA::Object::_duplicate(ref);
	    the_fileServiceList[i].uri = (char*)0;
	  }
	  break;
	}
      }
    }
  }
  if (ref && omniORB::trace(10)) {
    omniORB::logger l;
    l << "Initial reference `" << id
      << "' resolved from configuration file.\n";
  }
  return ref;
}

static CORBA::Object_ptr
resolveArgsDefault(const char* id, unsigned int cycles)
{
  CORBA::String_var uri;
  {
    omni_tracedmutex_lock sync(sl_lock);

    if (!the_argsDefaultInitRef) return 0;

    uri = CORBA::string_alloc(strlen(the_argsDefaultInitRef) + strlen(id) + 2);
    strcpy(uri, the_argsDefaultInitRef);
    strcat(uri, "/");
    strcat(uri, id);
  }
  try {
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "Trying to resolve initial reference `" << id << "'\n";
      l << " with default `" << (const char*)uri << "'\n";
    }
    CORBA::Object_ptr obj = omniURI::stringToObject(uri, cycles);

    // Store the retrieved object in the args list, so future
    // resolves return the same thing.

    // Note that there's a race condition here. Another thread might
    // have been through here just before us, and stored a different
    // object reference in the list. That's so unlikely that we don't
    // bother to prevent it. The spec. makes no guarantees about when
    // initial reference resolution happens anyway.
    setFromArgs(id, obj);
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "Initial reference `" << id
	<< "' resolved with -ORBDefaultInitRef prefix.\n";
    }
    return obj;
  }
  catch(...) {
  }
  return 0;
}

static CORBA::Object_ptr
resolveFileDefault(const char* id, unsigned int cycles)
{
  CORBA::String_var uri;
  {
    omni_tracedmutex_lock sync(sl_lock);

    if (!the_fileDefaultInitRef) return 0;

    uri = CORBA::string_alloc(strlen(the_fileDefaultInitRef) + strlen(id) + 2);
    strcpy(uri, the_fileDefaultInitRef);
    strcat(uri, "/");
    strcat(uri, id);
  }
  try {
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "Trying to resolve initial reference `" << id << "'\n";
      l << " with configuration file default `" << (const char*)uri << "'\n";
    }
    CORBA::Object_ptr obj = omniURI::stringToObject(uri, cycles);

    // Store the retrieved object in the file list, so future
    // resolves return the same thing.

    // Note that there's a race condition here. Another thread might
    // have been through here just before us, and stored a different
    // object reference in the list. That's so unlikely that we don't
    // bother to prevent it. The spec. makes no guarantees about when
    // initial reference resolution happens anyway.
    setFromFile(id, obj);
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "Initial reference `" << id
	<< "' resolved with configuration file ORBDefaultInitRef prefix.\n";
    }
    return obj;
  }
  catch(...) {
  }
  return 0;
}


static CORBA::Object_ptr
resolveBootAgent(const char* id, unsigned int cycles)
{
  CORBA::Object_ptr result = 0;
  CORBA::Boolean    update = 0;
  {
    omni_tracedmutex_lock sync(sl_lock);

    if (!the_bootagent || CORBA::is_nil(the_bootagent)) return 0;

    try {
      // XXX we will end up in a deadlock if this invocation turns
      //     out to be a chain of invocations and eventually go back
      //     to us.
      if (omniORB::trace(10)) {
	CORBA::String_var ior(omniURI::objectToString(the_bootagent));
	omniORB::logger l;
	l << "Trying to resolve initial reference `" << id << "'\n";
	l << " with boot agent: " << (const char*)ior << "\n";
      }
      result = the_bootagent->get(id);

      if (CORBA::is_nil(result))
	result = 0;
      else
	update = 1;

      if (omniORB::trace(10)) {
	omniORB::logger l;
	l << "Initial reference `" << id << "' resolved with boot agent.\n";
      }
    }
    catch(...) {
      if (omniORB::trace(10)) {
	omniORB::logger l;
	l << "Caught exception trying to resolve `" << id
	  << "' with boot agent.\n";
      }
    }
  }
  if (update) setFromArgs(id, result);
  return result;
}


CORBA::Object_ptr
omniInitialReferences::resolve(const char* id, unsigned int cycles)
{
  if (!id) throw CORBA::ORB::InvalidName();

  CORBA::Object_ptr result;

  // Look for the id in the order prescribed by the CORBA spec, with
  // extensions for ORBDefaultInitRef in the config file, and the
  // bootstrap agent.
  if ((result = resolvePseudo     (id, cycles))) return result;
  if ((result = resolveArgs       (id, cycles))) return result;
  if ((result = resolveFile       (id, cycles))) return result;
  if ((result = resolveArgsDefault(id, cycles))) return result;
  if ((result = resolveFileDefault(id, cycles))) return result;
  if ((result = resolveBootAgent  (id, cycles))) return result;

  // No more possibilities. Throw the correct exception to the caller.
  if (!strcmp(id, "InterfaceRepository") ||
      !strcmp(id, "NameService") ||
      !strcmp(id, "TradingService") ||
      !strcmp(id, "SecurityCurrent") ||
      !strcmp(id, "TransactionCurrent"))
    // Resource not found.
    OMNIORB_THROW(NO_RESOURCES,NO_RESOURCES_InitialRefNotFound,
		  CORBA::COMPLETED_NO);

  // The identifier is not defined.
  if (omniORB::trace(10)) {
    omniORB::logger l;
    l << "resolve_initial_references throws CORBA::ORB::InvalidName\n";
  }
  throw CORBA::ORB::InvalidName();

  // Never get here...
  return 0;
}


CORBA::ORB::ObjectIdList*
omniInitialReferences::list()
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ORB::ObjectIdList* result =
    new CORBA::ORB::ObjectIdList(the_argsServiceList.length() +
				 the_fileServiceList.length());
  CORBA::ORB::ObjectIdList& l = *result;

  l.length(the_argsServiceList.length() + the_fileServiceList.length());

  CORBA::ULong i, j;

  for (i=0,j=0; i < the_argsServiceList.length(); i++,j++) {
    l[j] = CORBA::string_dup(the_argsServiceList[i].id);
  }
  for (i=0; i < the_fileServiceList.length(); i++,j++) {
    l[j] = CORBA::string_dup(the_fileServiceList[i].id);
  }
  return result;
}



void
omniInitialReferences::remFromFile(const char* id)
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong i;
  for (i=0; i < the_fileServiceList.length(); i++) {

    if (strcmp((const char*)the_fileServiceList[i].id, id) == 0) {

      for (i++; i < the_fileServiceList.length(); i++) {
	the_fileServiceList[i-1].id  = the_fileServiceList[i].id;
	the_fileServiceList[i-1].uri = the_fileServiceList[i].uri;
	the_fileServiceList[i-1].ref = the_fileServiceList[i].ref;
      }
      the_fileServiceList.length(the_fileServiceList.length() - 1);
      return;
    }
  }
}

void
omniInitialReferences::remFromArgs(const char* id)
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong i;
  for (i=0; i < the_argsServiceList.length(); i++) {

    if (strcmp((const char*)the_argsServiceList[i].id, id) == 0) {

      for (i++; i < the_argsServiceList.length(); i++) {
	the_argsServiceList[i-1].id  = the_argsServiceList[i].id;
	the_argsServiceList[i-1].uri = the_argsServiceList[i].uri;
	the_argsServiceList[i-1].ref = the_argsServiceList[i].ref;
      }
      the_argsServiceList.length(the_argsServiceList.length() - 1);
      return;
    }
  }
}


void
omniInitialReferences::initialise_bootstrap_agentImpl()
{
  ba_lock.lock();

  if( !the_bootagentImpl )
    the_bootagentImpl = new CORBA_InitialReferences_i();

  ba_lock.unlock();
}


int
omniInitialReferences::invoke_bootstrap_agentImpl(omniCallHandle& handle)
{
  omni_tracedmutex_lock sync(ba_lock);

  if( !the_bootagentImpl )  return 0;

  // The upcall asserts that a localId is set, as a sanity-check. This
  // is the one dispatch case in which there isn't a localIdentity, so
  // we pretend.
  handle.localId((omniLocalIdentity*)1);

  ((omniServant*) the_bootagentImpl)->_dispatch(handle);
  return 1;
}


int
omniInitialReferences::is_bootstrap_agentImpl_initialised()
{
  return the_bootagentImpl ? 1 : 0;
}


void
omniInitialReferences::initialise_bootstrap_agent(const char* host,
						  CORBA::UShort port)
{
  omni_tracedmutex_lock sync(sl_lock);

  try {
    IIOP::Address addr;
    addr.host = host;
    addr.port = port;

    _CORBA_Unbounded_Sequence_Octet objkey;
    objkey.length(4);
    objkey[0] = 'I'; objkey[1] = 'N'; objkey[2] = 'I'; objkey[3] = 'T';

    GIOP::Version ver = { 1, 0 };
    omniIOR* ior= new omniIOR(CORBA_InitialReferences::_PD_repoId,
			      objkey,&addr,1,ver,omniIOR::NoInterceptor);
    
    omniObjRef* objref = omni::createObjRef(
                              CORBA_InitialReferences::_PD_repoId,ior,0);
    if (!objref->_is_nil()) {
      the_bootagent = (CORBA_InitialReferences_ptr) 
   	      objref->_ptrToObjRef(CORBA_InitialReferences::_PD_repoId);
      the_bootagent->_noExistentCheck();
    }
  }
  catch(...) {}
}

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class DefaultInitRefHandler : public orbOptions::Handler {
public:

  DefaultInitRefHandler() : 
    orbOptions::Handler("DefaultInitRef",
			"DefaultInitRef = <Default URI>",
			1,
			"-ORBDefaultInitRef <Default URI> (standard option)") {}


  void visit(const char* value,orbOptions::Source src)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    if (src == orbOptions::fromArgv || src == orbOptions::fromArray) {
      omniInitialReferences::setDefaultInitRefFromArgs(value);
    }
    else {
      omniInitialReferences::setDefaultInitRefFromFile(value);
    }
  }

  void dump(orbOptions::sequenceString& result) {
    const char* v;

    v = the_fileDefaultInitRef;
    orbOptions::addKVString("DefaultInitRef (file)", v ? v : "", result);

    v = the_argsDefaultInitRef;
    orbOptions::addKVString("DefaultInitRef (args)", v ? v : "", result);
  }
};

static DefaultInitRefHandler DefaultInitRefHandler_;

/////////////////////////////////////////////////////////////////////////////
class InitRefHandler : public orbOptions::Handler {
public:

  InitRefHandler() : 
    orbOptions::Handler("InitRef",
			"InitRef = <ObjectID>=<ObjectURI>",
			1,
			"-ORBInitRef <ObjectID>=<ObjectURI> (standard option)") {}


  void visit(const char* value, orbOptions::Source src)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    unsigned int slen = strlen(value) + 1;
    CORBA::String_var id(CORBA::string_alloc(slen));
    CORBA::String_var uri(CORBA::string_alloc(slen));
    if (sscanf(value, "%[^=]=%s", (char*)id, (char*)uri) != 2) {
      throw orbOptions::BadParam(key(),value,"Invalid argument, expect <ObjectID>=<ObjectURI>");
    }
    if (src == orbOptions::fromArgv) {
      if (!omniInitialReferences::setFromArgs(id, uri)) {
	throw orbOptions::BadParam(key(),value,"Invalid argument, expect <ObjectID>=<ObjectURI>");
      }
    }
    else {
      if (!omniInitialReferences::setFromFile(id, uri)) {
	throw orbOptions::BadParam(key(),value,"Invalid argument, expect <ObjectID>=<ObjectURI>");
      }
    }
  }

  void dumpRecord(serviceRecord& rec, orbOptions::sequenceString& result) {
    CORBA::String_var v;
    v = CORBA::string_alloc(strlen(rec.id)+strlen(rec.uri)+1);
    sprintf(v,"%s=%s",(const char*)rec.id,(const char*)rec.uri);
    orbOptions::addKVString(key(),v,result);
  }

  void dump(orbOptions::sequenceString& result) {

    CORBA::ULong i;
    for (i=0; i < the_argsServiceList.length(); i++) {
      dumpRecord(the_argsServiceList[i],result);
    }

    for (i=0; i < the_fileServiceList.length(); i++) {
      dumpRecord(the_fileServiceList[i],result);
    }
  }
};

static InitRefHandler InitRefHandler_;

/////////////////////////////////////////////////////////////////////////////
class supportBootstrapAgentHandler : public orbOptions::Handler {
public:

  supportBootstrapAgentHandler() : 
    orbOptions::Handler("supportBootstrapAgent",
			"supportBootstrapAgent = 0 or 1",
			1,
			"-ORBsupportBootstrapAgent < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::supportBootstrapAgent = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::supportBootstrapAgent,
			     result);
  }
};

static supportBootstrapAgentHandler supportBootstrapAgentHandler_;

/////////////////////////////////////////////////////////////////////////////
class bootstrapAgentPortHandler : public orbOptions::Handler {
public:

  bootstrapAgentPortHandler() : 
    orbOptions::Handler("bootstrapAgentPort",
			"bootstrapAgentPort = <1-65535>",
			1,
			"-ORBbootstrapAgentPort < 1-65535 >") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || !(v >=1 && v <=65535) ) {
      throw orbOptions::BadParam(key(),value,
				 "Invalid value, expect 1-65535");
    }
    orbParameters::bootstrapAgentPort = (CORBA::UShort)v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::bootstrapAgentPort,
			   result);
  }
};

static bootstrapAgentPortHandler bootstrapAgentPortHandler_;

/////////////////////////////////////////////////////////////////////////////
class bootstrapAgentHostnameHandler : public orbOptions::Handler {
public:

  bootstrapAgentHostnameHandler() : 
    orbOptions::Handler("bootstrapAgentHostname",
			"bootstrapAgentHostname = <hostname>",
			1,
			"-ORBbootstrapAgentHostname <hostname>") {}


  void visit(const char* value,orbOptions::Source)
    OMNI_THROW_SPEC (orbOptions::BadParam)
  {
    orbParameters::bootstrapAgentHostname = value;
  }

  void dump(orbOptions::sequenceString& result) {
    const char* v = orbParameters::bootstrapAgentHostname;
    if (!v) v = "";
    orbOptions::addKVString(key(),v,result);
  }
};

static bootstrapAgentHostnameHandler bootstrapAgentHostnameHandler_;

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_initRefs_initialiser : public omniInitialiser {
public:

  omni_initRefs_initialiser() {
    orbOptions::singleton().registerHandler(DefaultInitRefHandler_);
    orbOptions::singleton().registerHandler(InitRefHandler_);
    orbOptions::singleton().registerHandler(supportBootstrapAgentHandler_);
    orbOptions::singleton().registerHandler(bootstrapAgentHostnameHandler_);
    orbOptions::singleton().registerHandler(bootstrapAgentPortHandler_);
  }

#ifdef __GNUG__
  virtual
#endif
  ~omni_initRefs_initialiser() {
    omnivector<resolvePseudoEntry>*& the_list = thePseudoFnList();
    delete the_list;
    the_list = 0;
  }

  void attach() {

    const char* v = orbParameters::bootstrapAgentHostname;
    if (v && strlen(v)) {
      omniInitialReferences::remFromFile("NameService");
      omniInitialReferences::remFromFile("InterfaceRepository");
      omniInitialReferences::
	initialise_bootstrap_agent(orbParameters::bootstrapAgentHostname,
				   orbParameters::bootstrapAgentPort);
    }
  }

  void detach() {
    serviceRecord* sl;
    sl = the_argsServiceList.get_buffer(1);
    delete [] sl;
    sl = the_fileServiceList.get_buffer(1);
    delete [] sl;
    if (the_argsDefaultInitRef) CORBA::string_free(the_argsDefaultInitRef);
    the_argsDefaultInitRef = 0;
    if (the_fileDefaultInitRef) CORBA::string_free(the_fileDefaultInitRef);
    the_fileDefaultInitRef = 0;
    if( the_bootagentImpl ) delete the_bootagentImpl;
    the_bootagentImpl = 0;
    if( the_bootagent )  CORBA::release(the_bootagent);
    the_bootagent = 0;
  }
};

static omni_initRefs_initialiser initialiser;

omniInitialiser& omni_initRefs_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)

void
_omni_set_NameService(CORBA::Object_ptr ns)
{
  setFromArgs((const char*) "NameService", ns);
}

