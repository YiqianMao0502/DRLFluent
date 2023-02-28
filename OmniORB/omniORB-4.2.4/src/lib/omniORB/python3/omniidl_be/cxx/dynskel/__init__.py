# -*- python -*-
#                           Package   : omniidl
# __init__.py               Created on: 1999/11/11
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
#   Entrypoint to dynamic skeleton generation code

# All you really need to know for the moment is that the universe
# is a lot more complicated than you might think, even if you
# start from a position of thinking it's pretty damn complicated
# in the first place.       Douglas Adams, "Mostly Harmless"
#

# -----------------------------
# Configuration data
from omniidl_be.cxx import config

# -----------------------------
# Utility functions
from omniidl_be.cxx import output
from omniidl_be.cxx.dynskel import typecode
from omniidl_be.cxx.dynskel import main
from omniidl_be.cxx.dynskel import template

def generate(stream, tree):
    stream.out(template.header_comment,
               program = config.state['Program Name'],
               library = config.state['Library Version'])

    if config.state['Fragment']:
        stream.out(template.fragment_header,
                   prefix = config.state['Private Prefix'])
    else:
        stream.out(template.header,
                   basename = config.state['Basename'],
                   hh = config.state['HH Suffix'],
                   library = config.state['Library Version'],
                   prefix = config.state['Private Prefix'])

    typecode.init(stream)
    tree.accept(typecode)

    main.init(stream)
    tree.accept(main)

def run(tree):
    # create somewhere to put the output
    header_filename = config.state['Basename'] +\
                      config.state['DYNSK Suffix']
    
    stream = output.Stream(output.createFile(header_filename), 2)

    generate(stream, tree)
    stream.close()
