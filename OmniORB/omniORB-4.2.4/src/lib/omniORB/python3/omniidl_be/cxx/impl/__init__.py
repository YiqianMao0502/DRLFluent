# -*- python -*-
#                           Package   : omniidl
# __init__.py               Created on: 2000/02/03
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2011 Apasphere Ltd
#    Copyright (C) 2000 AT&T Laboratories Cambridge
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
#   Entrypoint to example implementation generation code

from omniidl_be.cxx import config, output
from omniidl_be.cxx.impl import main

def run(tree):
    hh_filename   = config.state['Basename'] + config.state['HH Suffix']
    idl_filename  = tree.file()
    impl_filename = config.state['Basename'] + config.state['IMPL Suffix']

    stream = output.Stream(output.createFile(impl_filename), 2)
    main.init(stream, idl_filename, hh_filename)

    main.run(tree)

    stream.close()
