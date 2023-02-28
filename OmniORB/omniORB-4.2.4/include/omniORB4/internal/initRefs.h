// -*- Mode: C++; -*-
//                            Package   : omniORB2
// initRefs.h                 Created on: 20/08/98
//                            Author    : Sai-Lai Lo
//
//    Copyright (C) 1996-2000 AT&T Laboratories Cambridge
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

#ifndef __INITREFS_H__
#define __INITREFS_H__

#include <omniORB4/bootstrap.hh>

OMNI_NAMESPACE_BEGIN(omni)

class CORBA_InitialReferences_i;


class omniInitialReferences {
public:
  static CORBA::Boolean setFromFile(const char* identifier, const char* uri);
  static CORBA::Boolean setFromArgs(const char* identifier, const char* uri);
  // Set initial references from the configuration file and the
  // command line arguments respectively. Return true if the uri looks
  // syntactically valid, false if it is definitely invalid.

  static void setFromORB(const char* identifier, CORBA::Object_ptr obj);
  // Implementation of CORBA 2.5 ORB::register_initial_reference().

  static void setDefaultInitRefFromFile(const char* defInit);
  static void setDefaultInitRefFromArgs(const char* defInit);
  // Default string set by -ORBDefaultInitRef

  static CORBA::Object_ptr resolve(const char*  identifier,
				   unsigned int cycles = 0);
  // Real implementation of ORB::resolve_initial_references(). cycles
  // is used to count recursive calls within stringToObject, and bail
  // out if we loop too much. Responsible for returning pseudo objects
  // (like "RootPOA") as well as normal CORBA objects.

  static CORBA::ORB::ObjectIdList* list();
  // Real implementation of ORB::list_initial_services().


  typedef CORBA::Object_ptr (*pseudoObj_fn)();
  static void registerPseudoObjFn(const char* identifier, pseudoObj_fn fn);
  // Function to register a pseudo object. If resolve() is called with
  // the given identifier, the function is called. The registered
  // function must return a suitable pseudo object when called, and
  // must be thread safe. The identifier string must exist for the
  // lifetime of the initRefs module.
  //  This function is NOT thread safe.


  // Deprecated INIT bootagent functions:

  static void remFromFile(const char* identifier);
  // Remove the specified identifier from the file list. Used to
  // remove NameService and InterfaceRepository if -ORBInitialHost is
  // given on the command line.

  static void remFromArgs(const char* identifier);
  // Not used, just here for symmetry

  static void initialise_bootstrap_agentImpl();

  static int invoke_bootstrap_agentImpl(omniCallHandle&);
  // Returns 0 if there is no bootstrap agent.  May throw
  // the usual exceptions for an object invocation...

  static int is_bootstrap_agentImpl_initialised();
  // Returns true if a boostrap agent exists.

  static void initialise_bootstrap_agent(const char* host, CORBA::UShort port);
};

OMNI_NAMESPACE_END(omni)

#endif  
