# -*- python -*-
#                           Package   : omniidl
# marshal.py                Created on: 1999/12/1
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
#   Produce the main header alignment and marshal function definitions
#   for the C++ backend

"""Produce the main header alignment and marshal function definitions
  for the C++ backend"""

from omniidl_be.cxx import ast, id
from omniidl_be.cxx.header import template

import sys
self = sys.modules[__name__]

stream = None

def init(s):
    global stream
    stream = s
    return self

# Control arrives here
#
def visitAST(node):
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    for n in node.definitions():
        n.accept(self)

def visitStruct(node):
    for n in node.members():
        n.accept(self)

def visitStructForward(node):
    pass

def visitUnion(node):
    pass

def visitUnionForward(node):
    pass

def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    pass

def visitInterface(node):
    # interfaces act as containers for other declarations
    # output their operators here
    for d in node.declarations():
        d.accept(self)

    name = id.Name(node.scopedName())
    cxx_name = name.fullyQualify()

    if node.local():
        stream.out(template.local_interface_marshal_forward,
                   name = cxx_name)

    elif node.abstract():
        stream.out(template.abstract_interface_marshal_forward,
                   name = cxx_name)
    else:
        stream.out(template.interface_marshal_forward,
                   name = cxx_name)

def visitTypedef(node):
    pass
        
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitException(node):
    pass
def visitValue(node):
    pass
def visitValueForward(node):
    pass
def visitValueAbs(node):
    pass
def visitValueBox(node):
    pass
