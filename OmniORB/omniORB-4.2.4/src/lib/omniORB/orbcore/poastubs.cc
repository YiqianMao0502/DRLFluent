// -*- Mode: C++; -*-
//                            Package   : omniORB
// poastubs.cc                Created on: 19/7/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2003-2012 Apasphere Ltd
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
// Description:
//
//    Hand-edited generated code for POA IDL.
 
#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/objTracker.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////////// ForwardRequest ///////////////////////////
//////////////////////////////////////////////////////////////////////

::CORBA::Exception::insertExceptionToAny PortableServer::ForwardRequest::insertToAnyFn = 0;
::CORBA::Exception::insertExceptionToAnyNCP PortableServer::ForwardRequest::insertToAnyFnNCP = 0;

PortableServer::ForwardRequest::ForwardRequest(const PortableServer::ForwardRequest& _s) : ::CORBA::UserException(_s)
{
  forward_reference = _s.forward_reference;
}

PortableServer::ForwardRequest::ForwardRequest(::CORBA::Object_ptr _forward_reference)
{
  pd_insertToAnyFn    = PortableServer::ForwardRequest::insertToAnyFn;
  pd_insertToAnyFnNCP = PortableServer::ForwardRequest::insertToAnyFnNCP;
  CORBA::Object::_duplicate(_forward_reference);
  forward_reference = _forward_reference;
}

PortableServer::ForwardRequest& PortableServer::ForwardRequest::operator=(const PortableServer::ForwardRequest& _s)
{
  ((::CORBA::UserException*) this)->operator=(_s);
  forward_reference = _s.forward_reference;
  return *this;
}

PortableServer::ForwardRequest::~ForwardRequest() {}

void PortableServer::ForwardRequest::_raise() const { throw *this; }

const char* PortableServer::ForwardRequest::_PD_repoId = "IDL:omg.org/PortableServer/ForwardRequest:1.0";
const char* PortableServer::ForwardRequest::_PD_typeId = "Exception/UserException/PortableServer::ForwardRequest";

PortableServer::ForwardRequest* PortableServer::ForwardRequest::_downcast(::CORBA::Exception* _e) {
  return (ForwardRequest*) _NP_is_a(_e, _PD_typeId);
}

const PortableServer::ForwardRequest* PortableServer::ForwardRequest::_downcast(const ::CORBA::Exception* _e) {
  return (const ForwardRequest*) _NP_is_a(_e, _PD_typeId);
}

::CORBA::Exception* PortableServer::ForwardRequest::_NP_duplicate() const {
  return new ForwardRequest(*this);
}

const char* PortableServer::ForwardRequest::_NP_typeId() const {
  return _PD_typeId;
}

const char* PortableServer::ForwardRequest::_NP_repoId(int* _size) const {
  *_size = sizeof("IDL:omg.org/PortableServer/ForwardRequest:1.0");
  return _PD_repoId;
}

void PortableServer::ForwardRequest::_NP_marshal(cdrStream& _s) const {
  *this >>= _s;
}

void
PortableServer::ForwardRequest::operator>>= (cdrStream& _n) const
{
  ::CORBA::Object::_marshalObjRef(forward_reference,_n);

}

void
PortableServer::ForwardRequest::operator<<= (cdrStream& _n)
{
  forward_reference = ::CORBA::Object::_unmarshalObjRef(_n);

}

PortableServer::AdapterActivator_ptr PortableServer::AdapterActivator_Helper::_nil() {
  return ::PortableServer::AdapterActivator::_nil();
}

::CORBA::Boolean PortableServer::AdapterActivator_Helper::is_nil(::PortableServer::AdapterActivator_ptr p) {
  return ::CORBA::is_nil(p);
}

void PortableServer::AdapterActivator_Helper::release(::PortableServer::AdapterActivator_ptr p) {
  ::CORBA::release(p);
}

void PortableServer::AdapterActivator_Helper::marshalObjRef(::PortableServer::AdapterActivator_ptr obj, cdrStream& s) {
  ::PortableServer::AdapterActivator::_marshalObjRef(obj, s);
}

PortableServer::AdapterActivator_ptr PortableServer::AdapterActivator_Helper::unmarshalObjRef(cdrStream& s) {
  return ::PortableServer::AdapterActivator::_unmarshalObjRef(s);
}


void
PortableServer::AdapterActivator::_NP_incrRefCount()
{
  if (_NP_is_pseudo())
    _add_ref();
  else
    omni::duplicateObjRef(_PR_getobj());
}

void
PortableServer::AdapterActivator::_NP_decrRefCount()
{
  if (_NP_is_pseudo())
    _remove_ref();
  else
    omni::releaseObjRef(_PR_getobj());
}

PortableServer::AdapterActivator_ptr
PortableServer::AdapterActivator::_duplicate(::PortableServer::AdapterActivator_ptr obj)
{
  if (obj && !obj->_NP_is_nil())  obj->_NP_incrRefCount();
  return obj;
}


PortableServer::AdapterActivator_ptr
PortableServer::AdapterActivator::_narrow(::CORBA::Object_ptr obj)
{
  if (!obj || obj->_NP_is_nil())
    return _nil();

  if (obj->_NP_is_pseudo()) {
    _ptr_type e = (_ptr_type) obj->_ptrToObjRef(_PD_repoId);
    if (e) {
      e->_NP_incrRefCount();
      return e;
    }
    else {
      return _nil();
    }
  }
  else {
    _ptr_type e = (_ptr_type) obj->_PR_getobj()->_realNarrow(_PD_repoId);
    return e ? e : _nil();
  }
}

PortableServer::AdapterActivator_ptr
PortableServer::AdapterActivator::_unchecked_narrow(::CORBA::Object_ptr obj)
{
  return _narrow(obj);
}


PortableServer::AdapterActivator_ptr
PortableServer::AdapterActivator::_nil()
{
  static _objref_AdapterActivator* _the_nil_ptr = 0;
  if (!_the_nil_ptr) {
    omni::nilRefLock().lock();
    if (!_the_nil_ptr) {
      _the_nil_ptr = new _objref_AdapterActivator;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char* PortableServer::AdapterActivator::_PD_repoId = "IDL:omg.org/PortableServer/AdapterActivator:1.0";


PortableServer::AdapterActivator::AdapterActivator()
{
  _PR_setobj((omniObjRef*)1);
}

PortableServer::AdapterActivator::~AdapterActivator() {}

PortableServer::_objref_AdapterActivator::~_objref_AdapterActivator() {}


PortableServer::_objref_AdapterActivator::_objref_AdapterActivator(omniIOR* ior, omniIdentity* id) :
   omniObjRef(::PortableServer::AdapterActivator::_PD_repoId, ior, id, 1)
{
  _PR_setobj(this);
}


void*
PortableServer::AdapterActivator::_ptrToObjRef(const char* id)
{
  if (id == ::PortableServer::AdapterActivator::_PD_repoId)
    return (::PortableServer::AdapterActivator_ptr) this;

  if (id == ::CORBA::LocalObject::_PD_repoId)
    return (::CORBA::LocalObject_ptr) this;

  if (id == ::CORBA::Object::_PD_repoId)
    return (::CORBA::Object_ptr) this;

  if (omni::strMatch(id, ::PortableServer::AdapterActivator::_PD_repoId))
    return (::PortableServer::AdapterActivator_ptr) this;

  if (omni::strMatch(id, ::CORBA::LocalObject::_PD_repoId))
    return (::CORBA::LocalObject_ptr) this;

  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (::CORBA::Object_ptr) this;

  return 0;
}

void*
PortableServer::_objref_AdapterActivator::_ptrToObjRef(const char* id)
{
  if (id == ::PortableServer::AdapterActivator::_PD_repoId)
    return (::PortableServer::AdapterActivator_ptr) this;

  if (id == ::CORBA::LocalObject::_PD_repoId)
    return (::CORBA::LocalObject_ptr) this;

  if (id == ::CORBA::Object::_PD_repoId)
    return (::CORBA::Object_ptr) this;

  if (omni::strMatch(id, ::PortableServer::AdapterActivator::_PD_repoId))
    return (::PortableServer::AdapterActivator_ptr) this;

  if (omni::strMatch(id, ::CORBA::LocalObject::_PD_repoId))
    return (::CORBA::LocalObject_ptr) this;

  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (::CORBA::Object_ptr) this;

  return 0;
}


// Proxy call descriptor class. Mangled signature:
//  _cboolean_i_cPortableServer_mPOA_i_cstring
class _0RL_cd_3c165f58b5a16b59_00000000
  : public omniLocalOnlyCallDescriptor
{
public:
  inline _0RL_cd_3c165f58b5a16b59_00000000(LocalCallFn lcfn, const char* op, size_t oplen, _CORBA_Boolean oneway, PortableServer::POA_ptr a_0, const char* a_1) :
    omniLocalOnlyCallDescriptor(lcfn, op, oplen, oneway),
    arg_0(a_0),
    arg_1(a_1)  {}

  inline CORBA::Boolean result() { return pd_result; }

  PortableServer::POA_ptr arg_0;
  const char* arg_1;
  CORBA::Boolean pd_result;
};


// Local call call-back function.
static void
_0RL_lcfn_3c165f58b5a16b59_10000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_3c165f58b5a16b59_00000000* tcd = (_0RL_cd_3c165f58b5a16b59_00000000*) cd;
  PortableServer::_impl_AdapterActivator* impl = (PortableServer::_impl_AdapterActivator*) svnt->_ptrToInterface(PortableServer::AdapterActivator::_PD_repoId);
  tcd->pd_result = impl->unknown_adapter(tcd->arg_0, tcd->arg_1);
}


CORBA::Boolean PortableServer::_objref_AdapterActivator::unknown_adapter(PortableServer::POA_ptr parent, const char* name)
{
  _0RL_cd_3c165f58b5a16b59_00000000 _call_desc(_0RL_lcfn_3c165f58b5a16b59_10000000, "unknown_adapter", 16, 0, parent, name);

  _invoke(_call_desc);
  return _call_desc.result();
}


PortableServer::_pof_AdapterActivator::~_pof_AdapterActivator() {}


omniObjRef*
PortableServer::_pof_AdapterActivator::newObjRef(omniIOR* ior, omniIdentity* id)
{
  return new ::PortableServer::_objref_AdapterActivator(ior, id);
}


CORBA::Boolean
PortableServer::_pof_AdapterActivator::is_a(const char* id) const
{
  if (omni::ptrStrMatch(id, PortableServer::AdapterActivator::_PD_repoId))
    return 1;

  return 0;
}


const PortableServer::_pof_AdapterActivator _the_pof_PortableServer_mAdapterActivator;


PortableServer::_impl_AdapterActivator::~_impl_AdapterActivator() {}


CORBA::Boolean
PortableServer::_impl_AdapterActivator::_dispatch(omniCallHandle& handle)
{
  return 0;
}


void*
PortableServer::_impl_AdapterActivator::_ptrToInterface(const char* id)
{
  if (id == ::PortableServer::AdapterActivator::_PD_repoId)
    return (::PortableServer::_impl_AdapterActivator*) this;
  
  if (id == ::CORBA::Object::_PD_repoId)
    return (void*) 1;

  if (omni::strMatch(id, ::PortableServer::AdapterActivator::_PD_repoId))
    return (::PortableServer::_impl_AdapterActivator*) this;
  
  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (void*) 1;

  return 0;
}


const char*
PortableServer::_impl_AdapterActivator::_mostDerivedRepoId()
{
  return ::PortableServer::AdapterActivator::_PD_repoId;
}


PortableServer::ServantManager_ptr PortableServer::ServantManager_Helper::_nil() {
  return ::PortableServer::ServantManager::_nil();
}

::CORBA::Boolean PortableServer::ServantManager_Helper::is_nil(::PortableServer::ServantManager_ptr p) {
  return ::CORBA::is_nil(p);
}

void PortableServer::ServantManager_Helper::release(PortableServer::ServantManager_ptr p) {
  CORBA::release(p);
}

void PortableServer::ServantManager_Helper::duplicate(PortableServer::ServantManager_ptr p) {
  if (p && !p->_NP_is_nil())  p->_NP_incrRefCount();
}

void PortableServer::ServantManager_Helper::marshalObjRef(PortableServer::ServantManager_ptr obj, cdrStream& s) {
  PortableServer::ServantManager::_marshalObjRef(obj, s);
}

PortableServer::ServantManager_ptr PortableServer::ServantManager_Helper::unmarshalObjRef(cdrStream& s) {
  return PortableServer::ServantManager::_unmarshalObjRef(s);
}

void
PortableServer::ServantManager::_NP_incrRefCount()
{
  if (_NP_is_pseudo())
    _add_ref();
  else
    omni::duplicateObjRef(_PR_getobj());
}

void
PortableServer::ServantManager::_NP_decrRefCount()
{
  if (_NP_is_pseudo())
    _remove_ref();
  else
    omni::releaseObjRef(_PR_getobj());
}


PortableServer::ServantManager_ptr
PortableServer::ServantManager::_duplicate(::PortableServer::ServantManager_ptr obj)
{
  if (obj && !obj->_NP_is_nil())  obj->_NP_incrRefCount();
  return obj;
}


PortableServer::ServantManager_ptr
PortableServer::ServantManager::_narrow(::CORBA::Object_ptr obj)
{
  if (!obj || obj->_NP_is_nil())
    return _nil();

  if (obj->_NP_is_pseudo()) {
    _ptr_type e = (_ptr_type) obj->_ptrToObjRef(_PD_repoId);
    if (e) {
      e->_NP_incrRefCount();
      return e;
    }
    else {
      return _nil();
    }
  }
  else {
    _ptr_type e = (_ptr_type) obj->_PR_getobj()->_realNarrow(_PD_repoId);
    return e ? e : _nil();
  }
}

PortableServer::ServantManager_ptr
PortableServer::ServantManager::_unchecked_narrow(::CORBA::Object_ptr obj)
{
  return _narrow(obj);
}

PortableServer::ServantManager_ptr
PortableServer::ServantManager::_nil()
{
  static _objref_ServantManager* _the_nil_ptr = 0;
  if (!_the_nil_ptr) {
    omni::nilRefLock().lock();
    if (!_the_nil_ptr) {
      _the_nil_ptr = new _objref_ServantManager;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char* PortableServer::ServantManager::_PD_repoId = "IDL:omg.org/PortableServer/ServantManager:1.0";


PortableServer::ServantManager::ServantManager()
{
  _PR_setobj((omniObjRef*)1);
}

PortableServer::ServantManager::~ServantManager() {}

PortableServer::_objref_ServantManager::~_objref_ServantManager() {}

PortableServer::_objref_ServantManager::_objref_ServantManager(omniIOR* ior, omniIdentity* id) :
   omniObjRef(::PortableServer::ServantManager::_PD_repoId, ior, id, 1)
{
  _PR_setobj(this);
}


void*
PortableServer::ServantManager::_ptrToObjRef(const char* id)
{
  if (id == ::PortableServer::ServantManager::_PD_repoId)
    return (::PortableServer::ServantManager_ptr) this;

  if (id == ::CORBA::LocalObject::_PD_repoId)
    return (::CORBA::LocalObject_ptr) this;

  if (id == ::CORBA::Object::_PD_repoId)
    return (::CORBA::Object_ptr) this;

  if (omni::strMatch(id, ::PortableServer::ServantManager::_PD_repoId))
    return (::PortableServer::ServantManager_ptr) this;

  if (omni::strMatch(id, ::CORBA::LocalObject::_PD_repoId))
    return (::CORBA::LocalObject_ptr) this;

  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (::CORBA::Object_ptr) this;

  return 0;
}

void*
PortableServer::_objref_ServantManager::_ptrToObjRef(const char* id)
{
  if (id == ::PortableServer::ServantManager::_PD_repoId)
    return (::PortableServer::ServantManager_ptr) this;

  if (id == ::CORBA::LocalObject::_PD_repoId)
    return (::CORBA::LocalObject_ptr) this;

  if (id == ::CORBA::Object::_PD_repoId)
    return (::CORBA::Object_ptr) this;

  if (omni::strMatch(id, ::PortableServer::ServantManager::_PD_repoId))
    return (::PortableServer::ServantManager_ptr) this;

  if (omni::strMatch(id, ::CORBA::LocalObject::_PD_repoId))
    return (::CORBA::LocalObject_ptr) this;

  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (::CORBA::Object_ptr) this;

  return 0;
}


PortableServer::_pof_ServantManager::~_pof_ServantManager() {}


omniObjRef*
PortableServer::_pof_ServantManager::newObjRef(omniIOR* ior, omniIdentity* id)
{
  return new ::PortableServer::_objref_ServantManager(ior, id);
}


::CORBA::Boolean
PortableServer::_pof_ServantManager::is_a(const char* id) const
{
  if (omni::ptrStrMatch(id, ::PortableServer::ServantManager::_PD_repoId))
    return 1;

  return 0;
}


const PortableServer::_pof_ServantManager _the_pof_PortableServer_mServantManager;


PortableServer::_impl_ServantManager::~_impl_ServantManager() {}


::CORBA::Boolean
PortableServer::_impl_ServantManager::_dispatch(omniCallHandle& _handle)
{
  return 0;
}


void*
PortableServer::_impl_ServantManager::_ptrToInterface(const char* id)
{
  if (id == ::PortableServer::ServantManager::_PD_repoId)
    return (::PortableServer::_impl_ServantManager*) this;
  
  if (id == ::CORBA::Object::_PD_repoId)
    return (void*) 1;

  if (omni::strMatch(id, ::PortableServer::ServantManager::_PD_repoId))
    return (::PortableServer::_impl_ServantManager*) this;
  
  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (void*) 1;

  return 0;
}


const char*
PortableServer::_impl_ServantManager::_mostDerivedRepoId()
{
  return ::PortableServer::ServantManager::_PD_repoId;
}


PortableServer::ServantActivator_ptr PortableServer::ServantActivator_Helper::_nil() {
  return ::PortableServer::ServantActivator::_nil();
}

::CORBA::Boolean PortableServer::ServantActivator_Helper::is_nil(::PortableServer::ServantActivator_ptr p) {
  return ::CORBA::is_nil(p);
}

void PortableServer::ServantActivator_Helper::release(PortableServer::ServantActivator_ptr p) {
  CORBA::release(p);
}

void PortableServer::ServantActivator_Helper::duplicate(PortableServer::ServantActivator_ptr p) {
  if (p && !p->_NP_is_nil())  p->_NP_incrRefCount();
}

void PortableServer::ServantActivator_Helper::marshalObjRef(::PortableServer::ServantActivator_ptr obj, cdrStream& s) {
  ::PortableServer::ServantActivator::_marshalObjRef(obj, s);
}

PortableServer::ServantActivator_ptr PortableServer::ServantActivator_Helper::unmarshalObjRef(cdrStream& s) {
  return ::PortableServer::ServantActivator::_unmarshalObjRef(s);
}


PortableServer::ServantActivator_ptr
PortableServer::ServantActivator::_duplicate(::PortableServer::ServantActivator_ptr obj)
{
  if (obj && !obj->_NP_is_nil())  obj->_NP_incrRefCount();
  return obj;
}


PortableServer::ServantActivator_ptr
PortableServer::ServantActivator::_narrow(::CORBA::Object_ptr obj)
{
  if (!obj || obj->_NP_is_nil())
    return _nil();

  if (obj->_NP_is_pseudo()) {
    _ptr_type e = (_ptr_type) obj->_ptrToObjRef(_PD_repoId);
    if (e) {
      e->_NP_incrRefCount();
      return e;
    }
    else {
      return _nil();
    }
  }
  else {
    _ptr_type e = (_ptr_type) obj->_PR_getobj()->_realNarrow(_PD_repoId);
    return e ? e : _nil();
  }
}

PortableServer::ServantActivator_ptr
PortableServer::ServantActivator::_unchecked_narrow(::CORBA::Object_ptr obj)
{
  return _narrow(obj);
}

PortableServer::ServantActivator_ptr
PortableServer::ServantActivator::_nil()
{
  static _objref_ServantActivator* _the_nil_ptr = 0;
  if (!_the_nil_ptr) {
    omni::nilRefLock().lock();
    if (!_the_nil_ptr) {
      _the_nil_ptr = new _objref_ServantActivator;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char* PortableServer::ServantActivator::_PD_repoId = "IDL:omg.org/PortableServer/ServantActivator:1.0";


PortableServer::ServantActivator::ServantActivator()
{
  _PR_setobj((omniObjRef*)1);
}

PortableServer::ServantActivator::~ServantActivator() {}

PortableServer::_objref_ServantActivator::~_objref_ServantActivator() {}


PortableServer::_objref_ServantActivator::_objref_ServantActivator(omniIOR* ior,
         omniIdentity* id)
 : omniObjRef(PortableServer::ServantActivator::_PD_repoId, ior, id, 1),
   OMNIORB_BASE_CTOR(PortableServer::)_objref_ServantManager(ior, id),
   _shortcut(0)
{
  _PR_setobj(this);
}


void*
PortableServer::ServantActivator::_ptrToObjRef(const char* id)
{
  if (id == ::PortableServer::ServantActivator::_PD_repoId)
    return (::PortableServer::ServantActivator_ptr) this;
  if (id == ::PortableServer::ServantManager::_PD_repoId)
    return (::PortableServer::ServantManager_ptr) this;


  if (id == ::CORBA::LocalObject::_PD_repoId)
    return (::CORBA::LocalObject_ptr) this;

  if (id == ::CORBA::Object::_PD_repoId)
    return (::CORBA::Object_ptr) this;

  if (omni::strMatch(id, ::PortableServer::ServantActivator::_PD_repoId))
    return (::PortableServer::ServantActivator_ptr) this;

  if (omni::strMatch(id, ::PortableServer::ServantManager::_PD_repoId))
    return (::PortableServer::ServantManager_ptr) this;

  if (omni::strMatch(id, ::CORBA::LocalObject::_PD_repoId))
    return (::CORBA::LocalObject_ptr) this;

  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (::CORBA::Object_ptr) this;

  return 0;
}

void*
PortableServer::_objref_ServantActivator::_ptrToObjRef(const char* id)
{
  if (id == ::PortableServer::ServantActivator::_PD_repoId)
    return (::PortableServer::ServantActivator_ptr) this;

  if (id == ::PortableServer::ServantManager::_PD_repoId)
    return (::PortableServer::ServantManager_ptr) this;

  if (id == ::CORBA::LocalObject::_PD_repoId)
    return (::CORBA::LocalObject_ptr) this;

  if (id == ::CORBA::Object::_PD_repoId)
    return (::CORBA::Object_ptr) this;

  if (omni::strMatch(id, ::PortableServer::ServantActivator::_PD_repoId))
    return (::PortableServer::ServantActivator_ptr) this;

  if (omni::strMatch(id, ::PortableServer::ServantManager::_PD_repoId))
    return (::PortableServer::ServantManager_ptr) this;

  if (omni::strMatch(id, ::CORBA::LocalObject::_PD_repoId))
    return (::CORBA::LocalObject_ptr) this;

  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (::CORBA::Object_ptr) this;

  return 0;
}

void
PortableServer::_objref_ServantActivator::_enableShortcut(omniServant* _svt, const _CORBA_Boolean* _inv)
{
  if (_svt)
    _shortcut = (_impl_ServantActivator*)_svt->_ptrToInterface(::PortableServer::ServantActivator::_PD_repoId);
  else
    _shortcut = 0;
  _invalid  = _inv;
  
}


// Proxy call descriptor class. Mangled signature:
//  _cshort_i_cPortableServer_mObjectId_i_cPortableServer_mPOA_e_cPortableServer_mForwardRequest
class _0RL_cd_3c165f58b5a16b59_20000000
  : public omniLocalOnlyCallDescriptor
{
public:
  inline _0RL_cd_3c165f58b5a16b59_20000000(LocalCallFn lcfn, const char* op, size_t oplen, _CORBA_Boolean oneway, const PortableServer::ObjectId& a_0, PortableServer::POA_ptr a_1) :
    omniLocalOnlyCallDescriptor(lcfn, op, oplen, oneway),
    arg_0(a_0),
    arg_1(a_1)  {}

  inline PortableServer::Servant result() { return pd_result; }

  const PortableServer::ObjectId& arg_0;
  PortableServer::POA_ptr arg_1;
  PortableServer::Servant pd_result;
};


// Local call call-back function.
static void
_0RL_lcfn_3c165f58b5a16b59_30000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_3c165f58b5a16b59_20000000* tcd = (_0RL_cd_3c165f58b5a16b59_20000000*) cd;
  PortableServer::_impl_ServantActivator* impl = (PortableServer::_impl_ServantActivator*) svnt->_ptrToInterface(PortableServer::ServantActivator::_PD_repoId);
  tcd->pd_result = impl->incarnate(tcd->arg_0, tcd->arg_1);
}


PortableServer::Servant PortableServer::_objref_ServantActivator::incarnate(const PortableServer::ObjectId& oid, PortableServer::POA_ptr adapter)
{
  _impl_ServantActivator* _s = _shortcut;
  if (_s) {
    if (!*_invalid) {
      return _s->incarnate(oid, adapter);
    }
    else {
      _enableShortcut(0,0);
      // drop through to normal invoke
    }
  }
  _0RL_cd_3c165f58b5a16b59_20000000 _call_desc(_0RL_lcfn_3c165f58b5a16b59_30000000, "incarnate", 10, 0, oid, adapter);

  _invoke(_call_desc);
  return _call_desc.result();
}


// Proxy call descriptor class. Mangled signature:
//  void_i_cPortableServer_mObjectId_i_cPortableServer_mPOA_i_cshort_i_cboolean_i_cboolean
class _0RL_cd_3c165f58b5a16b59_40000000
  : public omniLocalOnlyCallDescriptor
{
public:
  inline _0RL_cd_3c165f58b5a16b59_40000000(LocalCallFn lcfn, const char* op, size_t oplen, _CORBA_Boolean oneway, const PortableServer::ObjectId& a_0, PortableServer::POA_ptr a_1, PortableServer::Servant a_2, CORBA::Boolean a_3, CORBA::Boolean a_4) :
    omniLocalOnlyCallDescriptor(lcfn, op, oplen, oneway),
    arg_0(a_0),
    arg_1(a_1),
    arg_2(a_2),
    arg_3(a_3),
    arg_4(a_4)  {}

  const PortableServer::ObjectId& arg_0;
  PortableServer::POA_ptr arg_1;
  PortableServer::Servant arg_2;
  CORBA::Boolean arg_3;
  CORBA::Boolean arg_4;
};


// Local call call-back function.
static void
_0RL_lcfn_3c165f58b5a16b59_50000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_3c165f58b5a16b59_40000000* tcd = (_0RL_cd_3c165f58b5a16b59_40000000*) cd;
  PortableServer::_impl_ServantActivator* impl = (PortableServer::_impl_ServantActivator*) svnt->_ptrToInterface(PortableServer::ServantActivator::_PD_repoId);
  impl->etherealize(tcd->arg_0, tcd->arg_1, tcd->arg_2, tcd->arg_3, tcd->arg_4);
}


void PortableServer::_objref_ServantActivator::etherealize(const PortableServer::ObjectId& oid, PortableServer::POA_ptr adapter, PortableServer::Servant serv, CORBA::Boolean cleanup_in_progress, CORBA::Boolean remaining_activations)
{
  _impl_ServantActivator* _s = _shortcut;
  if (_s) {
    if (!*_invalid) {
      _s->etherealize(oid, adapter, serv, cleanup_in_progress, remaining_activations); return;
    }
    else {
      _enableShortcut(0,0);
      // drop through to normal invoke
    }
  }
  _0RL_cd_3c165f58b5a16b59_40000000 _call_desc(_0RL_lcfn_3c165f58b5a16b59_50000000, "etherealize", 12, 0, oid, adapter, serv, cleanup_in_progress, remaining_activations);

  _invoke(_call_desc);
}


PortableServer::_pof_ServantActivator::~_pof_ServantActivator() {}


omniObjRef*
PortableServer::_pof_ServantActivator::newObjRef(omniIOR* ior, omniIdentity* id)
{
  return new ::PortableServer::_objref_ServantActivator(ior, id);
}


::CORBA::Boolean
PortableServer::_pof_ServantActivator::is_a(const char* id) const
{
  if (omni::ptrStrMatch(id, ::PortableServer::ServantActivator::_PD_repoId))
    return 1;
  if (omni::ptrStrMatch(id, PortableServer::ServantManager::_PD_repoId))
    return 1;

  return 0;
}


const PortableServer::_pof_ServantActivator _the_pof_PortableServer_mServantActivator;


PortableServer::_impl_ServantActivator::~_impl_ServantActivator() {}


::CORBA::Boolean
PortableServer::_impl_ServantActivator::_dispatch(omniCallHandle& _handle)
{
  return 0;
}


void*
PortableServer::_impl_ServantActivator::_ptrToInterface(const char* id)
{
  if (id == ::PortableServer::ServantActivator::_PD_repoId)
    return (::PortableServer::_impl_ServantActivator*) this;
  if (id == ::PortableServer::ServantManager::_PD_repoId)
    return (::PortableServer::_impl_ServantManager*) this;


  if (id == ::CORBA::Object::_PD_repoId)
    return (void*) 1;

  if (omni::strMatch(id, ::PortableServer::ServantActivator::_PD_repoId))
    return (::PortableServer::_impl_ServantActivator*) this;
  if (omni::strMatch(id, ::PortableServer::ServantManager::_PD_repoId))
    return (::PortableServer::_impl_ServantManager*) this;


  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (void*) 1;
  return 0;
}


const char*
PortableServer::_impl_ServantActivator::_mostDerivedRepoId()
{
  return ::PortableServer::ServantActivator::_PD_repoId;
}


PortableServer::ServantLocator_ptr PortableServer::ServantLocator_Helper::_nil() {
  return ::PortableServer::ServantLocator::_nil();
}

::CORBA::Boolean PortableServer::ServantLocator_Helper::is_nil(::PortableServer::ServantLocator_ptr p) {
  return ::CORBA::is_nil(p);
}

void PortableServer::ServantLocator_Helper::release(PortableServer::ServantLocator_ptr p) {
  CORBA::release(p);
}

void PortableServer::ServantLocator_Helper::duplicate(PortableServer::ServantLocator_ptr p) {
  if (p && !p->_NP_is_nil())  p->_NP_incrRefCount();
}

void PortableServer::ServantLocator_Helper::marshalObjRef(::PortableServer::ServantLocator_ptr obj, cdrStream& s) {
  ::PortableServer::ServantLocator::_marshalObjRef(obj, s);
}

PortableServer::ServantLocator_ptr PortableServer::ServantLocator_Helper::unmarshalObjRef(cdrStream& s) {
  return ::PortableServer::ServantLocator::_unmarshalObjRef(s);
}


PortableServer::ServantLocator_ptr
PortableServer::ServantLocator::_duplicate(::PortableServer::ServantLocator_ptr obj)
{
  if (obj && !obj->_NP_is_nil())  obj->_NP_incrRefCount();
  return obj;
}


PortableServer::ServantLocator_ptr
PortableServer::ServantLocator::_narrow(::CORBA::Object_ptr obj)
{
  if (!obj || obj->_NP_is_nil())
    return _nil();

  if (obj->_NP_is_pseudo()) {
    _ptr_type e = (_ptr_type) obj->_ptrToObjRef(_PD_repoId);
    if (e) {
      e->_NP_incrRefCount();
      return e;
    }
    else {
      return _nil();
    }
  }
  else {
    _ptr_type e = (_ptr_type) obj->_PR_getobj()->_realNarrow(_PD_repoId);
    return e ? e : _nil();
  }
}

PortableServer::ServantLocator_ptr
PortableServer::ServantLocator::_unchecked_narrow(::CORBA::Object_ptr obj)
{
  return _narrow(obj);
}


PortableServer::ServantLocator_ptr
PortableServer::ServantLocator::_nil()
{
  static _objref_ServantLocator* _the_nil_ptr = 0;
  if (!_the_nil_ptr) {
    omni::nilRefLock().lock();
    if (!_the_nil_ptr) {
      _the_nil_ptr = new _objref_ServantLocator;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char* PortableServer::ServantLocator::_PD_repoId = "IDL:omg.org/PortableServer/ServantLocator:1.0";


PortableServer::ServantLocator::ServantLocator()
{
  _PR_setobj((omniObjRef*)1);
}

PortableServer::ServantLocator::~ServantLocator() {}


PortableServer::_objref_ServantLocator::~_objref_ServantLocator() {}


PortableServer::_objref_ServantLocator::_objref_ServantLocator(omniIOR* ior,
         omniIdentity* id)
 : omniObjRef(PortableServer::ServantLocator::_PD_repoId, ior, id, 1),
   OMNIORB_BASE_CTOR(PortableServer::)_objref_ServantManager(ior, id),
   _shortcut(0)
{
  _PR_setobj(this);
}


void*
PortableServer::ServantLocator::_ptrToObjRef(const char* id)
{
  if (id == ::PortableServer::ServantLocator::_PD_repoId)
    return (::PortableServer::ServantLocator_ptr) this;

  if (id == ::PortableServer::ServantManager::_PD_repoId)
    return (::PortableServer::ServantManager_ptr) this;

  if (id == ::CORBA::LocalObject::_PD_repoId)
    return (::CORBA::LocalObject_ptr) this;

  if (id == ::CORBA::Object::_PD_repoId)
    return (::CORBA::Object_ptr) this;

  if (omni::strMatch(id, ::PortableServer::ServantLocator::_PD_repoId))
    return (::PortableServer::ServantLocator_ptr) this;

  if (omni::strMatch(id, ::PortableServer::ServantManager::_PD_repoId))
    return (::PortableServer::ServantManager_ptr) this;

  if (omni::strMatch(id, ::CORBA::LocalObject::_PD_repoId))
    return (::CORBA::LocalObject_ptr) this;

  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (::CORBA::Object_ptr) this;

  return 0;
}

void*
PortableServer::_objref_ServantLocator::_ptrToObjRef(const char* id)
{
  if (id == ::PortableServer::ServantLocator::_PD_repoId)
    return (::PortableServer::ServantLocator_ptr) this;

  if (id == ::PortableServer::ServantManager::_PD_repoId)
    return (::PortableServer::ServantManager_ptr) this;

  if (id == ::CORBA::LocalObject::_PD_repoId)
    return (::CORBA::LocalObject_ptr) this;

  if (id == ::CORBA::Object::_PD_repoId)
    return (::CORBA::Object_ptr) this;

  if (omni::strMatch(id, ::PortableServer::ServantLocator::_PD_repoId))
    return (::PortableServer::ServantLocator_ptr) this;

  if (omni::strMatch(id, ::PortableServer::ServantManager::_PD_repoId))
    return (::PortableServer::ServantManager_ptr) this;

  if (omni::strMatch(id, ::CORBA::LocalObject::_PD_repoId))
    return (::CORBA::LocalObject_ptr) this;

  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (::CORBA::Object_ptr) this;

  return 0;
}

void
PortableServer::_objref_ServantLocator::_enableShortcut(omniServant* _svt, const _CORBA_Boolean* _inv)
{
  if (_svt)
    _shortcut = (_impl_ServantLocator*)_svt->_ptrToInterface(::PortableServer::ServantLocator::_PD_repoId);
  else
    _shortcut = 0;
  _invalid  = _inv;
  
}

// Proxy call descriptor class. Mangled signature:
//  _cshort_i_cPortableServer_mObjectId_i_cPortableServer_mPOA_i_cstring_o_cshort_e_cPortableServer_mForwardRequest
class _0RL_cd_3c165f58b5a16b59_60000000
  : public omniLocalOnlyCallDescriptor
{
public:
  inline _0RL_cd_3c165f58b5a16b59_60000000(LocalCallFn lcfn, const char* op, size_t oplen, _CORBA_Boolean oneway, const PortableServer::ObjectId& a_0, PortableServer::POA_ptr a_1, const char* a_2, PortableServer::ServantLocator::Cookie& a_3) :
    omniLocalOnlyCallDescriptor(lcfn, op, oplen, oneway),
    arg_0(a_0),
    arg_1(a_1),
    arg_2(a_2),
    arg_3(a_3)  {}

  inline PortableServer::Servant result() { return pd_result; }

  const PortableServer::ObjectId& arg_0;
  PortableServer::POA_ptr arg_1;
  const char* arg_2;
  PortableServer::ServantLocator::Cookie& arg_3;
  PortableServer::Servant pd_result;
};


// Local call call-back function.
static void
_0RL_lcfn_3c165f58b5a16b59_70000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_3c165f58b5a16b59_60000000* tcd = (_0RL_cd_3c165f58b5a16b59_60000000*) cd;
  PortableServer::_impl_ServantLocator* impl = (PortableServer::_impl_ServantLocator*) svnt->_ptrToInterface(PortableServer::ServantLocator::_PD_repoId);
  tcd->pd_result = impl->preinvoke(tcd->arg_0, tcd->arg_1, tcd->arg_2, tcd->arg_3);
}


PortableServer::Servant PortableServer::_objref_ServantLocator::preinvoke(const PortableServer::ObjectId& oid, PortableServer::POA_ptr adapter, const char* operation, PortableServer::ServantLocator::Cookie& the_cookie)
{
  _impl_ServantLocator* _s = _shortcut;
  if (_s) {
    if (!*_invalid) {
      return _s->preinvoke(oid, adapter, operation, the_cookie);
    }
    else {
      _enableShortcut(0,0);
      // drop through to normal invoke
    }
  }
  _0RL_cd_3c165f58b5a16b59_60000000 _call_desc(_0RL_lcfn_3c165f58b5a16b59_70000000, "preinvoke", 10, 0, oid, adapter, operation, the_cookie);

  _invoke(_call_desc);
  return _call_desc.result();
}


// Proxy call descriptor class. Mangled signature:
//  void_i_cPortableServer_mObjectId_i_cPortableServer_mPOA_i_cstring_i_cshort_i_cshort
class _0RL_cd_3c165f58b5a16b59_80000000
  : public omniLocalOnlyCallDescriptor
{
public:
  inline _0RL_cd_3c165f58b5a16b59_80000000(LocalCallFn lcfn, const char* op, size_t oplen, _CORBA_Boolean oneway, const PortableServer::ObjectId& a_0, PortableServer::POA_ptr a_1, const char* a_2, PortableServer::ServantLocator::Cookie a_3, PortableServer::Servant a_4) :
    omniLocalOnlyCallDescriptor(lcfn, op, oplen, oneway),
    arg_0(a_0),
    arg_1(a_1),
    arg_2(a_2),
    arg_3(a_3),
    arg_4(a_4)  {}

  const PortableServer::ObjectId& arg_0;
  PortableServer::POA_ptr arg_1;
  const char* arg_2;
  PortableServer::ServantLocator::Cookie arg_3;
  PortableServer::Servant arg_4;
};


// Local call call-back function.
static void
_0RL_lcfn_3c165f58b5a16b59_90000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_3c165f58b5a16b59_80000000* tcd = (_0RL_cd_3c165f58b5a16b59_80000000*) cd;
  PortableServer::_impl_ServantLocator* impl = (PortableServer::_impl_ServantLocator*) svnt->_ptrToInterface(PortableServer::ServantLocator::_PD_repoId);
  impl->postinvoke(tcd->arg_0, tcd->arg_1, tcd->arg_2, tcd->arg_3, tcd->arg_4);
}


void PortableServer::_objref_ServantLocator::postinvoke(const PortableServer::ObjectId& oid, PortableServer::POA_ptr adapter, const char* operation, PortableServer::ServantLocator::Cookie the_cookie, PortableServer::Servant the_servant)
{
  _impl_ServantLocator* _s = _shortcut;
  if (_s) {
    if (!*_invalid) {
      _s->postinvoke(oid, adapter, operation, the_cookie, the_servant); return;
    }
    else {
      _enableShortcut(0,0);
      // drop through to normal invoke
    }
  }
  _0RL_cd_3c165f58b5a16b59_80000000 _call_desc(_0RL_lcfn_3c165f58b5a16b59_90000000, "postinvoke", 11, 0, oid, adapter, operation, the_cookie, the_servant);

  _invoke(_call_desc);
}


PortableServer::_pof_ServantLocator::~_pof_ServantLocator() {}


omniObjRef*
PortableServer::_pof_ServantLocator::newObjRef(omniIOR* ior, omniIdentity* id)
{
  return new ::PortableServer::_objref_ServantLocator(ior, id);
}


::CORBA::Boolean
PortableServer::_pof_ServantLocator::is_a(const char* id) const
{
  if (omni::ptrStrMatch(id, ::PortableServer::ServantLocator::_PD_repoId))
    return 1;
  if (omni::ptrStrMatch(id, PortableServer::ServantManager::_PD_repoId))
    return 1;

  return 0;
}


const PortableServer::_pof_ServantLocator _the_pof_PortableServer_mServantLocator;


PortableServer::_impl_ServantLocator::~_impl_ServantLocator() {}


::CORBA::Boolean
PortableServer::_impl_ServantLocator::_dispatch(omniCallHandle& _handle)
{
  return 0;
}


void*
PortableServer::_impl_ServantLocator::_ptrToInterface(const char* id)
{
  if (id == ::PortableServer::ServantLocator::_PD_repoId)
    return (::PortableServer::_impl_ServantLocator*) this;
  if (id == ::PortableServer::ServantManager::_PD_repoId)
    return (::PortableServer::_impl_ServantManager*) this;


  if (id == ::CORBA::Object::_PD_repoId)
    return (void*) 1;

  if (omni::strMatch(id, ::PortableServer::ServantLocator::_PD_repoId))
    return (::PortableServer::_impl_ServantLocator*) this;
  if (omni::strMatch(id, ::PortableServer::ServantManager::_PD_repoId))
    return (::PortableServer::_impl_ServantManager*) this;


  if (omni::strMatch(id, ::CORBA::Object::_PD_repoId))
    return (void*) 1;
  return 0;
}


const char*
PortableServer::_impl_ServantLocator::_mostDerivedRepoId()
{
  return ::PortableServer::ServantLocator::_PD_repoId;
}


POA_PortableServer::AdapterActivator::~AdapterActivator() {}


POA_PortableServer::ServantManager::~ServantManager() {}


POA_PortableServer::ServantActivator::~ServantActivator() {}


POA_PortableServer::ServantLocator::~ServantLocator() {}


