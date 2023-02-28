// -*- Mode: C++; -*-
//                            Package   : omniORB
// pollablestub.cc            Created on: 2011/01/14
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2011 Apasphere Ltd.
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

#include <omniORB4/CORBA.h>

OMNI_USING_NAMESPACE(omni)

#define USE_core_stub_in_nt_dll
#include <omniORB4/pollableSK.cc>
#include <omniORB4/pollableDynSK.cc>
