// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_basetypes.h          Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2013 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories - Cambridge Ltd
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

#ifndef __CORBA_BASETYPES_H__
#define __CORBA_BASETYPES_H__

#ifdef HAS_Cplusplus_Bool
typedef bool                      _CORBA_Boolean;
#else
typedef unsigned char             _CORBA_Boolean;
#endif

typedef unsigned char             _CORBA_Char;

typedef unsigned char             _CORBA_Octet;

typedef short                     _CORBA_Short;

typedef unsigned short            _CORBA_UShort;

#if SIZEOF_LONG == 4
typedef long                      _CORBA_Long;

typedef unsigned long             _CORBA_ULong;

#elif SIZEOF_INT == 4
#  ifndef OMNI_LONG_IS_INT
#    define OMNI_LONG_IS_INT
#  endif

typedef int                       _CORBA_Long;

typedef unsigned int              _CORBA_ULong;
#else
# error "Can't map Long (32 bits) to a native type."
#endif

typedef _CORBA_WCHAR_DECL         _CORBA_WChar;

#ifdef HAS_LongLong
typedef _CORBA_LONGLONG_DECL      _CORBA_LongLong;
typedef _CORBA_ULONGLONG_DECL     _CORBA_ULongLong;
#endif


#ifndef NO_FLOAT

#ifndef __VMS

// This platform uses IEEE float
typedef float                     _CORBA_Float;
typedef double                    _CORBA_Double;

#ifdef HAS_LongDouble
typedef _CORBA_LONGDOUBLE_DECL    _CORBA_LongDouble;
#endif

#else	// VMS float test

// VMS now always uses proxies for float.
#define USING_PROXY_FLOAT

#undef cvt_
#if __D_FLOAT
#define cvt_ cvt_d_
#elif __G_FLOAT
#define cvt_ cvt_g_
#else
#define cvt_ cvt_ieee_
#endif

class _CORBA_Float {
  _CORBA_Long pd_f;
  void cvt_d_(float f);
  float cvt_d_() const;
  void cvt_g_(float f);
  float cvt_g_() const;
#ifndef __VAX
  void cvt_ieee_(float f);
  float cvt_ieee_() const;
#endif
public:
  // using compiler generated copy constructor and copy assignment
  inline _CORBA_Float() {cvt_(0.0f);}
  inline _CORBA_Float(float f) {cvt_(f);}
  inline operator float() const {return cvt_();}
  inline _CORBA_Float operator+() const {
    return *this;
  }
  inline _CORBA_Float operator-() const {
    return _CORBA_Float(-cvt_());
  }
  inline _CORBA_Float& operator+=(_CORBA_Float const& v) {
    cvt_(cvt_()+v.cvt_());
    return *this;
  }
  inline _CORBA_Float& operator-=(_CORBA_Float const& v) {
    cvt_(cvt_()-v.cvt_());
    return *this;
  }
  inline _CORBA_Float& operator*=(_CORBA_Float const& v) {
    cvt_(cvt_()*v.cvt_());
    return *this;
  }
  inline _CORBA_Float& operator/=(_CORBA_Float const& v) {
    cvt_(cvt_()/v.cvt_());
    return *this;
  }
  inline _CORBA_Float& operator+=(float v) {
    cvt_(cvt_()+v);
    return *this;
  }
  inline _CORBA_Float& operator-=(float v) {
    cvt_(cvt_()-v);
    return *this;
  }
  inline _CORBA_Float& operator*=(float v) {
    cvt_(cvt_()*v);
    return *this;
  }
  inline _CORBA_Float& operator/=(float v) {
    cvt_(cvt_()/v);
    return *this;
  }
  inline static int compare(_CORBA_Float const& lhs,
                            _CORBA_Float const& rhs) {
    float l=lhs.cvt_();
    float r=rhs.cvt_();
    if (l<r)
      return -1;
    if (l==r)
      return 0;
    return 1;
  }
};

class _CORBA_Double {
  _CORBA_Long pd_d[2];
  void cvt_d_(double d);
  double cvt_d_() const;
  void cvt_g_(double d);
  double cvt_g_() const;
#ifndef __VAX
  void cvt_ieee_(double d);
  double cvt_ieee_() const;
#endif
public:
  // using compiler generated copy constructor and copy assignment
  inline _CORBA_Double() {cvt_(0.0);}
  inline _CORBA_Double(double d) {cvt_(d);}
  inline operator double() const {return cvt_();}
  inline _CORBA_Double operator+() const {
    return *this;
  }
  inline _CORBA_Double operator-() const {
    return _CORBA_Double(-cvt_());
  }
  inline _CORBA_Double& operator+=(_CORBA_Double const& v) {
    cvt_(cvt_()+v.cvt_());
    return *this;
  }
  inline _CORBA_Double& operator-=(_CORBA_Double const& v) {
    cvt_(cvt_()-v.cvt_());
    return *this;
  }
  inline _CORBA_Double& operator*=(_CORBA_Double const& v) {
    cvt_(cvt_()*v.cvt_());
    return *this;
  }
  inline _CORBA_Double& operator/=(_CORBA_Double const& v) {
    cvt_(cvt_()/v.cvt_());
    return *this;
  }
  inline _CORBA_Double& operator+=(double v) {
    cvt_(cvt_()+v);
    return *this;
  }
  inline _CORBA_Double& operator-=(double v) {
    cvt_(cvt_()-v);
    return *this;
  }
  inline _CORBA_Double& operator*=(double v) {
    cvt_(cvt_()*v);
    return *this;
  }
  inline _CORBA_Double& operator/=(double v) {
    cvt_(cvt_()/v);
    return *this;
  }
  inline static int compare(_CORBA_Double const& lhs,
                            _CORBA_Double const& rhs) {
    double l=lhs.cvt_();
    double r=rhs.cvt_();
    if (l<r)
      return -1;
    if (l==r)
      return 0;
    return 1;
  }
};

inline _CORBA_Float operator+(_CORBA_Float const& lhs,
                              _CORBA_Float const& rhs) {
  _CORBA_Float f(lhs);
  f+=rhs;
  return f;
}

inline _CORBA_Float operator-(_CORBA_Float const& lhs,
                              _CORBA_Float const& rhs) {
  _CORBA_Float f(lhs);
  f-=rhs;
  return f;
}

inline _CORBA_Float operator*(_CORBA_Float const& lhs,
                              _CORBA_Float const& rhs) {
  _CORBA_Float f(lhs);
  f*=rhs;
  return f;
}

inline _CORBA_Float operator/(_CORBA_Float const& lhs,
                              _CORBA_Float const& rhs) {
  _CORBA_Float f(lhs);
  f/=rhs;
  return f;
}

inline bool operator==(_CORBA_Float const& lhs,
                       _CORBA_Float const& rhs) {
    return _CORBA_Float::compare(lhs,rhs)==0;
}

inline bool operator!=(_CORBA_Float const& lhs,
                       _CORBA_Float const& rhs) {
    return _CORBA_Float::compare(lhs,rhs)!=0;
}

inline bool operator<=(_CORBA_Float const& lhs,
                       _CORBA_Float const& rhs) {
    return _CORBA_Float::compare(lhs,rhs)<=0;
}

inline bool operator>=(_CORBA_Float const& lhs,
                       _CORBA_Float const& rhs) {
    return _CORBA_Float::compare(lhs,rhs)>=0;
}

inline bool operator<(_CORBA_Float const& lhs,
                       _CORBA_Float const& rhs) {
    return _CORBA_Float::compare(lhs,rhs)<0;
}

inline bool operator>(_CORBA_Float const& lhs,
                       _CORBA_Float const& rhs) {
    return _CORBA_Float::compare(lhs,rhs)>0;
}

inline _CORBA_Double operator+(_CORBA_Double const& lhs,
                               _CORBA_Double const& rhs) {
  _CORBA_Double d(lhs);
  d+=rhs;
  return d;
}

inline _CORBA_Double operator-(_CORBA_Double const& lhs,
                               _CORBA_Double const& rhs) {
  _CORBA_Double d(lhs);
  d-=rhs;
  return d;
}

inline _CORBA_Double operator*(_CORBA_Double const& lhs,
                               _CORBA_Double const& rhs) {
  _CORBA_Double d(lhs);
  d*=rhs;
  return d;
}

inline _CORBA_Double operator/(_CORBA_Double const& lhs,
                               _CORBA_Double const& rhs) {
  _CORBA_Double d(lhs);
  d/=rhs;
  return d;
}

inline bool operator==(_CORBA_Double const& lhs,
                       _CORBA_Double const& rhs) {
    return _CORBA_Double::compare(lhs,rhs)==0;
}

inline bool operator!=(_CORBA_Double const& lhs,
                       _CORBA_Double const& rhs) {
    return _CORBA_Double::compare(lhs,rhs)!=0;
}

inline bool operator<=(_CORBA_Double const& lhs,
                       _CORBA_Double const& rhs) {
    return _CORBA_Double::compare(lhs,rhs)<=0;
}

inline bool operator>=(_CORBA_Double const& lhs,
                       _CORBA_Double const& rhs) {
    return _CORBA_Double::compare(lhs,rhs)>=0;
}

inline bool operator<(_CORBA_Double const& lhs,
                      _CORBA_Double const& rhs) {
    return _CORBA_Double::compare(lhs,rhs)<0;
}

inline bool operator>(_CORBA_Double const& lhs,
                      _CORBA_Double const& rhs) {
    return _CORBA_Double::compare(lhs,rhs)>0;
}

#undef cvt_

//  Assume long double type is compatible with the CORBA standard.

#ifdef HAS_LongDouble
typedef _CORBA_LONGDOUBLE_DECL    _CORBA_LongDouble;
#endif

#endif   // VMS float test
#endif   // !defined(NO_FLOAT)

class cdrStream;

extern void _CORBA_new_operator_return_null();
extern void _CORBA_bound_check_error();
extern void _CORBA_marshal_sequence_range_check_error(cdrStream&);
extern _CORBA_Boolean _CORBA_use_nil_ptr_as_nil_objref();
extern void _CORBA_null_string_ptr(_CORBA_Boolean);
extern void _CORBA_invoked_nil_pseudo_ref();
extern void _CORBA_invoked_nil_objref();
extern _CORBA_Boolean
_CORBA_use_nil_ptr_as_nil_pseudo_objref(const char* objType);
extern void _CORBA_bad_param_freebuf();


#endif // __CORBA_BASETYPES_H__
