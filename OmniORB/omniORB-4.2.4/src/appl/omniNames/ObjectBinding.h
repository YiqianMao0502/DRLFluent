// -*- Mode: C++; -*-
//                          Package   : omniNames
// ObjectBinding.h          Author    : Tristan Richardson (tjr)
//
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
//
//  This file is part of omniNames.
//
//  omniNames is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see http://www.gnu.org/licenses/
//

#ifndef _ObjectBinding_h_
#define _ObjectBinding_h_

#include <NamingContext_i.h>

class ObjectBinding {

public:

  CosNaming::Binding binding;
  CORBA::Object_var object;

  NamingContext_i* nc;
  ObjectBinding* prev;
  ObjectBinding* next;

  ObjectBinding(const CosNaming::Name& n, CosNaming::BindingType t,
		CORBA::Object_ptr o, NamingContext_i* nct,
		ObjectBinding* nx = 0)
  {
    binding.binding_name = n;
    binding.binding_type = t;

    object = CORBA::Object::_duplicate(o);
    nc = nct;
    next = nx;

    if (next) {
      prev = next->prev;
      next->prev = this;
    } else {
      prev = nc->tailBinding;
      nc->tailBinding = this;
    }
    if (prev) {
      prev->next = this;
    } else {
      nc->headBinding = this;
    }
    nc->size++;
  }

  ~ObjectBinding()
  {
    if (prev) {
      prev->next = next;
    } else {
      nc->headBinding = next;
    }
    if (next) {
      next->prev = prev;
    } else {
      nc->tailBinding = prev;
    }
    nc->size--;
  }

};

#endif
