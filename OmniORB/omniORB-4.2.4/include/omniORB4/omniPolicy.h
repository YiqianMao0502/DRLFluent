// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniPolicy.h               Created on: 2001/11/07
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

#ifndef __OMNIPOLICY_H__
#define __OMNIPOLICY_H__

#include <omniORB4/giopEndpoint.h>


_CORBA_MODULE omniPolicy

_CORBA_MODULE_BEG

//
// Local shortcut

typedef CORBA::UShort LocalShortcutPolicyValue;

_CORBA_MODULE_VARINT
const LocalShortcutPolicyValue LOCAL_CALLS_THROUGH_POA _init_in_decl_( = 0 );

_CORBA_MODULE_VARINT
const LocalShortcutPolicyValue LOCAL_CALLS_SHORTCUT    _init_in_decl_( = 1 );

_CORBA_MODULE_VARINT
const CORBA::PolicyType LOCAL_SHORTCUT_POLICY_TYPE
                                              _init_in_decl_( = 0x41545401 );

OMNIORB_DECLARE_POLICY_OBJECT(LocalShortcutPolicy, 0x41545401)


_CORBA_MODULE_FN LocalShortcutPolicy_ptr
create_local_shortcut_policy(LocalShortcutPolicyValue v);


//
// EndPoint publishing

typedef CORBA::StringSeq EndPointPublishPolicyValue;

_CORBA_MODULE_VARINT
const CORBA::PolicyType ENDPOINT_PUBLISH_POLICY_TYPE
                                              _init_in_decl_( = 0x41545402 );


// Cannot use the OMNIORB_DECLARE_POLICY_OBJECT macro because the
// value is a sequence

class EndPointPublishPolicy;
typedef EndPointPublishPolicy* EndPointPublishPolicy_ptr;
typedef EndPointPublishPolicy_ptr EndPointPublishPolicyRef;

class EndPointPublishPolicy : public CORBA::Policy
{
public:
  EndPointPublishPolicy(const EndPointPublishPolicyValue& value_);
  inline EndPointPublishPolicy() : pd_eps(0) {}
  virtual ~EndPointPublishPolicy();

  virtual CORBA::Policy_ptr copy();
  virtual const EndPointPublishPolicyValue& value() { return pd_value; }

  virtual void* _ptrToObjRef(const char* repoId);

  _OMNI_NS(IORPublish)* getEPs();

  static EndPointPublishPolicy_ptr _duplicate(EndPointPublishPolicy_ptr p);
  static EndPointPublishPolicy_ptr _narrow(CORBA::Object_ptr p);
  static EndPointPublishPolicy_ptr _nil();

  static _core_attr const char* _PD_repoId;

private:
  EndPointPublishPolicyValue pd_value;
  _OMNI_NS(IORPublish)*      pd_eps;
};

typedef _CORBA_PseudoObj_Var<EndPointPublishPolicy> EndPointPublishPolicy_var;


_CORBA_MODULE_FN EndPointPublishPolicy_ptr
create_endpoint_publish_policy(const EndPointPublishPolicyValue& v);


_CORBA_MODULE_END


#endif // __OMNIPOLICY_H__
