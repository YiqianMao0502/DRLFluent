// -*- Mode: C++; -*-
//                            Package   : omniORB
// BiDirPolicy.h              Created on: 30/7/2001
//                            Author    : Sai Lai Lo (sll)
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
//	*** PROPRIETARY INTERFACE ***
//

#ifndef __BIDIRPOLICY_H__
#define __BIDIRPOLICY_H__

_CORBA_MODULE BiDirPolicy

_CORBA_MODULE_BEG

typedef CORBA::UShort BidirectionalPolicyValue;

_CORBA_MODULE_VARINT
const BidirectionalPolicyValue NORMAL             _init_in_decl_( = 0 );

_CORBA_MODULE_VARINT
const BidirectionalPolicyValue BOTH               _init_in_decl_( = 1 );

_CORBA_MODULE_VARINT
const CORBA::PolicyType BIDIRECTIONAL_POLICY_TYPE _init_in_decl_( = 37 );

OMNIORB_DECLARE_POLICY_OBJECT(BidirectionalPolicy, 37)

_CORBA_MODULE_END



#endif // __BIDIRPOLICY_H__
