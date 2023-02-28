// -*- c++ -*-
//                          Package   : omniidl
// idlconfig.h              Created on: 2000/03/06
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//   Global configuration for omniidl

#ifndef _idlconfig_h_
#define _idlconfig_h_

#include <idlutil.h>


class Config {
public:
  static IDL_Boolean quiet;           // Don't make any output
  static IDL_Boolean forwardWarning;  // Warn about unresolved forwards
  static IDL_Boolean keepComments;    // Keep comments from source
  static IDL_Boolean commentsFirst;   // Comments come before declarations
  static IDL_Boolean caseSensitive;   // Do not treat identifiers differing
                                      //  only in case as errors
};


#endif // _idlconfig_h_
