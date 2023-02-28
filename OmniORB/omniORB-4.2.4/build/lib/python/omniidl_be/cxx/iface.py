# -*- python -*-
#                           Package   : omniidl
# iface.py                  Created on: 2000/8/10
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2002-2011 Apasphere Ltd
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
#   Code associated with IDL interfaces

from omniidl import idlast, idltype
from omniidl_be.cxx import types, id, call, ast, cxx, output, config, descriptor

header_template = None
skel_template   = None

def init():
    global header_template, skel_template
    import omniidl_be.cxx.header.template
    import omniidl_be.cxx.skel.template

    header_template = omniidl_be.cxx.header.template
    skel_template   = omniidl_be.cxx.skel.template


# We access the templates from sys.modules to avoid circular import problems.
#header_template = sys.modules["omniidl_be.cxx.header.template"]
#skel_template   = sys.modules["omniidl_be.cxx.skel.template"]


# Interface is a wrapper around an IDL interface
#  .callables():   get a list of Callable objects representing the operations
#                  and attributes
#  .inherits():    get a list of all directly inherited interfaces
#  .allInherits(): get all inherited interfaces (using a breadth first search)
#  .name():        return the IDL fully scoped name (as an id.Name)
#  .environment(): returns the IDL environment where this interface was
#                  declared
class Interface:
    """Wrapper around an IDL interface"""
    def __init__(self, node):
        self._node          = node
        self._environment   = id.lookup(node)
        self._node_name     = id.Name(node.scopedName())
        self._callables     = None
        self._ami_callables = None
        self._ami_handler   = None
        self._ami_poller    = None


    def callables(self):
        """
        Return a list of Callable objects representing the combined
        operations and attributes for this interface
        """
        if self._callables is not None:
            return self._callables
        
        # build a list of all the Callable objects
        self._callables = []

        for c in self._node.callables():
            if isinstance(c, idlast.Operation):
                self._callables.append(call.operation(self, c))
            else:
                self._callables.extend(call.read_attributes(self, c))
                if not c.readonly():
                    self._callables.extend(call.write_attributes(self, c))
            
        return self._callables

    def ami_callables(self):
        """
        Return a list of Callable objects representing the AMI
        pseudo-operations for this interface.
        """
        if self._ami_callables is not None:
            return self._ami_callables
        
        self._ami_callables = []

        if not hasattr(self._node, "_ami_ops"):
            return self._ami_callables

        for c in self._node._ami_ops:
            self._ami_callables.append(call.operation(self, c))
            
        return self._ami_callables


    def amiHandler(self):
        """
        Return Interface object corresponding to AMI handler, or None
        if no AMI handler.
        """
        if self._ami_handler is not None:
            return self._ami_handler

        if not hasattr(self._node, "_ami_handler"):
            return None

        self._ami_handler = Interface(self._node._ami_handler)
        return self._ami_handler

    def amiPoller(self):
        """
        Return Value object corresponding to AMI poller, or None if no
        AMI poller.
        """
        if self._ami_poller is not None:
            return self._ami_poller

        if not hasattr(self._node, "_ami_poller"):
            return None

        from omniidl_be.cxx import value
        self._ami_poller = value.ValueType(self._node._ami_poller)
        return self._ami_poller

    def inherits(self):
        return [Interface(x) for x in self._node.fullDecl().inherits()]

    def allInherits(self):
        return [Interface(x) for x in ast.allInherits(self._node)]

    def local(self):
        return self._node.fullDecl().local()

    def abstract(self):
        return self._node.fullDecl().abstract()

    def name(self):
        return self._node_name
        
    def environment(self):
        return self._environment
    
    
_proxy_call_descriptors = {}


# Class associated with an IDL interface.
#  .interface():   return the associated Interface object
#  .methods():     return a list of Method objects
#  .environment(): return the IDL environment associated with the interface
class Class(cxx.Class):
    def __init__(self, interface):
        assert isinstance(interface, Interface)
        cxx.Class.__init__(self, interface.name())
        
        self._interface   = interface
        self._environment = interface.environment()
        self._methods     = []
        self._callables   = {}

    def interface(self):   return self._interface
    def methods(self):     return self._methods
    def environment(self): return self._environment


class _objref_Method(cxx.Method):
    def __init__(self, callable, parent_class):
        assert isinstance(callable, call.Callable)
        self._callable     = callable
        self._parent_class = parent_class
        self.from_Callable()

    def callable(self):
        return self._callable

    def from_Callable(self):
        use_out = not config.state["Impl Mapping"]
        self._from_Callable(use_out = use_out)

    def _from_Callable(self, use_out):
        # Grab the IDL environment
        ifc = self.callable().interface()
        environment = ifc.environment().enter("_objref_" + ifc.name().simple())

        # Kept as a type object because in .cc part the _return_ type
        # must be fully qualified.
        self._return_type = types.Type(self.callable().returnType())

        # Parameters are always relative, both in .hh and .cc
        (param_types, param_names) = ([], [])
        for p in self.callable().parameters():
            pType = types.Type(p.paramType())
            direction = types.direction(p)
            param_types.append(pType.op(direction, environment,
                                        use_out = use_out))

            # Special ugly case. If the IDL says something like (in foo::bar
            # bar), the parameter name may be the same as the relative type
            # name. We mangle the parameter name if this happens.

            typeBase = pType.base(environment)
            ident    = id.mapID(p.identifier())

            if typeBase == ident:
                ident = "_" + ident

            param_names.append(ident)
            
        # an operation has optional context
        if self.callable().contexts() != []:
            param_types.append("::CORBA::Context_ptr")
            param_names.append("_ctxt")

        self._arg_types = param_types
        self._arg_names = param_names
        self._name = self.callable().method_name()


class _impl_Method(_objref_Method):
    def __init__(self, callable, parent_class):
        _objref_Method.__init__(self, callable, parent_class)

    def from_Callable(self):
        self._from_Callable(use_out = 0)


class I_Helper(Class):
    def __init__(self, I):
        Class.__init__(self, I)
        self._name = self._name.suffix("_Helper")

    def hh(self, stream):
        class_sk_name = ""
        if config.state['BOA Skeletons']:
            class_sk_name = ("class %s;" %
                             self.interface().name().prefix("_sk_").simple())

        if self.interface().abstract():
            helper_tpl = header_template.abstract_interface_Helper

        elif self.interface().local():
            helper_tpl = header_template.local_interface_Helper

        else:
            helper_tpl = header_template.interface_Helper

        stream.out(helper_tpl,
                   class_sk_name = class_sk_name,
                   name = self.interface().name().simple(),
                   guard = self.interface().name().guard())

    def cc(self, stream):
        stream.out(skel_template.interface_Helper,
                   name = self.interface().name().fullyQualify())


class I(Class):
    def __init__(self, i, other_idl):
        Class.__init__(self, i)
        self._other_idl = other_idl

    def hh(self, stream):
        env = self._environment

        if self.interface().abstract():

            for callable in self.interface().callables():
                method = _impl_Method(callable, self)
                self._methods.append(method)
                self._callables[method] = callable

            methodl = []
            for method in self.methods():
                    methodl.append(method.hh(virtual = 1, pure = 1))

            inh = self.interface().inherits()
            if inh:
                inheritl = []
                for i in inh:
                    iname = i.name().unambiguous(env)
                    inheritl.append("public virtual " + iname)
            else:
                inheritl = [ "public virtual ::CORBA::AbstractBase" ]

            stream.out(header_template.abstract_interface_type,
                       name       = self.interface().name().simple(),
                       Other_IDL  = self._other_idl,
                       inherits   = ",\n".join(inheritl),
                       operations = "\n".join(methodl))

        elif self.interface().local():
            abstract_base = 0
            for i in self.interface().allInherits():
                if i.abstract():
                    abstract_base = 1
                    break

            if abstract_base:
                abstract_narrows = header_template.interface_abstract_narrows
            else:
                abstract_narrows = ""

            for callable in self.interface().callables():
                method = _impl_Method(callable, self)
                self._methods.append(method)
                self._callables[method] = callable

            # Override methods from inherited non-local interfaces
            for i in self.interface().allInherits():
                if not i.local():
                    for c in i.callables():
                        # Make new callable with ref to this interface
                        # instead of inherited interface
                        callable = call.Callable(self.interface(),
                                                 c.node(),
                                                 c.operation_name(),
                                                 c.method_name(),
                                                 c.returnType(),
                                                 c.parameters(),
                                                 c.oneway(),
                                                 c.raises(),
                                                 c.contexts())
                        
                        method = _impl_Method(callable, self)
                        self._methods.append(method)
                        self._callables[method] = callable

            bmethodl = []
            nmethodl = []
            for method in self.methods():
                bmethodl.append(method.hh(virtual = 1, pure = 1))
                nmethodl.append(method.hh(virtual = 1, pure = 0))

            local_base = 0
            inheritl   = []
            ninheritl  = []

            for i in self.interface().inherits():
                if i.local():
                    local_base = 1
                    iname  = i.name().unambiguous(env)
                    niname = i.name().prefix("_nil_").unambiguous(env)
                    ninheritl.append(niname)
                else:
                    iname = i.name().prefix("_objref_").unambiguous(env)

                inheritl.append("public virtual " + iname)

            if not local_base:
                inheritl.append("public virtual ::CORBA::LocalObject")

            if ninheritl:
                nil_inherits = ",\n".join(ninheritl) + ","
            else:
                nil_inherits = ""

            stream.out(header_template.local_interface_type,
                       name             = self.interface().name().simple(),
                       abstract_narrows = abstract_narrows,
                       Other_IDL        = self._other_idl,
                       inherits         = ",\n".join(inheritl),
                       operations       = "\n".join(bmethodl),
                       nil_operations   = "\n".join(nmethodl),
                       nil_inherits     = nil_inherits)

        else:
            abstract_base = 0
            for i in self.interface().allInherits():
                if i.abstract():
                    abstract_base = 1
                    break

            if abstract_base:
                abstract_narrows = header_template.interface_abstract_narrows
            else:
                abstract_narrows = ""

            stream.out(header_template.interface_type,
                       name             = self.interface().name().simple(),
                       abstract_narrows = abstract_narrows,
                       Other_IDL        = self._other_idl)



class AMIPoller (cxx.Class):

    def __init__(self, name):
        cxx.Class.__init__(self, id.Name([name]))
        self._environment = id.Environment()

    def environment(self):
        return self._environment


class _objref_I(Class):
    def __init__(self, I):
        Class.__init__(self, I)
        self._name = self._name.prefix("_objref_")

        # In abstract interfaces, signatures are like _impl classes, not
        # normal _objref classes.
        if self.interface().abstract():
            cls = _impl_Method
        else:
            cls = _objref_Method

        intf = self.interface()

        method_map = {}

        for callable in intf.callables():
            method = cls(callable, self)
            self._methods.append(method)
            self._callables[method] = callable
            method_map[callable.operation_name()] = method

        self._ami_methods = []
        for callable in intf.ami_callables():
            method = cls(callable, self)
            self._ami_methods.append(method)
            self._callables[method] = callable

            node       = callable.node()
            ami_from   = node._ami_from
            from_ident = ami_from.identifier()

            if isinstance(ami_from, idlast.Declarator):
                # Attribute
                if node._ami_setter:
                    from_op = "_set_" + from_ident
                else:
                    from_op = "_get_" + from_ident
            else:
                from_op = from_ident

            from_method = method_map[from_op]
            try:
                from_method._ami_methods.append(method)

            except AttributeError:
                from_method._ami_methods = [method]


    def ami_methods(self):
        return self._ami_methods


    def hh(self, stream):
        # build the inheritance list

        objref_inherits = []

        inh = self.interface().inherits()
        if inh:
            for i in inh:
                objref_inherited_name = i.name().prefix("_objref_")
                uname = objref_inherited_name.unambiguous(self._environment)
                objref_inherits.append("public virtual " + uname)
        else:
            if self.interface().abstract():
                objref_inherits = [
                    "public virtual ::CORBA::_omni_AbstractBaseObjref",
                    "public virtual omniObjRef"
                    ]
            else:
                objref_inherits = [
                    "public virtual ::CORBA::Object",
                    "public virtual omniObjRef"
                    ]

        if self.interface().abstract():
            objref_inherits.append("public virtual " +
                                   self.interface().name().simple())

        methods = []
        for method in self.methods():
            if config.state['Virtual Objref Methods']:
                methods.append(method.hh(virtual = 1, pure = 0))
            else:
                methods.append(method.hh())

        if self.ami_methods():
            methods.append("\n// AMI methods")

            for method in self.ami_methods():
                methods.append(method.hh())


        if config.state['Shortcut']:
            shortcut = output.StringStream()
            shortcut.out(header_template.interface_shortcut,
                         name = self.interface().name().simple())
            shortcut = str(shortcut)
            init_shortcut = ": _shortcut(0)"
        else:
            shortcut = ""
            init_shortcut = ""

        stream.out(header_template.interface_objref,
                   name          = self.interface().name().simple(),
                   inherits      = ",\n".join(objref_inherits),
                   operations    = "\n".join(methods),
                   shortcut      = shortcut,
                   init_shortcut = init_shortcut)


    def cc(self, stream):

        def _ptrToObjRef_ptr(self = self, stream = stream):
            has_abstract = self.interface().abstract()

            for i in self.interface().allInherits():
                if i.abstract(): has_abstract = 1

                stream.out(skel_template.interface_objref_repoID_ptr,
                           inherits_fqname = i.name().fullyQualify())

            if has_abstract:
                stream.out(skel_template.interface_objref_repoID_ptr,
                           inherits_fqname = "CORBA::AbstractBase")

        def _ptrToObjRef_str(self = self, stream = stream):
            has_abstract = self.interface().abstract()

            for i in self.interface().allInherits():
                if i.abstract(): has_abstract = 1

                stream.out(skel_template.interface_objref_repoID_str,
                           inherits_fqname = i.name().fullyQualify())

            if has_abstract:
                stream.out(skel_template.interface_objref_repoID_str,
                           inherits_fqname = "CORBA::AbstractBase")

        #
        # Build the inherits list
        
        inherits_str_list = []
        for i in self.interface().inherits():
            objref_name = i.name().prefix("_objref_")

            objref_str = objref_name.unambiguous(self._environment)

            if objref_name.needFlatName(self._environment):
                objref_str = objref_name.flatName()

            this_inherits_str = objref_str + "(ior, id)"
            inherits_str_list.append(this_inherits_str)

        inherits_str = ",\n".join(inherits_str_list)
        if inherits_str:
            comma = ","
        else:
            comma = ""

        if config.state['Shortcut']:
            inherits_str  = inherits_str + ","
            init_shortcut = "_shortcut(0)"
        else:
            init_shortcut = ""

        if config.state['Shortcut'] == 2:
            release_shortcut = skel_template.interface_release_refcount_shortcut
        else:
            release_shortcut = ""

        stream.out(skel_template.interface_objref,
                   name             = self.interface().name().fullyQualify(),
                   fq_objref_name   = self.name().fullyQualify(),
                   objref_name      = self.name().simple(),
                   inherits_str     = inherits_str,
                   comma            = comma,
                   _ptrToObjRef_ptr = _ptrToObjRef_ptr,
                   _ptrToObjRef_str = _ptrToObjRef_str,
                   init_shortcut    = init_shortcut,
                   release_shortcut = release_shortcut)


        #
        # Shortcut

        shortcut_mode = config.state['Shortcut']

        if shortcut_mode:
            inherited = output.StringStream()
            for i in self.interface().inherits():
                objref_name = i.name().prefix("_objref_")

                objref_str = objref_name.unambiguous(self._environment)

                if objref_name.needFlatName(self._environment):
                    objref_str = objref_name.flatName()

                inherited.out(skel_template.interface_shortcut_inh,
                              parent=objref_str)
                
            if shortcut_mode == 1:
                tmpl = skel_template.interface_shortcut
            else:
                tmpl = skel_template.interface_shortcut_refcount
                
            stream.out(tmpl,
                       name           = self.interface().name().fullyQualify(),
                       basename       = self.interface().name().simple(),
                       fq_objref_name = self.name().fullyQualify(),
                       inherited      = str(inherited))


        #
        # AMI poller

        poller           = self.interface().amiPoller()
        poller_impl_name = None
        poller_methods   = {}
        
        if poller is not None:
            astdecl = poller.astdecl()

            poller_impl_name = descriptor.ami_poller_impl(poller.name())
            poller_class = AMIPoller(poller_impl_name)

            method_decls      = []
            inherited_methods = []
            direct_callables  = astdecl.callables()

            for c in astdecl.all_callables():
                op = call.operation(poller, c)
                method_decl = _impl_Method(op, poller_class)
                method_decls.append(method_decl.hh())

                if c not in direct_callables:
                    inherited_methods.append(method_decl)
                else:
                    poller_methods[op.operation_name()] = method_decl

            stream.out(skel_template.interface_ami_poller_impl,
                       poller_impl_name = poller_impl_name,
                       poller_name      = poller.name().fullyQualify(),
                       method_decls     = "\n".join(method_decls))

            for method in inherited_methods:
                method.cc(stream, "_wrongOperation();")


        #
        # Methods

        ami_descriptors = {}

        for method in self.methods():
            callable = self._callables[method]

            stream.out(skel_template.interface_operation_marker,
                       iface = self.interface().name().fullyQualify(),
                       operation = callable.operation_name())

            # signature is a text string form of the complete
            # operation signature
            signature = callable.signature()

            # we only need one descriptor for each _signature_ (not operation)

            if signature in _proxy_call_descriptors:
                call_descriptor = _proxy_call_descriptors[signature]
            else:
                call_descriptor = call.CallDescriptor(signature, callable)
                call_descriptor.out_desc(stream)
                _proxy_call_descriptors[signature] = call_descriptor

            # produce a localcall function
            node_name = self.interface().name()
            localcall_fn = descriptor.\
                           local_callback_fn(node_name,
                                             callable.operation_name(),
                                             signature)
            call_descriptor.out_localcall(stream,
                                          node_name,
                                          callable.method_name(),
                                          localcall_fn)

            # produce member function for this operation/attribute.
            body = output.StringStream()

            argnames = method.arg_names()

            if config.state['Shortcut']:
                if method.return_type().kind() != idltype.tk_void:
                    callreturn = "return "
                    voidreturn = ""
                else:
                    callreturn = ""
                    voidreturn = " return;"

                objref_class = method.parent_class()
                interface = objref_class.interface()
                implname = interface.name().prefix("_impl_").\
                           unambiguous(self._environment)

                if config.state['Shortcut'] == 2:
                    tmpl = skel_template.interface_operation_shortcut_refcount
                else:
                    tmpl = skel_template.interface_operation_shortcut
                
                body.out(tmpl,
                         impl_type  = implname,
                         callreturn = callreturn,
                         voidreturn = voidreturn,
                         args       = ", ".join(argnames),
                         name       = method.name())


            intf_env = self._environment.enter("_objref_" +
                                               self.interface().name().simple())

            call_descriptor.out_objrefcall(body,
                                           callable.operation_name(),
                                           argnames,
                                           localcall_fn,
                                           intf_env)
            method.cc(stream, body)

            # Generate AMI descriptor
            if callable.ami():
                cd_names = call_descriptor.out_ami_descriptor(stream, callable,
                                                              node_name,
                                                              localcall_fn,
                                                              poller_impl_name)

                for ami_method in method._ami_methods:

                    ami_callable = ami_method.callable()

                    body = output.StringStream()

                    ami_op_name = ami_callable.operation_name()
                    if ami_op_name.startswith("sendc_"):
                        call_descriptor.out_ami_sendc(body, ami_method,
                                                      cd_names, intf_env)

                    else:
                        # sendp
                        call_descriptor.out_ami_sendp(body, ami_method,
                                                      cd_names, intf_env)

                        poller_body   = output.StringStream()
                        poller_method = poller_methods[ami_op_name[6:]]

                        call_descriptor.out_ami_poller(poller_body,
                                                       ami_method,
                                                       poller_method,
                                                       cd_names)

                        poller_method.cc(stream, poller_body)

                    ami_method.cc(stream, body)


class _nil_I(Class):
    def __init__(self, I):
        Class.__init__(self, I)
        self._name = self._name.prefix("_nil_")

        for callable in self.interface().callables():
            method = _impl_Method(callable, self)
            self._methods.append(method)
            self._callables[method] = callable

        for i in self.interface().allInherits():
            if not i.local():
                for c in i.callables():
                    # Make new callable with ref to this interface instead of
                    # inherited interface
                    callable = call.Callable(self.interface(),
                                             c.node(),
                                             c.operation_name(),
                                             c.method_name(),
                                             c.returnType(),
                                             c.parameters(),
                                             c.oneway(),
                                             c.raises(),
                                             c.contexts())

                    method = _impl_Method(callable, self)
                    self._methods.append(method)
                    self._callables[method] = callable

    def hh(self, stream):
        pass

    def cc(self, stream):

        def _ptrToObjRef_ptr(self = self, stream = stream):
            has_abstract = 0

            for i in self.interface().allInherits():
                if i.abstract():
                    has_abstract = 1

                stream.out(skel_template.interface_objref_repoID_ptr,
                           inherits_fqname = i.name().fullyQualify())

            stream.out(skel_template.interface_objref_repoID_ptr,
                       inherits_fqname = "CORBA::LocalObject")

            if has_abstract:
                stream.out(skel_template.interface_objref_repoID_ptr,
                           inherits_fqname = "CORBA::AbstractBase")

        def _ptrToObjRef_str(self = self, stream = stream):
            has_abstract = 0

            for i in self.interface().allInherits():
                if i.abstract():
                    has_abstract = 1

                stream.out(skel_template.interface_objref_repoID_str,
                           inherits_fqname = i.name().fullyQualify())

            stream.out(skel_template.interface_objref_repoID_str,
                       inherits_fqname = "CORBA::LocalObject")

            if has_abstract:
                stream.out(skel_template.interface_objref_repoID_str,
                           inherits_fqname = "CORBA::AbstractBase")

        stream.out(skel_template.local_interface_objref,
                   name             = self.interface().name().fullyQualify(),
                   sname            = self.interface().name().simple(),
                   fq_nil_name      = self.name().fullyQualify(),
                   nil_name         = self.name().simple(),
                   _ptrToObjRef_ptr = _ptrToObjRef_ptr,
                   _ptrToObjRef_str = _ptrToObjRef_str)

        for method in self.methods():
            callable = self._callables[method]

            # signature is a text string form of the complete
            # operation signature
            signature = callable.signature()

            # produce member function for this operation/attribute.
            body = output.StringStream()

            argnames = method.arg_names()

            body.out(skel_template.local_interface_nil_operation)

            if not method.return_type().void():
                body.out(skel_template.local_interface_nil_dummy_return,
                         method = method.name(),
                         args   = ", ".join(method.arg_names()))
            
            method.cc(stream, body)
            stream.out("\n")



class _pof_I(Class):
    def __init__(self, I):
        Class.__init__(self, I)
        self._name = self._name.prefix("_pof_")

    def hh(self, stream):
        stream.out(header_template.interface_pof,
                   name = self.interface().name().simple())
        
    def cc(self, stream):
        inherits = output.StringStream()
        for i in self.interface().allInherits():
            ancestor = i.name().fullyQualify()
            inherits.out(skel_template.interface_pof_repoID,
                         inherited = ancestor)

        node_name = self.interface().name()
        objref_name = node_name.prefix("_objref_")
        pof_name = node_name.prefix("_pof_")
        stream.out(skel_template.interface_pof,
                   pof_name      = pof_name.fullyQualify(),
                   objref_fqname = objref_name.fullyQualify(),
                   name          = node_name.fullyQualify(),
                   uname         = pof_name.simple(),
                   Other_repoIDs = inherits,
                   idname        = node_name.guard())
        

class _impl_I(Class):
    def __init__(self, I):
        Class.__init__(self, I)
        self._name = self._name.prefix("_impl_")

        for callable in self.interface().callables():
            method = _impl_Method(callable, self)
            self._methods.append(method)
            self._callables[method] = callable

    def hh(self, stream):
        # build the inheritance list
        environment = self._environment
        impl_inherits = []
        inherit_abstract = 0
        
        for i in self.interface().inherits():
            if i.abstract():
                inherit_abstract = 1

            impl_inherited_name = i.name().prefix("_impl_")
            uname = impl_inherited_name.unambiguous(environment)
            impl_inherits.append("public virtual " + uname)

        # if already inheriting, the base class will be present
        # (transitivity of the inherits-from relation)
        if self.interface().inherits() == []:
            impl_inherits = [ "public virtual omniServant" ]

        methods = []
        for method in self.methods():
            methods.append(method.hh(virtual = 1, pure = 1))

        if self.interface().abstract():
            abstract = header_template.interface_impl_abstract

        elif inherit_abstract:
            abstract = header_template.interface_impl_not_abstract

        else:
            abstract = ""
                
        stream.out(header_template.interface_impl,
                   name       = self.interface().name().simple(),
                   inherits   = ",\n".join(impl_inherits),
                   operations = "\n".join(methods),
                   abstract   = abstract)

    def cc(self, stream):

        # Function to write the _impl_I::dispatch method
        def dispatch(self = self, stream = stream):
            # first check if method is from this interface
            dispatched = []
            for method in self.methods():
                callable = self._callables[method]
                operation_name = callable.operation_name()
                if operation_name not in dispatched:
                    signature = callable.signature()
                    call_descriptor = _proxy_call_descriptors[signature]
                    localcall_fn = descriptor.\
                                   local_callback_fn(self.interface().name(),
                                                     operation_name,signature)
                    call_descriptor.out_implcall(stream, operation_name,
                                                 localcall_fn)
                    dispatched.append(operation_name)

            # next call dispatch methods of superclasses
            for i in self.interface().inherits():
                inherited_name = i.name().prefix("_impl_")
                impl_inherits = inherited_name.simple()

                # The MSVC workaround might be needed here again
                if inherited_name.needFlatName(self._environment):
                    impl_inherits = inherited_name.flatName()

                stream.out(skel_template.interface_impl_inherit_dispatch,
                           impl_inherited_name = impl_inherits)

        # For each of the inherited interfaces, check their repoId strings
        def _ptrToInterface_ptr(self = self, stream = stream):
            for i in self.interface().allInherits():
                inherited_name      = i.name()
                impl_inherited_name = inherited_name.prefix("_impl_")

                # HERE: using the fully scoped name may fail on old MSVC
                # versions, but it is required by newer MSVC versions.
                # Marvellous.
                inherited_str      = inherited_name.fullyQualify()
                impl_inherited_str = impl_inherited_name.fullyQualify()

                stream.out(skel_template.interface_impl_repoID_ptr,
                           inherited_name      = inherited_str,
                           impl_inherited_name = impl_inherited_str)

        def _ptrToInterface_str(self = self, stream = stream):
            for i in self.interface().allInherits():
                inherited_name      = i.name()
                impl_inherited_name = inherited_name.prefix("_impl_")
                inherited_str       = inherited_name.fullyQualify()
                impl_inherited_str  = impl_inherited_name.fullyQualify()

                stream.out(skel_template.interface_impl_repoID_str,
                           inherited_name      = inherited_str,
                           impl_inherited_name = impl_inherited_str)

        node_name = self.interface().name()
        impl_name = node_name.prefix("_impl_")
        if self.methods():
            getopname = "const char* op = _handle.operation_name();"
        else:
            getopname = ""

        env = self._environment
        stream.out(skel_template.interface_impl,
                   impl_fqname         = impl_name.fullyQualify(),
                   uname               = node_name.simple(),
                   getopname           = getopname,
                   dispatch            = dispatch,
                   impl_name           = impl_name.unambiguous(env),
                   _ptrToInterface_ptr = _ptrToInterface_ptr,
                   _ptrToInterface_str = _ptrToInterface_str,
                   name                = node_name.fullyQualify())

        if not self.interface().abstract():
            # Are we derived from an abstract interface?
            inherit_abstract = 0
            for i in self.interface().inherits():
                if i.abstract():
                    inherit_abstract = 1
                    break
            if inherit_abstract:
                stream.out(skel_template.interface_impl_not_abstract,
                           impl_fqname = impl_name.fullyQualify())


class _sk_I(Class):
    def __init__(self, I):
        Class.__init__(self, I)

    def hh(self, stream):
        # build the inheritance list
        environment = self._environment
        sk_inherits = []
        for i in self.interface().inherits():
            sk_inherited_name = i.name().prefix("_sk_")
            uname = sk_inherited_name.unambiguous(environment)
            sk_inherits.append("public virtual " + uname)

        # if already inheriting, the base class will be present
        if self.interface().inherits() == []:
            sk_inherits = [ "public virtual omniOrbBoaServant" ]

        stream.out(header_template.interface_sk,
                   name = self.interface().name().simple(),
                   inherits = ",\n".join(sk_inherits))
