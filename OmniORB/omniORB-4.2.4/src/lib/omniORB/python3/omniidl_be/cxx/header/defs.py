# -*- python -*-
#                           Package   : omniidl
# defs.py                   Created on: 1999/11/2
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
#   Produce the main header definitions for the C++ backend

"""Produce the main header definitions"""

from omniidl import idlast, idltype
from omniidl_be.cxx import id, output, config, types, iface, cxx, ast, util
from omniidl_be.cxx import value
from omniidl_be.cxx.header import template

# We behave as if the global code here is really inside a class
import sys
self = sys.modules[__name__]

stream = None

_insideClass      = 0
_insideModule     = 0
_insideInterface  = 0
_interfaces       = {}
_completedModules = {}

def pushInsideClass():
    global _insideClass
    _insideClass = _insideClass + 1

def popInsideClass():
    global _insideClass
    _insideClass = _insideClass - 1

def pushInsideModule():
    global _insideModule
    _insideModule = _insideModule + 1

def popInsideModule():
    global _insideModule
    _insideModule = _insideModule - 1

def pushInsideInterface():
    global _insideInterface
    _insideInterface = _insideInterface + 1

def popInsideInterface():
    global _insideInterface
    _insideInterface = _insideInterface - 1


def init(_stream):
    global stream, _insideClass, _insideModule, _insideInterface
    global _interfaces, _completedModules

    stream = _stream

    # Need to keep track of how deep within the AST we are.
    # In a recursive procedure these would be extra arguments,
    # but the visitor pattern necessitates them being global.
    _insideInterface = 0
    _insideModule    = 0
    _insideClass     = 0

    # A repository id entry in this hash indicates that an interface
    # has been declared- therefore any more AST forward nodes for this
    # interface are ignored.
    _interfaces = {}

    # When we first encounter a module, we sometimes deal with all the
    # continuations straight away. Therefore when we reencounter a
    # continuation later, we don't duplicate the definitions.
    _completedModules = {}

    return self


# Returns the prefix required inside a const declaration (it depends on
# exactly what the declaration is nested inside)
def const_qualifier(insideModule=None, insideClass=None):
    if insideModule is None:
        insideModule = _insideModule
        insideClass  = _insideClass

    if not insideModule and not insideClass:
        return "_CORBA_GLOBAL_VAR"
    elif insideClass:
        return "static"
    else:
        return "_CORBA_MODULE_VAR"

# Same logic for function qualifiers
def func_qualifier():
    return const_qualifier(_insideModule, _insideClass)

# Inline functions are subtly different
def inline_qualifier():
    if not _insideModule and not _insideClass:
        return "inline"
    elif _insideClass:
        return "static inline"
    else:
        return "_CORBA_MODULE_INLINE"


#
# Control arrives here
#
def visitAST(node):
    global stream, _insideClass, _insideModule, _insideInterface
    global _interfaces, _completedModules

    _insideInterface  = 0
    _insideModule     = 0
    _insideClass      = 0
    _interfaces       = {}
    _completedModules = {}

    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    # Ensure we only output the definitions once.
    # In particular, when the splice-modules flag is set and this is
    # a reopened module, the node will be marked as completed already.
    if node in _completedModules:
        return
    _completedModules[node] = 1
    
    ident = node.identifier()
    cxx_id = id.mapID(ident)

    if not config.state['Fragment']:
        stream.out(template.module_begin, name = cxx_id)
        stream.inc_indent()

    # push self.__insideModule, true
    pushInsideModule()

    for n in node.definitions():
        n.accept(self)

    # deal with continuations (only if the splice-modules flag is set)
    if config.state['Splice Modules']:
        for c in node.continuations():
            for n in c.definitions():
                n.accept(self)
            _completedModules[c] = 1

    popInsideModule()
    
    if not config.state['Fragment']:
        stream.dec_indent()
        stream.out(template.module_end, name = cxx_id)

        

def visitInterface(node):
    # It's legal to have a forward interface declaration after
    # the actual interface definition. Make sure we ignore these.
    _interfaces[node.repoId()] = 1

    name = node.identifier()
    cxx_name = id.mapID(name)

    outer_environment = id.lookup(node)
    outer_environment.enter(name)

    pushInsideInterface()
    pushInsideClass()

    # make the necessary forward references, typedefs and define
    # the _Helper class
    I = iface.Interface(node)

    I_Helper = iface.I_Helper(I)
    I_Helper.hh(stream)

    # recursively take care of other IDL declared within this
    # scope (evaluate function later- lazy eval though 'thunking')
    def Other_IDL(node = node):
        for n in node.declarations():
            n.accept(self)

    # Output this interface's corresponding class
    Ibase = iface.I(I,Other_IDL)
    Ibase.hh(stream)

    if not node.local():
        _objref_I = iface._objref_I(I)
        _objref_I.hh(stream)

        _pof_I = iface._pof_I(I)
        _pof_I.hh(stream)

        # Skeleton class
        _impl_I = iface._impl_I(I)
        _impl_I.hh(stream)

        # Generate BOA compatible skeletons?
        if config.state['BOA Skeletons']:
            _sk_I = iface._sk_I(I)
            _sk_I.hh(stream)

    popInsideClass()
    popInsideInterface()

    # Typecode and Any
    if config.state['Typecode']:
        qualifier = const_qualifier(_insideModule, _insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_name)
        
    return
    

def visitForward(node):
    # Note it's legal to have multiple forward declarations
    # of the same name. So ignore the duplicates.
    if node.repoId() in _interfaces:
        return
    _interfaces[node.repoId()] = 1

    environment = id.lookup(node)
    name        = id.Name(node.scopedName())
    guard       = name.guard()

    # Potentially forward declare BOA skeleton class
    class_sk = ""
    if config.state['BOA Skeletons']:
        class_sk = "class _sk_" + name.simple() + ";"

    # output the definition
    if node.abstract():
        stream.out(template.abstract_interface_Helper,
                   guard = guard,
                   class_sk_name = class_sk,
                   name = name.simple())
    elif node.local():
        stream.out(template.local_interface_Helper,
                   guard = guard,
                   class_sk_name = class_sk,
                   name = name.simple())
    else:
        stream.out(template.interface_Helper,
                   guard = guard,
                   class_sk_name = class_sk,
                   name = name.simple())


def visitConst(node):
    environment = id.lookup(node)

    constType = types.Type(node.constType())
    d_constType = constType.deref()
    if d_constType.string():
        type_string = "char *"
    elif d_constType.wstring():
        type_string = "::CORBA::WChar *"
    elif d_constType.fixed():
        type_string = constType.member()
    else:
        type_string = d_constType.member()
        # should this be .base?

    cxx_name = id.mapID(node.identifier())

    value = d_constType.literal(node.value(), environment)

    representedByInteger = d_constType.representable_by_int()

    # depends on whether we are inside a class / in global scope
    # etc
    # should be rationalised with tyutil.const_qualifier
    if _insideClass:
        if representedByInteger:
            stream.out(template.const_inclass_isinteger,
                       type = type_string, name = cxx_name, val = value)
        else:
            stream.out(template.const_inclass_notinteger,
                       type = type_string, name = cxx_name)
    else:
        where = "GLOBAL"
        if _insideModule:
            where = "MODULE"
        if representedByInteger:
            stream.out(template.const_outsideclass_isinteger,
                       where = where,
                       type = type_string,
                       name = cxx_name,
                       val = value)
        else:
            stream.out(template.const_outsideclass_notinteger,
                       where = where,
                       type = type_string,
                       name = cxx_name)


def visitTypedef(node):
    environment = id.lookup(node)
    
    is_global_scope = not (_insideModule or _insideInterface)
    
    aliasType = types.Type(node.aliasType())

    # is _this_ type a constructed type?
    if node.constrType():
        node.aliasType().decl().accept(self)
    
    d_type = aliasType.deref()
    derefTypeID = d_type.base(environment)

    basicReferencedTypeID = aliasType.member(environment)

    # each one is handled independently
    for d in node.declarators():
        
        # derivedName is the new typedef'd name
        # alias_dims is a list of dimensions of the type being aliased

        derivedName = id.mapID(d.identifier())
        
        alias_dims = aliasType.dims()

        # array_declarator indicates whether this is a simple (non-array)
        # declarator or not
        array_declarator = d.sizes() != []

        # Typecode and Any
        if config.state['Typecode']:
            qualifier = const_qualifier(_insideModule,_insideClass)
            stream.out(template.typecode,
                       qualifier = qualifier,
                       name = derivedName)
                    
        # is it a simple alias (ie not an array at this level)?
        if not array_declarator:
            # not an array declarator but a simple declarator to an array
            if aliasType.array():
                # simple alias to an array should alias all the
                # array handling functions, but we don't need to duplicate
                # array looping code since we can just call the functions
                # for the base type
                stream.out(template.typedef_simple_to_array,
                           base = basicReferencedTypeID,
                           derived = derivedName,
                           qualifier = func_qualifier(),
                           inline_qualifier = inline_qualifier())
                           
            # Non-array of string
            elif d_type.string():
                stream.out(template.typedef_simple_string,
                           name = derivedName)
            elif d_type.wstring():
                stream.out(template.typedef_simple_wstring,
                           name = derivedName)
            elif d_type.typecode():
                stream.out(template.typedef_simple_typecode,
                           name = derivedName)
            elif d_type.any():
                stream.out(template.typedef_simple_any,
                           name = derivedName)

            elif d_type.fixed():
                stream.out(template.typedef_simple_fixed,
                           name = derivedName,
                           digits = d_type.type().digits(),
                           scale = d_type.type().scale())

            # Non-array of basic type
            elif isinstance(d_type.type(), idltype.Base):
                stream.out(template.typedef_simple_basic,
                           base = basicReferencedTypeID,
                           derived = derivedName)

            # a typedef to a struct or union, or a typedef to a
            # typedef to a sequence
            elif d_type.struct() or d_type.structforward() or \
                 d_type.union() or d_type.unionforward() or \
                 (d_type.sequence() and aliasType.typedef()):
                
                stream.out(template.typedef_simple_constructed,
                           base = basicReferencedTypeID,
                           name = derivedName)
                    
            # Non-array of object reference
            elif d_type.interface():
                derefTypeID = derefTypeID.replace("_ptr","")
                # Note that the base name is fully flattened
                is_CORBA_Object = d_type.type().scopedName() ==\
                                  ["CORBA", "Object"]
                impl_base = ""
                objref_base = ""
                sk_base = ""
                if not is_CORBA_Object:
                    scopedName = d_type.type().decl().scopedName()
                    name = id.Name(scopedName)
                    impl_scopedName = name.prefix("_impl_")
                    objref_scopedName = name.prefix("_objref_")
                    sk_scopedName = name.prefix("_sk_")
                    impl_name = impl_scopedName.unambiguous(environment)
                    objref_name = objref_scopedName.unambiguous(environment)
                    sk_name = sk_scopedName.unambiguous(environment)

                    impl_base =   "typedef " + impl_name   + " _impl_"   +\
                                   derivedName + ";"
                    objref_base = "typedef " + objref_name + " _objref_" +\
                                   derivedName + ";"
                    sk_base =     "typedef " + sk_name     + " _sk_"     +\
                                   derivedName + ";"

                stream.out(template.typedef_simple_objref,
                           base = derefTypeID,
                           name = derivedName,
                           impl_base = impl_base,
                           objref_base = objref_base)
                if config.state['BOA Skeletons']:
                    stream.out(sk_base)

            # Non-array of valuetype
            elif d_type.value() or d_type.valuebox():
                basicReferencedTypeID = basicReferencedTypeID.replace("_member",
                                                                      "")
                stream.out(template.typedef_simple_constructed,
                           base = basicReferencedTypeID,
                           name = derivedName)
                    
            # Non-array of enum
            elif d_type.enum():
                stream.out(template.typedef_simple_basic,
                           base = basicReferencedTypeID,
                           derived = derivedName)

            # Non-array of sequence
            elif d_type.sequence():
                seqType = types.Type(d_type.type().seqType())
                d_seqType = seqType.deref()
                bounded = d_type.type().bound()
                
                templateName = d_type.sequenceTemplate(environment)
                
                if d_seqType.structforward() or d_seqType.unionforward():
                    # Sequence of forward-declared struct or union.
                    # We cannot use the normal sequence templates
                    # since they have inline methods that require the
                    # full definition of the member type. We use
                    # templates with abstract virtual functions
                    # instead.

                    element = element_ptr = seqType.base(environment)

                    def bounds(bounded = bounded,
                               derivedName = derivedName,
                               derived = templateName,
                               element = element):
                        if bounded:
                            ct = template.sequence_bounded_ctors
                        else:
                            ct = template.sequence_unbounded_ctors
                        stream.out(ct, name = derivedName, element=element,
                                   bound=bounded, derived=derived)

                    stream.out(template.sequence_forward_type,
                               name = derivedName,
                               derived = templateName,
                               element = element,
                               bounds = bounds)

                else:
                    # Normal case using a template class.

                    if seqType.array():
                        element = "*** INVALID"
                        element_ptr = seqType.base(environment)
                    else:
                        if d_seqType.string():
                            element = "_CORBA_String_element"
                            element_ptr = "char*"
                        elif d_seqType.wstring():
                            element = "_CORBA_WString_element"
                            element_ptr = "::CORBA::WChar*"
                        elif d_seqType.interface():
                            element = seqType.base(environment)
                            element_ptr = element
                        elif d_seqType.value() or d_seqType.valuebox():
                            element = seqType.base(environment)
                            element_ptr = element + "*"
                        # only if an anonymous sequence
                        elif seqType.sequence():
                            element = d_seqType.sequenceTemplate(environment)
                            element_ptr = element
                        elif d_seqType.typecode():
                            element = "::CORBA::TypeCode_member"
                            element_ptr = element
                        else:
                            element = seqType.base(environment)
                            element_ptr = element

                    # enums are a special case
                    # ----
                    # gcc requires that the marshalling operators for the
                    # element be declared before the sequence template is
                    # typedef'd. This is a problem for enums, as the
                    # marshalling operators are not yet defined (and are
                    # not part of the type itself).
                    # ----
                    # Note that the fully dereferenced name is used
                    friend = "friend"
                    if is_global_scope:
                        friend = ""

                    if d_seqType.enum() and not seqType.array():
                        stream.out(template.typedef_enum_oper_friend,
                                   element = d_seqType.base(environment),
                                   friend = friend)

                    # derivedName is the new type identifier
                    # element is the name of the basic element type
                    # templateName contains the template instantiation

                    def bounds(bounded = bounded, derivedName = derivedName,
                               element_ptr = element_ptr,
                               templateName = templateName):
                        if bounded:
                            ctor_template = template.sequence_bounded_ctors
                        else:
                            ctor_template = template.sequence_unbounded_ctors
                        stream.out(ctor_template,
                                   name = derivedName,
                                   element = element_ptr,
                                   derived = templateName)

                    # output the main sequence definition
                    stream.out(template.sequence_type,
                               name = derivedName,
                               derived = templateName,
                               bounds = bounds)
                

                # start building the _var and _out types
                element_reference = "*** INVALID"
                if not aliasType.array():
                    if d_seqType.string():
                        # special case alert
                        element_reference = element
                    elif d_seqType.wstring():
                        # special case alert
                        element_reference = element
                    elif d_seqType.interface():
                        element_reference = d_seqType.objRefTemplate("Element",
                                                                     environment)
                    elif d_seqType.value() or d_seqType.valuebox():
                        element_reference = d_seqType.valueTemplate("Element",
                                                                    environment)
                    # only if an anonymous sequence
                    elif seqType.sequence():
                        element_reference = d_seqType.sequenceTemplate(environment) + "&"
                    else:
                        element_reference = element + "&"

                def subscript_operator_var(stream = stream,
                                           is_array = seqType.array(),
                                           element_ptr = element_ptr,
                                           element_ref = element_reference):

                    if is_array:
                        stream.out(template.sequence_var_array_subscript,
                                   element = element_ptr)
                    else:
                        stream.out(template.sequence_var_subscript,
                                   element = element_ref)

                def subscript_operator_out(stream = stream,
                                           is_array = seqType.array(),
                                           element_ptr = element_ptr,
                                           element_ref = element_reference):
                    if is_array:
                        stream.out(template.sequence_out_array_subscript,
                                   element = element_ptr)
                    else:
                        stream.out(template.sequence_out_subscript,
                                   element = element_ref)
                    
                # write the _var class definition
                stream.out(template.sequence_var,
                           name = derivedName,
                           subscript_operator = subscript_operator_var)

                # write the _out class definition
                stream.out(template.sequence_out,
                           name = derivedName,
                           subscript_operator = subscript_operator_out)

            else:
                util.fatalError("Inexhaustive Case Match")


        # ----------------------------------------------------------------
        # declarator is an array typedef declarator
        elif array_declarator:

            all_dims = d.sizes() + alias_dims
            dimsString = cxx.dimsToString(d.sizes())
            taildims = cxx.dimsToString(d.sizes()[1:])
            
            typestring = aliasType.member(environment)

            # build the _dup loop
            def dup_loop(stream = stream, all_dims = all_dims):
                loop = cxx.For(stream, all_dims)
                stream.out("\n_data@index@ = _s@index@;\n",
                           index = loop.index())
                loop.end()

            # build the _copy loop
            def copy_loop(stream = stream, all_dims = all_dims):
                loop = cxx.For(stream, all_dims)
                stream.out("\n_to@index@ = _from@index@;\n",
                           index = loop.index())
                loop.end()

            stream.out(template.typedef_array,
                       name = derivedName,
                       type = typestring,
                       dims = dimsString,
                       taildims = taildims,
                       firstdim = repr(all_dims[0]),
                       dup_loop = dup_loop,
                       copy_loop = copy_loop,
                       qualifier = func_qualifier(),
                       inline_qualifier = inline_qualifier())

            # output the _copyHelper class
            fqname = id.Name(d.scopedName()).fullyQualify()

            if types.variableDecl(node):
                stream.out(template.typedef_array_copyHelper,
                           var_or_fix = "Variable",
                           name = derivedName,
                           fqname = fqname)

                stream.out(template.typedef_array_variable_out_type,
                           name = derivedName)
            else:
                stream.out(template.typedef_array_copyHelper,
                           var_or_fix = "Fix",
                           name = derivedName,
                           fqname = fqname)

                stream.out(template.typedef_array_fix_out_type,
                           name = derivedName)
               
     

def visitMember(node):
    memberType = node.memberType()
    if node.constrType():
        # if the type was declared here, it must be an instance
        # of idltype.Declared!
        assert isinstance(memberType, idltype.Declared)
        memberType.decl().accept(self)


def visitStruct(node):
    name = node.identifier()
    cxx_name = id.mapID(name)

    outer_environment = id.lookup(node)
    environment = outer_environment.enter(name)
    
    pushInsideClass()
            
    # Deal with types constructed here
    def Other_IDL(stream = stream, node = node, environment = environment):
        for m in node.members():
            if m.constrType():
                m.memberType().decl().accept(self)
            
    # Deal with the actual struct members
    def members(stream = stream, node = node, environment = environment):
        for m in node.members():
            memberType = types.Type(m.memberType())

            memtype = memberType.member(environment)

            for d in m.declarators():
                ident = d.identifier()

                cxx_id = id.mapID(ident)

                decl_dims = d.sizes()
                is_array_declarator = decl_dims != []

                # non-arrays of direct sequences are done via a typedef
                if not is_array_declarator and memberType.sequence():
                    stream.out(template.struct_nonarray_sequence,
                               memtype = memtype,
                               cxx_id = cxx_id)
                else:
                    dims_string = cxx.dimsToString(decl_dims)
                    if is_array_declarator:
                        stream.out(template.struct_array_declarator,
                                   memtype = memtype,
                                   cxx_id = cxx_id,
                                   dims = dims_string,
                                   tail_dims = cxx.dimsToString(d.sizes()[1:]),
                                   prefix = config.state['Private Prefix'])

                    stream.out(template.struct_normal_member,
                               memtype = memtype,
                               cxx_id = cxx_id,
                               dims = dims_string)
            
    # Output the structure itself
    if types.variableDecl(node):
        stream.out(template.struct,
                   name = cxx_name,
                   fix_or_var = "Variable",
                   Other_IDL = Other_IDL,
                   members = members)
        stream.out(template.struct_variable_out_type,
                   name = cxx_name)
    else:
        stream.out(template.struct,
                   name = cxx_name,
                   fix_or_var = "Fix",
                   Other_IDL = Other_IDL,
                   members = members)
        stream.out(template.struct_fix_out_type,
                   name = cxx_name)

    
    popInsideClass()

    # TypeCode and Any
    if config.state['Typecode']:
        # structs in C++ are classes with different default privacy policies
        qualifier = const_qualifier(_insideModule, _insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_name)


def visitStructForward(node):
    cxx_name = id.mapID(node.identifier())
    stream.out(template.struct_forward, name = cxx_name)


def visitException(node):
    exname = node.identifier()

    cxx_exname = id.mapID(exname)

    outer_environment = id.lookup(node)
    environment = outer_environment.enter(exname)
    
    pushInsideClass()

    # if the exception has no members, inline some no-ops
    no_members = (node.members() == [])

    # other types constructed within this one
    def Other_IDL(stream = stream, node = node):
        for m in node.members():
            if m.constrType():
                m.memberType().decl().accept(self)

    # deal with the exceptions members
    def members(stream = stream, node = node, environment = environment):
        for m in node.members():
            memberType = types.Type(m.memberType())

            for d in m.declarators():
                decl_dims = d.sizes()
                is_array_declarator = decl_dims != []
                
                memtype = memberType.member(environment)
                ident = d.identifier()

                cxx_id = id.mapID(ident)

                dims_string = cxx.dimsToString(decl_dims)
                
                if is_array_declarator:
                    stream.out(template.exception_array_declarator,
                               memtype = memtype,
                               cxx_id = cxx_id,
                               dims = dims_string,
                               tail_dims = cxx.dimsToString(d.sizes()[1:]),
                               private_prefix = config.state['Private Prefix'])

                stream.out(template.exception_member,
                           memtype = memtype,
                           cxx_id = cxx_id,
                           dims = dims_string)

    # deal with ctor args
    ctor_args = []
    for m in node.members():
        memberType = types.Type(m.memberType())
        d_memberType = memberType.deref()
        for d in m.declarators():
            decl_dims = d.sizes()
            is_array_declarator = decl_dims != []
            ctor_arg_type = memberType.op(types.IN, environment)
            # sequences are not passed by reference here
            if d_memberType.sequence():
                if memberType.typedef():
                    ctor_arg_type = "const " + id.Name(memberType.type().decl().scopedName()).unambiguous(environment)
                else:
                    ctor_arg_type = "const " + memberType.sequenceTemplate(environment)
            elif d_memberType.typecode():
                ctor_arg_type = "::CORBA::TypeCode_ptr"
                
            ident = d.identifier()

            cxx_id = id.mapID(ident)

            if is_array_declarator:
                ctor_arg_type = "const " + config.state['Private Prefix'] +\
                                "_" + cxx_id
            ctor_args.append(ctor_arg_type + " i_" + cxx_id)

       
    ctor = ""
    if ctor_args != []:
        ctor = cxx_exname + "(" + ", ".join(ctor_args) + ");"

    if no_members:
        inline = "inline "
        body = "{ }"
        alignedSize = ""
    else:
        inline = ""
        body = ";"
        alignedSize = "size_t _NP_alignedSize(size_t) const;"

    # output the main exception declaration
    stream.out(template.exception,
               name = cxx_exname,
               Other_IDL = Other_IDL,
               members = members,
               constructor = ctor,
               alignedSize = alignedSize,
               inline = inline,
               body = body)
               
    popInsideClass()

    # Typecode and Any
    if config.state['Typecode']:
        qualifier = const_qualifier(_insideModule, _insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_exname)
    


def visitUnion(node):
    ident = node.identifier()

    cxx_id = id.mapID(ident)
    outer_environment = id.lookup(node)
    environment = outer_environment.enter(ident)
    
    pushInsideClass()
    
    switchType = types.Type(node.switchType())
    d_switchType = switchType.deref()

    ####################################################################
    # in the case where there is no default case and an implicit default
    # member, choose a discriminator value to set. Note that attempting
    # to access the data is undefined
    def chooseArbitraryDefault(switchType = switchType,
                               values = ast.allCaseLabelValues(node),
                               environment = environment):
        
        # dereference the switch_type (ie if CASE <scoped_name>)
        switchType = switchType.deref()

        # for integer types, find the lowest unused number
        def min_unused(start, used = values):
            x = start
            while x in used:
                x = x + 1
            return x

        kind = switchType.type().kind()
        if switchType.integer():
            (low, high) = ast.integer_type_ranges[kind]
            s = switchType.literal(min_unused(low+1))
            return s

        # for other types, first compute the set of all legal values
        # (sets are all fairly small)
        elif kind == idltype.tk_char:
            all = list(map(chr, list(range(0, 255))))
        elif kind == idltype.tk_boolean:
            all = [0, 1]
        elif kind == idltype.tk_enum:
            all = switchType.type().decl().enumerators()
        else:
            util.fatalError("Failed to generate a default union "
                            "discriminator value")

        for item in all:
            if item not in values:
                return switchType.literal(item, environment)

        return None
    #
    ###############################################################
    
    # does the IDL union have any default case?
    # It'll be handy to know which case is the default one later-
    # so add a new attribute to mark it
    ast.markDefaultCase(node)
    hasDefault = ast.defaultCase(node) != None
        
    # CORBA 2.3 C++ Mapping 1-34
    # "A union has an implicit default member if it does not have
    # a default case and not all permissible values of the union
    # discriminant are listed"
    exhaustive = ast.exhaustiveMatch(switchType, ast.allCaseLabelValues(node))
    implicitDefault = not hasDefault and not exhaustive

    if types.variableDecl(node):
        fixed = "Variable"
    else:
        fixed = "Fix"

    def Other_IDL(stream = stream, node = node):
        # deal with constructed switch type
        if node.constrType():
            node.switchType().decl().accept(self)
        
        # deal with children defined in this scope
        for n in node.cases():
            if n.constrType():
                n.caseType().decl().accept(self)

    
    # create the default constructor body
    def default_constructor(stream = stream,
                            implicitDefault = implicitDefault,
                            hasDefault = hasDefault,
                            choose = chooseArbitraryDefault):
        if implicitDefault:
            stream.out(template.union_constructor_implicit)
        elif hasDefault:
            stream.out(template.union_constructor_default,
                       default = choose())
        return

    def ctor_cases(stream = stream, node = node, switchType = switchType,
                   environment = environment, exhaustive = exhaustive):

        booleanWrap = switchType.boolean() and exhaustive
        trueName    = None

        for c in node.cases():
            for l in c.labels():
                if l.default(): continue
                
                discrimvalue = switchType.literal(l.value(), environment)
                name = id.mapID(c.declarator().identifier())

                if booleanWrap and discrimvalue == "1":
                    trueName = name

                stream.out(template.union_ctor_case,
                           discrimvalue = discrimvalue,
                           name = name)

        # Booleans are a special case (isn't everything?)
        if booleanWrap:
            stream.out(template.union_ctor_bool_default,
                       name = trueName)
        else:
            stream.out(template.union_ctor_default)
        return

    # create the copy constructor and the assignment operator
    # bodies
    def copy_constructor(stream = stream, exhaustive = exhaustive,
                         node = node, ctor_cases = ctor_cases):
        if not exhaustive:
            # grab the default case
            default = "_release_member();"
            for c in node.cases():
                if c.isDefault:
                    case_id = c.declarator().identifier()
                    cxx_case_id = id.mapID(case_id)
                    default = cxx_case_id + "(_value." + cxx_case_id + "());"


            stream.out(template.union_ctor_nonexhaustive,
                       default = default,
                       cases = ctor_cases)
        else:
            stream.out(template.union_ctor_exhaustive,
                       cases = ctor_cases)
        return
        
    # do we need an implicit _default function?
    def implicit_default(stream = stream, choose = chooseArbitraryDefault,
                         implicitDefault = implicitDefault):

        if implicitDefault:
            stream.out(template.union_implicit_default,
                       arbitraryDefault = choose())
        return

    # The body of the union _d(_value) function generated here
    def _d_fn(stream = stream, node = node, switchType = switchType,
              implicitDefault = implicitDefault,
              environment = environment):

        # The plan:
        #  * Check the _pd__initialised flag is set, else throw BAD_PARAM
        #  * Check for the simple case where _value == _pd__d and return
        #  * Have a nested switch, the outer switch is keyed on the current
        #    discriminator value and the inner one is the requested new value
        #
        # Possibilities:
        #  * Could perform some code minimisation eg for the case
        #      union foo switch(boolean){
        #         case TRUE:
        #         case FALSE:
        #           T bar;
        #      };
        #    This is equivalent to a single default: case and no switch is
        #    required.
        
        # Make sure we don't output a switch with no cases (if there is a
        # one-to-one mapping of labels to cases)
        need_switch = 0

        # Need to fill in a default case only if the union has none itself
        outer_has_default = 0

        cases = output.StringStream()

        # Produce a set of "case <foo>: goto fail;" for every label
        # except those in an exception list
        def fail_all_but(exceptions, node = node, cases = cases,
                         switchType = switchType, environment = environment):
            for c in node.cases():
                for l in c.labels():
                    if l not in exceptions:
                        cases.out("case @label@: goto fail;",
                                  label = switchType.literal(l.value(),
                                                             environment))
                        

        # switch (currently active case){
        #
        outer_has_default = 0 # only mention default: once
        for c in node.cases():

            need_switch = 1

            # If the currently active case has only one non-default label,
            # then the only legal action is to set it to its current value.
            # We've already checked for this in an if (...) statement before
            # here.
            if len(c.labels()) == 1 and not c.labels()[0].default():
                cases.out("case @label@: goto fail;",
                          label = switchType.literal(c.labels()[0].value(),
                                                     environment))
                continue

            # output one C++ case label for each IDL case label
            # case 1:
            # case 2:
            # default:

            this_case_is_default = 0
            for l in c.labels():
                if l.default():
                    this_case_is_default = 1
                    outer_has_default = 1
                    cases.out("default:")
                    continue

                cases.out("case @label@:",
                          label = switchType.literal(l.value(), environment))

            # switch (case to switch to){
            #
            cases.inc_indent()
            cases.out("switch (_value){\n")
            cases.inc_indent()

            # If we currently are in the default case, fail all attempts
            # to switch cases.
            if this_case_is_default:
                fail_all_but(c.labels())
                cases.out("default: _pd__d = _value; return;")
                cases.dec_indent()
                cases.out("}\n")
                cases.dec_indent()
                continue
                
            # This is not the default case, all possibilities have associated
            # UnionCaseLabels
            for l in c.labels():
                cases.out("case @label@: _pd__d = @label@; return;",
                          label = switchType.literal(l.value(), environment))

            
            cases.out("default: goto fail;")
            cases.dec_indent()
            cases.out("}\n")
            cases.dec_indent()
            
        if not outer_has_default and not implicitDefault:
            cases.out("default: goto fail;")

        # handle situation where have an implicit default member
        # (ie no actual case, but a legal set of discriminator values)
        # (assumes that the current discriminator is set to one of the
        # defaults)
        if implicitDefault:
            need_switch = 1
            cases.out("default:")
            cases.out("switch (_value){")
            cases.inc_indent()
            # again, make sure we aren't currently in the default state
            # and trying to set the discriminator to a non-default state
            fail_all_but([])

            cases.out("default: _pd__d = _value; return;")

            cases.dec_indent()
            cases.out("}")
                      

        # output the code here
        switch = output.StringStream()
        if need_switch:
            switch.out("switch (_pd__d){\n  @cases@\n};", cases = cases)
        stream.out(template.union_d_fn_body, switch = switch)
            

    # get and set functions for each case:
    def members(stream = stream, node = node, environment = environment,
                switchType = switchType):

        for c in node.cases():
            # Following the typedef chain will deliver the base type of
            # the alias. Whether or not it is an array is stored in an
            # ast.Typedef node.
            caseType = types.Type(c.caseType())
            d_caseType = caseType.deref()

            # the mangled name of the member
            decl = c.declarator()
            decl_dims = decl.sizes()

            full_dims = decl_dims + caseType.dims()
            
            is_array = full_dims != []
            is_array_declarator = decl_dims != []

            member = id.mapID(decl.identifier())
            
            memtype = caseType.member(environment)
            
            # Pick a discriminator value to use when selecting the
            # case. Pick the first non-default label if there is one.
            labels = c.labels()

            label = labels[0]
            if label.default() and len(labels) > 1:
                # Pick a non-default label if there is one
                label = labels[1]

            discrimvalue = switchType.literal(label.value(), environment)


            # only different when array declarator
            const_type_str = memtype

            # create typedefs for anonymous arrays
            if is_array_declarator:
                prefix = config.state['Private Prefix']
                stream.out(template.union_array_declarator,
                           prefix = prefix,
                           memtype = memtype,
                           name = member,
                           dims = cxx.dimsToString(decl.sizes()),
                           tail_dims = cxx.dimsToString(decl.sizes()[1:]))
                const_type_str = prefix + "_" + member
                memtype = "_" + member

            if is_array:
                # build the loop
                def loop(stream = stream, full_dims = full_dims,
                         member = member):
                    loop = cxx.For(stream, full_dims)
                    index = loop.index()
                    stream.out("\n_pd_" + member + index + " = _value" +\
                               index + ";\n")
                    loop.end()
                    return

                stream.out(template.union_array,
                           memtype = memtype,
                           const_type = const_type_str,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue,
                           first_dim = repr(full_dims[0]),
                           loop = loop)

            elif d_caseType.any():
                # note type != CORBA::Any when its an alias...
                stream.out(template.union_any,
                           type = memtype,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif d_caseType.typecode():
                stream.out(template.union_typecode,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif isinstance(d_caseType.type(), idltype.Base) or \
                 d_caseType.enum():
                # basic type
                stream.out(template.union_basic,
                           type = memtype,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif d_caseType.string():
                stream.out(template.union_string,
                           type = memtype,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif d_caseType.wstring():
                stream.out(template.union_wstring,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif d_caseType.interface():
                scopedName = d_caseType.type().decl().scopedName()

                name     = id.Name(scopedName)
                ptr_name = name.suffix("_ptr").unambiguous(environment)
                var_name = name.suffix("_var").unambiguous(environment)

                if isinstance(d_caseType.type().decl(), idlast.Forward):
                    helper = name.suffix("_Helper").unambiguous(environment)
                    duplicate = helper + "::duplicate"
                else:
                    iclass    = name.unambiguous(environment)
                    duplicate = iclass + "::_duplicate"

                stream.out(template.union_objref,
                           member = member,
                           memtype = memtype,
                           ptr_name = ptr_name,
                           var_name = var_name,
                           duplicate = duplicate,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif caseType.typedef() or d_caseType.struct() or \
                 d_caseType.union() or d_caseType.fixed():
                stream.out(template.union_constructed,
                           type = memtype,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif d_caseType.sequence():
                sequence_template = d_caseType.sequenceTemplate(environment)
                stream.out(template.union_sequence,
                           sequence_template = sequence_template,
                           member = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif d_caseType.value() or d_caseType.valuebox():
                scopedName = d_caseType.type().decl().scopedName()
                name = id.Name(scopedName)
                type = name.unambiguous(environment)

                stream.out(template.union_value,
                           member=member,
                           type=type,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            else:
                util.fatalError("Unknown union case type encountered")

        return

    # declare the instance of the discriminator and
    # the actual data members (shock, horror)
    union_body       = output.StringStream()
    release_body     = output.StringStream()
    need_release     = 0
    explicit_default = 0
    
    for c in node.cases():

        # find the dereferenced type of the member if it's an alias
        caseType   = types.Type(c.caseType())
        d_caseType = caseType.deref()
        case_kind  = d_caseType.type().kind()

        decl       = c.declarator()
        decl_dims  = decl.sizes()
        full_dims  = caseType.dims() + decl_dims
        
        is_array            = full_dims != []
        is_array_declarator = decl_dims != []

        member_name = id.mapID(c.declarator().identifier())
        member_type = caseType.base(environment)


        # Work out type for the union member
        if is_array_declarator:
            # _slice typedef defined earlier
            member_type = "_" + member_name + "_slice*"

        elif caseType.sequence():
            # _seq typedef defined earlier
            member_type = "_" + member_name + "_seq*"

        elif is_array:
            member_type = member_type + "_slice*"

        elif case_kind in [ idltype.tk_any,
                            idltype.tk_struct,
                            idltype.tk_union,
                            idltype.tk_sequence,
                            idltype.tk_except,
                            idltype.tk_fixed,
                            idltype.tk_value,
                            idltype.tk_value_box,
                            ]:
            member_type = member_type + "*"

        # Float types have conditional code that may or may not store
        # them by pointer.
        if not is_array and case_kind in [ idltype.tk_float,
                                           idltype.tk_double ]:
            union_member = template.union_member_float
        else:
            union_member = template.union_member

        # Output union member
        union_body.out(union_member,
                       type = member_type,
                       name = member_name)


        # Code to release the member
        release = None
        helper  = None

        if is_array:
            release = template.union_release_array

        elif case_kind in [ idltype.tk_any,
                            idltype.tk_struct,
                            idltype.tk_union,
                            idltype.tk_sequence,
                            idltype.tk_except,
                            idltype.tk_fixed,
                            ]:
            release = template.union_release_delete

        elif case_kind in [ idltype.tk_objref,
                            idltype.tk_abstract_interface,
                            idltype.tk_local_interface,
                            ]:

            if isinstance(d_caseType.type().decl(), idlast.Forward):
                # Use the helper class to call release
                release = template.union_release_helper

                scopedName = d_caseType.type().decl().scopedName()
                name       = id.Name(scopedName)
                helper     = name.suffix("_Helper").unambiguous(environment)
            else:
                release = template.union_release_corba_release

        elif case_kind in [ idltype.tk_TypeCode,
                            ]:
            release = template.union_release_corba_release

        elif case_kind in [ idltype.tk_value,
                            idltype.tk_value_box,
                            ]:
            release = template.union_release_valuetype

        elif case_kind == idltype.tk_string:
            release = template.union_release_string

        elif case_kind == idltype.tk_wstring:
            release = template.union_release_wstring

        if release is not None:
            need_release = 1
            release_member = output.StringStream()
            release_member.out(release, name=member_name, helper=helper)
        else:
            release_member = ""

        cases = output.StringStream()
        for label in c.labels():
            if label.default():
                cases.out("default:")
                explicit_default = 1
            else:
                cases.out("case @label@:",
                          label = switchType.literal(label.value(),
                                                     environment))

        release_body.out(template.union_release_case,
                         cases          = cases,
                         release_member = release_member)
        
    if need_release:
        if not explicit_default:
            # Output an empty default case to prevent fussy compilers
            # complaining
            release_body.out(template.union_release_case,
                             cases          = "default:",
                             release_member = "")
                
        release_member = output.StringStream()
        release_member.out(template.union_release_member,
                                 cases = release_body)
    else:
        release_member = template.union_release_member_empty

    # write out the union class
    stream.out(template.union,
               unionname           = cxx_id,
               fixed               = fixed,
               Other_IDL           = Other_IDL,
               default_constructor = default_constructor,
               copy_constructor    = copy_constructor,
               discrimtype         = d_switchType.base(environment),
               _d_body             = _d_fn,
               implicit_default    = implicit_default,
               members             = members,
               union_body          = union_body,
               release_member      = release_member)

    if types.variableDecl(node):
        stream.out(template.union_variable_out_type,
                   unionname = cxx_id)
    else:
        stream.out(template.union_fix_out_type,
                   unionname = cxx_id)

    popInsideClass()

    # TypeCode and Any
    if config.state['Typecode']:
        qualifier = const_qualifier(_insideModule, _insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_id)

    return


def visitUnionForward(node):
    cxx_name = id.mapID(node.identifier())
    stream.out(template.union_forward, name = cxx_name)


def visitEnum(node):
    name = id.mapID(node.identifier())

    enumerators = node.enumerators()
    memberlist = [id.Name(x.scopedName()).simple() for x in enumerators]
    stream.out(template.enum,
               name = name,
               memberlist = ", ".join(memberlist))

    # TypeCode and Any
    if config.state['Typecode']:
        qualifier = const_qualifier(_insideModule, _insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier, name = name)
    
    return

def visitValue(node):
    v = value.getValueType(node)
    v.module_decls(stream, self)

def visitValueAbs(node):
    v = value.getValueType(node)
    v.module_decls(stream, self)

def visitValueForward(node):
    v = value.getValueType(node)
    v.module_decls(stream, self)

def visitValueBox(node):
    v = value.getValueType(node)
    v.module_decls(stream, self)
