// -*- c++ -*-
//                          Package   : omniidl
// idlrepoId.h              Created on: 1999/10/11
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
//   Definitions for repoId management

#ifndef _idlrepoId_h
#define _idlrepoId_h

#include <idlutil.h>

class Prefix {
public:
  // Static prefix manipulation functions

  // Return the current prefix string
  static const char* current();

  // Make prefix for a new scope or file
  static void newScope(const char* name);
  static void newFile();

  // Set prefix for current scope
  static void setPrefix(const char* prefix);

  // Finish with a scope or file, reverting to the previous prefix
  static void endScope();
  static void endFile();
  static void endOuterFile();


protected:
  Prefix(char* str, IDL_Boolean isfile);
  ~Prefix();

  // Get/set operations on this prefix node
  const char*    get();
  void           set(const char* setTo);
  IDL_Boolean    isfile();

private:
  char*          str_;		// Prefix string
  Prefix*        parent_;	// Previous prefix
  IDL_Boolean    isfile_;	// True if prefix is at file scope

  static Prefix* current_;
};


#endif // _idlrepoId_h
