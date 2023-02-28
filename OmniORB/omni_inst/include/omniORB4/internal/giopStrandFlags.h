// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopStrandFlags.h          Created on: 2006-06-16
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2006 Apasphere Ltd.
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
//    Flags values used in the giopStrand flags field. For use by
//    interceptors to maintain per-connection information.
//

#ifndef __GIOPSTRANDFLAGS_H__
#define __GIOPSTRANDFLAGS_H__


//
// Application specific space
//

#define GIOPSTRAND_APPLICATION_MASK 0xff000000
// The top 8 bits of the flags are reserved for application-specific
// use. Flags below will never be allocated in this space.


//
// Allocated flag bits
//

#define GIOPSTRAND_BIDIR (1 << 0)
// Strand supports bidirectional GIOP.

#define GIOPSTRAND_ENABLE_TRANSPORT_BATCHING (1 << 1)
// Normally omniORB sets connections to send data as soon as possible,
// e.g. with the TCP_NODELAY socket option. This flag means that
// message batching in the transport should be enabled.

#define GIOPSTRAND_HOLD_OPEN (1 << 2)
// If set, the connection is held open, rather than being scavenged
// when idle.

#define GIOPSTRAND_COMPRESSION (1 << 3)
// If set, the connection supports ZIOP compression.

#define GIOPSTRAND_CONNECTION_MANAGEMENT (1 << 4)
// If set, the connection is managed by the omniConnectionMgmt extension.


#endif // __GIOPSTRANDFLAGS_H__
