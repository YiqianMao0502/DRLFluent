# -*- python -*-
#                           Package   : omniidl
# main.py                   Created on: 1999/11/12
#			    Author1   : David Scott (djs)
#                           Author2   : Duncan Grisby (dgrisby)
#
#  Copyright (C) 2004-2011 Apasphere Ltd.
#  Copyright (C) 1999 AT&T Laboratories Cambridge
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

"""Produce the main dynamic skeleton definitions"""

from omniidl_be.cxx import ast, id, output, types, config, skutil
from omniidl_be.cxx.dynskel import template

import sys
self = sys.modules[__name__]

stream = None

def init(stream):
    self.stream = stream
    return self


# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

# ------------------------------------

def visitModule(node):
    for n in node.definitions():
        n.accept(self)

# -----------------------------------

def visitConst(node):
    pass

# -----------------------------------

def visitInterface(node):
    for n in node.declarations():
        n.accept(self)

    scopedName = id.Name(node.scopedName())
    fqname     = scopedName.fullyQualify()
    guard_name = scopedName.guard()
    tc_name    = scopedName.prefix("_tc_").fullyQualify()
    prefix     = config.state['Private Prefix']

    if node.abstract():
        stream.out(template.abstract_interface,
                   guard_name = guard_name,
                   fqname = fqname, tc_name = tc_name,
                   private_prefix = prefix)
    else:
        stream.out(template.interface,
                   guard_name = guard_name,
                   fqname = fqname, tc_name = tc_name,
                   private_prefix = prefix)


def visitForward(node):
    pass

# -----------------------------------

def visitEnum(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    stream.out(template.enum,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)


# -----------------------------------

def visitStruct(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    # output code for constructed members (eg nested structs)
    for m in node.members():
        memberType = m.memberType()
        if m.constrType():
            memberType.decl().accept(self)

    stream.out(template.struct,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)

def visitStructForward(node):
    pass


# -----------------------------------

def visitException(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    # output code for constructed members (eg nested structs)
    for m in node.members():
        memberType = m.memberType()
        if m.constrType():
            memberType.decl().accept(self)

    stream.out(template.exception,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)

# -----------------------------------

def visitUnion(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    if node.constrType():
        node.switchType().decl().accept(self)

    for n in node.cases():
        if n.constrType():
            n.caseType().decl().accept(self)

    stream.out(template.union,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)

def visitUnionForward(node):
    pass

# -----------------------------------


def visitTypedef(node):
    if node.constrType():
        node.aliasType().decl().accept(self)

    aliasType = types.Type(node.aliasType())
    prefix    = config.state['Private Prefix']

    for d in node.declarators():
        scopedName = id.Name(d.scopedName())
        guard_name = scopedName.guard()
        fqname     = scopedName.fullyQualify()

        if d.sizes():
            # Array
            marshal = output.StringStream()
            skutil.marshall(marshal, None,
                            aliasType, d, "_a", "_s")

            unmarshal = output.StringStream()
            skutil.unmarshall(unmarshal, None,
                              aliasType, d, "_a", "_s")

            stream.out(template.array,
                       guard_name     = guard_name,
                       fqname         = fqname,
                       marshal        = marshal,
                       unmarshal      = unmarshal,
                       private_prefix = prefix)

        elif aliasType.sequence():
            stream.out(template.sequence,
                       guard_name     = guard_name,
                       fqname         = fqname,
                       private_prefix = prefix)


def visitValueForward(node):
    pass


def visitValue(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    for d in node.declarations():
        d.accept(self)
    for s in node.statemembers():
        memberType = s.memberType()
        if s.constrType():
            memberType.decl().accept(self)

    stream.out(template.value,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)


def visitValueBox(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    stream.out(template.value,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)


def visitValueAbs(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    for d in node.declarations():
        d.accept(self)

    stream.out(template.value,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)
