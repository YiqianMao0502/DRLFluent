// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA.h                    Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2011 Apasphere Ltd
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
//    A complete set of C++ definitions for the CORBA module.
//

#ifndef __CORBA_H__
#define __CORBA_H__

#ifndef __CORBA_H_EXTERNAL_GUARD__
# define __CORBA_H_EXTERNAL_GUARD__
#endif

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#ifdef _dyn_attr
# error "A local CPP macro _dyn_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY) && defined(_OMNIORB_DYNAMIC_LIBRARY)
# error "_OMNIORB_LIBRARY and _OMNIORB_DYNAMIC_LIBRARY are both defined."
#endif

#if    defined(_OMNIORB_LIBRARY)
#         define _core_attr
#         define _dyn_attr  _OMNIORB_NTDLL_IMPORT
#elif  defined(_OMNIORB_DYNAMIC_LIBRARY)
#         define _core_attr _OMNIORB_NTDLL_IMPORT
#         define _dyn_attr
#else
#         define _core_attr _OMNIORB_NTDLL_IMPORT
#         define _dyn_attr  _OMNIORB_NTDLL_IMPORT
#endif

#ifndef USE_omniORB_logStream
# define USE_omniORB_logStream
#endif

#include <omniORB4/omniInternal.h>

// Forward declarations.
class omniOrbBoaServant;
class _omni_ValueFactoryManager;
struct _omni_ValueIds;
class cdrAnyMemoryStream;

_CORBA_MODULE CORBA

_CORBA_MODULE_BEG

# define INSIDE_OMNIORB_CORBA_MODULE

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// Forward decls /////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Object;
  class Object_var;
  class Object_OUT_arg;
  typedef Object* Object_ptr;
  typedef Object_ptr ObjectRef;

  class TypeCode;
  typedef TypeCode* TypeCode_ptr;
  typedef TypeCode_ptr TypeCodeRef;

  class Fixed;
  class Any;
  class ValueBase;

  class AbstractBase;
  typedef AbstractBase* AbstractBase_ptr;

  //////////////////////////////////////////////////////////////////////
  ////////////////////// Definitions ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////

# include "CORBA_primitive_types.h"
# include "CORBA_String.h"
# include "CORBA_Exception.h"
# include "CORBA_Environment.h"
# include "CORBA_NamedValue.h"
# include "CORBA_Context.h"
# include "CORBA_Principal.h"
# include "CORBA_ExceptionList.h"
# include "CORBA_TypeCode_member.h"
# include "CORBA_Object.h"
# include "CORBA_Object_vartypes.h"
# include "CORBA_LocalObject.h"
# include "CORBA_LocalObject_vartypes.h"
# include "CORBA_Any.h"
# include "CORBA_Any_vartypes.h"

  //////////////////////////////////////////////////////////////////////
  ////////////////////// Generated from corbaidl.idl ///////////////////
  //////////////////////////////////////////////////////////////////////

# include <omniORB4/corbaidl_defs.hh>

  //////////////////////////////////////////////////////////////////////
  ////////////////////// More Definitions //////////////////////////////
  //////////////////////////////////////////////////////////////////////

# include "CORBA_TypeCode.h"
# include "CORBA_Request.h"
# include "CORBA_ServerRequest.h"
# include "CORBA_ValueBase.h"
# include "CORBA_ValueBase_vartypes.h"
# include "CORBA_AbstractBase.h"
# include "CORBA_UnknownUserException.h"
# include "CORBA_Policy.h"
# include "CORBA_Current.h"
# include "CORBA_DomainManager.h"
# include "CORBA_BOA.h"
# include "CORBA_ORB.h"
# include "CORBA_Fixed.h"
# include "CORBA_static_fns.h"
# include "CORBA_vartypes.h"

  //////////////////////////////////////////////////////////////////////
  ////////////////////// Generated code ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  // "core" attributes in these headers are exported by the dynamic library

# if defined(_OMNIORB_LIBRARY)
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
# elif defined(_OMNIORB_DYNAMIC_LIBRARY)
#    undef   _core_attr
#    define  _core_attr
# endif

# if defined(ENABLE_CLIENT_IR_SUPPORT)
#    include <omniORB4/ir_defs.hh>
# endif

# include <omniORB4/boxes_defs.hh>
# include <omniORB4/pollable_defs.hh>

# if defined(_OMNIORB_LIBRARY)
#    undef   _core_attr
#    define  _core_attr
# elif defined(_OMNIORB_DYNAMIC_LIBRARY)
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
# endif


  //////////////////////////////////////////////////////////////////////
  ////////////////////// End of CORBA module ///////////////////////////
  //////////////////////////////////////////////////////////////////////

#  undef INSIDE_OMNIORB_CORBA_MODULE

_CORBA_MODULE_END


//?? These really want to be renamed and put elsewhere.
extern CORBA::Boolean
_omni_callTransientExceptionHandler(omniObjRef*             obj,
                                    CORBA::ULong            retries,
				    const CORBA::TRANSIENT& ex,
                                    const char*             op);
extern CORBA::Boolean
_omni_callTimeoutExceptionHandler(omniObjRef*           obj,
                                  CORBA::ULong          retries,
				  const CORBA::TIMEOUT& ex,
                                  const char*           op);
extern CORBA::Boolean
_omni_callCommFailureExceptionHandler(omniObjRef*                obj,
				      CORBA::ULong               retries,
				      const CORBA::COMM_FAILURE& ex,
                                      const char*                op);
extern CORBA::Boolean
_omni_callSystemExceptionHandler(omniObjRef*                   obj,
                                 CORBA::ULong                  retries,
				 const CORBA::SystemException& ex,
                                 const char*                   op);


extern void _omni_set_NameService(CORBA::Object_ptr);

#include <omniORB4/omniIOR.h>
#include <omniORB4/omniORB.h>
#include <omniORB4/proxyFactory.h>
#include <omniORB4/valueType.h>

#include <omniORB4/templatedefns.h>
#include <omniORB4/corba_operators.h>
#include <omniORB4/poa.h>
#include <omniORB4/fixed.h>
#include <omniORB4/BiDirPolicy.h>
#include <omniORB4/omniPolicy.h>
#include <omniORB4/dynAny.h>

#include <omniORB4/minorCode.h>

#include <omniORB4/omniAsyncInvoker.h>

#include <omniORB4/corbaidl_operators.hh>

#if defined(ENABLE_CLIENT_IR_SUPPORT)
#  include <omniORB4/ir_operators.hh>
#endif

#include <omniORB4/boxes_operators.hh>
#include <omniORB4/pollable_operators.hh>


_CORBA_MODULE POA_CORBA
_CORBA_MODULE_BEG

#include <omniORB4/corbaidl_poa.hh>
#if defined(ENABLE_CLIENT_IR_SUPPORT)
#  include <omniORB4/ir_poa.hh>
#endif
#include <omniORB4/boxes_poa.hh>
#include <omniORB4/pollable_poa.hh>

_CORBA_MODULE_END

#include <omniORB4/boa.h>

#undef _core_attr
#undef _dyn_attr

// Define external guards for CORBA module IDL files.
#ifndef __corbaidl_hh_EXTERNAL_GUARD__
#  define __corbaidl_hh_EXTERNAL_GUARD__
#endif

#ifdef ENABLE_CLIENT_IR_SUPPORT
#  ifndef __ir_hh_EXTERNAL_GUARD__
#    define __ir_hh_EXTERNAL_GUARD__
#  endif
#endif

#ifndef __boxes_hh_EXTERNAL_GUARD__
#  define __boxes_hh_EXTERNAL_GUARD__
#endif

#ifndef __pollable_hh_EXTERNAL_GUARD__
#  define __pollable_hh_EXTERNAL_GUARD__
#endif

// Include headers for generated code outside the CORBA module

#if !defined(_OMNIORB_LIBRARY) && !defined(_OMNIORB_DYNAMIC_LIBRARY)
#  ifndef USE_core_stub_in_nt_dll
#    define USE_core_stub_in_nt_dll
#    define USE_core_stub_in_nt_dll_NOT_DEFINED
#  endif
#  ifndef USE_dyn_stub_in_nt_dll
#    define USE_dyn_stub_in_nt_dll
#    define USE_dyn_stub_in_nt_dll_NOT_DEFINED
#  endif
#  include <omniORB4/Naming.hh>
#  include <omniORB4/messaging.hh>
#  ifdef  USE_core_stub_in_nt_dll_NOT_DEFINED
#    undef  USE_core_stub_in_nt_dll
#    undef  USE_core_stub_in_nt_dll_NOT_DEFINED
#  endif
#  ifdef  USE_dyn_stub_in_nt_dll_NOT_DEFINED
#    undef  USE_dyn_stub_in_nt_dll
#    undef  USE_dyn_stub_in_nt_dll_NOT_DEFINED
#  endif
#endif

// OMG COS IDLs refer to CosNaming IDL as "CosNaming.idl".
// omniORB uses the file name "Naming.idl". Any IDLs that include
// CosNaming.idl will have in their stubs #include "CosNaming.hh".
// Define the external guard for CosNaming to stop include to have
// any effect. This works because the stubs generated by omniidl
// put external guards around the include.
#ifndef __CosNaming_hh_EXTERNAL_GUARD__
#  define __CosNaming_hh_EXTERNAL_GUARD__
#endif

// Define __CosNaming_hh__ in case application code tries to import a
// header generated from CosNaming.idl.
#ifndef __CosNaming_hh__
#  define __CosNaming_hh__
#endif

#ifndef __messaging_hh_EXTERNAL_GUARD__
#  define __messaging_hh_EXTERNAL_GUARD__
#endif

#endif // __CORBA_H__
