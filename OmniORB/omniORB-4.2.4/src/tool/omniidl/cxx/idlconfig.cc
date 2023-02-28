// -*- c++ -*-
//                          Package   : omniidl
// idlconfig.cc             Created on: 2000/03/06
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

#include <idlconfig.h>


IDL_Boolean Config::quiet          = 0;
IDL_Boolean Config::forwardWarning = 1;
IDL_Boolean Config::keepComments   = 0;
IDL_Boolean Config::commentsFirst  = 0;
IDL_Boolean Config::caseSensitive  = 0;
