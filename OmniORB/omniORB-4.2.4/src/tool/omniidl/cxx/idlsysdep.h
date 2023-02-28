// -*- c++ -*-
//                          Package   : omniidl
// idlsysdep.h              Created on: 1999/11/04
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2005 Apasphere Ltd
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see http://www.gnu.org/licenses/
//
// Description:
//   
//   System dependencies

#ifndef _idlsysdep_h_
#define _idlsysdep_h_

#define USE_omniORB_logStream
#include <omniORB4/CORBA_sysdep.h>


// As usual, Windows manages to be different...
#ifdef __WIN32__
// declspec is a define in windef.h under MINGW
#ifdef __MINGW32__
#  define DLL_EXPORT __attribute__((dllexport))
#else
#  define DLL_EXPORT _declspec(dllexport)
#endif
#else
#  define DLL_EXPORT
#  include <strings.h>
#endif


#ifdef HAS_Cplusplus_Bool
typedef bool                      IDL_Boolean;
#else
typedef unsigned char             IDL_Boolean;
#endif

typedef unsigned char             IDL_Char;

typedef unsigned char             IDL_Octet;

typedef short                     IDL_Short;

typedef unsigned short            IDL_UShort;

typedef unsigned short            IDL_WChar;

#if SIZEOF_LONG == 4
typedef long                      IDL_Long;

typedef unsigned long             IDL_ULong;
#elif SIZEOF_INT == 4
typedef int                       IDL_Long;

typedef unsigned int              IDL_ULong;
#else
# error "Can't map Long (32 bits) to a native type."
#endif

#ifdef HAS_LongLong
typedef _CORBA_LONGLONG_DECL      IDL_LongLong;
typedef _CORBA_ULONGLONG_DECL     IDL_ULongLong;
#endif


#ifndef NO_FLOAT

#ifndef __VAX

// This platform uses IEEE float
typedef float                     IDL_Float;
typedef double                    IDL_Double;

#ifdef HAS_LongDouble
typedef _CORBA_LONGDOUBLE_DECL    IDL_LongDouble;
#endif

#else	// VAX float test

// VAX uses proxies for float.
#define USING_PROXY_FLOAT

class IDL_Float {
  IDL_Long pd_f;
  void cvt_(float f);
  float cvt_() const;
public:
  // using compiler generated copy constructor and copy assignment
  inline IDL_Float() {cvt_(0.0f);}
  inline IDL_Float(float f) {cvt_(f);}
  inline operator float() const {return cvt_();}
};

class IDL_Double {
  IDL_Long pd_d[2];
  void cvt_(double d);
  double cvt_() const;
public:
  // using compiler generated copy constructor and copy assignment
  inline IDL_Double() {cvt_(0.0);}
  inline IDL_Double(double d) {cvt_(d);}
  inline operator double() const {return cvt_();}
};

#undef cvt_

//  Assume long double type is compatible with the CORBA standard.

#ifdef HAS_LongDouble
typedef _CORBA_LONGDOUBLE_DECL    IDL_LongDouble;
#endif

#endif   // VAX float test
#endif   // !defined(NO_FLOAT)


#endif // _idlsysdep_h_
