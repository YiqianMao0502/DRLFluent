# -*- python -*-
#                           Package   : omniidl
# __init__.py               Created on: 1999/11/3
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
#   Entrypoint to header generation code

# output generation
import omniidl_be.cxx.header.opers
import omniidl_be.cxx.header.poa
import omniidl_be.cxx.header.obv
import omniidl_be.cxx.header.tie
import omniidl_be.cxx.header.forward
import omniidl_be.cxx.header.marshal
import omniidl_be.cxx.header.defs

from omniidl_be.cxx.header import template
from omniidl_be.cxx import config, output, ast, id

import os.path


def header(stream, filename):
    stream.out(template.header,
               program      = config.state['Program Name'],
               library      = config.state['Library Version'],
               guard_prefix = config.state['GuardPrefix'],
               guard        = filename)

def footer(stream):
    stream.out(template.footer)

def defs_fragment(stream, tree):
    """Creates the defs fragment only"""
    filename = config.state['Basename'] + config.state['_DEFS Fragment']
    header(stream, filename)

    # generate the header definitions
    forward = omniidl_be.cxx.header.forward.init(stream)
    tree.accept(forward)
    
    defs = omniidl_be.cxx.header.defs.init(stream)
    tree.accept(defs)

    footer(stream)

def opers_fragment(stream, tree):
    """Creates the opers fragment only"""
    filename = config.state['Basename'] + config.state['_OPERS Fragment']
    header(stream, filename)

    opers = omniidl_be.cxx.header.opers.init(stream)
    tree.accept(opers)

    marshal = omniidl_be.cxx.header.marshal.init(stream)
    tree.accept(marshal)

    footer(stream)

def poa_fragment(stream, tree):
    """Creates the poa fragment only"""
    filename = config.state['Basename'] + config.state['_POA Fragment']
    header(stream, filename)

    poa = omniidl_be.cxx.header.poa.init(stream)
    tree.accept(poa)

    footer(stream)

def monolithic(stream, tree):
    """Creates one large header with all definitions inside"""

    guard = id.Name([config.state['Basename']]).guard()

    header(stream, guard)

    # Extra DLL include stuff?
    if config.state['DLLIncludes']:
        sub_include_pre  = output.StringStream()
        sub_include_post = output.StringStream()
        sub_include_pre .out(template.sub_include_pre,  guard=guard)
        sub_include_post.out(template.sub_include_post, guard=guard)
    else:
        sub_include_pre  = ""
        sub_include_post = ""

    # Add in any direct C++ from toplevel pragma if present
    cxx_direct_include = []
    directive = "hh "
    for pragma in tree.pragmas():
        # ignore all pragmas but those in the main file
        if pragma.file() != tree.file(): continue
        
        if pragma.text()[:len(directive)] == directive:
            cxx_direct_include.append(pragma.text()[len(directive):])
    
    includes = output.StringStream()

    # produce #includes for all files included by the IDL
    for include in ast.includes():
        # skip the main file
        if ast.mainFile() == include:
            continue
        
        # the old C++ BE makes orb.idl a special case
        # (might now be a redundant test)
        
        # dirname  == directory containing the include file
        # filename == extensionless filename
        # ext      == extension (typically .idl)
        (root, ext) = os.path.splitext(include)
        (dirname, filename) = os.path.split(root)

        # Name for external include guard. Always use the same suffix,
        # rather than taking suffix from the config.
        guardname = id.Name([filename]).guard() + "_hh"

        if config.state['Keep Include Path']:
            filename = root

        cxx_include = filename + config.state['HH Suffix']

        if config.state['Use Quotes']:
            cxx_include = '"' + cxx_include + '"'
        else:
            cxx_include = "<" + cxx_include + ">"
            
        includes.out(template.main_include,
                     guardname    = guardname,
                     guard_prefix = config.state['GuardPrefix'],
                     filename     = cxx_include)

    # generate the header definitions
    def forward_dec(stream = stream, tree = tree):
        forward = omniidl_be.cxx.header.forward.init(stream)
        tree.accept(forward)

    def main_defs(stream = stream, tree = tree):
        defs = omniidl_be.cxx.header.defs.init(stream)
        tree.accept(defs)

    def main_poa(stream = stream, tree = tree):
        # includes inline (non-flat) tie templates
        poa = omniidl_be.cxx.header.poa.init(stream)
        tree.accept(poa)

    def main_obv(stream = stream, tree = tree):
        obv = omniidl_be.cxx.header.obv.init(stream)
        tree.accept(obv)

    def other_tie(stream = stream, tree = tree):
        if config.state['Normal Tie'] and config.state['BOA Skeletons']:
            tie = omniidl_be.cxx.header.tie.BOATieTemplates(stream)
            tree.accept(tie)
        
        if config.state['Flattened Tie']:
            tie = omniidl_be.cxx.header.tie.FlatTieTemplates(stream)
            tree.accept(tie)

    def main_opers(stream = stream, tree = tree):
        opers = omniidl_be.cxx.header.opers.init(stream)
        tree.accept(opers)
        
    def main_marshal(stream = stream, tree = tree):
        marshal = omniidl_be.cxx.header.marshal.init(stream)
        tree.accept(marshal)

    # other stuff
    stream.out(template.main,
               cxx_direct_include = "\n".join(cxx_direct_include),
               includes = includes,
               forward_declarations = forward_dec,
               defs = main_defs,
               poa = main_poa,
               obv = main_obv,
               other_tie = other_tie,
               operators = main_opers,
               marshalling = main_marshal,
               guard = guard,
               sub_include_pre = sub_include_pre,
               sub_include_post = sub_include_post)


def run(tree):
    if config.state['Fragment']:
        # build the defs file
        defs_filename = config.state['Basename']         +\
                        config.state['_DEFS Fragment']   +\
                        config.state['HH Suffix']
        defs_stream = output.Stream(output.createFile(defs_filename), 2)
        defs_fragment(defs_stream, tree)

        # build the opers file
        opers_filename = config.state['Basename']        +\
                         config.state['_OPERS Fragment'] +\
                         config.state['HH Suffix']
        opers_stream = output.Stream(output.createFile(opers_filename), 2)
        opers_fragment(opers_stream, tree)

        # build the poa file
        poa_filename = config.state['Basename']          +\
                       config.state['_POA Fragment']     +\
                       config.state['HH Suffix']
        poa_stream = output.Stream(output.createFile(poa_filename), 2)
        poa_fragment(poa_stream, tree)

        defs_stream.close()
        opers_stream.close()
        poa_stream.close() 

    else:
        # build the full header file
        header_filename = config.state['Basename']       +\
                          config.state['HH Suffix']
        stream = output.Stream(output.createFile(header_filename), 2)
        # generate one big chunk of header
        monolithic(stream, tree)

        stream.close()
