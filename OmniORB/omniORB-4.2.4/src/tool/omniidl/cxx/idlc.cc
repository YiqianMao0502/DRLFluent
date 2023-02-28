// -*- c++ -*-
//                          Package   : omniidl
// idlc.cc                  Created on: 1999/10/20
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
//   Simple main() function to test front-end

#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>

#include <idlerr.h>
#include <idlutil.h>
#include <idlrepoId.h>
#include <idlast.h>
#include <idlscope.h>
#include <idldump.h>

extern int yydebug;

int main(int argc, char** argv)
{
  if (argc != 1 && argc != 2) {
    cerr << "Usage: " << argv[0] << " [idl file]" << endl;
    exit(2);
  }

  //  yydebug = 1;

  FILE*       f;
  const char* name;

  if (argc == 2) {
    name = argv[1];

    if (!((f = fopen(name, "r")))) {
      cerr << "Can't open " << name << endl;
      exit(2);
    }
  }
  else {
    name = "<stdin>";
    f    = stdin;
  }

  IDL_Boolean success = AST::process(f, name);

  fclose(f);

  if (!success) exit(1);

  DumpVisitor v;
  AST::tree()->accept(v);

  cout << "Done." << endl;

  return 0;
}
