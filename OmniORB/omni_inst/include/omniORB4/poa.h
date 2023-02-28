// -*- Mode: C++; -*-
//                            Package   : omniORB
// poa.h                      Created on: 2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2004-2012 Apasphere Ltd
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
//
// Description:
//

#ifndef __OMNIPOA_H__
#define __OMNIPOA_H__

#ifndef __OMNISERVANT_H__
#include <omniORB4/omniServant.h>
#endif
#ifndef __OMNIOBJREF_H__
#include <omniORB4/omniObjRef.h>
#endif
#include <omniORB4/minorCode.h>


OMNI_NAMESPACE_BEGIN(omni)

class IOP_S;
class omniOrbPOA;

OMNI_NAMESPACE_END(omni)


_CORBA_MODULE PortableServer
_CORBA_MODULE_BEG

  //////////////////////////////////////////////////////////////////////
  //////////////////////// Forward Declarations ////////////////////////
  //////////////////////////////////////////////////////////////////////

  class POA;
  typedef POA* POA_ptr;
  typedef POA_ptr POARef;

  class POA_Helper {
  public:
    typedef POA_ptr _ptr_type;
 
    static _ptr_type _nil();
    static _CORBA_Boolean is_nil(_ptr_type);
    static void release(_ptr_type);
    static void duplicate(_ptr_type);
    static inline void marshalObjRef(_ptr_type, cdrStream&) {}
    static inline _ptr_type unmarshalObjRef(cdrStream&) { return 0; }
  };

  class ServantBase;
  typedef ServantBase* Servant;


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// POAList //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class POAList_var;
 
  class POAList : public _CORBA_Unbounded_Sequence_ObjRef<POA, _CORBA_ObjRef_Element<POA, POA_Helper>, POA_Helper> {
  public:
    typedef POAList_var _var_type;
    inline POAList() {}
    inline POAList(const POAList& s)
      : _CORBA_Unbounded_Sequence_ObjRef<POA, _CORBA_ObjRef_Element<POA, POA_Helper>, POA_Helper>(s) {}
    inline POAList(_CORBA_ULong _max)
      : _CORBA_Unbounded_Sequence_ObjRef<POA, _CORBA_ObjRef_Element<POA, POA_Helper>, POA_Helper>(_max) {}
    inline POAList(_CORBA_ULong _max, _CORBA_ULong _len, POA_ptr* _val, _CORBA_Boolean _rel=0)
      : _CORBA_Unbounded_Sequence_ObjRef<POA, _CORBA_ObjRef_Element<POA, POA_Helper>, POA_Helper>(_max, _len, _val, _rel) {}
    inline POAList& operator = (const POAList& s) {
      _CORBA_Unbounded_Sequence_ObjRef<POA, _CORBA_ObjRef_Element<POA, POA_Helper>, POA_Helper>::operator=(s);
      return *this;
    }
  };


  class POAList_var {
  public:
    typedef POAList T;
    typedef POAList_var T_var;
 
    inline POAList_var() : pd_seq(0) {}
    inline POAList_var(T* s) : pd_seq(s) {}
    inline POAList_var(const T_var& s) {
      if( s.pd_seq )  pd_seq = new T(*s.pd_seq);
      else             pd_seq = 0;
    }
    inline ~POAList_var() { if( pd_seq )  delete pd_seq; }
 
    inline T_var& operator = (T* s) {
      if( pd_seq )  delete pd_seq;
      pd_seq = s;
      return *this;
    }
    inline T_var& operator = (const T_var& s) {
      if( s.pd_seq ) {
        if( !pd_seq )  pd_seq = new T;
        *pd_seq = *s.pd_seq;
      } else if( pd_seq ) {
        delete pd_seq;
        pd_seq = 0;
      }
      return *this;
    }
 
    inline _CORBA_ObjRef_Element<POA, POA_Helper> operator [] (_CORBA_ULong s) {
      return (*pd_seq)[s];
    }
    inline T* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
    inline operator T& () const { return *pd_seq; }
#else
    inline operator const T& () const { return *pd_seq; }
    inline operator T& () { return *pd_seq; }
#endif
 
    inline const T& in() const { return *pd_seq; }
    inline T&       inout()    { return *pd_seq; }
    inline T*&      out() {
      if( pd_seq ) { delete pd_seq; pd_seq = 0; }
      return pd_seq;
    }
    inline T* _retn() { T* tmp = pd_seq; pd_seq = 0; return tmp; }
 
  private:
    T* pd_seq;
  };

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// ObjectId //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ObjectId_var;

  class ObjectId : public _CORBA_Unbounded_Sequence_Octet {
  public:
    typedef ObjectId_var _var_type;
    inline ObjectId() {}
    inline ObjectId(const ObjectId& seq)
      : _CORBA_Unbounded_Sequence_Octet(seq) {}
    inline ObjectId(CORBA::ULong max)
      : _CORBA_Unbounded_Sequence_Octet(max) {}
    inline ObjectId(CORBA::ULong max, CORBA::ULong len, CORBA::Octet* val, CORBA::Boolean rel=0)
      : _CORBA_Unbounded_Sequence_Octet(max, len, val, rel) {}
    inline ObjectId& operator = (const ObjectId& seq) {
      _CORBA_Unbounded_Sequence_Octet::operator=(seq);
      return *this;
    }
  };

  class ObjectId_out;

  class ObjectId_var {
  public:
    typedef ObjectId T;
    typedef ObjectId_var T_var;

    inline ObjectId_var() : pd_seq(0) {}
    inline ObjectId_var(T* s) : pd_seq(s) {}
    inline ObjectId_var(const T_var& sv) {
      if( sv.pd_seq ) {
        pd_seq = new T;
        *pd_seq = *sv.pd_seq;
      } else
        pd_seq = 0;
    }
    inline ~ObjectId_var() { if( pd_seq ) delete pd_seq; }

    inline T_var& operator = (T* s) {
      if( pd_seq )  delete pd_seq;
      pd_seq = s;
      return *this;
    }
    inline T_var& operator = (const T_var& sv) {
      if( sv.pd_seq ) {
        if( !pd_seq )  pd_seq = new T;
        *pd_seq = *sv.pd_seq;
      } else if( pd_seq ) {
        delete pd_seq;
        pd_seq = 0;
      }
      return *this;
    }

    inline CORBA::Octet& operator [] (_CORBA_ULong i) {   return (*pd_seq)[i]; }
    inline T* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
    inline operator T& () const { return *pd_seq; }
#else
    inline operator const T& () const { return *pd_seq; }
    inline operator T& () { return *pd_seq; }
#endif

    inline const T& in() const { return *pd_seq; }
    inline T& inout() { return *pd_seq; }
    inline T*& out() { if (pd_seq) { delete pd_seq; pd_seq = 0; } return pd_seq; }
    inline T* _retn() { T* tmp = pd_seq; pd_seq = 0; return tmp; }

    friend class ObjectId_out;

  private:
    T* pd_seq;
  };

  class ObjectId_out {
  public:
    typedef ObjectId T;
    typedef ObjectId_var T_var;

    inline ObjectId_out(T*& s) : _data(s) { _data = 0; }
    inline ObjectId_out(T_var& sv)
      : _data(sv.pd_seq) { sv = (T*) 0; }
    inline ObjectId_out(const ObjectId_out& s) : _data(s._data) { }
    inline ObjectId_out& operator=(const ObjectId_out& s) { _data = s._data; return *this; }
    inline ObjectId_out& operator=(T* s) { _data = s; return *this; }
    inline operator T*&() { return _data; }
    inline T*& ptr() { return _data; }
    inline T* operator->() { return _data; }
    inline CORBA::Octet& operator [] (_CORBA_ULong i) {   return (*_data)[i]; }
    T*& _data;

  private:
    ObjectId_out();
    ObjectId_out operator=( const T_var&);
  };

  //////////////////////////////////////////////////////////////////////
  ////////////////////////// Policy Interfaces /////////////////////////
  //////////////////////////////////////////////////////////////////////

  _CORBA_MODULE_VARINT const CORBA::ULong
    THREAD_POLICY_ID _init_in_decl_( = 16 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    LIFESPAN_POLICY_ID _init_in_decl_( = 17 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    ID_UNIQUENESS_POLICY_ID _init_in_decl_( = 18 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    ID_ASSIGNMENT_POLICY_ID _init_in_decl_( = 19 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    IMPLICIT_ACTIVATION_POLICY_ID _init_in_decl_( = 20 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    SERVANT_RETENTION_POLICY_ID _init_in_decl_( = 21 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    REQUEST_PROCESSING_POLICY_ID _init_in_decl_( = 22 );


#include <omniORB4/poa_enums_defs.hh>


  OMNIORB_DECLARE_POLICY_OBJECT(ThreadPolicy, THREAD_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(LifespanPolicy, LIFESPAN_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(IdUniquenessPolicy, ID_UNIQUENESS_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(IdAssignmentPolicy, ID_ASSIGNMENT_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(ImplicitActivationPolicy,
				IMPLICIT_ACTIVATION_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(ServantRetentionPolicy,
				SERVANT_RETENTION_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(RequestProcessingPolicy,
				REQUEST_PROCESSING_POLICY_ID)

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// POAManager /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class POAManager;
  typedef POAManager* POAManager_ptr;
  typedef POAManager_ptr POAManagerRef;

  class POAManager : public CORBA::Object {
  public:
    OMNIORB_DECLARE_USER_EXCEPTION(AdapterInactive, _core_attr)

    virtual void activate() = 0;
    virtual void hold_requests(CORBA::Boolean wait_for_completion) = 0;
    virtual void discard_requests(CORBA::Boolean wait_for_completion) = 0;
    virtual void deactivate(CORBA::Boolean etherealize_objects,
			    CORBA::Boolean wait_for_completion) = 0;

    enum State { HOLDING, ACTIVE, DISCARDING, INACTIVE };

    virtual State get_state() = 0;


    // CORBA Object members
    static POAManager_ptr _duplicate(POAManager_ptr);
    static POAManager_ptr _narrow(CORBA::Object_ptr);
    static POAManager_ptr _nil();

    // omniORB internal.
    static _core_attr const char* _PD_repoId;

  protected:
    inline POAManager(int is_nil = 0) {
      if (is_nil)
	_PR_setobj((omniObjRef*)0);
      else
	_PR_setobj((omniObjRef*)1);
    }
    virtual ~POAManager();

  private:
    POAManager(const POAManager&);
    POAManager& operator=(const POAManager&);
  };

  typedef _CORBA_PseudoObj_Var<POAManager> POAManager_var;

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////////// POA ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class AdapterActivator;
  class ServantManager;


  class POA : public CORBA::Object {
  public:

    typedef POA*                      _ptr_type;
    typedef _CORBA_PseudoObj_Var<POA> _var_type;

    // Exceptions

    OMNIORB_DECLARE_USER_EXCEPTION(AdapterAlreadyExists, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(AdapterInactive, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(AdapterNonExistent, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(NoServant, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(ObjectAlreadyActive, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(ObjectNotActive, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(ServantAlreadyActive, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(ServantNotActive, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(WrongAdapter, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(WrongPolicy, _core_attr)

    class InvalidPolicy : public CORBA::UserException {
    public:
      CORBA::UShort index;

      inline InvalidPolicy() {
        pd_insertToAnyFn    = insertToAnyFn;
        pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      InvalidPolicy(const InvalidPolicy&);
      InvalidPolicy(CORBA::UShort  i_index);
      InvalidPolicy& operator=(const InvalidPolicy&);
      virtual ~InvalidPolicy();
      virtual void _raise() const;
      static InvalidPolicy* _downcast(CORBA::Exception*);
      static const InvalidPolicy* _downcast(const CORBA::Exception*);
      static inline InvalidPolicy* _narrow(CORBA::Exception* e) {
	return _downcast(e);
      }

      void operator>>=(cdrStream&) const;
      void operator<<=(cdrStream&);

      static _core_attr insertExceptionToAny    insertToAnyFn;
      static _core_attr insertExceptionToAnyNCP insertToAnyFnNCP;

      static _core_attr const char* _PD_repoId;

    private:
      virtual CORBA::Exception* _NP_duplicate() const;
      virtual const char* _NP_typeId() const;
      virtual const char* _NP_repoId(int*) const;
      virtual void _NP_marshal(cdrStream&) const;
    };
    static _dyn_attr const CORBA::TypeCode_ptr _tc_InvalidPolicy;


    // POA creation and destruction

    virtual POA_ptr create_POA(const char* adapter_name,
			       POAManager_ptr a_POAManager,
			       const CORBA::PolicyList& policies) = 0;

    virtual POA_ptr find_POA(const char* adapter_name,
			     CORBA::Boolean activate_it) = 0;

    virtual void destroy(CORBA::Boolean etherealize_objects,
			 CORBA::Boolean wait_for_completion) = 0;

    // Factories for Policy objects

    virtual ThreadPolicy_ptr
      create_thread_policy(ThreadPolicyValue v);
    virtual LifespanPolicy_ptr
      create_lifespan_policy(LifespanPolicyValue v);
    virtual IdUniquenessPolicy_ptr
      create_id_uniqueness_policy(IdUniquenessPolicyValue v);
    virtual IdAssignmentPolicy_ptr
      create_id_assignment_policy(IdAssignmentPolicyValue v);
    virtual ImplicitActivationPolicy_ptr
      create_implicit_activation_policy(ImplicitActivationPolicyValue v);
    virtual ServantRetentionPolicy_ptr
      create_servant_retention_policy(ServantRetentionPolicyValue v);
    virtual RequestProcessingPolicy_ptr
      create_request_processing_policy(RequestProcessingPolicyValue v);

    // POA attributes

    virtual char* the_name() = 0;
    virtual POA_ptr the_parent() = 0;
    virtual POAList* the_children() = 0;
    virtual POAManager_ptr the_POAManager() = 0;
    virtual AdapterActivator* the_activator() = 0;
    virtual void the_activator(AdapterActivator* aa) = 0;

    // Servant Manager registration

    virtual ServantManager* get_servant_manager() = 0;
    virtual void set_servant_manager(ServantManager* imgr) = 0;

    // Operations for the USE_DEFAULT_SERVANT policy

    virtual Servant get_servant() = 0;
    virtual void set_servant(Servant p_servant) = 0;

    // Object activation and deactivation

    virtual ObjectId* activate_object(Servant p_servant) = 0;
    virtual void activate_object_with_id(const ObjectId& id,
					 Servant p_servant) = 0;
    virtual void deactivate_object(const ObjectId& oid) = 0;

    // Reference creation operations

    virtual CORBA::Object_ptr create_reference(const char* intf) = 0;
    virtual CORBA::Object_ptr create_reference_with_id(const ObjectId& oid,
						       const char* intf) = 0;

    // Identity mapping operations

    virtual ObjectId* servant_to_id(Servant p_servant) = 0;
    virtual CORBA::Object_ptr servant_to_reference(Servant p_servant) = 0;
    virtual Servant reference_to_servant(CORBA::Object_ptr reference) = 0;
    virtual ObjectId* reference_to_id(CORBA::Object_ptr reference) = 0;
    virtual Servant id_to_servant(const ObjectId& oid) = 0;
    virtual CORBA::Object_ptr id_to_reference(const ObjectId& oid) = 0;

    virtual CORBA::OctetSeq* id() = 0;

    //////////////////////////
    // CORBA Object members //
    //////////////////////////

    static POA_ptr _duplicate(POA_ptr);
    static POA_ptr _narrow(CORBA::Object_ptr);
    static POA_ptr _nil();

    // omniORB internal.
    static _core_attr const char* _PD_repoId;
    static POA_ptr _the_root_poa();

  protected:
    inline POA(int nil) {
      if (nil)
	_PR_setobj((omniObjRef*)0);
      else
	_PR_setobj((omniObjRef*)1);
    }
    virtual ~POA();

  private:
    POA(const POA&);
    POA& operator=(const POA&);
  };

  typedef POA::_var_type                        POA_var;
  typedef _CORBA_PseudoObj_Member<POA, POA_var> POA_member;

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// Current //////////////////////////////
  //////////////////////////////////////////////////////////////////////
  class Current;
  typedef class Current* Current_ptr;
  typedef Current_ptr CurrentRef;

  class Current : public CORBA::Current {
  public:
    typedef Current*                      _ptr_type;
    typedef _CORBA_PseudoObj_Var<Current> _var_type;

    OMNIORB_DECLARE_USER_EXCEPTION(NoContext, _core_attr)

    virtual POA_ptr get_POA() = 0;
    virtual ObjectId* get_object_id() = 0;
    virtual CORBA::Object_ptr get_reference() = 0;
    virtual Servant get_servant() = 0;

    // CORBA Object members
    static Current_ptr _duplicate(Current_ptr);
    static Current_ptr _narrow(CORBA::Object_ptr);
    static Current_ptr _nil();

    // omniORB internal.
    static _core_attr const char* _PD_repoId;
  
  protected:
    inline Current(int is_nil = 0) : CORBA::Current(is_nil) {}
    virtual ~Current();

  private:
    Current(const Current&);
    Current& operator=(const Current&);
  };

  typedef Current::_var_type                            Current_var;
  typedef _CORBA_PseudoObj_Member<Current, Current_var> Current_member;

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// ServantBase ////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ServantBase : public virtual omniServant {
  public:
    virtual ~ServantBase();

    virtual POA_ptr _default_POA();
    // May be overriden by user-defined servants to specify a
    // default POA for a particular servant.
    //  Returns the root POA by default.

    virtual CORBA::_objref_InterfaceDef* _get_interface();
    // May be overriden by user-defined servants.  Should
    // return a reference to a CORBA::InterfaceDef object
    // representing the most-derived interface of this object.
    //  The default version attempts to lookup the interface
    // in the interface repository returned by
    // CORBA::ORB::resolve_initial_references().

    //virtual CORBA::Boolean _is_a(const char* logical_type_id);
    //virtual CORBA::Boolean _non_existent();
    // These are provided by omniServant.  May be overridden
    // by application defined servants.

    virtual void _add_ref();
    virtual void _remove_ref();

    virtual CORBA::ULong _refcount_value();

  protected:
    inline ServantBase() : _pd_refCount(1) {}
    inline ServantBase(const ServantBase& _v) :
      omniServant (_v), _pd_refCount (1) {}

    inline ServantBase& operator = (const ServantBase&) { return *this; }

    void* _do_this(const char* repoId);

  private:
    virtual omniObjRef* _do_get_interface();
    virtual void* _downcast();
    // Overrides omniServant.

    omni_refcount _pd_refCount;
  };

  //////////////////////////////////////////////////////////////////////
  ///////////////////////// RefCountServantBase ////////////////////////
  //////////////////////////////////////////////////////////////////////

  struct RefCountServantBase {};
  // Deprecated.


  //////////////////////////////////////////////////////////////////////
  ///////////////////////// ValueRefCountBase //////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ValueRefCountBase
    : public virtual CORBA::ValueBase,
      public virtual ServantBase
  {
  public:
    ValueRefCountBase() {}

    virtual void _add_ref();
    virtual void _remove_ref();
    virtual CORBA::ULong _refcount_value();

  protected:
    virtual ~ValueRefCountBase();
    ValueRefCountBase(const ValueRefCountBase&);

  private:
    ValueRefCountBase& operator= (const ValueRefCountBase&);
    // Not implemented
  };


  //////////////////////////////////////////////////////////////////////
  /////////////////////////// Servant_var //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  template <class T>
  class Servant_var {
  public:
    inline Servant_var() : pd_data(0) {}
    inline Servant_var(T* p) : pd_data(p) {}
    inline Servant_var(const Servant_var& v) : pd_data(v.pd_data)
      { if (pd_data) pd_data->_add_ref(); }

    inline ~Servant_var() { if (pd_data) pd_data->_remove_ref(); }

    inline Servant_var& operator= (T* p) {
      if (pd_data != p) {
	if (pd_data)
          pd_data->_remove_ref();
	pd_data = p;
      }
      return *this;
    }
    inline Servant_var& operator= (const Servant_var& v) {
      if (v.pd_data != pd_data) {
	if (pd_data)
          pd_data->_remove_ref();

        pd_data = v.pd_data;
        if (pd_data)
          pd_data->_add_ref();
      }
      return *this;
    }

    inline operator T* () const    { return pd_data; }
    inline T* operator -> () const { return pd_data; }

    inline T* in() const { return pd_data; }
    inline T*& inout() { return pd_data; }
    inline T*& out() {
      if( pd_data ) { pd_data->_remove_ref();  pd_data = 0; }
      return pd_data;
    }
    inline T* _retn() { T* tmp = pd_data;  pd_data = 0;  return tmp; }

  private:
    T* pd_data;
  };


  //////////////////////////////////////////////////////////////////////
  /////////////////////////// ServantBase_var //////////////////////////
  //////////////////////////////////////////////////////////////////////

  typedef Servant_var<ServantBase> ServantBase_var;


  //////////////////////////////////////////////////////////////////////
  //////////////////////// DynamicImplementation ///////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynamicImplementation : public virtual ServantBase {
  public:
    virtual ~DynamicImplementation();

    CORBA::Object_ptr _this();

    virtual void invoke(CORBA::ServerRequest_ptr request) = 0;
    virtual char* _primary_interface(const ObjectId& oid, POA_ptr poa) = 0;

    //virtual CORBA::_objref_InterfaceDef* _get_interface();
    virtual CORBA::Boolean _is_a(const char* logical_type_id);
    // These default implementations use _primary_interface(),
    // but may be overridden by subclasses.

    // omniORB internal.
    virtual _CORBA_Boolean _dispatch(omniCallHandle&);

  private:
    virtual omniObjRef* _do_get_interface();
  };

  //////////////////////////////////////////////////////////////////////
  //////////////////////// C++ Mapping Specific ////////////////////////
  //////////////////////////////////////////////////////////////////////

  _CORBA_MODULE_FN char* ObjectId_to_string(const ObjectId& id);
  _CORBA_MODULE_FN _CORBA_WChar* ObjectId_to_wstring(const ObjectId& id);

  _CORBA_MODULE_FN ObjectId* string_to_ObjectId(const char* s);
  _CORBA_MODULE_FN ObjectId* wstring_to_ObjectId(const _CORBA_WChar* s);


#include <omniORB4/poa_defs.h>
// This brings in the declarations for:
//  AdapterActivator
//  ServantManager
//  ServantActivator
//  ServantLocator


_CORBA_MODULE_END  // PortableServer


#include <omniORB4/poa_enums_operators.hh>

#include <omniORB4/poa_poa.h>
// This brings in the skeletons for:
//  AdapterActivator
//  ServantActivator
//  ServantLocator


#endif  // __OMNIPOA_H__
