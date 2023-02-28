// -*- Mode: C++; -*-
//                          Package   : omniNames
// INISMapper.h             Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2004 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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

#include <omniORB4/CORBA.h>

class INSMapper :
  public PortableServer::ServantBase
{
public:

  INSMapper(PortableServer::POA_ptr inspoa, CORBA::Object_ptr obj)
    : obj_(obj)
  {
    PortableServer::ObjectId oid(11, 11, (CORBA::Octet*)"NameService", 0);
    inspoa->activate_object_with_id(oid, this);
  }

  ~INSMapper() {}

  CORBA::Boolean _dispatch(omniCallHandle&) {
    throw omniORB::LOCATION_FORWARD(CORBA::Object::_duplicate(obj_),0);
    return 1;
  }
  CORBA::Boolean _is_a(const char* id) {
    throw omniORB::LOCATION_FORWARD(CORBA::Object::_duplicate(obj_),0);
    return 1;
  }
private:
  CORBA::Object_var obj_;
};
