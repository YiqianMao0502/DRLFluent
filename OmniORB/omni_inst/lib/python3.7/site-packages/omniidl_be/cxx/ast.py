# -*- python -*-
#                           Package   : omniidl
# ast.py                    Created on: 2000/8/10
#                           Author    : David Scott (djs)
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
#   Routines for manipulating the AST

"""Routines for mangipulating the AST"""

from omniidl import idlast, idltype, idlvisitor
from omniidl_be.cxx import util, config


# Global values used by functions in this module
_initialised = 0
_ast         = None
_includes    = None

def __init__(tree):
    assert isinstance(tree, idlast.AST)
    global _initialised, _ast

    _initialised = 1
    _ast         = tree

    walker = WalkTreeForIncludes()
    tree.accept(walker)

# Returns the main IDL filename
def mainFile():
    assert(_initialised)
    return _ast.file()

# Returns a list of all IDL files included from this one
def includes():
    assert(_initialised)
    return _includes

# Traverses the AST compiling the list of files #included at top level
# in the main IDL file. Marks all nodes for which code should be
# generated.

class WalkTreeForIncludes(idlvisitor.AstVisitor):

    def __init__(self):
        global _includes
        _includes = []

    def add(self, node):
        global _includes
        file = node.file()

        if file not in _includes and file != "<built in>":
            _includes.append(file)

    def visitAST(self, node):
        self.add(node)
        for d in node.declarations():
            if not config.state['Inline Includes']:
                self.add(d)
            if d.mainFile() or config.state['Inline Includes']:
                d.accept(self)

    def visitModule(self, node):
        node.cxx_generate = 1
        for n in node.definitions():
            n.accept(self)

    def visitInterface(self, node):
        node.cxx_generate = 1
        for n in node.contents():
            n.accept(self)

    def visitForward(self, node):
        node.cxx_generate = 1

    def visitConst(self, node):
        node.cxx_generate = 1

    def visitDeclarator(self, node):
        node.cxx_generate = 1

    def visitTypedef(self, node):
        node.cxx_generate = 1
        for n in node.declarators():
            n.accept(self)

        if node.constrType():
            node.aliasType().decl().accept(self)

    def visitMember(self, node):
        node.cxx_generate = 1
        for n in node.declarators():
            n.accept(self)

        if node.constrType():
            node.memberType().decl().accept(self)

    def visitStruct(self, node):
        node.cxx_generate = 1
        for n in node.members():
            n.accept(self)

    def visitStructForward(self, node):
        node.cxx_generate = 1

    def visitException(self, node):
        node.cxx_generate = 1
        for n in node.members():
            n.accept(self)

    def visitCaseLabel(self, node):
        node.cxx_generate = 1

    def visitUnionCase(self, node):
        node.cxx_generate = 1
        for n in node.labels():
            n.accept(self)

        if node.constrType():
            node.caseType().decl().accept(self)

    def visitUnion(self, node):
        node.cxx_generate = 1
        for n in node.cases():
            n.accept(self)

        if node.constrType():
            node.switchType().decl().accept(self)

    def visitUnionForward(self, node):
        node.cxx_generate = 1

    def visitEnumerator(self, node):
        node.cxx_generate = 1

    def visitEnum(self, node):
        node.cxx_generate = 1
        for n in node.enumerators():
            n.accept(self)

    def visitAttribute(self, node):
        node.cxx_generate = 1
        for n in node.declarators():
            n.accept(self)

    def visitParameter(self, node):
        node.cxx_generate = 1

    def visitOperation(self, node):
        node.cxx_generate = 1
        for n in node.parameters():
            n.accept(self)

    def visitNative(self, node):
        node.cxx_generate = 1

    def visitStateMember(self, node):
        node.cxx_generate = 1
        for n in node.declarators():
            n.accept(self)

        if node.constrType():
            node.memberType().decl().accept(self)

    def visitFactory(self, node):
        node.cxx_generate = 1
        for n in node.parameters():
            n.accept(self)

    def visitValueForward(self, node):
        node.cxx_generate = 1

    def visitValueBox(self, node):
        node.cxx_generate = 1

        if node.constrType():
            node.boxedType().decl().accept(self)

    def visitValueAbs(self, node):
        node.cxx_generate = 1
        for n in node.contents():
            n.accept(self)

    def visitValue(self, node):
        node.cxx_generate = 1
        for n in node.contents():
            n.accept(self)

def shouldGenerateCodeForDecl(decl):

    """Return true if full code should be generated for the specified Decl node"""
    return hasattr(decl, "cxx_generate")


#
# Union-related functions
#

# Returns the list of all non-default union case labels (ie those which
# have an explicit value)
def allCaseLabels(union):
    assert isinstance(union, idlast.Union)
    lst = []
    for case in union.cases():
        for label in case.labels():
            if not label.default():
                lst.append(label)
    return lst

# Returns the list of all case label values
def allCaseLabelValues(union):
    labels = allCaseLabels(union)
    return [x.value() for x in labels]

# Returns the default case of a union or None if it doesn't exist
def defaultCase(union):
    assert isinstance(union, idlast.Union)
    for case in union.cases():
        for label in case.labels():
            if label.default():
                return case
    return None

# Adds a new attribute to all the union cases:
#  isDefault: boolean, true if case is the default one
def markDefaultCase(union):
    assert isinstance(union, idlast.Union)
    for case in union.cases():
        case.isDefault = 0
        for label in case.labels():
            if label.default():
                case.isDefault = 1
                break

# Returns the default label of a case
def defaultLabel(case):
    assert isinstance(case, idlast.UnionCase)
    for label in case.labels():
        if label.default(): return label
    assert(0)


# from CORBA 2.3 spec 3.10.1.1 Integer types
SHORT_MIN     = - 2 ** 15
SHORT_MAX     =   2 ** 15 - 1
USHORT_MAX    =   2 ** 16 - 1

LONG_MIN      = - 2 ** 31
LONG_MAX      =   2 ** 31 - 1
ULONG_MAX     =   2 ** 32 - 1

LONGLONG_MIN  = - 2 ** 63
LONGLONG_MAX  =   2 ** 63 - 1
ULONGLONG_MAX =   2 ** 64 - 1

integer_type_ranges = {
    idltype.tk_short:     (SHORT_MIN,    SHORT_MAX),
    idltype.tk_ushort:    (0,            USHORT_MAX),
    idltype.tk_long:      (LONG_MIN,     LONG_MAX),
    idltype.tk_ulong:     (0,            ULONG_MAX),
    idltype.tk_longlong:  (LONGLONG_MIN, LONGLONG_MAX),
    idltype.tk_ulonglong: (0,            ULONGLONG_MAX)
    }

# Returns true if the case values represent all the possible values for
# the supplied type.
def exhaustiveMatch(type, values):
    type = type.deref()

    # Note that we can only enumerate values for the types:
    #  integers, chars, boolean, enumeration

    # Returns true if everything from low to high is in set
    def in_set(low, high, set):
        test = low
        while (test <= high):
            if test not in set: return 0
            test = test + 1
        return 1

    if type.integer():
        (low, high) = integer_type_ranges[type.kind()]
        return in_set(low, high, values)

    if type.char():
        return in_set(0, 255, list(map(ord, values)))

    if type.boolean():
        if (0 in values) and (1 in values): return 1
        return 0

    if type.enum():
        all_enums = type.type().decl().enumerators()
        for enum in all_enums:
            if enum not in values: return 0
        return 1

    assert 0


# Return the base AST node after following all the typedef chains
def remove_ast_typedefs(node):
    while isinstance(node, idlast.Declarator):
        node = node.alias().aliasType().decl()

    return node

def remove_ast_typedefs_and_forwards(node):
    while isinstance(node, idlast.Declarator):
        node = node.alias().aliasType().decl()

    if isinstance(node, idlast.Forward):
        node = node.fullDecl()

    return node

# Returns _all_ inherited interfaces by performing a breadth-first search
# of the inheritance graph.
def allInherits(interface):
    assert isinstance(interface, idlast.Interface)
    
    # It is possible to inherit from an interface through a chain of
    # typedef nodes. These need to be removed...

    # breadth first search
    def bfs(current, bfs):
        if current == []:
            return []
        
        # extend search one level deeper than current
        next = []
        for c in current:
            next.extend(list(map(remove_ast_typedefs_and_forwards, c.inherits())))

        return next + bfs(next, bfs)

    start = list(map(remove_ast_typedefs_and_forwards, interface.inherits()))
    lst   = start + bfs(start, bfs)

    return util.setify(lst)
