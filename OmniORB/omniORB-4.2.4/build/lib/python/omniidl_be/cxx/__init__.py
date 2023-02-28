# -*- python -*-
#                           Package   : omniidl
# __init__.py               Created on: 1999/11/3
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2002-2011 Apasphere Ltd
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
#   Entrypoint to the C++ backend

# From http://www-i3.informatik.rwth-aachen.de/funny/babbage.html:
# ...
# C. A. R. Hoare, in his 1980 ACM Turing Award lecture, told of two
# ways of constructing a software design: "One way is to make it so
# simple that there are obviously no deficiencies and the other way
# is to make it so complicated that there are no obvious deficiencies." 
#

## Import Output generation functions ###################################
##
from omniidl_be.cxx import header, skel, dynskel, impl, util

## Utility functions
from omniidl_be.cxx import id, config, ast, output, support, descriptor
import os.path

## Resolve recursive imports
from omniidl_be.cxx import call, iface


cpp_args = ["-D__OMNIIDL_CXX__"]
usage_string = """\
  -Wbh=<suffix>     Specify suffix for generated header files
  -Wbs=<suffix>     Specify suffix for generated stub files
  -Wbd=<suffix>     Specify suffix for generated dynamic files

  -Wba              Generate code for TypeCode and Any
  -Wbtp             Generate 'tie' implementation skeletons
  -Wbtf             Generate flattened 'tie' implementation skeletons
  -Wbami            Generate code for AMI
  -Wbexample        Generate example implementation code

  -Wbinline         Generate code for #included files inline with the main file
  -Wbuse-quotes     Use quotes in #includes: "foo" rather than <foo>
  -Wbkeep-inc-path  Preserve IDL #include path in header #includes

  -Wbvirtual-objref Use virtual functions in object references
  -Wbimpl-mapping   Use 'impl' mapping for object reference methods

  -Wbsplice-modules Splice together multiply opened modules into one 
  -WbBOA            Generate BOA compatible skeletons
  -Wbold            Generate old CORBA 2.1 signatures for skeletons
  -Wbold-prefix     Map C++ reserved words with prefix _ instead of _cxx_

  -Wbdll-includes   Extra support for #included IDL in DLLs
  -Wbguard-prefix   Prefix for include guards in generated headers
  -WbF              Generate code fragments (for experts only)
"""

def process_args(args):
    for arg in args:
        if arg == "a":
            config.state['Typecode']          = 1

        elif arg == "tp":
            config.state['Normal Tie']        = 1

        elif arg == "tf":
            config.state['Flattened Tie']     = 1

        elif arg == "splice-modules":
            config.state['Splice Modules']    = 1

        elif arg == "example":
            config.state['Example Code']      = 1

        elif arg == "F":
            config.state['Fragment']          = 1

        elif arg == "BOA":
            config.state['BOA Skeletons']     = 1

        elif arg == "old":
            config.state['Old Signatures']    = 1

        elif arg == "old-prefix" or arg == "old_prefix":
            config.state['Reserved Prefix']   = "_"

        elif arg == "keep-inc-path" or arg == "keep_inc_path":
            config.state['Keep Include Path'] = 1

        elif arg == "use-quotes" or arg == "use_quotes":
            config.state['Use Quotes']        = 1

        elif arg == "virtual-objref" or arg == "virtual_objref":
            config.state['Virtual Objref Methods'] = 1

        elif arg == "impl-mapping" or arg == "impl_mapping":
            config.state['Impl Mapping'] = 1

        elif arg == "debug":
            config.state['Debug']             = 1

        elif arg[:2] == "h=":
            config.state['HH Suffix']         = arg[2:]

        elif arg[:2] == "s=":
            config.state['SK Suffix']         = arg[2:]

        elif arg[:2] == "d=":
            config.state['DYNSK Suffix']      = arg[2:]

        elif arg[:2] == "e=":
            config.state['IMPL Suffix']       = arg[2:]

        elif arg == "inline":
            config.state['Inline Includes']   = 1

        elif arg == "shortcut":
            config.state['Shortcut']          = 1

        elif arg[:9] == "shortcut=":
            if arg[9:] == "refcount":
                config.state['Shortcut']      = 2
            elif arg[9:] == "simple":
                config.state['Shortcut']      = 1
            else:
                util.fatalError('Unknown shortcut option "%s"' % arg[9:])

        elif arg == "dll-includes" or arg == "dll_includes":
            config.state['DLLIncludes']       = 1

        elif arg[:13] == "guard-prefix=" or arg[:13] == "guard_prefix=":
            config.state['GuardPrefix']       = arg[14:]

        elif arg == "ami":
            config.state['AMI']               = 1

        else:
            util.fatalError('Argument "%s" is unknown' % arg)

run_before = 0

def run(tree, args):
    """Entrypoint to the C++ backend"""

    global run_before

    if run_before:
        util.fatalError("Sorry, the C++ backend cannot process more "
                        "than one IDL file at a time.")
    run_before = 1

    # Initialise modules that would otherwise contain circular imports
    call.init()
    iface.init()

    dirname, filename = os.path.split(tree.file())
    basename,ext      = os.path.splitext(filename)
    config.state['Basename']  = basename
    config.state['Directory'] = dirname

    process_args(args)

    try:
        # Check the input tree only contains stuff we understand
        support.checkIDL(tree)

        # Add AMI implied IDL if required
        if config.state['AMI']:
            from omniidl_be import ami
            tree.accept(ami.AMIVisitor())
        
        # initialise the handy ast module
        ast.__init__(tree)

        # Initialise the descriptor generating code
        descriptor.__init__(tree)

        # Build the map of AST nodes to Environments
        tree.accept(id.WalkTree())

        header.run(tree)
        
        skel.run(tree)
        
        # if we're generating code for Typecodes and Any then
        # we need to create the DynSK.cc file
        if config.state['Typecode']:
            dynskel.run(tree)

        if config.state['Example Code']:
            impl.run(tree)

    except SystemExit:
        # fatalError function throws SystemExit exception
        # delete all possibly partial output files
        for file in output.listAllCreatedFiles():
            os.unlink(file)
        
        raise
