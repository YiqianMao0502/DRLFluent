// -*- Mode: C++; -*-
//                            Package   : omniORB
// portableserver.cc          Created on: 11/5/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2004-2012 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
//    Misc code from PortableServer module.
//
 
#define ENABLE_CLIENT_IR_SUPPORT
#include <omniORB4/CORBA.h>
#include <poaimpl.h>
#include <poacurrentimpl.h>
#include <localIdentity.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/objTracker.h>
#include <initRefs.h>
#include <dynamicLib.h>
#include <exceptiondefs.h>
#include <omniCurrent.h>
#include <objectTable.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
////////////////////////// Policy Interfaces /////////////////////////
//////////////////////////////////////////////////////////////////////


_init_in_def_( const CORBA::ULong
	       PortableServer::THREAD_POLICY_ID = 16; )
_init_in_def_( const CORBA::ULong
	       PortableServer::LIFESPAN_POLICY_ID = 17; )
_init_in_def_( const CORBA::ULong
	       PortableServer::ID_UNIQUENESS_POLICY_ID = 18; )
_init_in_def_( const CORBA::ULong
	       PortableServer::ID_ASSIGNMENT_POLICY_ID = 19; )
_init_in_def_( const CORBA::ULong
	       PortableServer::IMPLICIT_ACTIVATION_POLICY_ID = 20; )
_init_in_def_( const CORBA::ULong
	       PortableServer::SERVANT_RETENTION_POLICY_ID = 21; )
_init_in_def_( const CORBA::ULong
	       PortableServer::REQUEST_PROCESSING_POLICY_ID = 22; )


#define DEFINE_POLICY_OBJECT(name)  \
  \
PortableServer::name::~name() {}  \
  \
CORBA::Policy_ptr  \
PortableServer::name::copy()  \
{  \
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();  \
  return new name(pd_value);  \
}  \
  \
void*  \
PortableServer::name::_ptrToObjRef(const char* repoId)  \
{  \
  OMNIORB_ASSERT(repoId );  \
  \
  if( omni::ptrStrMatch(repoId, PortableServer::name::_PD_repoId) )  \
    return (PortableServer::name##_ptr) this;  \
  if( omni::ptrStrMatch(repoId, CORBA::Policy::_PD_repoId) )  \
    return (CORBA::Policy_ptr) this;  \
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )  \
    return (CORBA::Object_ptr) this;  \
  \
  return 0;  \
}  \
  \
PortableServer::name##_ptr  \
PortableServer::name::_duplicate(PortableServer::name##_ptr obj)  \
{  \
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();  \
  \
  return obj;  \
}  \
  \
PortableServer::name##_ptr  \
PortableServer::name::_narrow(CORBA::Object_ptr obj)  \
{  \
  if( CORBA::is_nil(obj) )  return _nil();  \
  \
  name##_ptr p = (name##_ptr) obj->_ptrToObjRef(name::_PD_repoId);  \
  \
  if( p )  p->_NP_incrRefCount();  \
  \
  return p ? p : _nil();  \
}  \
  \
PortableServer::name##_ptr  \
PortableServer::name::_nil()  \
{  \
  static name* _the_nil_ptr = 0;  \
  if( !_the_nil_ptr ) {  \
    omni::nilRefLock().lock();  \
    if( !_the_nil_ptr ) { \
      _the_nil_ptr = new name;  \
      registerNilCorbaObject(_the_nil_ptr); \
    } \
    omni::nilRefLock().unlock();  \
  }  \
  return _the_nil_ptr;  \
}  \
  \
const char*  \
PortableServer::name::_PD_repoId = "IDL:omg.org/PortableServer/" #name PS_VERSION;


DEFINE_POLICY_OBJECT(ThreadPolicy)
DEFINE_POLICY_OBJECT(LifespanPolicy)
DEFINE_POLICY_OBJECT(IdUniquenessPolicy)
DEFINE_POLICY_OBJECT(IdAssignmentPolicy)
DEFINE_POLICY_OBJECT(ImplicitActivationPolicy)
DEFINE_POLICY_OBJECT(ServantRetentionPolicy)
DEFINE_POLICY_OBJECT(RequestProcessingPolicy)

#undef DEFINE_POLICY_OBJECT

//////////////////////////////////////////////////////////////////////
///////////////////////////// ServantBase ////////////////////////////
//////////////////////////////////////////////////////////////////////

PortableServer::ServantBase::~ServantBase() {}


PortableServer::POA_ptr
PortableServer::ServantBase::_default_POA()
{
  return omniOrbPOA::rootPOA();
}


CORBA::_objref_InterfaceDef*
PortableServer::ServantBase::_get_interface()
{
  // Return 0 to indicate to _do_get_interface() that we have not
  // been overriden.  We cannot implement this method here, because
  // we would have to do a downcast to InterfaceDef, which would
  // introduce a dependency on the dynamic library.
  return 0;
}


void
PortableServer::ServantBase::_add_ref()
{
  _pd_refCount.inc();
}


void
PortableServer::ServantBase::_remove_ref()
{
  int val = _pd_refCount.dec();

  if (val > 0)
    return;

  if (val < 0) {
    omniORB::logs(1, "ServantBase has negative ref count!");
    return;
  }

  omniORB::logs(15, "ServantBase has zero ref count -- deleted.");

  try {
    delete this;
  }
  catch (...) {
    omniORB::logs(1, "Error: Servant destructor threw an exception.");
  }
}

CORBA::ULong
PortableServer::ServantBase::_refcount_value()
{
  return _pd_refCount.value();
}

void*
PortableServer::ServantBase::_do_this(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if (!omni::internalLock) {
    // Not initalised yet
    OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_POANotInitialised,
		  CORBA::COMPLETED_NO);
  }

  omniCurrent* current = omniCurrent::get();
  if (current) {
    omniCallDescriptor* call_desc = current->callDescriptor();

    if (call_desc &&
	call_desc->localId()->servant() == (omniServant*)this) {

      // In context of an invocation on this servant
      omniObjRef* ref = omniOrbPOACurrent::real_get_reference(call_desc);
      OMNIORB_ASSERT(ref);
      return ref->_ptrToObjRef(repoId);
    }
  }

  {
    omni_tracedmutex_lock sync(*omni::internalLock);

    if (_activations().size() == 1) {
      // We only have a single activation -- return a reference to it.
      omniObjTableEntry* entry = _activations()[0];
      omniOrbPOA* poa = omniOrbPOA::_downcast(entry->adapter());
      omniIORHints hints(poa ? poa->policy_list() : 0);
      omniObjRef* ref = omni::createLocalObjRef(_mostDerivedRepoId(), repoId,
						entry, hints);
      OMNIORB_ASSERT(ref);
      return ref->_ptrToObjRef(repoId);
    }
  }

  PortableServer::POA_var poa = this->_default_POA();

  if( CORBA::is_nil(poa) )
    OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_POANotInitialised,
		  CORBA::COMPLETED_NO);

  return ((omniOrbPOA*)(PortableServer::POA_ptr) poa)->
    servant__this(this, repoId);
}


omniObjRef*
PortableServer::ServantBase::_do_get_interface()
{
  CORBA::_objref_InterfaceDef* p = _get_interface();
  if( p )  return p->_PR_getobj();

  // If we get here then we assume that _get_interface has not
  // been overriden, and provide the default implementation.

  // repoId should not be empty for statically defined
  // servants.  This version should not have been called
  // if it is a dynamic implementation.
  const char* repoId = _mostDerivedRepoId();
  OMNIORB_ASSERT(repoId && *repoId);

  // Obtain the object reference for the interface repository.
  CORBA::Object_var repository = CORBA::Object::_nil();
  try {
    repository = omniInitialReferences::resolve("InterfaceRepository");
  }
  catch (...) {
  }
  if( CORBA::is_nil(repository) )
    OMNIORB_THROW(INTF_REPOS,INTF_REPOS_NotAvailable, CORBA::COMPLETED_NO);

  // Make a call to the interface repository.
  omniStdCallDesc::_cCORBA_mObject_i_cstring
    call_desc(omniDynamicLib::ops->lookup_id_lcfn, "lookup_id", 10, repoId);
  repository->_PR_getobj()->_invoke(call_desc);

  CORBA::Object_ptr result = call_desc.result();
  return result ? result->_PR_getobj() : 0;
}


void*
PortableServer::ServantBase::_downcast()
{
  return (Servant) this;
}


//////////////////////////////////////////////////////////////////////
//////////////////////// C++ Mapping Specific ////////////////////////
//////////////////////////////////////////////////////////////////////

char*
PortableServer::ObjectId_to_string(const ObjectId& id)
{
  int len = id.length();

  char* s = _CORBA_String_helper::alloc(len);

  for( int i = 0; i < len; i++ )
    if( (char) (s[i] = id[i]) == '\0' ) {
      _CORBA_String_helper::dealloc(s);
      OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectId, CORBA::COMPLETED_NO);
    }

  s[len] = '\0';
  return s;
}


_CORBA_WChar*
PortableServer::ObjectId_to_wstring(const ObjectId& id)
{
  if (id.length() % SIZEOF_WCHAR != 0)
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectId, CORBA::COMPLETED_NO);

  int len = id.length() / SIZEOF_WCHAR;

  _CORBA_WChar* s = _CORBA_WString_helper::alloc(len);

  _CORBA_WChar* buf = (_CORBA_WChar*)id.NP_data();

  for( int i = 0; i < len; i++ )
    if( (s[i] = buf[i]) == 0 ) {
      _CORBA_WString_helper::dealloc(s);
      OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectId, CORBA::COMPLETED_NO);
    }

  s[len] = '\0';
  return s;
}


PortableServer::ObjectId*
PortableServer::string_to_ObjectId(const char* s)
{
  int len = strlen(s);
  ObjectId* pid = new ObjectId(len);
  ObjectId& id = *pid;

  id.length(len);

  for( int i = 0; i < len; i++ )  id[i] = *s++;

  return pid;
}


PortableServer::ObjectId*
PortableServer::wstring_to_ObjectId(const _CORBA_WChar* s)
{
  int len = _CORBA_WString_helper::len(s);
  ObjectId* pid = new ObjectId(len * SIZEOF_WCHAR);
  
  pid->length(len * SIZEOF_WCHAR);

  _CORBA_WChar* buf = (_CORBA_WChar*)pid->NP_data();

  for( int i = 0; i < len; i++ )  *buf++ = *s++;

  return pid;
}
