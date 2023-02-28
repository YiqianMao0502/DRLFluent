// -*- c++ -*-
//                          Package   : omniidl
// idlutil.h                Created on: 1999/10/11
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
//   Utility functions

#ifndef _idlutil_h_
#define _idlutil_h_

#include <assert.h>

#include <idlsysdep.h>

#ifdef HAS_LongLong
typedef IDL_ULongLong IdlIntLiteral;
#else
typedef IDL_ULong IdlIntLiteral;
#endif

#ifdef HAS_LongDouble
typedef IDL_LongDouble IdlFloatLiteral;
#else
typedef IDL_Double IdlFloatLiteral;
#endif

// Version of strdup which uses new
char*      idl_strdup(const char* s);
IDL_WChar* idl_wstrdup(const IDL_WChar* s);

// strlen, strcpy and strcat for wstring
int        idl_wstrlen(const IDL_WChar* s);
IDL_WChar* idl_wstrcpy(IDL_WChar* a, const IDL_WChar* b);
IDL_WChar* idl_wstrcat(IDL_WChar* a, const IDL_WChar* b);

#ifndef HAVE_STRCASECMP
int strcasecmp(const char* s1, const char* s2);
#endif


// Versions of strtoul and strtod which work with the type sizes in use

IdlIntLiteral   idl_strtoul(const char* text, int base);
IdlFloatLiteral idl_strtod (const char* text);


#endif // _idlutil_h_
