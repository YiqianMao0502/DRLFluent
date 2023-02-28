# -*- python -*-
#                           Package   : omniidl
# descriptor.py             Created on: 2000/08/23
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2003-2011 Apasphere Ltd
#    Copyright (C) 1999-2000 AT&T Laboratories Cambridge
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
#   Produce internal descriptors
#

from omniidl import idlvisitor, idlast
from omniidl_be.cxx import id, config, ast

# All descriptors are of the form:
#  TAG _ PREFIX _ BASE
# TAG:    groups descriptors by type (eg context, call descriptor, callback)
# PREFIX: derived from a hash of the first callable in the IDL
# BASE:   counter added  to guarantee uniqueness

# There are two categories of descriptors.
#  o Signature dependent (where there is only one instance per operation
#    signature, which can be shared by multiple interfaces) eg normal
#    call descriptors.
#  o Interface & operation dependent eg local callback functions


def __init__(ast):
    global prefix, counter, signature_descriptors, iface_descriptors
    global poller_impls

    prefix  = ""
    counter = 0

    # Descriptors keyed by signature alone
    signature_descriptors = {}

    # Descriptors keyed by interface and operation name
    iface_descriptors = {}

    # Poller implementation classes
    poller_impls = {}

    # initialise the prefix
    HV = HashVisitor()
    ast.accept(HV)


def call_descriptor(signature):
    return private_prefix + "_cd_" + get_signature_descriptor(signature)


def context_descriptor(signature):
    return private_prefix + "_ctx_" + get_signature_descriptor(signature)


def local_callback_fn(iname, operation_name, signature):
    return private_prefix + "_lcfn_" + \
       get_interface_operation_descriptor(iname, operation_name, signature)


def ami_poller_impl(pname):
    return private_prefix + "_poll_" + get_poller_impl(pname)

def ami_call_descriptor(iname, operation_name, signature):
    desc = get_interface_operation_descriptor(iname, operation_name, signature)
    return (private_prefix + "_amic_" + desc,
            private_prefix + "_amip_" + desc)
       


####################################################################
## Internals

private_prefix = config.state['Private Prefix']

# Walks over the AST, finds the first callable and creates the prefix hash
class HashVisitor(idlvisitor.AstVisitor):

    def __init__(self):
        self.base_initialised = 0
    
    def visitAST(self, node):
        for declaration in node.declarations():
            if self.base_initialised:
                return
            if ast.shouldGenerateCodeForDecl(declaration):
                declaration.accept(self)

    def visitModule(self, node):
        for definition in node.definitions():
            if self.base_initialised:
                return
            definition.accept(self)

    def visitInterface(self, node):
        if node.callables() != []:
            name = node.scopedName()

            # Use an op first if available
            for c in node.callables():
                if isinstance(c, idlast.Operation):
                    self.initialise_base(name + [c.identifier()])
                    return

            # Use first attribute
            self.initialise_base(name + [node.callables()[0].identifiers()[0]])

    # Knuth-style string -> int hash
    def initialise_base(self, name):
        if self.base_initialised: return
        self.base_initialised = 1
        
        string_seed = id.Name(name).guard()
    
        # equivalent to >> only without sign extension
        # (python uses 2's complement signed arithmetic)
        def rshift(x, distance):
            sign_bit = x & 0x80000000
            # remove the sign bit to make it unsigned
            x = x & 0x7fffffff
            # perform shift (thinks number is unsigned, no extension)
            x = x >> distance
            # add sign bit back in
            if sign_bit:
                x = x | (1 << (32 - distance -1))
            return x

        def lshift(x, distance):
            # same as non-sign extended case
            return (x << distance) & 0xffffffff

        high = low = 0
        for char in string_seed:
            tmp  = rshift((high & 0xfe000000), 25)
            high = (lshift(high, 7)) ^ (rshift((low & 0xfe000000), 25))
            low  = lshift(low, 7) ^ tmp
            low  = low ^ (ord(char))

        high = list(hex_word(high))
        low  = list(hex_word(low))

        high.reverse()
        low.reverse()
        
        global prefix
        prefix = "".join(high + low)


# Return a unique PREFIX + BASE
def unique():
    global counter
    clist = list(hex_word(counter))
    clist.reverse()

    name = prefix + "_" + "".join(clist)
    counter = counter + 1
    
    return name


def get_signature_descriptor(signature):
    try:
        return signature_descriptors[signature]

    except KeyError:
        desc = unique()
        signature_descriptors[signature] = desc
        return desc


def get_interface_operation_descriptor(iname, operation_name, signature):
    assert isinstance(iname, id.Name)

    key = iname.hash()
    iface_table = iface_descriptors.setdefault(key, {})
    
    key = signature + "/" + operation_name

    try:
        return iface_table[key]

    except KeyError:
        descriptor = unique()
        iface_table[key] = descriptor
        return descriptor


def get_poller_impl(pname):
    assert isinstance(pname, id.Name)

    key = pname.hash()
    try:
        return poller_impls[key]

    except KeyError:
        impl_name = unique()
        poller_impls[key] = impl_name
        return impl_name


# Takes an int and returns the int in hex, without leading 0x and with
# 0s padding.

def hex_word(x):
    return "%08x" % x
