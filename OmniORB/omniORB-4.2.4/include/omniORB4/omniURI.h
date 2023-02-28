// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniURI.h                  Created on: 2000/04/03
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2005-2006 Apasphere Ltd
//    Copyright (C) 2000      AT&T Laboratories Cambridge
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
//      Parsing for object reference URIs
//	*** PROPRIETARY INTERFACE ***
//

#ifndef _omniURI_h_
#define _omniURI_h_


#include <omniORB4/CORBA.h>
#include <omniORB4/Naming.hh>

OMNI_NAMESPACE_BEGIN(omni)

class omniURI {
public:

  // The omniURI class contains all functions which manipulate object
  // URIs, and convert them to-and-from CORBA::Objects.

  static char* buildURI(const char*   prefix,
			const char*   host,
			CORBA::UShort port);
  // Build a URI with the prefix, containing the host and port,
  // properly escaping the host if need be.

  static char* extractHostPort(const char*    addr,
			       CORBA::UShort& port,
			       const char**   rest = 0);
  // Extract host and port from the part of a URI containing the
  // address. If rest is non-zero, the pointer is set to the address
  // of the character following the port number, otherwise anything
  // after the port number renders the URI invalid. Returns zero if
  // the address is invalid.

  static char* extractHostPortRange(const char*    addr,
				    CORBA::UShort& port_min,
				    CORBA::UShort& port_max);
  // Extract host and port range from the part of a URI containing the
  // address. Accepts a port range in the form min-max. Returns zero
  // if the address is invalid.

  static CORBA::Boolean validHostPort(const char* addr);
  // True if addr is a valid host:port; false otherwise.

  static CORBA::Boolean validHostPortRange(const char* addr);
  // True if addr is a valid host:port_min-port_max; false otherwise.


  static char* objectToString(CORBA::Object_ptr obj);
  // Return a stringified IOR for the given object reference.
  //  Does not throw any exceptions.

  static CORBA::Object_ptr stringToObject(const char*  uri,
					  unsigned int cycles = 0);
  // Converts the given URI to an object reference. Currently supports
  // IOR:, corbaloc: and corbaname: URIs.
  //
  // cycles is used to count recursive calls to stringToObject, and
  // bail out if we loop too much.
  //
  // Throws CORBA::MARSHAL and CORBA::BAD_PARAM

  static CORBA::Boolean uriSyntaxIsValid(const char* uri);
  // Return true if the given URI is syntactically valid, false
  // otherwise.
  //  Does not throw any exceptions.


  // URIs are parsed and validated by objects derived from URIHandler
  class URIHandler {
  public:
    virtual CORBA::Boolean supports(const char* uri) = 0;
    // Returns true if the handler can parse the URI, false otherwise
    //  Does not throw any exceptions.

    virtual CORBA::Object_ptr toObject(const char* uri,
				       unsigned int cycles) = 0;
    // Convert the given URI to an object reference. If the processing
    // involves a (potential) recursive call to stringToObject(),
    // cycles should be incremented.
    //  Throws CORBA system exceptions

    virtual CORBA::Boolean syntaxIsValid(const char* uri) = 0;
    // Return true if the URI is syntactically valid.
    //  Does not throw any exceptions.

    virtual ~URIHandler();
  };

  // The following functions implement the stringified name operations
  // of CosNaming::NamingContextExt. They are available here to avoid
  // the overhead of remote calls just to do some string bashing.

  static CosNaming::Name* stringToName(const char* sname);
  // Convert a stringified CosNaming::Name into a CosNaming::Name. The
  // caller is responsible for freeing it.

  static char* nameToString(const CosNaming::Name& name);
  // Convert the CosNaming::Name into a stringified name. Throws
  // CosNaming::NamingContext::InvalidName if the name sequence has
  // zero length.

  static char* addrAndNameToURI(const char* addr, const char* sn);
  // Convert the given address and stringified name into a corbaname:
  // URI. Throws CosNaming::NamingContextExt::InvalidAddress if the
  // address syntax is invalid; CosNaming::NamingContext::InvalidName
  // if the name syntax is invalid. It does not check if the name
  // actually exists in the specified naming service.
};

OMNI_NAMESPACE_END(omni)

#endif // _omniURI_h_
