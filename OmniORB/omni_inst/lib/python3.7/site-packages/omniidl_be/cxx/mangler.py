# -*- python -*-
#                           Package   : omniidl
# mangler.py                Created on: 1999/11/16
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2011 Apasphere Ltd
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
#   Produce mangled names for types and operation signatures
#   

from omniidl import idlast, idltype
from omniidl_be.cxx import types, id, skutil, util


#######################################################################
## Produce cannonical names for types and operation signatures

# Separator constants
SCOPE_SEPARATOR         =    "_m"
ARRAY_SEPARATOR         =    "_a"
SEQ_SEPARATOR           =    "_s"
FORWARD_SEQ_SEPARATOR   =    "_f"
CANNON_NAME_SEPARATOR   =    "_c"
ONEWAY_SEPARATOR        =    "_w"
IN_SEPARATOR            =    "_i"
OUT_SEPARATOR           =    "_o"
INOUT_SEPARATOR         =    "_n"
EXCEPTION_SEPARATOR     =    "_e"
ASYNC_TAG               =    "_z"

# Canonical names for basic types
name_map = {
    idltype.tk_short:       "short",
    idltype.tk_long:        "long",
    idltype.tk_longlong:    "longlong",
    idltype.tk_ushort:      "unsigned_pshort",
    idltype.tk_ulong:       "unsigned_plong",
    idltype.tk_ulonglong:   "unsigned_plonglong",
    idltype.tk_float:       "float",
    idltype.tk_double:      "double",
    idltype.tk_longdouble:  "longdouble",
    idltype.tk_char:        "char",
    idltype.tk_wchar:       "wchar",
    idltype.tk_boolean:     "boolean",
    idltype.tk_octet:       "octet",
    idltype.tk_void:        "void",
    idltype.tk_any:         "any",
    idltype.tk_TypeCode:    "TypeCode",
    }

# Produce a cannonical type name
def canonTypeName(type, decl = None, useScopedName = 0):
    assert isinstance(type, types.Type)

    type_dims = type.dims()
    decl_dims = []
    if decl != None:
        assert isinstance(decl, idlast.Declarator)
        decl_dims = decl.sizes()

    # flatten a list of dimensions into a string
    def dims(d):
        if d == []: return ""
        d_str = [ARRAY_SEPARATOR + str(x) for x in d]
        return "".join(d_str)

    full_dims = decl_dims + type_dims
    is_array = full_dims != []
    dims_string = dims(full_dims)

    # The canonical type name always has the full dimensions
    # prepended to it.
    canon_name = dims_string

    d_type = type.deref()

    # consider anonymous sequence<sequence<....
    if type.sequence() and types.Type(type.type().seqType()).sequence():
        bound = type.type().bound()
        canon_name = canon_name + SEQ_SEPARATOR + str(bound) +\
                     canonTypeName(types.Type(type.type().seqType()), None,
                                   useScopedName)
        
        return canon_name
        

    # sometimes we don't want to call a sequence a sequence
    # (operation signatures)
    if useScopedName and not is_array and \
       type.typedef() and d_type.sequence():
        # find the last typedef in the chain
        while types.Type(type.type().decl().alias().aliasType()).typedef():
            type = types.Type(type.type().decl().alias().aliasType())
        scopedName = id.Name(type.type().scopedName()).guard()
        return canon_name + CANNON_NAME_SEPARATOR + scopedName

    # _arrays_ of sequences seem to get handled differently
    # to simple aliases to sequences
    if d_type.sequence():
        bound = d_type.type().bound()
        seqType = types.Type(d_type.type().seqType())

        if seqType.structforward() or seqType.unionforward():
            canon_name = canon_name + FORWARD_SEQ_SEPARATOR + str(bound)
        else:
            canon_name = canon_name + SEQ_SEPARATOR + str(bound)


        while seqType.sequence():
            bound = seqType.type().bound()
            canon_name = canon_name + SEQ_SEPARATOR + str(bound)
            seqType = types.Type(seqType.type().seqType())

        # straight forward sequences of sequences use their
        # flattened scoped name
        dkd_seqType = seqType.deref(keep_dims = 1)
        if not dkd_seqType.sequence():
            canon_name = canon_name + canonTypeName(dkd_seqType)
            return canon_name
        type = seqType
        d_type = type.deref()
        

    # add in the name for the most dereferenced type
    def typeName(type):
        assert isinstance(type, types.Type)
        d_type = type.deref()
        # dereference the type, until just -before- it becomes a
        # sequence. Since a sequence doesn't have a scopedName(),
        # we use the scopedName() of the immediately preceeding
        # typedef which is an instance of idltype.Declared
        while type.typedef() and \
              not types.Type(type.type().decl().alias().aliasType()).sequence():
            type = types.Type(type.type().decl().alias().aliasType())

        if type.type().kind() in name_map:
            return name_map[type.type().kind()]
        if type.string():
            bound = ""
            if type.type().bound() != 0:
                bound = str(type.type().bound())
            return bound + "string"
        if type.wstring():
            bound = ""
            if type.type().bound() != 0:
                bound = str(type.type().bound())
            return bound + "wstring"

        if isinstance(type.type(), idltype.Fixed):
            return str(type.type().digits()) + "_" + \
                   str(type.type().scale()) + "fixed"

        if isinstance(type.type(), idltype.Declared):
            return id.Name(type.type().scopedName()).guard()

        util.fatalError("Error generating mangled name")

    canon_name = canon_name + CANNON_NAME_SEPARATOR + typeName(type)
    return canon_name


def produce_signature(returnType, parameters, raises, oneway, ami):

    returnType = types.Type(returnType)
    d_returnType = returnType.deref()

    # return type
    if d_returnType.void():
        sig = "void"
    else:
        sig = canonTypeName(returnType, useScopedName = 1)

    if oneway:
        # Can only validly happen with void return, but you never know
        # what the future may hold.
        sig = ONEWAY_SEPARATOR + sig
        
    # parameter list
    for param in parameters:
        if param.is_in() and param.is_out():
            sig = sig + INOUT_SEPARATOR
        elif param.is_in():
            sig = sig + IN_SEPARATOR
        elif param.is_out():
            sig = sig + OUT_SEPARATOR

        sig = sig + canonTypeName(types.Type(param.paramType()),
                                  useScopedName = 1)

    # exception list
    raises = skutil.sort_exceptions(raises)

    def exception_signature(exception):
        cname = CANNON_NAME_SEPARATOR +\
                id.Name(exception.scopedName()).guard()
        return EXCEPTION_SEPARATOR + cname
    
    raises_sigs = list(map(exception_signature, raises))
    raises_str = "".join(raises_sigs)

    sig = sig + raises_str

    if ami:
        sig = sig + ASYNC_TAG

    return sig
