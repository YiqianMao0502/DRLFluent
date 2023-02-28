// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_sysdep.h             Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2012 Apasphere Ltd
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
//	*** PROPRIETARY INTERFACE ***
//
// 	Define a set of flags in C++ macros. These flags provide information
//      about the system and the compiler used.
// 

#ifndef __CORBA_SYSDEP_H__
#define __CORBA_SYSDEP_H__

#ifndef __cplusplus
#error "Cannot use this C++ header file for non C++ programs."
#endif


#include <omniORB4/local_config.h>


//
// Most system dependencies either come from autoconf or from
// hard-coded values in CORBA_sysdep_trad.h
//

#ifndef OMNI_CONFIG_EXTERNAL
#  include <omniconfig.h>
#endif

#ifdef OMNI_CONFIG_TRADITIONAL
#  include <omniORB4/CORBA_sysdep_trad.h>
#else
#  include <omniORB4/CORBA_sysdep_auto.h>
#endif


//
// Pointer arithmetic type
//

#if SIZEOF_PTR == SIZEOF_LONG
typedef unsigned long omni_ptr_arith_t;
#elif SIZEOF_PTR == SIZEOF_INT
typedef unsigned int omni_ptr_arith_t;
#elif defined (_WIN64)
typedef size_t omni_ptr_arith_t;
#else
#error "No suitable type to do pointer arithmetic"
#endif


//
// Dependencies that are always hard-coded
//

#if defined(__WIN32__)
#  define NTArchitecture 1
#elif defined(__VMS)
#else
#  define UnixArchitecture 1
#endif

//
// Processor dependencies
//

// __VFP_FP__ means that the floating point format in use is that of the ARM 
// VFP unit, which is native-endian IEEE-754.
#if defined(__arm__)
#  if defined(__armv5teb__) || defined(__VFP_FP__)
#    define NO_OMNI_MIXED_ENDIAN_DOUBLE
#  else
#    define OMNI_MIXED_ENDIAN_DOUBLE
#  endif
#endif


//
// Macro to provide const_cast functionality on all platforms.
//
#ifdef HAS_Cplusplus_const_cast
#  define OMNI_CONST_CAST(_t, _v) const_cast< _t >(_v)
#  define OMNI_CONST_VOID_CAST(_v) const_cast<void*>(static_cast<const void*>(_v))
#else
#  define OMNI_CONST_CAST(_t, _v) (_t)(_v)
#  define OMNI_CONST_VOID_CAST(_v) (void*)(_t)
#endif

#ifdef HAS_Cplusplus_reinterpret_cast
#  define OMNI_REINTERPRET_CAST(_t, _v) reinterpret_cast< _t const& >(_v)
#else
#  define OMNI_REINTERPRET_CAST(_t, _v) (*(_t*)(&_v))
#endif


#if defined(__GNUG__)
// GNU G++ compiler
#  define NEED_DUMMY_RETURN
#endif

#if defined(__DECCXX)
// DEC C++ compiler
#  if __DECCXX_VER < 60000000
//    Compaq C++ 5.x
//    Work-around for OpenVMS VAX Compaq C++ 5.6 compiler problem with
//    %CXX-W-CANTCOMPLETE messages.  Note that this cannot be disabled with a
//    compiler switch if the message occurs in a template instantiation (but
//    this pragma wasn't implemented until 6.something on Unix).
#     ifdef __VMS
#       pragma message disable CANTCOMPLETE
#     endif
#     define NEED_DUMMY_RETURN
#     define OMNI_OPERATOR_REFPTR_REQUIRES_TYPEDEF
#     define OMNI_PREMATURE_INSTANTIATION
//    Extra macros from the Compaq C++ 5.x patch (in <top>/patches/) to be
//    added here
#     ifndef OMNI_OPERATOR_REFPTR
#       error "Patch for Compaq C++ 5.x has not been applied."
#     endif
#  endif
#endif

#if defined(__SUNPRO_CC) 
// SUN C++ compiler
#  define NEED_DUMMY_RETURN

// XXX
// This is a hack to work around a bug in SUN C++ compiler (seen on 4.2).
// When instantiating templates, the compiler may generate code in Template.DB.
// It stores in the directory the dependency and compiler options to
// regenerate the template code if necessary. Unfortunately, it stores the
// option -D__OSVERSION__=5 as -D__OSVERSION__='5'. This of course causes
// the compilation to fail because the source code assume that the CPP macro
// __OSVERSION__ is an interger. The following hack detects this condition
// and revert the macro to its expected form.
#  ifdef __OSVERSION__
#    if __OSVERSION__ != 5
#       if __OSVERSION__ == '5'
#          undef __OSVERSION__
#          define __OSVERSION__ 5
#       endif
#    endif
#  endif
#endif

#if defined(__HP_aCC) || ( defined(__hpux__) && !defined(__GNUG__) )
#  define NEED_DUMMY_RETURN
#endif

#if defined(__hpux__) && __OSVERSION__ < 11
// *** Is this really needed?
#  include <stdio.h>
#  undef __ptr
#endif


#if defined(__sgi)
#  define OMNI_NEED_STATIC_FUNC_TO_FORCE_LINK
#endif


#if defined(__aix__) && defined(__xlC__)
#  define OMNI_NO_INLINE_FRIENDS
#  define NEED_DUMMY_RETURN
#endif


#if defined(__clang__)
#  define OMNI_NO_INLINE_FRIENDS
#endif


//
// Windows DLL hell
//

#if defined(_MSC_VER)

#  define NEED_DUMMY_RETURN

// VC.NET 2003 (v. 7.1) has problems recognizing inline friend
// operators.

#  if (_MSC_VER >= 1310)
#    define OMNI_NO_INLINE_FRIENDS
#  endif

//
// _OMNIORB_LIBRARY         is defined when the omniORB library is compiled.
// _OMNIORB_DYNAMIC_LIBRARY is defined when the dynamic library is compiled.
//  These are defined on the command line when compiling the libraries.
//
// _WINSTATIC                is defined when an application is compiled to
//                           use the static library.
//
// To package stubs into dlls:
//   1. Define the cpp macro _OMNIORB_STUB_DLL when the stub _SK.cc is
//      compiled.
//   2. A .def file has to be created to export the symbols in the dll.
//      The .def file can be generated automatically based on the output
//      of dumpbin. For an example, look at how the omniORB2 dll is created.
//
// To use stubs that has been packaged into dlls:
//   1. Make sure that the cpp macro USE_stub_in_nt_dll is defined before
//      the stub header (.hh) is included.
//
// Use _OMNIORB_NTDLL_IMPORT to ensure that MSVC++ use the correct linkage
// for constants and variables exported by a DLL.
//

#  ifdef _WINSTATIC
#    define _OMNIORB_NTDLL_IMPORT
#  else
#    define _OMNIORB_NTDLL_IMPORT  __declspec(dllimport)
#  endif

#  if defined(_DEBUG)
// The type name instantiated from the sequence templates could exceeds the
// 255 char limit of the debug symbol names. It is harmless except that one
// cannot read their values with the debugger. Disable the warning about
// the symbol name truncation.
#    pragma warning(disable: 4786)
#  endif

// Disable warnings about a member function in a derived class overriding
// a member function in the base class.
#  pragma warning(disable: 4250)

#elif defined(__DMC__)
#  define NEED_DUMMY_RETURN

#  ifdef _WINSTATIC
#    define _OMNIORB_NTDLL_IMPORT
#  else
#    define _OMNIORB_NTDLL_IMPORT  __declspec(dllimport)
#  endif

#elif defined(__MINGW32__)

#  ifdef _WINSTATIC
#    define _OMNIORB_NTDLL_IMPORT
#  else
#    define _OMNIORB_NTDLL_IMPORT  __attribute__((dllimport))
#  endif

#else

// For non-MSVC++ compiler, this macro expands to nothing.
#  define _OMNIORB_NTDLL_IMPORT

#endif


//
// Module mapping using namespaces or classes
//

#ifdef _CORBA_MODULE
#  error "Name conflict: _CORBA_MODULE is already defined."
#endif

#ifdef _CORBA_MODULE_BEG
#  error "Name conflict: _CORBA_MODULE_BEG is already defined."
#endif

#ifdef _CORBA_MODULE_END
#  error "Name conflict: _CORBA_MODULE_END is already defined."
#endif

#ifdef _CORBA_MODULE_OP
#  error "Name conflict: _CORBA_MODULE_OP is already defined."
#endif

#ifdef _CORBA_MODULE_FN
#  error "Name conflict: _CORBA_MODULE_FN is already defined."
#endif

#ifdef _CORBA_MODULE_VAR
#  error "Name conflict: _CORBA_MODULE_VAR is already defined."
#endif

#ifdef _CORBA_MODULE_INLINE
#  error "Name conflict: _CORBA_MODULE_INLINE is already defined."
#endif

#ifdef _CORBA_GLOBAL_VAR
#  error "Name conflict: _CORBA_GLOBAL_VAR is already defined."
#endif

#ifdef _CORBA_MODULE_VARINT
#  error "Name conflict: _CORBA_MODULE_VARINT is already defined."
#endif

#ifdef _CORBA_GLOBAL_VARINT
#  error "Name conflict: _CORBA_GLOBAL_VARINT is already defined."
#endif

#ifdef OMNI_NAMESPACE_BEGIN
#  error "Name conflict: OMNI_NAMESPACE_BEGIN is already defined."
#endif

#ifdef OMNI_NAMESPACE_END
#  error "Name conflict: OMNI_NAMESPACE_END is already defined."
#endif

#ifdef OMNI_USING_NAMESPACE
#  error "Name conflict: OMNI_USING_NAMESPACE is already defined."
#endif

#ifdef _OMNI_NS
#  error "Name conflict: _OMNI_NS is already defined."
#endif

#ifdef _init_in_decl_
#  error "Name conflict: _init_in_decl_ is already defined."
#endif

#ifdef _init_in_def_
#  error "Name conflict: _init_in_def_ is already defined."
#endif

#ifdef _init_in_cldecl_
#  error "Name conflict: _init_in_cldecl_ is already defined."
#endif

#ifdef _init_in_cldef_
#  error "Name conflict: _init_in_cldef_ is already defined."
#endif


#ifdef HAS_Cplusplus_Namespace

#  define _CORBA_MODULE        namespace
#  define _CORBA_MODULE_BEG    {
#  define _CORBA_MODULE_END    }
#  define _CORBA_MODULE_OP
#  define _CORBA_MODULE_FN
#  define _CORBA_MODULE_VAR    extern
#  define _CORBA_MODULE_INLINE inline
#  define _CORBA_GLOBAL_VAR    extern
#  define _CORBA_MODULE_VARINT
#  define _CORBA_GLOBAL_VARINT
#  define OMNI_NAMESPACE_BEGIN(name) namespace name {
#  define OMNI_NAMESPACE_END(name)   }
#  define OMNI_USING_NAMESPACE(name) using namespace name;
#  define _OMNI_NS(x) omni::x

// Integral and enumeration constants are declared in the stub headers as:
//    e.g.  class A {
//              static const CORBA::Long AA _init_in_cldecl_( = 4 );
//          };
//          namespace B {
//              const CORBA::Long BB _init_in_decl_( = 5 );
//          };
// And defined in the SK.cc:
//    e.g.   const CORBA::Long A::AA _init_in_cldef_( = 4 );
//           _init_in_def_( const CORBA::Long B::BB = 5 );
//  
// Final Draft (FD) allows declaration of static const integral or enum type 
// be specified with a constant-initializer whereas ARM does not.
// The _init_in_decl_ and _init_in_def_ macros are defined so that the same 
// stub will compile on both FD and ARM compilers.
// Older MSVC++ are somewhere between FD and ARM. 
//  _MSC_VER = 1100 for 5.0, 1200 for 6.0.

#  define _init_in_decl_(x) x
#  define _init_in_def_(x)

#  if defined(__INTEL_COMPILER)
#    define _init_in_cldecl_(x)
#    define _init_in_cldef_(x) x
#  elif defined (_MSC_VER) && (_MSC_VER < 1500)
#    define _init_in_cldecl_(x)
#    define _init_in_cldef_(x) x
#  else
#    define _init_in_cldecl_(x) x
#    define _init_in_cldef_(x)
#  endif

#else // No namespace support

#  define _CORBA_MODULE        class
#  define _CORBA_MODULE_BEG    { public:
#  define _CORBA_MODULE_END    };
#  define _CORBA_MODULE_OP     friend
#  define _CORBA_MODULE_FN     static
#  define _CORBA_MODULE_VAR    static
#  define _CORBA_MODULE_INLINE static inline
#  define _CORBA_GLOBAL_VAR    extern
#  define _CORBA_MODULE_VARINT static _core_attr
#  define _CORBA_GLOBAL_VARINT extern _core_attr
#  define OMNI_NAMESPACE_BEGIN(name)
#  define OMNI_NAMESPACE_END(name)
#  define OMNI_USING_NAMESPACE(name)
#  define _OMNI_NS(x) x

#  define _init_in_decl_(x)
#  define _init_in_def_(x) x
#  define _init_in_cldecl_(x)
#  define _init_in_cldef_(x) x

#endif // HAS_Cplusplus_Namespace


#ifdef OMNI_REQUIRES_FQ_BASE_CTOR
#  define OMNIORB_BASE_CTOR(a)   a
#else
#  define OMNIORB_BASE_CTOR(a)
#endif

#ifndef OMNI_OPERATOR_REFPTR
// Only used when the source tree is patched with DEC C++ 5.6 workarounds
#  define OMNI_OPERATOR_REFPTR(T) inline operator T*&()
#endif

#ifndef OMNI_CONSTRTYPE_FIX_VAR_MEMBER
// Only used when the source tree is patched with DEC C++ 5.6 workarounds
#  define OMNI_CONSTRTYPE_FIX_VAR_MEMBER(T) \
   typedef _CORBA_ConstrType_Fix_Var<T> _var_type;
#endif

#ifndef OMNI_CONSTRTYPE_FIX_VAR
// Only used when the source tree is patched with DEC C++ 5.6 workarounds
#  define OMNI_CONSTRTYPE_FIX_VAR(T) typedef T::_var_type T##_var;
#endif


//
// Suppress throw specifications that are not valid from C++-17. Note
// that we keep them for older compilers because removing them may
// break ABI compatibility.

#if __cplusplus >= 201703L
#  define OMNI_THROW_SPEC(...)
#else
#  define OMNI_THROW_SPEC throw
#endif



// #define ENABLE_CLIENT_IR_SUPPORT
// Define ENABLE_CLIENT_IR_SUPPORT to use as client to an Interface Repository


#endif // __CORBA_SYSDEP_H__
