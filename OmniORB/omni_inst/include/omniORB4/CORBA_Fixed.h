// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Fixed.h              Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
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
//    CORBA::Fixed
//

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

// Some Solaris versions helpfully #define truncate to truncate64...
#ifdef truncate
#  undef truncate
#endif


//////////////////////////////////////////////////////////////////////
/////////////////////////////// Fixed ////////////////////////////////
//////////////////////////////////////////////////////////////////////

class Fixed {

#define OMNI_FIXED_DIGITS 31

public:
  // Constructors
  Fixed(int val = 0);
  Fixed(unsigned val);

#ifndef OMNI_LONG_IS_INT
  Fixed(Long val);
  Fixed(ULong val);
#endif
#ifdef HAS_LongLong
  Fixed(LongLong val);
  Fixed(ULongLong val);
#endif
#ifndef NO_FLOAT
  Fixed(Double val);
#endif
#ifdef HAS_LongDouble
  Fixed(LongDouble val);
#endif
  Fixed(const Fixed& val);
  Fixed(const char* val);

  Fixed(const Octet* val, UShort digits, UShort scale, Boolean negative);
  // omniORB specific constructor

  ~Fixed();

  // Conversions
#ifdef HAS_LongLong
  operator LongLong() const;
#else
  operator Long() const;
#endif
#ifndef NO_FLOAT
#  ifdef HAS_LongDouble
  operator LongDouble() const;
#  else
  operator Double() const;
#  endif
#endif
  Fixed round   (UShort scale) const;
  Fixed truncate(UShort scale) const;

  // Operators
  Fixed&  operator= (const Fixed& val);
  Fixed&  operator+=(const Fixed& val);
  Fixed&  operator-=(const Fixed& val);
  Fixed&  operator*=(const Fixed& val);
  Fixed&  operator/=(const Fixed& val);

  Fixed&  operator++();
  Fixed   operator++(int);
  Fixed&  operator--();
  Fixed   operator--(int);
  Fixed   operator+ () const;
  Fixed   operator- () const;
  Boolean operator! () const;

  // Other member functions
  UShort fixed_digits() const { return pd_digits; }
  UShort fixed_scale()  const { return pd_scale;  }

  char*  to_string()    const;
  // Return a string with trailing zeros to match the scale.


  // omniORB specific functions

  char* NP_asString() const;
  // Return a string containing the fixed. Caller frees with
  // CORBA::string_free(). Does not include trailing zeros.

  CORBA::Boolean NP_fromString(const char* val, Boolean ignore_end = 0);
  // Set the value from the given string.
  // If ignore_end is true, do not complain about trailing garbage.
  // Return true if the value fit without truncation; false if the
  // value had to be truncated.

  static int NP_cmp(const Fixed& a, const Fixed& b);
  // Compare a and b. Returns -1 if a < b, 1 if a > b, 0 if a == b.


  // "Private" functions only to be called by omniORB internals.
  // Application code which uses these is asking for trouble.

  const Octet* PR_val() const { return pd_val; }
  // Return the internal value buffer. Used by arithmetic functions.

  Boolean PR_negative() const { return pd_negative; }
  // True if the value is negative, false if positive or zero.

  CORBA::Boolean PR_checkLimits();
  // Function to check that this fixed point value fits in the
  // digits/scale limits declared in the IDL. Truncates the value,
  // or throws DATA_CONVERSION if the value is too big. Does nothing
  // for base CORBA::Fixed, where there are no limits.
  // Returns true if the value fit without truncation; false if the
  // value had to be truncated.

  void PR_setLimits(UShort idl_digits, UShort idl_scale);
  // Set and check the digits/scale limits.

  void PR_changeScale(UShort new_scale);
  // Modify the scale. Updates the number of digits if necessary.

  // Marshalling operators
  void operator>>= (cdrStream& s) const;
  void operator<<= (cdrStream& s);

private:
  Octet   pd_val[OMNI_FIXED_DIGITS]; // Value stored least sig. digit first

  UShort  pd_digits;     // Digits and scale the number has
  UShort  pd_scale;
  Boolean pd_negative;   // True if value is negative
  UShort  pd_idl_digits; // Digits and scale the IDL says it should have
  UShort  pd_idl_scale;  //  (zero for base CORBA::Fixed).
};
