// -*- Mode: C++; -*-
//                            Package   : omniORB
// poa_defs.h                 Created on: 8/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2005-2012 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
// Description:
//
//    Hand-edited generated code for POA IDL.

#ifndef __OMNIPOA_H__
#error poa_defs.h should only be included by poa.h
#endif

#ifndef __OMNI_POA_DEFS_H__
#define __OMNI_POA_DEFS_H__


//////////////////////////////////////////////////////////////////////
/////////////////////////// ForwardRequest ///////////////////////////
//////////////////////////////////////////////////////////////////////

class ForwardRequest : public CORBA::UserException {
public:
  CORBA::Object_Member forward_reference;

  inline ForwardRequest() {
    pd_insertToAnyFn    = insertToAnyFn;
    pd_insertToAnyFnNCP = insertToAnyFnNCP;
  }
  ForwardRequest(const ForwardRequest&);
  ForwardRequest(::CORBA::Object_ptr i_forward_reference);
  ForwardRequest& operator=(const ForwardRequest&);
  virtual ~ForwardRequest();
  virtual void _raise() const;
  static ForwardRequest* _downcast(::CORBA::Exception*);
  static const ForwardRequest* _downcast(const ::CORBA::Exception*);
  static inline ForwardRequest* _narrow(::CORBA::Exception* _e) {
    return _downcast(_e);
  }
  
  void operator>>=(cdrStream&) const ;
  void operator<<=(cdrStream&) ;

  static _core_attr insertExceptionToAny    insertToAnyFn;
  static _core_attr insertExceptionToAnyNCP insertToAnyFnNCP;

  virtual ::CORBA::Exception* _NP_duplicate() const;

  static _core_attr const char* _PD_repoId;
  static _core_attr const char* _PD_typeId;

private:
  virtual const char* _NP_typeId() const;
  virtual const char* _NP_repoId(int*) const;
  virtual void _NP_marshal(cdrStream&) const;
};


_CORBA_MODULE_VAR _dyn_attr const ::CORBA::TypeCode_ptr _tc_ForwardRequest;

//////////////////////////////////////////////////////////////////////
////////////////////////// AdapterActivator //////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef __PortableServer_mAdapterActivator__
#define __PortableServer_mAdapterActivator__

class AdapterActivator;
typedef AdapterActivator* AdapterActivator_ptr;
typedef AdapterActivator_ptr AdapterActivatorRef;

class AdapterActivator_Helper {
public:
  typedef AdapterActivator_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static void marshalObjRef(_ptr_type, cdrStream&);
  static _ptr_type unmarshalObjRef(cdrStream&);
};

typedef _CORBA_ObjRef_Var<AdapterActivator, AdapterActivator_Helper> AdapterActivator_var;
typedef _CORBA_ObjRef_OUT_arg<AdapterActivator,AdapterActivator_Helper > AdapterActivator_out;

#endif


class AdapterActivator :
  public virtual CORBA::LocalObject
{
public:
  // Declarations for this interface type.
  typedef AdapterActivator_ptr _ptr_type;
  typedef AdapterActivator_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(::CORBA::Object_ptr);
  static _ptr_type _unchecked_narrow(::CORBA::Object_ptr);
  
  static _ptr_type _nil();

  static inline void _marshalObjRef(_ptr_type, cdrStream& s) {
    OMNIORB_THROW(MARSHAL, _OMNI_NS(MARSHAL_LocalObject),
                  (CORBA::CompletionStatus)s.completion());
  }

  static inline _ptr_type _unmarshalObjRef(cdrStream& s) {
    OMNIORB_THROW(MARSHAL, _OMNI_NS(MARSHAL_LocalObject),
                  (CORBA::CompletionStatus)s.completion());
  }

  static inline _ptr_type _fromObjRef(omniObjRef* o) {
    if (o)
      return (_ptr_type) o->_ptrToObjRef(_PD_repoId);
    else
      return _nil();
  }

  static _core_attr const char* _PD_repoId;

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();

  // Other IDL defined within this scope.

  // Operations declared in this local interface
  virtual CORBA::Boolean unknown_adapter(POA_ptr parent, const char* name) = 0;

private:
  virtual void* _ptrToObjRef(const char*);

protected:
  AdapterActivator();
  virtual ~AdapterActivator();
};


class _objref_AdapterActivator :
  public virtual AdapterActivator, public virtual omniObjRef
{
public:
  CORBA::Boolean unknown_adapter(POA_ptr parent, const char* name);

  inline _objref_AdapterActivator() { _PR_setobj(0); }  // nil
  _objref_AdapterActivator(omniIOR*, omniIdentity*);

protected:
  virtual ~_objref_AdapterActivator();

private:
  virtual void* _ptrToObjRef(const char*);

  _objref_AdapterActivator(const _objref_AdapterActivator&);
  _objref_AdapterActivator& operator = (const _objref_AdapterActivator&);
  // not implemented
};


class _pof_AdapterActivator : public _OMNI_NS(proxyObjectFactory) {
public:
  inline _pof_AdapterActivator() : _OMNI_NS(proxyObjectFactory)(AdapterActivator::_PD_repoId) {}
  virtual ~_pof_AdapterActivator();

  virtual omniObjRef* newObjRef(omniIOR*,omniIdentity*);
  virtual _CORBA_Boolean is_a(const char*) const;
};


class _impl_AdapterActivator :
  public virtual omniServant
{
public:
  virtual ~_impl_AdapterActivator();

  virtual CORBA::Boolean unknown_adapter(POA_ptr parent, const char* name) = 0;
  
public:  // Really protected, workaround for xlC
  virtual _CORBA_Boolean _dispatch(omniCallHandle&);

private:
  virtual void* _ptrToInterface(const char*);
  virtual const char* _mostDerivedRepoId();
};


_CORBA_MODULE_VAR _dyn_attr const ::CORBA::TypeCode_ptr _tc_AdapterActivator;

//////////////////////////////////////////////////////////////////////
/////////////////////////// ServantManager ///////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef __PortableServer_mServantManager__
#define __PortableServer_mServantManager__

class ServantManager;
typedef ServantManager* ServantManager_ptr;
typedef ServantManager_ptr ServantManagerRef;

class ServantManager_Helper {
public:
  typedef ServantManager_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static void marshalObjRef(_ptr_type, cdrStream&);
  static _ptr_type unmarshalObjRef(cdrStream&);
};

typedef _CORBA_ObjRef_Var<ServantManager, ServantManager_Helper> ServantManager_var;
typedef _CORBA_ObjRef_OUT_arg<ServantManager,ServantManager_Helper > ServantManager_out;

#endif


class ServantManager :
  public virtual CORBA::LocalObject
{
public:
  // Declarations for this interface type.
  typedef ServantManager_ptr _ptr_type;
  typedef ServantManager_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(::CORBA::Object_ptr);
  static _ptr_type _unchecked_narrow(::CORBA::Object_ptr);
  
  static _ptr_type _nil();

  static inline void _marshalObjRef(_ptr_type, cdrStream& s) {
    OMNIORB_THROW(MARSHAL, _OMNI_NS(MARSHAL_LocalObject),
                  (CORBA::CompletionStatus)s.completion());
  }

  static inline _ptr_type _unmarshalObjRef(cdrStream& s) {
    OMNIORB_THROW(MARSHAL, _OMNI_NS(MARSHAL_LocalObject),
                  (CORBA::CompletionStatus)s.completion());
  }

  static inline _ptr_type _fromObjRef(omniObjRef* o) {
    if (o)
      return (_ptr_type) o->_ptrToObjRef(_PD_repoId);
    else
      return _nil();
  }

  static _core_attr const char* _PD_repoId;

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();

  // Other IDL defined within this scope.

  // Operations declared in this local interface

private:
  virtual void* _ptrToObjRef(const char*);

protected:
  ServantManager();
  virtual ~ServantManager();
};


class _objref_ServantManager :
  public virtual ServantManager, public virtual omniObjRef
{
public:

  inline _objref_ServantManager() { _PR_setobj(0); }  // nil
  _objref_ServantManager(omniIOR*, omniIdentity*);

protected:
  virtual ~_objref_ServantManager();

private:
  virtual void* _ptrToObjRef(const char*);

  _objref_ServantManager(const _objref_ServantManager&);
  _objref_ServantManager& operator = (const _objref_ServantManager&);
  // not implemented
};


class _pof_ServantManager : public _OMNI_NS(proxyObjectFactory) {
public:
  inline _pof_ServantManager() : _OMNI_NS(proxyObjectFactory)(ServantManager::_PD_repoId) {}
  virtual ~_pof_ServantManager();

  virtual omniObjRef* newObjRef(omniIOR*,omniIdentity*);
  virtual _CORBA_Boolean is_a(const char*) const;
};


class _impl_ServantManager :
  public virtual omniServant
{
public:
  virtual ~_impl_ServantManager();

  
  
public:  // Really protected, workaround for xlC
  virtual _CORBA_Boolean _dispatch(omniCallHandle&);

private:
  virtual void* _ptrToInterface(const char*);
  virtual const char* _mostDerivedRepoId();
};


_CORBA_MODULE_VAR _dyn_attr const ::CORBA::TypeCode_ptr _tc_ServantManager;

//////////////////////////////////////////////////////////////////////
////////////////////////// ServantActivator //////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef __PortableServer_mServantActivator__
#define __PortableServer_mServantActivator__

class ServantActivator;
typedef ServantActivator* ServantActivator_ptr;
typedef ServantActivator_ptr ServantActivatorRef;

class ServantActivator_Helper {
public:
  typedef ServantActivator_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static void marshalObjRef(_ptr_type, cdrStream&);
  static _ptr_type unmarshalObjRef(cdrStream&);
};

typedef _CORBA_ObjRef_Var<ServantActivator, ServantActivator_Helper> ServantActivator_var;
typedef _CORBA_ObjRef_OUT_arg<ServantActivator,ServantActivator_Helper > ServantActivator_out;

#endif


class ServantActivator :
  public virtual ServantManager
{
public:
  // Declarations for this interface type.
  typedef ServantActivator_ptr _ptr_type;
  typedef ServantActivator_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(::CORBA::Object_ptr);
  static _ptr_type _unchecked_narrow(::CORBA::Object_ptr);
  
  static _ptr_type _nil();

  static inline void _marshalObjRef(_ptr_type, cdrStream& s) {
    OMNIORB_THROW(MARSHAL, _OMNI_NS(MARSHAL_LocalObject),
                  (CORBA::CompletionStatus)s.completion());
  }

  static inline _ptr_type _unmarshalObjRef(cdrStream& s) {
    OMNIORB_THROW(MARSHAL, _OMNI_NS(MARSHAL_LocalObject),
                  (CORBA::CompletionStatus)s.completion());
  }

  static _core_attr const char* _PD_repoId;

  // Other IDL defined within this scope.

  // Operations declared in this local interface
  virtual Servant incarnate(const ObjectId& oid, POA_ptr adapter) = 0;
  virtual void etherealize(const ObjectId& oid, POA_ptr adapter, Servant serv, CORBA::Boolean cleanup_in_progress, CORBA::Boolean remaining_activations) = 0;

private:
  virtual void* _ptrToObjRef(const char*);

protected:
  ServantActivator();
  virtual ~ServantActivator();
};

class _impl_ServantActivator;

class _objref_ServantActivator :
  public virtual ServantActivator, public virtual _objref_ServantManager
{
public:
  Servant incarnate(const ObjectId& oid, POA_ptr adapter);
  void etherealize(const ObjectId& oid, POA_ptr adapter, Servant serv, CORBA::Boolean cleanup_in_progress, CORBA::Boolean remaining_activations);

  inline _objref_ServantActivator() : _shortcut(0) { _PR_setobj(0); }  // nil
  _objref_ServantActivator(omniIOR*, omniIdentity*);

protected:
  virtual ~_objref_ServantActivator();

  virtual void _enableShortcut(omniServant*, const _CORBA_Boolean*);
  _impl_ServantActivator* _shortcut;
  const _CORBA_Boolean* _invalid;

private:
  virtual void* _ptrToObjRef(const char*);

  _objref_ServantActivator(const _objref_ServantActivator&);
  _objref_ServantActivator& operator = (const _objref_ServantActivator&);
  // not implemented
};


class _pof_ServantActivator : public _OMNI_NS(proxyObjectFactory) {
public:
  inline _pof_ServantActivator() : _OMNI_NS(proxyObjectFactory)(ServantActivator::_PD_repoId) {}
  virtual ~_pof_ServantActivator();

  virtual omniObjRef* newObjRef(omniIOR*,omniIdentity*);
  virtual _CORBA_Boolean is_a(const char*) const;
};


class _impl_ServantActivator :
  public virtual _impl_ServantManager
{
public:
  virtual ~_impl_ServantActivator();

  virtual Servant incarnate(const ObjectId& oid, POA_ptr adapter) = 0;
  virtual void etherealize(const ObjectId& oid, POA_ptr adapter, Servant serv, CORBA::Boolean cleanup_in_progress, CORBA::Boolean remaining_activations) = 0;

public:  // Really protected, workaround for xlC
  virtual _CORBA_Boolean _dispatch(omniCallHandle&);

private:
  virtual void* _ptrToInterface(const char*);
  virtual const char* _mostDerivedRepoId();
};


_CORBA_MODULE_VAR _dyn_attr const ::CORBA::TypeCode_ptr _tc_ServantActivator;

//////////////////////////////////////////////////////////////////////
/////////////////////////// ServantLocator ///////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef __PortableServer_mServantLocator__
#define __PortableServer_mServantLocator__

class ServantLocator;
typedef ServantLocator* ServantLocator_ptr;
typedef ServantLocator_ptr ServantLocatorRef;

class ServantLocator_Helper {
public:
  typedef ServantLocator_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static void marshalObjRef(_ptr_type, cdrStream&);
  static _ptr_type unmarshalObjRef(cdrStream&);
};

typedef _CORBA_ObjRef_Var<ServantLocator, ServantLocator_Helper> ServantLocator_var;
typedef _CORBA_ObjRef_OUT_arg<ServantLocator,ServantLocator_Helper > ServantLocator_out;

#endif


class ServantLocator :
  public virtual ServantManager
{
public:
  // Declarations for this interface type.
  typedef ServantLocator_ptr _ptr_type;
  typedef ServantLocator_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(CORBA::Object_ptr);
  static _ptr_type _unchecked_narrow(CORBA::Object_ptr);
  static _ptr_type _nil();

  static inline void _marshalObjRef(_ptr_type, cdrStream& s) {
    OMNIORB_THROW(MARSHAL, _OMNI_NS(MARSHAL_LocalObject),
                  (CORBA::CompletionStatus)s.completion());
  }

  static inline _ptr_type _unmarshalObjRef(cdrStream& s) {
    OMNIORB_THROW(MARSHAL, _OMNI_NS(MARSHAL_LocalObject),
                  (CORBA::CompletionStatus)s.completion());
  }

  static _core_attr const char* _PD_repoId;

  // Other IDL defined within this scope.

  static _dyn_attr const ::CORBA::TypeCode_ptr _tc_Cookie;
  typedef void* Cookie;

  // Operations declared in this local interface
  virtual Servant preinvoke(const ObjectId& oid, POA_ptr adapter, const char* operation, Cookie& the_cookie) = 0;
  virtual void postinvoke(const ObjectId& oid, POA_ptr adapter, const char* operation, Cookie the_cookie, Servant the_servant) = 0;

private:
  virtual void* _ptrToObjRef(const char*);

protected:
  ServantLocator();
  virtual ~ServantLocator();
};

class _impl_ServantLocator;

class _objref_ServantLocator :
  public virtual ServantLocator, public virtual _objref_ServantManager
{
public:
  Servant preinvoke(const ObjectId& oid, POA_ptr adapter, const char* operation, ServantLocator::Cookie& the_cookie);
  void postinvoke(const ObjectId& oid, POA_ptr adapter, const char* operation, ServantLocator::Cookie the_cookie, Servant the_servant);

  inline _objref_ServantLocator() : _shortcut(0) { _PR_setobj(0); }  // nil
  _objref_ServantLocator(omniIOR*, omniIdentity*);

protected:
  virtual ~_objref_ServantLocator();

  virtual void _enableShortcut(omniServant*, const _CORBA_Boolean*);
  _impl_ServantLocator* _shortcut;
  const _CORBA_Boolean* _invalid;

private:
  virtual void* _ptrToObjRef(const char*);

  _objref_ServantLocator(const _objref_ServantLocator&);
  _objref_ServantLocator& operator = (const _objref_ServantLocator&);
  // not implemented
};


class _pof_ServantLocator : public _OMNI_NS(proxyObjectFactory) {
public:
  inline _pof_ServantLocator() : _OMNI_NS(proxyObjectFactory)(ServantLocator::_PD_repoId) {}
  virtual ~_pof_ServantLocator();

  virtual omniObjRef* newObjRef(omniIOR*,omniIdentity*);
  virtual _CORBA_Boolean is_a(const char*) const;
};


class _impl_ServantLocator :
  public virtual _impl_ServantManager
{
public:
  virtual ~_impl_ServantLocator();

  virtual Servant preinvoke(const ObjectId& oid, POA_ptr adapter, const char* operation, ServantLocator::Cookie& the_cookie) = 0;
  virtual void postinvoke(const ObjectId& oid, POA_ptr adapter, const char* operation, ServantLocator::Cookie the_cookie, Servant the_servant) = 0;

protected:
  virtual _CORBA_Boolean _dispatch(omniCallHandle&);

private:
  virtual void* _ptrToInterface(const char*);
  virtual const char* _mostDerivedRepoId();
};


_CORBA_MODULE_VAR _dyn_attr const ::CORBA::TypeCode_ptr _tc_ServantLocator;


#endif  // __OMNI_POA_DEFS_H__
