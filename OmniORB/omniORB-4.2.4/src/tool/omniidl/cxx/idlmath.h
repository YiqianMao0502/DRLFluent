// -*- c++ -*-
//                          Package   : omniidl
// idlmath.cc               Created on: 1999/10/19
//			    Author    : Duncan Grisby (dpg1)
//
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
//   Floating point maths functions

#ifndef _idlmath_h_
#define _idlmath_h_

#include <math.h>
#include <idlutil.h>

#ifdef HAVE_NAN_H
#  include <nan.h>
#endif

#if defined(HAVE_ISINF) && defined(HAVE_ISINFF) && (defined(HAVE_ISINFL) || !defined(HAS_LongDouble))

inline IDL_Boolean IdlFPOverflow(IDL_Double f) {
  return isinf(f) || isnan(f);
}

inline IDL_Boolean IdlFPOverflow(IDL_Float f) {
  return isinff(f) || isnanf(f);
}
#  ifdef HAS_LongDouble
inline IDL_Boolean IdlFPOverflow(IDL_LongDouble f) {
  return isinfl(f) || isnanl(f);
}
#  endif

#elif defined(HAVE_ISNANORINF)

inline IDL_Boolean IdlFPOverflow(IDL_Float f) {
  double d = f;
  return IsNANorINF(d);
}
inline IDL_Boolean IdlFPOverflow(IDL_Double f) {
  return IsNANorINF(f);
}
#ifdef HAS_LongDouble
inline IDL_Boolean IdlFPOverflow(IDL_LongDouble f) {
  return 0;
}
#endif

#else // No FP overflow detection

inline IDL_Boolean IdlFPOverflow(IDL_Float f) {
  return 0;
}
inline IDL_Boolean IdlFPOverflow(IDL_Double f) {
  return 0;
}
#ifdef HAS_LongDouble
inline IDL_Boolean IdlFPOverflow(IDL_LongDouble f) {
  return 0;
}
#endif

#endif

#endif // _idlmath_h_
