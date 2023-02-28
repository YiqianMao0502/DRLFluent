# -*- python -*-
#                           Package   : omniidl
# util.py                   Created on: 1999/11/2
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
#   General utility functions designed for the C++ backend

"""General utility functions used by the C++ backend"""

from omniidl_be.cxx import config

import sys

try:
    import traceback
    have_traceback = 1
except:
    have_traceback = 0


## Fatal error handling function ##################################
##
def fatalError(explanation):
    if config.state['Debug']:
        print("omniidl: fatalError occurred, in debug mode.")
        for line in explanation.split("\n"):
            print(">> " + line)

        if have_traceback:
            print("Stack:")
            print("-------------------------")
            traceback.print_stack()
            print("Exception:")
            print("-------------------------")
            traceback.print_exc()
        sys.exit(1)
    
    lines = explanation.split("\n")
    lines = [ "Fatal error in C++ backend", "" ] + lines

    for line in lines:
        sys.stderr.write("omniidl: %s\n" % line)

    sys.stderr.write("""\

For more information (mailing list archives, bug reports etc.) please visit
the webpage:

  http://omniorb.sourceforge.net/

""")
    sys.exit(1)

# Called whenever an unsupported IDL construct is found in the input
# (necessary because the front end supports all the new CORBA 2.3
# constructs whereas the ORB and correspondingly this backend does not)
def unsupportedIDL():
    e = """\
Unsupported IDL construct encountered in input.
"""
    fatalError(e)
    

def setify(lst):
    """
    setify(lst) -- return list with each item only appearing once
    """
    new_set = []
    for x in lst:
        if x not in new_set:
            new_set.append(x)

    return new_set
