// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_sysdep.h             Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
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
//      Define various symbols based on things detected by autoconf

#ifndef __CORBA_SYSDEP_AUTO_H__
#define __CORBA_SYSDEP_AUTO_H__


#ifdef HAVE_BOOL
#  define HAS_Cplusplus_Bool
#endif

#ifdef HAVE_CATCH_BY_BASE
#  define HAS_Cplusplus_catch_exception_by_base
#endif

#ifdef HAVE_CONST_CAST
#  define HAS_Cplusplus_const_cast
#endif

#ifdef HAVE_REINTERPRET_CAST
#  define HAS_Cplusplus_reinterpret_cast
#endif

#ifdef HAVE_NAMESPACES
#  define HAS_Cplusplus_Namespace
#endif

#define SIZEOF_PTR SIZEOF_VOIDP

#if defined(SIZEOF_LONG) && (SIZEOF_LONG == 8)
#  define HAS_LongLong
#  define _CORBA_LONGLONG_DECL     long
#  define _CORBA_ULONGLONG_DECL    unsigned long
#  define _CORBA_LONGLONG_CONST(x) (x)

#elif defined(SIZEOF_LONG_LONG) && (SIZEOF_LONG_LONG == 8)
#  define HAS_LongLong
#  define _CORBA_LONGLONG_DECL     long long
#  define _CORBA_ULONGLONG_DECL    unsigned long long
#  define _CORBA_LONGLONG_CONST(x) (x##LL)
#endif


#if !defined(OMNIORB_DISABLE_LONGDOUBLE)
#  if defined(SIZEOF_LONG_DOUBLE) && (SIZEOF_LONG_DOUBLE == 16)
#    define HAS_LongDouble
#    define _CORBA_LONGDOUBLE_DECL long double
#  endif

#  if defined(SIZEOF_LONG_DOUBLE) && (SIZEOF_LONG_DOUBLE == 12) && defined(__i386__)
#    define HAS_LongDouble
#    define _CORBA_LONGDOUBLE_DECL long double
#  endif
#endif

#ifndef _CORBA_WCHAR_DECL
#  if defined(SIZEOF_WCHAR_T) && (SIZEOF_WCHAR_T > 0)
#    define _CORBA_WCHAR_DECL wchar_t
#    define SIZEOF_WCHAR SIZEOF_WCHAR_T
#  else
#    define _CORBA_WCHAR_DECL _CORBA_Short
#    define SIZEOF_WCHAR 2
#  endif
#endif

#if !defined(SIZEOF_FLOAT) || (SIZEOF_FLOAT == 0)
#  define NO_FLOAT
#endif

#ifdef WORDS_BIGENDIAN
#  define _OMNIORB_HOST_BYTE_ORDER_ 0
#else
#  define _OMNIORB_HOST_BYTE_ORDER_ 1
#endif


#endif // __CORBA_SYSDEP_AUTO_H__
