# -*- python -*-
#                           Package   : omniidl
# template.py               Created on: 2000/02/13
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2003-2011 Apasphere Ltd
#    Copyright (C) 1999 AT&T Laboratories Cambridge
#
#  This file is part of omniidl.
#
#  omniidl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see http://www.gnu.org/licenses/
#
# Description:
#   
#   Example interface implementation templates


## Example code to actually implement an interface
##
interface_def = """\
//
// Example class implementing IDL interface @fq_name@
//
class @impl_fqname@ : public @fq_POA_name@ {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  //virtual ~@impl_name@();

public:
  // standard constructor
  @impl_name@();
  virtual ~@impl_name@();

  // methods corresponding to defined IDL attributes and operations
  @operations@
};
"""

interface_code = """\
//
// Example implementation code for IDL interface '@fqname@'
//
@impl_fqname@::@impl_name@(){
  // add extra constructor code here
}
@impl_fqname@::~@impl_name@(){
  // add extra destructor code here
}

// Methods corresponding to IDL attributes and operations
@operations@

// End of example implementation code
"""

interface_ior = """\
{
  // IDL interface: @fqname@
  CORBA::Object_var ref = @inst_name@->_this();
  CORBA::String_var sior(orb->object_to_string(ref));
  std::cout << "IDL object @fqname@ IOR = '" << (char*)sior << "'" << std::endl;
}
"""

main = """\
//
// Example code for implementing IDL interfaces in file @file@
//

#include <iostream>
#include <@idl_hh@>

@interfaces@

int main(int argc, char** argv)
{
  try {
    // Initialise the ORB.
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    // Obtain a reference to the root POA.
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    // We allocate the objects on the heap.  Since these are reference
    // counted objects, they will be deleted by the POA when they are no
    // longer needed.
    @allocate_objects@

    // Activate the objects.  This tells the POA that the objects are
    // ready to accept requests.
    @activate_objects@

    // Obtain a reference to each object and output the stringified
    // IOR to stdout
    @output_references@

    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    orb->run();
    orb->destroy();
  }
  catch(CORBA::TRANSIENT&) {
    std::cerr << "Caught system exception TRANSIENT -- unable to contact the "
              << "server." << std::endl;
  }
  catch(CORBA::SystemException& ex) {
    std::cerr << "Caught a CORBA::" << ex._name() << std::endl;
  }
  catch(CORBA::Exception& ex) {
    std::cerr << "Caught CORBA::Exception: " << ex._name() << std::endl;
  }
  return 0;
}
"""
