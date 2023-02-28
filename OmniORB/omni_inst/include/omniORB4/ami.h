// -*- Mode: C++; -*-
//                            Package   : omniORB
// ami.h                      Created on: 2012-02-06
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2012 Apasphere Ltd
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
//    AMI support

#ifndef _OMNIORB_AMI_H_
#define _OMNIORB_AMI_H_

#include <omniORB4/messaging.hh>
#include <omniORB4/callDescriptor.h>


#ifdef _dyn_attr
# error "A local CPP macro _dyn_attr has already been defined."
#endif

#if defined(_OMNIORB_DYNAMIC_LIBRARY)
#  define _dyn_attr
#else
#  define _dyn_attr _OMNIORB_NTDLL_IMPORT
#endif


_CORBA_MODULE omniAMI
_CORBA_MODULE_BEG


//
// Implementation of standard Messaging::ExceptionHolder valuetype

class ExceptionHolder
  : public virtual ::Messaging::ExceptionHolder,
    public virtual ::CORBA::DefaultValueRefCountBase
{
public:
  inline ExceptionHolder(omniAsyncCallDescriptor* cd)
    : pd_cd(cd)
  {}

  virtual ~ExceptionHolder();

  virtual void raise_exception();

private:
  omniAsyncCallDescriptor* pd_cd;

  // Not implemented
  ExceptionHolder(const ExceptionHolder&);
  ExceptionHolder& operator=(const ExceptionHolder&);
};


//
// Mixin class implementing base Messaging::Poller methods

class PollerImpl
  : public virtual ::Messaging::Poller,
    public virtual ::CORBA::DefaultValueRefCountBase
{
public:
  PollerImpl(omniAsyncCallDescriptor* _cd)
    : _pd_cd(_cd), _pd_is_from_poller(0), _pd_retrieved(0)
  {}

  ~PollerImpl();

  // Standard interface

  ::CORBA::Boolean         is_ready(::CORBA::ULong timeout);
  ::CORBA::PollableSet_ptr create_pollable_set();
  ::CORBA::Object_ptr      operation_target();
  char*                    operation_name();

  ::Messaging::ReplyHandler_ptr associated_handler();
  void associated_handler(::Messaging::ReplyHandler_ptr v);

  ::CORBA::Boolean         is_from_poller();

  // omniORB internal
  inline omniAsyncCallDescriptor* _PR_cd()        { return _pd_cd; }
  inline ::CORBA::Boolean         _PR_retrieved() { return _pd_retrieved; }

  static _dyn_attr const char* _PD_repoId;

protected:
  void _wrongOperation();
  void _checkResult(const char* op, ::CORBA::ULong timeout);

  omniAsyncCallDescriptor* _pd_cd;
  ::CORBA::Boolean         _pd_is_from_poller;
  ::CORBA::Boolean         _pd_retrieved;
};


//
// DIIPollable implementation

class DIIPollableImpl
  : public virtual ::CORBA::DIIPollable
{
public:
  DIIPollableImpl()
    : pd_cond(&omniAsyncCallDescriptor::sd_lock, "DIIPollableImpl::pd_cond"),
      pd_set_cond(0),
      pd_ready(0)
  {}

  ~DIIPollableImpl();

  void           _add_ref();
  void           _remove_ref();
  ::CORBA::ULong _refcount_value();

  // Standard interface
  ::CORBA::Boolean         is_ready(::CORBA::ULong timeout);
  ::CORBA::PollableSet_ptr create_pollable_set();

  inline ::CORBA::Boolean _addToSet(omni_tracedcondition* set_cond)
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(omniAsyncCallDescriptor::sd_lock, 1);

    if (pd_set_cond)
      return 0;

    pd_set_cond = set_cond;
    return 1;
  }

  inline ::CORBA::Boolean _remFromSet(omni_tracedcondition* set_cond)
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(omniAsyncCallDescriptor::sd_lock, 1);

    if (pd_set_cond != set_cond)
      return 0;
    
    pd_set_cond = 0;
    return 1;
  }

  inline void _replyReady()
  {
    omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);
    ++pd_ready;
    pd_cond.broadcast();
    if (pd_set_cond)
      pd_set_cond->signal();
  }

  inline void _replyCollected()
  {
    omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);
    --pd_ready;
  }

  inline ::CORBA::Boolean _lockedIsReady()
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(omniAsyncCallDescriptor::sd_lock, 1);
    return pd_ready ? 1 : 0;
  }

  static _dyn_attr DIIPollableImpl _PD_instance;

private:
  omni_tracedcondition  pd_cond;
  omni_tracedcondition* pd_set_cond;
  CORBA::ULong          pd_ready;
};


//
// PollableSet implementation

typedef _CORBA_Value_Element<
  PollerImpl, ::Messaging::Poller_Helper> PollerImplElmt;

typedef _CORBA_Unbounded_Sequence_Value<
  PollerImpl, PollerImplElmt, ::Messaging::Poller_Helper> PollerImplSeq;


class PollableSetImpl
  : public virtual ::CORBA::PollableSet
{
public:
  PollableSetImpl(PollerImpl* poller);
  PollableSetImpl(DIIPollableImpl* dii_pollable);

  ~PollableSetImpl();

  // Standard interface

  ::CORBA::DIIPollable* create_dii_pollable();
  void                  add_pollable(::CORBA::Pollable* potential);
  ::CORBA::Pollable*    get_ready_pollable(::CORBA::ULong timeout);
  void                  remove(::CORBA::Pollable* potential);
  ::CORBA::UShort       number_left();

  void _add_ref();
  void _remove_ref();

private:
  omni_tracedcondition pd_cond;
  PollerImplSeq        pd_ami_pollers;
  DIIPollableImpl*     pd_dii_pollable;
  omni_refcount        pd_ref_count;

  ::CORBA::Pollable*   getAndRemoveReadyPollable();

  // Not implemented
  PollableSetImpl(const PollableSetImpl&);
  PollableSetImpl& operator=(const PollableSetImpl&);
};


_CORBA_MODULE_END  // omniAMI



#endif // _OMNIORB_AMI_H_
