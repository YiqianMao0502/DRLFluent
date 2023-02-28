// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniPolicy.cc              Created on: 2001/11/07
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2013 Apasphere Ltd
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
//    omniORB specific policies
//

#include <omniORB4/CORBA.h>
#include <omniORB4/objTracker.h>

OMNI_USING_NAMESPACE(omni)

#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the variables external linkage otherwise. Its a bug.
namespace omniPolicy {

_init_in_def_( const CORBA::PolicyType
	       LOCAL_SHORTCUT_POLICY_TYPE = 0x41545401; )

_init_in_def_( const LocalShortcutPolicyValue
               LOCAL_CALLS_THROUGH_POA = 0; )

_init_in_def_( const LocalShortcutPolicyValue
               LOCAL_CALLS_SHORTCUT    = 0; )

_init_in_def_( const CORBA::PolicyType
	       ENDPOINT_PUBLISH_POLICY_TYPE = 0x41545402; )
}
#else
_init_in_def_( const CORBA::PolicyType
	       omniPolicy::LOCAL_SHORTCUT_POLICY_TYPE = 0x41545401; )

_init_in_def_( const omniPolicy::LocalShortcutPolicyValue
               omniPolicy::LOCAL_CALLS_THROUGH_POA = 0; )

_init_in_def_( const omniPolicy::LocalShortcutPolicyValue
               omniPolicy::LOCAL_CALLS_SHORTCUT    = 0; )

_init_in_def_( const CORBA::PolicyType
	       omniPolicy::ENDPOINT_PUBLISH_POLICY_TYPE = 0x41545402; )

#endif

//
// Local shortcut

omniPolicy::LocalShortcutPolicy::~LocalShortcutPolicy() {}

CORBA::Policy_ptr
omniPolicy::LocalShortcutPolicy::copy()
{
  if (_NP_is_nil())  _CORBA_invoked_nil_pseudo_ref();
  return new LocalShortcutPolicy(pd_value);
}

void*
omniPolicy::LocalShortcutPolicy::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if (omni::ptrStrMatch(repoId, omniPolicy::LocalShortcutPolicy::_PD_repoId))
    return (omniPolicy::LocalShortcutPolicy_ptr) this;
  if (omni::ptrStrMatch(repoId, CORBA::Policy::_PD_repoId))
    return (CORBA::Policy_ptr) this;
  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))
    return (CORBA::Object_ptr) this;

  return 0;
}

omniPolicy::LocalShortcutPolicy_ptr
omniPolicy::LocalShortcutPolicy::_duplicate(omniPolicy::LocalShortcutPolicy_ptr obj)
{
  if (!CORBA::is_nil(obj))  obj->_NP_incrRefCount();

  return obj;
}

omniPolicy::LocalShortcutPolicy_ptr
omniPolicy::LocalShortcutPolicy::_narrow(CORBA::Object_ptr obj)
{
  if (CORBA::is_nil(obj))  return _nil();

  LocalShortcutPolicy_ptr p = (LocalShortcutPolicy_ptr) obj->_ptrToObjRef(LocalShortcutPolicy::_PD_repoId);

  if (p)  p->_NP_incrRefCount();

  return p ? p : _nil();
}

omniPolicy::LocalShortcutPolicy_ptr
omniPolicy::LocalShortcutPolicy::_nil()
{
  static LocalShortcutPolicy* _the_nil_ptr = 0;
  if (!_the_nil_ptr) {
    omni::nilRefLock().lock();
    if (!_the_nil_ptr) {
      _the_nil_ptr = new LocalShortcutPolicy;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char*
omniPolicy::LocalShortcutPolicy::_PD_repoId = "IDL:omniorb.net/omniPolicy/LocalShortcutPolicy:1.0";


omniPolicy::LocalShortcutPolicy_ptr
omniPolicy::create_local_shortcut_policy(LocalShortcutPolicyValue v)
{
  return new LocalShortcutPolicy(v);
}


//
// EndPoint publishing

// EndPointPublishPolicy::getEPs and EndPointPublishPolicy destructor
// are implemented in ior.cc, alongside the declaration of IORPublish.

omniPolicy::EndPointPublishPolicy::
EndPointPublishPolicy(const omniPolicy::EndPointPublishPolicyValue& value)
  : CORBA::Policy(omniPolicy::ENDPOINT_PUBLISH_POLICY_TYPE),
    pd_value(value),
    pd_eps(0)
{
  for (CORBA::ULong idx=0; idx != value.length(); ++idx) {
    if (!giopEndpoint::strIsValidEndpoint(value[idx])) {
      
      if (omniORB::trace(1)) {
        omniORB::logger log;
        log << "Invalid endPoint '" << value[idx]
            << "' in EndPointPublishPolicy\n";
      }
      throw CORBA::PolicyError(CORBA::BAD_POLICY_VALUE);
    }
  }
}

CORBA::Policy_ptr
omniPolicy::EndPointPublishPolicy::copy()
{
  if (_NP_is_nil())  _CORBA_invoked_nil_pseudo_ref();
  return new EndPointPublishPolicy(pd_value);
}

void*
omniPolicy::EndPointPublishPolicy::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if (omni::ptrStrMatch(repoId, omniPolicy::EndPointPublishPolicy::_PD_repoId))
    return (omniPolicy::EndPointPublishPolicy_ptr) this;
  if (omni::ptrStrMatch(repoId, CORBA::Policy::_PD_repoId))
    return (CORBA::Policy_ptr) this;
  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))
    return (CORBA::Object_ptr) this;

  return 0;
}

omniPolicy::EndPointPublishPolicy_ptr
omniPolicy::EndPointPublishPolicy::_duplicate(omniPolicy::EndPointPublishPolicy_ptr obj)
{
  if (!CORBA::is_nil(obj))  obj->_NP_incrRefCount();

  return obj;
}

omniPolicy::EndPointPublishPolicy_ptr
omniPolicy::EndPointPublishPolicy::_narrow(CORBA::Object_ptr obj)
{
  if (CORBA::is_nil(obj))  return _nil();

  EndPointPublishPolicy_ptr p = (EndPointPublishPolicy_ptr) obj->_ptrToObjRef(EndPointPublishPolicy::_PD_repoId);

  if (p)  p->_NP_incrRefCount();

  return p ? p : _nil();
}

omniPolicy::EndPointPublishPolicy_ptr
omniPolicy::EndPointPublishPolicy::_nil()
{
  static EndPointPublishPolicy* _the_nil_ptr = 0;
  if (!_the_nil_ptr) {
    omni::nilRefLock().lock();
    if (!_the_nil_ptr) {
      _the_nil_ptr = new EndPointPublishPolicy;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char*
omniPolicy::EndPointPublishPolicy::_PD_repoId = "IDL:omniorb.net/omniPolicy/EndPointPublishPolicy:1.0";


omniPolicy::EndPointPublishPolicy_ptr
omniPolicy::
create_endpoint_publish_policy(const EndPointPublishPolicyValue& v)
{
  return new EndPointPublishPolicy(v);
}
