// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixTransportImpl.cc       Created on: 6 Aug 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2013 Apasphere Ltd
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

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <objectAdapter.h>
#include <SocketCollection.h>
#include <orbParameters.h>
#include <unix/unixConnection.h>
#include <unix/unixAddress.h>
#include <unix/unixEndpoint.h>
#include <unix/unixTransportImpl.h>
#include <omniORB4/linkHacks.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>


OMNI_FORCE_LINK(unixAddress);
OMNI_FORCE_LINK(unixConnection);
OMNI_FORCE_LINK(unixEndpoint);
OMNI_FORCE_LINK(unixActive);

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixTransportImpl);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
unixTransportImpl::unixTransportImpl() : giopTransportImpl("giop:unix") {
}

/////////////////////////////////////////////////////////////////////////
unixTransportImpl::~unixTransportImpl() {
}

/////////////////////////////////////////////////////////////////////////
giopEndpoint*
unixTransportImpl::toEndpoint(const char* param) {

  if (!param)  return 0;

  CORBA::String_var dname;
  CORBA::String_var fname;
  struct stat sb;

  if (strlen(param) == 0) {
    param = orbParameters::unixTransportDirectory;
    
    char* p = (char*) strchr(param,'%');
    if (p && *(p+1) == 'u') {
      struct passwd* pw = getpwuid(getuid());
      if (!pw) {
	if (omniORB::trace(1)) {
	  omniORB::logger l;	
	  l << "Error: cannot get password entry of uid: " << getuid() << "\n";
	}
	return 0;
      }
      CORBA::String_var format = param;
      p = (char*) strchr(format,'%');
      *(p+1) = 's';
      dname = CORBA::string_alloc(strlen(format)+strlen(pw->pw_name));
      sprintf(dname,format,pw->pw_name);
      param = dname;
    }

    while (1) {
      if (stat(param,&sb) == 0) {
        if (!S_ISDIR(sb.st_mode)) {
          if (omniORB::trace(1)) {
            omniORB::logger log;	
            log << "Error: " << param << " exists and is not a directory. "
                << "Please remove it and try again\n";
          }
          return 0;
        }
        break;
      }
      else {
        if (mkdir(param,0755) == 0)
          break;

        if (errno == EEXIST) // race with something else creating the directory
          continue;

	if (omniORB::trace(1)) {
	  omniORB::logger log;	
	  log << "Error: cannot create directory: " << param << "\n";
	}
	return 0;
      }
    }
  }

  if (stat(param,&sb) == 0 && S_ISDIR(sb.st_mode)) {
    const char* format = "%s/%09u-%09u";
    fname = CORBA::string_alloc(strlen(param)+24);

    unsigned long now_sec, now_nsec;
    omni_thread::get_time(&now_sec,&now_nsec);
    
    sprintf(fname,format,param,(unsigned int)getpid(),(unsigned int)now_sec);
    param = fname;
  }

  return (giopEndpoint*)(new unixEndpoint(param));
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixTransportImpl::isValid(const char* param) {

  if (!param || strlen(param) == 0) return 0;
  return 1;
}


/////////////////////////////////////////////////////////////////////////
giopAddress*
unixTransportImpl::toAddress(const char* param) {

  if (param) {
    return (giopAddress*)(new unixAddress(param));
  }
  else {
    return 0;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixTransportImpl::addToIOR(const char* param, IORPublish* eps) {

  if (param) {
    omniIOR::add_TAG_OMNIORB_UNIX_TRANS(param, eps);
    return 1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
const omnivector<const char*>* 
unixTransportImpl::getInterfaceAddress() {
  // There is no sensible interface address. Return an empty list.
  static omnivector<const char*> empty;
  return &empty;
}

const unixTransportImpl _the_unixTransportImpl;

OMNI_NAMESPACE_END(omni)
