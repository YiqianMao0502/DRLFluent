# -*- python -*-
#                           Package   : omniidl
# forward.py                Created on: 1999/12/1
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
#   Produce ancillary forward declarations for the header file

"""Produce ancillary forward declarations for the header file"""

from omniidl_be.cxx import id, ast

import sys
self = sys.modules[__name__]

def init(_):
    return self


# Control arrives here
#
def visitAST(node):
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    # again check what happens here wrt reopening modules spanning
    # multiple files
    for n in node.definitions():
        n.accept(self)


def visitStruct(node):
    for n in node.members():
        n.accept(self)

def visitStructForward(node):
    pass

def visitUnion(node):
    for n in node.cases():
        if n.constrType():
            n.caseType().decl().accept(self)
            
def visitUnionForward(node):
    pass

def visitInterface(node):
    for n in node.declarations():
        n.accept(self)



def visitException(node):
    for n in node.members():
        n.accept(self)
        
def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    pass


def visitTypedef(node):
    if node.constrType():
        node.aliasType().decl().accept(self)
    
        
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass

def visitValueForward(node):
    pass
def visitValue(node):
    pass
def visitValueAbs(node):
    pass
def visitValueBox(node):
    pass
