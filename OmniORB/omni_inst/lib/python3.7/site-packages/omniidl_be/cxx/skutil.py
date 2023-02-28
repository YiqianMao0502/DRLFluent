# -*- python -*-
#                           Package   : omniidl
# skutil.py                 Created on: 1999/11/15
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
#   Skeleton utility functions designed for the C++ backend

SMALL_ARRAY_THRESHOLD = 128

try:
    # Python 3 does not have a built in reduce()
    from functools import reduce
except ImportError:
    pass


from omniidl import idltype, idlast
from omniidl_be.cxx import util, types, id, ast, output, cxx

# From http://www-i3.informatik.rwth-aachen.de/funny/babbage.html:
#
# A hotly contested issue among language designers is the method for
# passing parameters to subfunctions. Some advocate "call by name," others
# prefer "call by value." Babbage uses a new method - "call by telephone."
# This is especially effective for long-distance parameter passing.

# Code for marshalling and unmarshalling various data types.

def marshall(to, environment, type, decl, argname, to_where,
             exception = "BAD_PARAM", is_union=0):
    assert isinstance(type, types.Type)

    d_type = type.deref()

    # If this is an array of base types, the quick marshalling option is used.
    # slice_cast is set to the casting required to cast the variable to a
    # pointer to the beginning of the array. There are 3 possibilities:
    #  1. The variable is a declarator, e.g. it is a member of a struct or a
    #     union. No casting is required. slice_cast = ""
    #  2. The variable is not a declarator and its dimension is > 1,
    #     cast the variable to its array slice pointer.
    #  3. Same as 2 but its dimension == 1, just cast the variable to a pointer
    #     to its base class (because the type's array slice may not be
    #     defined).

    if decl:
        assert isinstance(decl, idlast.Declarator)
        dims = decl.sizes() + type.dims()
        slice_cast = ""
    else:
        dims = type.dims()
        if len(dims) != 1:
            slice_cast = "(" + type.base(environment) + "_slice" + "*)"
        else:
            slice_cast = "(" + d_type.base(environment) + "*)"

    if is_union and not dims:
        if d_type.kind() in [ idltype.tk_any,
                              idltype.tk_struct,
                              idltype.tk_union,
                              idltype.tk_sequence,
                              idltype.tk_except,
                              idltype.tk_fixed,
                              ]:
            argname = "(*" + argname + ")"


    if dims:
        n_elements = reduce(lambda x,y:x*y, dims, 1)
        array_marshal_helpers = {
          idltype.tk_octet:     ("omni::ALIGN_1",1),
          idltype.tk_boolean:   ("omni::ALIGN_1",1),
          idltype.tk_short:     ("omni::ALIGN_2",2),
          idltype.tk_long:      ("omni::ALIGN_4",4),
          idltype.tk_ushort:    ("omni::ALIGN_2",2),
          idltype.tk_ulong:     ("omni::ALIGN_4",4),
          idltype.tk_float:     ("omni::ALIGN_4",4),
          idltype.tk_double:    ("omni::ALIGN_8",8),
          idltype.tk_longlong:  ("omni::ALIGN_8",8),
          idltype.tk_ulonglong: ("omni::ALIGN_8",8)
        }
        kind = d_type.type().kind()
        if kind in array_marshal_helpers:
            (alignment,elmsize) = array_marshal_helpers[kind]
            if alignment != "omni::ALIGN_1":
                if kind == idltype.tk_double:
                    to.out("""
#ifndef OMNI_MIXED_ENDIAN_DOUBLE""")

                to.out("""\
if (! @where@.marshal_byte_swap()) {
  @where@.put_octet_array((_CORBA_Octet*)(@slice_cast@@name@),@num@,@align@);
}
else """,
                       where = to_where,
                       name = argname,
                       slice_cast = slice_cast,
                       num = str(n_elements * elmsize),
                       align = alignment)

                if kind == idltype.tk_double:
                    to.out("""\
#endif""")

                # Do not return here.
                # let the code below to deal with the else block.
            else:
                if kind == idltype.tk_boolean:
                    to.out("""
#if !defined(HAS_Cplusplus_Bool) || (SIZEOF_BOOL == 1)""")

                if n_elements <= SMALL_ARRAY_THRESHOLD:
                    put_op = "put_small_octet_array"
                else:
                    put_op = "put_octet_array"

                to.out("@where@.@put_op@((_CORBA_Octet*)(@slice_cast@@name@),@num@);",
                       where = to_where,
                       put_op = put_op,
                       name = argname,
                       slice_cast = slice_cast,
                       num = str(n_elements))

                if kind == idltype.tk_boolean:
                    to.out("""\
#else""")
                else:
                    return

        # No quick route, generate iteration loop
        block = cxx.Block(to)
        akind = d_type.type().kind()

        # Valuetype chunked encoding needs to know array length before
        # we marshal it item by item.
        if akind in array_marshal_helpers:
            to.out("""\
@where@.declareArrayLength(@align@, @num@);""",
                   where = to_where,
                   num = str(n_elements * elmsize),
                   align = alignment)

        elif akind == idltype.tk_char:
            to.out("""\
@where@.declareArrayLength(omni::ALIGN_1, @num@);""",
                   where = to_where,
                   num = str(n_elements))

        elif akind == idltype.tk_longdouble:
            to.out("""\
@where@.declareArrayLength(omni::ALIGN_8, @num@);""",
                   where = to_where,
                   num = str(n_elements * 16))

    loop = cxx.For(to, dims)
    indexing_string = loop.index()
    element_name = argname + indexing_string

    if dims != []:
        type_name = d_type.base(environment)
        if type_name == element_name:
            type_name = d_type.base()
    else:
        type_name = type.base(environment)
        if type_name == element_name:
            type_name = type.base()
            
    bounded = ""
    kind = d_type.type().kind()
    
    if d_type.interface():
        if type_name.endswith("_ptr"):
            type_name = type_name[:-4]
        if isinstance(d_type.type().decl(),idlast.Forward):
            # hack to denote an interface forward declaration
            # kind is used to index the dictionary below
            kind = idltype.tk_objref * 1000

    elif d_type.value() or d_type.valuebox():
        if isinstance(d_type.type().decl(),idlast.ValueForward):
            kind = idltype.tk_value * 1000

    elif d_type.string() or d_type.wstring():
        bounded = str(d_type.type().bound())

    if not d_type.is_basic_data_types() and not d_type.enum():
        type_cast = "(const " + type_name + "&) "
    else:
        type_cast = ""

    special_marshal_functions = {
      idltype.tk_boolean:
      "@to_where@.marshalBoolean(@element_name@);",
      idltype.tk_octet:
      "@to_where@.marshalOctet(@element_name@);",
      idltype.tk_char:
      "@to_where@.marshalChar(@element_name@);",
      idltype.tk_wchar:
      "@to_where@.marshalWChar(@element_name@);",
      idltype.tk_string:
      "@to_where@.marshalString(@element_name@,@bounded@);",
      idltype.tk_wstring:
      "@to_where@.marshalWString(@element_name@,@bounded@);",
      idltype.tk_objref:
      "@type@::_marshalObjRef(@element_name@,@to_where@);",
      idltype.tk_TypeCode:
      "::CORBA::TypeCode::marshalTypeCode(@element_name@,@to_where@);",
      idltype.tk_objref * 1000:
      "@type@_Helper::marshalObjRef(@element_name@,@to_where@);",
      idltype.tk_value:
      "@type@::_NP_marshal(@element_name@,@to_where@);",
      idltype.tk_value * 1000:
      "@type@_Helper::marshal(@element_name@,@to_where@);",
      idltype.tk_value_box:
      "@type@::_NP_marshal(@element_name@,@to_where@);",
      idltype.tk_abstract_interface:
      "@type@::_marshalObjRef(@element_name@,@to_where@);",
      idltype.tk_local_interface:
      "@type@::_marshalObjRef(@element_name@,@to_where@);",
      }

    if kind in special_marshal_functions:
        out_template = special_marshal_functions[kind]

    else:
        out_template = "@type_cast@@element_name@ >>= @to_where@;"


    to.out(out_template,
           to_where     = to_where,
           element_name = element_name,
           bounded      = bounded,
           type         = type_name,
           type_cast    = type_cast,
           dtype        = type.member(environment))
    loop.end()

    if dims != []:
        block.end()
        if kind == idltype.tk_boolean:
            to.out("""\
#endif""")


        
def unmarshall(to, environment, type, decl, name, from_where, is_union=0):
    assert isinstance(type, types.Type)

    d_type = type.deref()

    # If this is an array of base types, the quick marshalling option is used.
    # slice_cast is set to the casting required to cast the variable to a
    # pointer to the beginning of the array. There are 3 possibilities:
    #  1. The variable is a declarator, e.g. it is a member of a struct or a
    #     union. No casting is required. slice_cast = ""
    #  2. The variable is not a declarator and its dimension is > 1,
    #     cast the variable to its array slice pointer.
    #  3. Same as 2 but its dimension == 1, just cast the variable to a pointer
    #     to its base class (because the type's array slice may not be
    #     defined).

    if decl:
        assert isinstance(decl, idlast.Declarator)
        dims = decl.sizes() + type.dims()
        slice_cast = ""
    else:
        dims = type.dims()
        if len(dims) != 1:
            slice_cast = "(" + type.base(environment) + "_slice" + "*)"
        else:
            slice_cast = "(" + d_type.base(environment) + "*)"

    if dims:

        if is_union:
            if decl.sizes():
                # Anonymous array
                slice_type = name[3:] + "_slice"
            else:
                slice_type = type.base(environment) + "_slice"

            to.out("@name@ = new @slice_type@[@dim@];",
                   name=name, slice_type=slice_type, dim = str(dims[0]))

        n_elements = reduce(lambda x,y:x*y, dims, 1)
        array_unmarshal_helpers = {
          idltype.tk_octet:  ("get_octet_array","(_CORBA_Octet*)"),
          idltype.tk_boolean:("unmarshalArrayBoolean","(_CORBA_Boolean*)"),
          idltype.tk_short:  ("unmarshalArrayShort","(_CORBA_Short*)"),
          idltype.tk_long:   ("unmarshalArrayLong","(_CORBA_Long*)"),
          idltype.tk_ushort: ("unmarshalArrayUShort","(_CORBA_UShort*)"),
          idltype.tk_ulong:  ("unmarshalArrayULong","(_CORBA_ULong*)"),
          idltype.tk_float:  ("unmarshalArrayFloat","(_CORBA_Float*)"),
          idltype.tk_double: ("unmarshalArrayDouble","(_CORBA_Double*)"),
          idltype.tk_longlong:("unmarshalArrayLongLong","(_CORBA_LongLong*)"),
          idltype.tk_ulonglong:("unmarshalArrayULongLong","(_CORBA_ULongLong*)")
          }
        kind = d_type.type().kind()

        if kind in array_unmarshal_helpers:
            (helper,typecast) = array_unmarshal_helpers[kind]
            to.out("@where@.@helper@(@typecast@(@slice_cast@@name@), @num@);",
                   helper = helper,
                   where = from_where, typecast = typecast,
                   name = name,
                   slice_cast = slice_cast,
                   num = str(n_elements))
            return

        # No quick route, generate iteration loop
        block = cxx.Block(to)

    loop = cxx.For(to, dims)
    indexing_string = loop.index()
    element_name = name + indexing_string
    
    if dims != []:
        type_name = d_type.base(environment)
        if type_name == element_name:
            type_name = d_type.base()
    else:
        type_name = type.base(environment)
        if type_name == element_name:
            type_name = type.base()
        
    bounded = ""
    kind = d_type.type().kind()

    if d_type.interface():
        if type_name.endswith("_ptr"):
            type_name = type_name[:-4]
        if isinstance(d_type.type().decl(),idlast.Forward):
            # hack to denote an interface forward declaration
            # kind is used to index the associative array below
            kind = idltype.tk_objref * 1000

    elif d_type.value() or d_type.valuebox():
        if isinstance(d_type.type().decl(),idlast.ValueForward):
            kind = idltype.tk_value * 1000

    elif d_type.string() or d_type.wstring():
        bounded = str(d_type.type().bound())
        
    special_unmarshal_functions = {
      idltype.tk_boolean:
      "@element_name@ = @where@.unmarshalBoolean();",
      idltype.tk_octet:
      "@element_name@ = @where@.unmarshalOctet();",
      idltype.tk_char:
      "@element_name@ = @where@.unmarshalChar();",
      idltype.tk_wchar:
      "@element_name@ = @where@.unmarshalWChar();",
      idltype.tk_string:
      "@element_name@ = @where@.unmarshalString(@bounded@);",
      idltype.tk_wstring:
      "@element_name@ = @where@.unmarshalWString(@bounded@);",
      idltype.tk_objref:
      "@element_name@ = @type@::_unmarshalObjRef(@where@);",
      idltype.tk_TypeCode:
      "@element_name@ = ::CORBA::TypeCode::unmarshalTypeCode(@where@);",
      idltype.tk_objref * 1000:
      "@element_name@ = @type@_Helper::unmarshalObjRef(@where@);",
      idltype.tk_value:
      "@element_name@ = @type@::_NP_unmarshal(@where@);",
      idltype.tk_value * 1000:
      "@element_name@ = @type@_Helper::unmarshal(@where@);",
      idltype.tk_value_box:
      "@element_name@ = @type@::_NP_unmarshal(@where@);",
      idltype.tk_abstract_interface:
      "@element_name@ = @type@::_unmarshalObjRef(@where@);",
      idltype.tk_local_interface:
      "@element_name@ = @type@::_unmarshalObjRef(@where@);",
      }

    if kind in special_unmarshal_functions:
        out_template = special_unmarshal_functions[kind]
    else:
        if not dims and is_union and kind in [ idltype.tk_any,
                                               idltype.tk_struct,
                                               idltype.tk_union,
                                               idltype.tk_sequence,
                                               idltype.tk_except,
                                               idltype.tk_fixed ] :
            out_template = """\
@element_name@ = new @type@;
(*@element_name@) <<= @where@;
"""
        else:
            out_template = "(@type@&)@element_name@ <<= @where@;"

    to.out(out_template,
           type         = type_name,
           element_name = element_name,
           where        = from_where,
           bounded      = bounded,
           dtype        = type.member(environment))

    loop.end()

    if dims != []:
        block.end()


def sort_exceptions(ex):
    # sort the exceptions into lexicographical order
    try:
        return sorted(ex, key = lambda e: e.repoId())

    except NameError:
        def lexicographic(exception_a, exception_b):
            name_a = exception_a.repoId()
            name_b = exception_b.repoId()
            return cmp(name_a, name_b)

        raises = ex[:]
        raises.sort(lexicographic)
        return raises
