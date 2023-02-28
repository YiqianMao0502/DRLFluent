// -*- Mode: C++; -*-
//                          Package   : omniNames
// BindingIterator_i.h      Author    : Tristan Richardson (tjr)
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

#ifndef _BindingIterator_i_h_
#define _BindingIterator_i_h_

#include <NamingContext_i.h>


#if defined(__sgi) && defined(_COMPILER_VERSION)
# if _COMPILER_VERSION == 721
#  define MIPSPRO_WORKAROUND
# endif
#endif


class BindingIterator_i : public POA_CosNaming::BindingIterator
{
public:

  BindingIterator_i(PortableServer::POA_ptr poa, CosNaming::BindingList* l)
    : list(l)
  {
    PortableServer::ObjectId_var id = poa->activate_object(this);
  }

  CORBA::Boolean next_one(CosNaming::Binding_out b)
  {
    CosNaming::BindingList_var bl;
    CORBA::Boolean ret = next_n(1, bl);

    b = new CosNaming::Binding;
    if (ret)
      *b = bl[0];
    else
#ifndef MIPSPRO_WORKAROUND
      b->binding_type = CosNaming::nobject;
#else
      b.ptr()->binding_type = CosNaming::nobject;
#endif
    return ret;
  }

  CORBA::Boolean next_n(CORBA::ULong how_many, CosNaming::BindingList_out bl)
  {
    //
    // What we do here is return the current list to the caller, shortening
    // it to the required length.  Before this however, we create a new list
    // and copy the rest of the bindings into the new list.
    //

    if (list->length() < how_many) {
      how_many = list->length();
    }
    bl = list._retn();

    list = new CosNaming::BindingList(bl->length() - how_many);
    list->length(bl->length() - how_many);

    for (unsigned int i = 0; i < list->length(); i++) {
      list[i] = (*bl)[i + how_many];
    }

    bl->length(how_many);

    if (how_many == 0)
      return 0;
    else
      return 1;
  }

  void destroy(void)
  {
    PortableServer::ObjectId_var id = names_poa->servant_to_id(this);
    names_poa->deactivate_object(id);
  }

private:

  CosNaming::BindingList_var list;

  // The destructor for an object should never be called explicitly.
  ~BindingIterator_i() {
  }
};

#endif
