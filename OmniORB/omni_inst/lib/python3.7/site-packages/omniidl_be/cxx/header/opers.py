# -*- python -*-
#                           Package   : omniidl
# opers.py                  Created on: 1999/11/4
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

"""Produce the main header operator definitions"""

from omniidl_be.cxx import config, id, types, ast
from omniidl_be.cxx.header import template

import sys
self = sys.modules[__name__]

stream = None

def init(_stream):
    global stream
    stream = _stream
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

    # TypeCode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()

        stream.out(template.any_struct,
                   fqname = fqname)

def visitStructForward(node):
    pass

def visitUnion(node):
    # deal with constructed switch type
    if node.constrType():
        node.switchType().decl().accept(self)

    # deal with constructed member types
    for n in node.cases():
        if n.constrType():
            n.caseType().decl().accept(self)
    
    # TypeCode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()
    
        stream.out(template.any_union,
                   fqname = fqname)

def visitUnionForward(node):
    pass

def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    cxx_fqname = id.Name(node.scopedName()).fullyQualify()
    last_item  = id.Name(node.enumerators()[-1].scopedName()).fullyQualify()

    stream.out(template.enum_operators,
               name = cxx_fqname,
               private_prefix = config.state['Private Prefix'],
               last_item = last_item)

    # Typecode and Any
    if config.state['Typecode']:
        stream.out(template.any_enum,
                   name = cxx_fqname)

def visitInterface(node):
    # interfaces act as containers for other declarations
    # output their operators here
    for d in node.declarations():
        d.accept(self)

    # Typecode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()
    
        stream.out(template.any_interface,
                   fqname = fqname)
        

def visitTypedef(node):
    aliasType = types.Type(node.aliasType())

    if node.constrType():
        aliasType.type().decl().accept(self)

    # don't need to do anything unless generating TypeCodes and Any
    if not config.state['Typecode']:
        return
    
    for d in node.declarators():
        decl_dims = d.sizes()
        fqname = id.Name(d.scopedName()).fullyQualify()

        array_declarator = decl_dims != []

        if array_declarator:
            stream.out(template.any_array_declarator,
                       fqname = fqname)
        # only need to generate these operators if the typedef
        # introduces a new sequence- they already exist for a simple
        # typedef. Hence aliasType rather than deref_aliasType.
        elif aliasType.sequence():
            stream.out(template.any_sequence,
                       fqname = fqname)
            
            
        
def visitForward(node):
    pass

def visitConst(node):
    pass

def visitDeclarator(node):
    pass

def visitException(node):
    for m in node.members():
        if m.constrType():
            m.memberType().decl().accept(self)
    
    # don't need to do anything unless generating TypeCodes and Any
    if not config.state['Typecode']:
        return

    fqname = id.Name(node.scopedName()).fullyQualify()
    stream.out(template.any_exception,
               fqname = fqname)


def visitValue(node):
    # Nested declarations
    for d in node.declarations():
        d.accept(self)

    # Typecode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()
    
        stream.out(template.any_value,
                   fqname = fqname)

def visitValueForward(node):
    pass

def visitValueAbs(node):
    # Nested declarations
    for d in node.declarations():
        d.accept(self)

    # Typecode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()
    
        stream.out(template.any_value,
                   fqname = fqname)

def visitValueBox(node):
    # Typecode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()
    
        stream.out(template.any_value,
                   fqname = fqname)
