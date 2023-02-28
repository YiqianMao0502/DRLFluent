# -*- python -*-
#                           Package   : omniidl
# poa.py                    Created on: 1999/11/12
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

"""Produce the main POA skeleton definitions"""

from omniidl_be.cxx import ast, id
from omniidl_be.cxx.skel import template

import sys
self = sys.modules[__name__]

stream = None

def init(_stream):
    global stream, __nested
    stream = _stream
    __nested = 0
    return self

def POA_prefix():
    if not self.__nested:
        return "POA_"
    return ""

# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    for n in node.definitions():
        nested = self.__nested
        self.__nested = 1
        
        n.accept(self)

        self.__nested = nested


def visitInterface(node):
    if node.local():
        return
    
    name = id.mapID(node.identifier())
    fqname = id.Name(node.scopedName()).fullyQualify()
    stream.out(template.interface_POA,
               POA_prefix = POA_prefix(),
               name = name,
               fqname = fqname)

def visitTypedef(node):
    pass

def visitEnum(node):
    pass
def visitStruct(node):
    pass
def visitStructForward(node):
    pass
def visitUnion(node):
    pass
def visitUnionForward(node):
    pass
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitMember(node):
    pass
def visitException(node):
    pass


def visitValue(node):
    pass
def visitValueForward(node):
    pass
def visitValueBox(node):
    pass
def visitValueAbs(node):
    pass
