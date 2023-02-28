# -*- python -*-
#                           Package   : omniidl
# main.py                   Created on: 1999/11/12
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
#   Produce the main skeleton definitions

"""Produce the main skeleton definitions"""

from omniidl import idlast
from omniidl_be.cxx import cxx, ast, output, id, config, skutil, types, iface
from omniidl_be.cxx.skel import template


import sys
self = sys.modules[__name__]

stream = None

def init(stream):
    self.stream = stream

    # To keep track of our depth with the AST
    self.__insideInterface = 0
    self.__insideModule = 0

    # An entry in this dictionary indicates a flattened typedef
    # already exists for this interface. See comments later for
    # explanation.
    self.__flattened_interfaces = {}
    
    return self

# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    insideModule = self.__insideModule
    self.__insideModule = 1
    
    for n in node.definitions():
        n.accept(self)

    self.__insideModule = insideModule


def visitInterface(node):
    ident = node.identifier()

    outer_environment = id.lookup(node)
    environment = outer_environment.enter(ident)

    insideInterface = self.__insideInterface
    self.__insideInterface = 1

    # produce skeletons for types declared here
    for n in node.declarations():
        n.accept(self)

    self.__insideInterface = insideInterface

    # Call descriptor names are of the form:
    #  TAG _ PREFIX _ BASE
    # Tag represents the type of thing {call descriptor, local callback...}
    # Prefix is derived from the first encountered scopedname[1]
    # Base is a counter to uniquify the identifier
    #
    # [1] Since the names are guaranteed unique, the prefix makes the
    #     names used by two different modules disjoint too. Not sure why
    #     as they are not externally visible?

    I = iface.Interface(node)
    I_Helper = iface.I_Helper(I)
    I_Helper.cc(stream)


    # the class itself
    node_name = id.Name(node.scopedName())

    if node.local():
        objref_name = node_name.prefix("_nil_")
    else:
        objref_name = node_name.prefix("_objref_")

    if node.abstract():
        stream.out(template.abstract_interface_duplicate_narrow,
                   name = node_name.fullyQualify())
    else:
        if node.local():
            stream.out(template.local_interface_duplicate_narrow,
                       name = node_name.fullyQualify())
        else:
            stream.out(template.interface_duplicate_narrow,
                       name = node_name.fullyQualify())

        for i in I.allInherits():
            if i.abstract():
                stream.out(template.interface_narrow_abstract,
                           name = node_name.fullyQualify())
                break

    stream.out(template.interface_nil,
               name = node_name.fullyQualify(),
               objref_name = objref_name.unambiguous(environment),
               repoID = node.repoId())

    # Output flattened aliases to inherited classes, to workaround an
    # MSVC bug.
    for i in ast.allInherits(node):
        inherits_name = id.Name(i.scopedName())
        if inherits_name.needFlatName(environment):
            guard_name = inherits_name.guard()
            flat_fqname = inherits_name.flatName()

            if i.local():
                inherits_nil_name  = inherits_name.prefix("_nil_")
                nil_flat_fqname    = inherits_nil_name.flatName()

                stream.out(template.local_interface_ALIAS,
                           guard_name = guard_name,
                           fqname = inherits_name.fullyQualify(),
                           flat_fqname = flat_fqname,
                           nil_fqname = inherits_nil_name.fullyQualify(),
                           nil_flat_fqname = nil_flat_fqname)

            else:
                inherits_impl_name   = inherits_name.prefix("_impl_")
                inherits_objref_name = inherits_name.prefix("_objref_")

                impl_flat_fqname   = inherits_impl_name.flatName()
                objref_flat_fqname = inherits_objref_name.flatName()

                stream.out(template.interface_ALIAS,
                           guard_name = guard_name,
                           fqname = inherits_name.fullyQualify(),
                           flat_fqname = flat_fqname,
                           impl_fqname = inherits_impl_name.fullyQualify(),
                           impl_flat_fqname = impl_flat_fqname,
                           objref_fqname = inherits_objref_name.fullyQualify(),
                           objref_flat_fqname = objref_flat_fqname)

    if node.local():
        _nil_I = iface._nil_I(I)
        _nil_I.cc(stream)

    else:
        _objref_I = iface._objref_I(I)
        _objref_I.cc(stream)

        _pof_I = iface._pof_I(I)
        _pof_I.cc(stream)

        _impl_I = iface._impl_I(I)
        _impl_I.cc(stream)


        # BOA compatible skeletons
        if config.state['BOA Skeletons']:
            sk_name = node_name.prefix("_sk_")
            stream.out(template.interface_sk,
                       sk_fqname = sk_name.fullyQualify(),
                       sk_name = sk_name.unambiguous(environment))


def visitTypedef(node):
    aliasType = types.Type(node.aliasType())
    d_type = aliasType.deref()

    if node.constrType():
        aliasType.type().decl().accept(self)

    for d in node.declarators():
        scopedName = id.Name(d.scopedName())
        
        if d_type.sequence() and not aliasType.typedef():
            seqType = types.Type(d_type.type().seqType())
            d_seqType = seqType.deref()

            if d_seqType.structforward() or d_seqType.unionforward():
                fqname  = scopedName.fullyQualify()
                name    = id.mapID(d.identifier())
                element = d_seqType.base()
                bound   = d_type.type().bound()
                derived = d_type.sequenceTemplate()
                
                if bound > 0:
                    stream.out(template.sequence_forward_bounded_defns,
                               bound=bound, fqname=fqname, name=name,
                               element=element, derived=derived)
                else:
                    stream.out(template.sequence_forward_unbounded_defns,
                               fqname=fqname, name=name,
                               element=element, derived=derived)

                stream.out(template.sequence_forward_defns,
                           fqname=fqname, name=name, element=element)

def visitEnum(node):
    return

def visitMember(node):
    memberType = node.memberType()
    if node.constrType():
        memberType.decl().accept(self)
        
def visitStruct(node):

    outer_environment = id.lookup(node)
    environment = outer_environment.enter(node.identifier())
    
    scopedName = id.Name(node.scopedName())

    for n in node.members():
        n.accept(self)

    def marshal(stream = stream, node = node, env = environment):
        for n in node.members():
            memberType = types.Type(n.memberType())
            for d in n.declarators():
                scopedName = id.Name(d.scopedName())
                member_name = scopedName.simple()
                skutil.marshall(stream, env, memberType, d, member_name, "_n")
        return

    def unmarshal(stream = stream, node = node, env = environment):
        for n in node.members():
            memberType = types.Type(n.memberType())
            for d in n.declarators():
                scopedName = id.Name(d.scopedName())
                member_name = scopedName.simple()
                skutil.unmarshall(stream, env,
                                  memberType, d, member_name, "_n")
        return

    stream.out(template.struct,
               name = scopedName.fullyQualify(),
               marshall_code = marshal,
               unmarshall_code = unmarshal)

    stream.reset_indent()
    
def visitStructForward(node):
    pass

def visitUnion(node):
    outer_environment = id.lookup(node)
    environment = outer_environment.enter(node.identifier())

    scopedName = id.Name(node.scopedName())
    name = scopedName.fullyQualify()

    switchType = types.Type(node.switchType())

    exhaustive = ast.exhaustiveMatch(switchType, ast.allCaseLabelValues(node))
    defaultCase = ast.defaultCase(node)
    ast.markDefaultCase(node)
        
    hasDefault = defaultCase != None

    # deal with types constructed here
    if node.constrType():
        node.switchType().decl().accept(self)
    for n in node.cases():
        if n.constrType():
            n.caseType().decl().accept(self)

    # --------------------------------------------------------------
    # union::operator{>>, <<}= (cdrStream& _n) [const]
    #
    # marshal/ unmarshal individual cases
    marshal_discriminator = output.StringStream()
    unmarshal_discriminator = output.StringStream()
    
    skutil.marshall(marshal_discriminator,environment,
                    switchType, None, "_pd__d", "_n")

    skutil.unmarshall(unmarshal_discriminator,environment,
                      switchType, None, "_pd__d", "_n")

    marshal_cases   = output.StringStream()
    unmarshal_cases = output.StringStream()

    for c in node.cases():
        caseType = types.Type(c.caseType())
        decl = c.declarator()
        decl_scopedName = id.Name(decl.scopedName())
        decl_name = decl_scopedName.simple()

        if defaultCase == c:
            isDefault = 1
        else:
            isDefault = 0
        
        for l in c.labels():
            value = l.value()
            discrim_value = switchType.literal(value, environment)
            if l.default():
                unmarshal_cases.out("default:")
                marshal_cases.out("default:")
            else:
                unmarshal_cases.out("case " + discrim_value + ":")
                marshal_cases.out("case " + discrim_value + ":")

        marshal_cases.inc_indent()
        skutil.marshall(marshal_cases, environment,
                        caseType, decl, "_pd_" + decl_name, "_n",
                        is_union=1)
        marshal_cases.out("break;")
        marshal_cases.dec_indent()

        unmarshal_cases.inc_indent()
        unmarshal_cases.out("_pd__default = %d;" % isDefault)
        skutil.unmarshall(unmarshal_cases, environment,
                          caseType, decl, "_pd_" + decl_name, "_n",
                          is_union=1)
        unmarshal_cases.out("break;")
        unmarshal_cases.dec_indent()


    if not hasDefault and not exhaustive:
        unmarshal_cases.out("""\
default:
  _pd__default = 1;
  break;""")

    # write the operators
    stream.out(template.union_operators,
               name = name,
               marshal_discriminator = str(marshal_discriminator),
               unmarshal_discriminator = str(unmarshal_discriminator),
               marshal_cases = str(marshal_cases),
               unmarshal_cases = str(unmarshal_cases))
                
        
    return
    
def visitUnionForward(node):
    pass
    
def visitForward(node):
    return

def visitConst(node):
    environment = id.lookup(node)
    
    constType = types.Type(node.constType())
    d_constType = constType.deref()
    
    if d_constType.string():
        type_string = "char *"
    elif d_constType.wstring():
        type_string = "::CORBA::WChar *"
    elif d_constType.fixed():
        type_string = constType.base()
    else:
        type_string = d_constType.base()

    scopedName = id.Name(node.scopedName())
    name = scopedName.fullyQualify()
    value = d_constType.literal(node.value(), environment)
    
    init_in_def = d_constType.representable_by_int()
    
    if init_in_def:
        if self.__insideInterface:
            stream.out(template.const_in_interface,
                       type = type_string, name = name, value = value)
        else:
            stream.out(template.const_init_in_def,
                       type = type_string, name = name, value = value)
        return

    # not init_in_def
    if self.__insideModule and not self.__insideInterface:
        scopedName = node.scopedName()
        scopedName = list(map(id.mapID, scopedName))

        open_namespace  = ""
        close_namespace = ""

        for s in scopedName[:-1]:
            open_namespace  = open_namespace  + "namespace " + s + " { "
            close_namespace = close_namespace + "} "

        simple_name = scopedName[-1]

        stream.out(template.const_namespace,
                   open_namespace = open_namespace,
                   close_namespace = close_namespace,
                   type = type_string, simple_name = simple_name,
                   name = name, value = value)
        
    else:
        stream.out(template.const_simple,
                   type = type_string, name = name, value = value)
        


def visitDeclarator(node):
    pass

def visitException(node):
    scopedName = id.Name(node.scopedName())
    name = scopedName.simple()
    
    outer_environment = id.lookup(node)
    environment = outer_environment.enter(name)
    
    scoped_name = scopedName.fullyQualify()

    # build the default ctor, copy ctor, assignment operator
    copy_ctor_body = output.StringStream()
    default_ctor_body = output.StringStream()
    default_ctor_args = []
    assign_op_body = output.StringStream()
    has_default_ctor = 0

    for m in node.members():
        has_default_ctor = 1
        memberType = types.Type(m.memberType())
        if m.constrType():
            memberType.type().decl().accept(self)
        d_memberType = memberType.deref()

        memberType_fqname = memberType.base()
            
        for d in m.declarators():
            decl_scopedName = id.Name(d.scopedName())
            decl_name = decl_scopedName.simple()
            
            decl_dims = d.sizes()
            full_dims = decl_dims + memberType.dims()
            is_array = full_dims != []
            is_array_declarator = decl_dims != []

            memberType_name_arg = memberType.op(types.IN, environment)

            if is_array_declarator:
                # we use the internal typedef'ed type if the member is an array
                # declarator
                memberType_name_arg = "const "                       +\
                                      config.state['Private Prefix'] +\
                                      "_" + decl_name
            elif d_memberType.sequence():
                if memberType.typedef():
                    memberType_name_arg = "const " + id.Name(memberType.type().decl().scopedName()).unambiguous(environment)
                else:
                    memberType_name_arg = "const " + memberType.sequenceTemplate(environment)
            elif memberType.typecode():
                memberType_name_arg = "::CORBA::TypeCode_ptr"
                
            index = ""

            if is_array:
                blocks = [cxx.Block(copy_ctor_body),
                          cxx.Block(default_ctor_body),
                          cxx.Block(assign_op_body)]
                loops = [cxx.For(copy_ctor_body, full_dims),
                         cxx.For(default_ctor_body, full_dims),
                         cxx.For(assign_op_body, full_dims)]
                index = loops[0].index() # all the same

            copy_ctor_body.out("""\
@member_name@@index@ = _s.@member_name@@index@;""", member_name = decl_name,
                               index = index)

            if (d_memberType.interface() and not is_array):

                # these are special resources which need to be explicitly
                # duplicated (but not if an array?)
                duplicate = memberType_fqname.replace("_ptr", "") + \
                            "::_duplicate"

                if isinstance(d_memberType.type().decl(),idlast.Forward):
                    duplicate = duplicate.replace("::_dup", "_Helper::dup")

                default_ctor_body.out("""\
@duplicate@(_@member_name@@index@);""",
                                      duplicate = duplicate,
                                      member_name = decl_name,
                                      index = index)
            
            default_ctor_args.append(memberType_name_arg + " _" + decl_name)
            default_ctor_body.out("""\
@member_name@@index@ = _@member_name@@index@;""", member_name = decl_name,
                                  index = index)

            assign_op_body.out("""\
@member_name@@index@ = _s.@member_name@@index@;""", member_name = decl_name,
                               index = index)
            
            if is_array:
                for loop in loops: loop.end()
                for block in blocks: block.end()
          
        
    default_ctor = output.StringStream()
    if has_default_ctor:
        default_ctor.out(template.exception_default_ctor,
                         scoped_name = scoped_name,
                         name = name,
                         ctor_args = ", ".join(default_ctor_args),
                         default_ctor_body = str(default_ctor_body))

    # write the main chunk
    stream.out(template.exception,
               scoped_name = scoped_name,
               name = name,
               copy_ctor_body = str(copy_ctor_body),
               default_ctor = str(default_ctor),
               ctor_args = ", ".join(default_ctor_args),
               default_ctor_body = str(default_ctor_body),
               repoID = node.repoId(),
               assign_op_body = str(assign_op_body))
    

    # deal with marshalling and demarshalling
    needs_marshalling = node.members() != []
    marshal = output.StringStream()
    unmarshal = output.StringStream()
    
    for m in node.members():
        memberType = types.Type(m.memberType())
        d_memberType = memberType.deref()
        for d in m.declarators():
            decl_scopedName = id.Name(d.scopedName())
            decl_name = decl_scopedName.simple()
            is_array_declarator = d.sizes() != []
            
            skutil.unmarshall(unmarshal, environment,
                              memberType, d, decl_name, "_n")

            skutil.marshall(marshal, environment,
                            memberType, d, decl_name, "_n")

    if needs_marshalling:
        stream.out(template.exception_operators,
                   scoped_name = scoped_name,
                   marshal = str(marshal),
                   unmarshal = str(unmarshal))


    return


def visitValue(node):
    from omniidl_be.cxx import value
    v = value.getValueType(node)
    v.skel_defs(stream, self)

def visitValueForward(node):
    from omniidl_be.cxx import value
    v = value.getValueType(node)
    v.skel_defs(stream, self)

def visitValueAbs(node):
    from omniidl_be.cxx import value
    v = value.getValueType(node)
    v.skel_defs(stream, self)

def visitValueBox(node):
    from omniidl_be.cxx import value
    v = value.getValueType(node)
    v.skel_defs(stream, self)

