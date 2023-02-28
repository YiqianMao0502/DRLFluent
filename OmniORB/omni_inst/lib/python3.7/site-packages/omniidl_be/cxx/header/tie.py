# -*- python -*-
#                           Package   : omniidl
# tie.py                    Created on: 1999/12/13
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2007-2011 Apasphere Ltd
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

"""Produce the 'tie' templates"""

from omniidl import idlast, idlvisitor
from omniidl_be.cxx import id, config, types, output, ast
from omniidl_be.cxx.header import template

import sys
self = sys.modules[__name__]


# Write a single tie template class called <name>, inheriting from <inherits>
# and grab the operations from <node>
def write_template(name, inherits, node, stream,
                   Template = template.tie_template):
    # build methods which bind the interface operations and attributes
    # note that this includes inherited callables since tie
    # templates are outside the normal inheritance structure
    where = output.StringStream()

    # defined_so_far contains keys corresponding to method names which
    # have been defined already (and which should not be included twice)
    def buildCallables(interface, where, continuation, defined_so_far = {}):
        interface = ast.remove_ast_typedefs(interface)
        
        callables = interface.callables()
        operations = [x for x in callables if isinstance(x, idlast.Operation)]
        for operation in operations:
            returnType = types.Type(operation.returnType())
            identifier = operation.identifier()
            if (identifier in defined_so_far):
                # don't repeat it
                continue
            defined_so_far[identifier] = 1
            
            parameters = operation.parameters()
            has_return_value = not returnType.void()
            # FIXME: return types are fully scoped but argument types
            # arent?
            returnType_name = returnType.op(types.RET)

            operation_name = id.mapID(identifier)
            
            signature = []
            call = []

            for parameter in parameters:
                paramType = types.Type(parameter.paramType())
                # Need to call the _impl operation not the _objref operation
                param_type_name = paramType.op(types.direction(parameter),
                                               use_out = 0)
                param_id = id.mapID(parameter.identifier())
                signature.append(param_type_name + " " + param_id)
                call.append(param_id)

            # deal with call contextx
            if operation.contexts() != []:
                signature.append("::CORBA::Context_ptr _ctxt")
                call.append("_ctxt")

            if has_return_value:
                return_str = "return "
            else:
                return_str = ""
                
            where.out("""\
@return_type_name@ @operation_name@(@signature@) { @return_str@pd_obj->@operation_name@(@call@); }""", return_type_name = returnType_name,
                      operation_name = operation_name,
                      return_str = return_str,
                      signature = ", ".join(signature),
                      call = ", ".join(call))
                    
        attributes = [x for x in callables if isinstance(x, idlast.Attribute)]
        for attribute in attributes:
            identifiers = attribute.identifiers()
            attrType = types.Type(attribute.attrType())

            attrType_name_RET = attrType.op(types.RET)
            attrType_name_IN = attrType.op(types.IN)
            
            for identifier in identifiers:
                if identifier in defined_so_far:
                    # don't repeat it
                    continue
                defined_so_far[identifier] = 1
                
                ident = id.mapID(identifier)
                where.out("""\
@attr_type_ret_name@ @attribute_name@() { return pd_obj->@attribute_name@(); }""", attr_type_ret_name = attrType_name_RET,
                          attribute_name = ident)

                if not attribute.readonly():
                    where.out("""\
void @attribute_name@(@attr_type_in_name@ _value) { pd_obj->@attribute_name@(_value); }""", attribute_name = ident,
                              attr_type_in_name = attrType_name_IN)                    
        # do the recursive bit
        for i in interface.inherits():
            i = i.fullDecl()
            continuation(i, where, continuation, defined_so_far)

        # done
        return

    buildCallables(node, where, buildCallables)
                
    stream.out(Template,
               tie_name = name,
               inherits = inherits,
               callables = str(where))
    return


# Unflattened BOA tie templates are built in a block out of line.
# IDL name       template name
#  ::A             ::_tie_A
#  ::B             ::_tie_B
#  ::M::C          ::_tie_M::C

class BOATieTemplates(idlvisitor.AstVisitor):
    def __init__(self, stream):
        self.stream = stream
        
    def visitAST(self, node):
        for d in node.declarations():
            if ast.shouldGenerateCodeForDecl(d):
                d.accept(self)

    def visitModule(self, node):
        name = id.Name(node.scopedName())
        
        self.stream.out(template.module_begin,
                   name = "_tie_" + name.simple())
        self.stream.inc_indent()
        
        for d in node.definitions(): d.accept(self)

        self.stream.dec_indent()
        self.stream.out(template.module_end)
        

    def visitInterface(self, node):
        name = id.Name(node.scopedName())

        tie_name = name.simple()
        if len(node.scopedName()) == 1: tie_name = "_tie_" + tie_name
        
        sk_name = name.prefix("_sk_")
        
        write_template(tie_name, sk_name.fullyQualify(), node, self.stream,
                       Template = template.tie_template_old)


# Flat Tie Templates are all (by definition) in the global scope,
# so can combine POA and BOA code into one
class FlatTieTemplates(idlvisitor.AstVisitor):
    def __init__(self, stream):
        self.stream = stream
        
    def visitAST(self, node):
        for d in node.declarations():
            if ast.shouldGenerateCodeForDecl(d):
                d.accept(self)
                
    def visitModule(self, node):
        for d in node.definitions():
            d.accept(self)

    def visitInterface(self, node):
        self.generate_POA_tie(node)
        if config.state['BOA Skeletons']:
            self.generate_BOA_tie(node)

    def generate_BOA_tie(self, node):
        name = id.Name(node.scopedName())
        tie_name = "_tie_" + "_".join(name.fullName())
        sk_name = name.prefix("_sk_")

        write_template(tie_name, sk_name.fullyQualify(), node, self.stream,
                       Template = template.tie_template_old)

    def generate_POA_tie(self, node):
        name = id.Name(node.scopedName())
        tie_name = "POA_" + "_".join(name.fullName()) + "_tie"
        poa_name = "POA_" + name.fullyQualify()

        write_template(tie_name, poa_name, node, self.stream)
