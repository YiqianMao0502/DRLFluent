# -*- python -*-
#                           Package   : omniidl
# call.py                   Created on: 2000/08/03
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2002-2013 Apasphere Ltd
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
#   Produce local callback functions
#

"""Produce call descriptors and local callback functions"""

from omniidl import idlast, idltype
from omniidl_be.cxx import types, id, skutil, output
from omniidl_be.cxx import descriptor, mangler, config

template = None

def init():
    global template
    import omniidl_be.cxx.skel.template
    template = omniidl_be.cxx.skel.template


# Callable- represents the notion of a callable entity (eg operation or
# attribute accessor method). Note that a read/write attribute is really
# two such entities paired together.
#
#  .interface():      get the associated Interface object
#  .operation_name(): get the IIOP operation name (eg _get_colour)
#  .method_name():    get the C++ mapped method name (eg colour)
#  .returnType():     idltype
#  .parameters():     idlast.Parameter
#  .oneway():         boolean
#  .contexts():
#  .signature():      string representing the IDL signature
#  .method(use_out):  C++ mapping
#
class Callable:
    def __init__(self, interface, node, operation_name, method_name,
                 returnType, parameters, oneway = 0,
                 raises = [], contexts = []):

        self.__interface = interface
        self.__node = node
        self.__operation_name = operation_name
        self.__method_name = method_name
        self.__returnType = returnType
        self.__parameters = parameters
        self.__oneway = oneway
        self.__raises = raises
        self.__contexts = contexts

        self.__ami = 0

        if not oneway:
            try:
                # interface may actually be a ValueType, not an Interface,
                # and thus not have an ami_callables method.
                if interface.ami_callables():
                    self.__ami = 1

            except AttributeError:
                pass

        self.__signature = mangler.produce_signature(returnType, parameters,
                                                     raises, oneway,
                                                     self.__ami)

    def interface(self):      return self.__interface
    def node(self):           return self.__node
    def operation_name(self): return self.__operation_name
    def method_name(self):    return self.__method_name
    def returnType(self):     return self.__returnType
    def parameters(self):     return self.__parameters
    def oneway(self):         return self.__oneway
    def raises(self):         return self.__raises
    def contexts(self):       return self.__contexts
    def signature(self):      return self.__signature
    def ami(self):            return self.__ami


# Utility functions to build Callables #################################
#
def operation(interface, operation):
    assert isinstance(operation, idlast.Operation)

    return Callable(interface,
                    operation,
                    operation.identifier(),
                    id.mapID(operation.identifier()),
                    operation.returnType(),
                    operation.parameters(),
                    operation.oneway(),
                    operation.raises(),
                    operation.contexts())

def read_attributes(interface, attribute):
    assert isinstance(attribute, idlast.Attribute)
    callables = []
    for declarator in attribute.declarators():
        identifier = declarator.identifier()
        callables.append(Callable(interface,
                                  declarator,
                                  "_get_" + identifier,
                                  id.mapID(identifier),
                                  attribute.attrType(),
                                  [], 0, [], []))
    return callables

def write_attributes(interface, attribute):
    assert isinstance(attribute, idlast.Attribute)
    voidType = idltype.Base(idltype.tk_void)
    callables = []
    param = idlast.Parameter(attribute.file(), attribute.line(),
                             attribute.mainFile(), [], [],
                             0, attribute.attrType(), "_v")

    for declarator in attribute.declarators():
        identifier = declarator.identifier()
        callables.append(Callable(interface,
                                  declarator,
                                  "_set_" + identifier,
                                  id.mapID(identifier),
                                  voidType, [param], 0, [], []))
    return callables


#####################################################################
class CallDescriptor:

# How this works:
#
# A call descriptor can be used on both sides:
#   1. on the client side to drive a remote call
#   2. on the server side to drive an upcall
#
# Therefore, it has to have a way to hold the arguments that is suitable
# for both the client and the server side.
#
# On the server side there is an additional requirement that it must
# provide the storage for the in and inout arguments.
#
# For this requirement, the call descriptor must contain 2 member per
# argument:
#     1. an argument holder, as the name suggests, is to hold the argument
#        passed by the proxy call on the client side.
#     2. an argument storage which is used to store the argument and to
#        remove it from the heap when finished.
#
# The naming convention is that a holder is named "arg_n" and
# a storage is named "arg_n_".
#
# In many cases, the holder member is the same as the storage member. Under
# these circumstances, only a single member is provided in the call descriptor
# and is named "arg_n".
#
# Some examples:
#
# For a string IN argument, the call descriptor members are:
#     const char* arg_0;
#     CORBA::String_var arg_0_;
#
# For a string OUT argument, the member is just
#     CORBA::String_var arg_0;
#
# For a variable length struct IN argument:
#     const varStruct* arg_0;
#     varStruct_var    arg_0_;
# Notice that the argument holder is a pointer to the struct whereas
# the mapping signature is const varStruct&.
#
# To determine what is most appropriate given a type and direction combination,
# the helper function _arg_info() is defined. The function is called with
# the type and direction as input and it returns a tuple of this form:
#   ((is_const, is_ptr), (same_as_holder, is_var))
#
# The 1st element of the tuple describes the argument holder. The element
# itself is also a tuple:
#   is_const == 1 if the holder is a const (e.g. IN string)
#   is_ptr == 1 if the holder is a pointer (e.g. IN varStruct and ! IN string)
# 
# The 2nd decribes the argument storage:
#   same_as_holder == 1 if both holder & storage are the same (e.g. OUT string)
#   is_var == 1 if the storage is a _var type (e.g. OUT string, OUT varStruct).
#                  If it is a _var type, of course the actual storage for
#                  the argument must be allocated on the heap.
#
# All the member functions of this class generates code based on this tuple
#

    def __init__(self, signature, callable):
        self.__signature        = signature
        self.__name             = descriptor.call_descriptor(signature)
        self.__oneway           = callable.oneway()
        self.__arguments        = callable.parameters()
        self.__returntype       = callable.returnType()
        self.__exceptions       = callable.raises()
        self.__contexts         = callable.contexts()
        self.__has_in_args      = 0
        self.__has_out_args     = 0
        self.__has_return_value = 0

        for argument in self.__arguments:
            if argument.is_in():
                self.__has_in_args = 1
            if argument.is_out():
                self.__has_out_args = 1

        if self.__returntype.kind() != idltype.tk_void:
            self.__has_return_value = 1

        if self.__contexts:
            self.__context_name = descriptor.\
                                  context_descriptor(self.__signature)

        self.__ami = callable.ami()


    def name(self):
        return self.__name


    def out_desc(self, stream):
        self.__out_declaration(stream)
        self.__out_contextDescriptor(stream)
        self.__out_marshalArgument(stream)
        self.__out_unmarshalArgument(stream)
        self.__out_marshalReturnedValues(stream)
        self.__out_unmarshalReturnedValues(stream)
        self.__out_userException(stream)


    def out_localcall(self, stream, interface_name, operation, function_name):
        assert isinstance(stream, output.Stream)
        assert isinstance(interface_name, id.Name)

        impl_args = []

        for n, argument in enumerate(self.__arguments):
            arg_n = "tcd->arg_" + str(n)
            argtype = types.Type(argument.paramType())
            ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                         _arg_info(argtype,argument.direction())
            if h_is_ptr:
                arg_n = "*" + arg_n
            if s_is_holder and s_is_var:
                if argument.is_in():
                    if argument.is_out():
                        arg_n = arg_n + ".inout()"
                    else:
                        arg_n = arg_n + ".in()"
                else:
                    arg_n = arg_n + ".out()"
            impl_args.append(arg_n)

        if self.__has_return_value:
            result_string = "tcd->result = "
        else:
            result_string = ""
        
        if self.__contexts:
            impl_args.append("ctxt")

        # If we have no return value, no arguments, no exceptions, and
        # no contexts, we don't use the call descriptor argument at
        # all, so we do not give it a name, so as to avoid warnings on
        # some compilers.
        if result_string or impl_args or self.__contexts:
            cd_arg = " cd"

        elif self.__exceptions:
            cd_arg = " _OMNIORB_EX_ONLY_CD(cd)"

        else:
            cd_arg = ""

        # If we have no return value and no arguments at all then we don't
        # need to fetch the call descriptor. This suppresses a warning in gcc
        # about an unused variable.
        if result_string or impl_args:
            get_cd = self.__name + "* tcd = (" + self.__name + "*)cd;"
        else:
            get_cd = ""

        impl_call = output.StringStream()
        catch = output.StringStream()

        # Deal with context
        if self.__contexts:
            impl_call.out(template.interface_callback_context,
                          cname = self.__context_name,
                          count = str(len(self.__contexts)))

        # Deal with user exceptions
        raises = self.__exceptions
        if raises:
            for exception in raises:
                ex_scopedName = id.Name(exception.scopedName())
                catch.out(template.interface_operation_catch_exn,
                          exname = ex_scopedName.fullyQualify())

            impl_call.out(template.interface_callback_tryblock,
                          result = result_string,
                          cxx_operation_name = operation,
                          operation_arguments = ", ".join(impl_args),
                          catch = str(catch))
        else:
            impl_call.out(template.interface_callback_invoke,
                          result = result_string,
                          cxx_operation_name = operation,
                          operation_arguments = ", ".join(impl_args))

        stream.out(template.interface_callback,
                   local_call_descriptor = function_name,
                   cd_arg = cd_arg,
                   get_call_descriptor = get_cd,
                   impl_fqname =interface_name.prefix("_impl_").fullyQualify(),
                   name = interface_name.fullyQualify(),
                   impl_call = str(impl_call))


    def out_objrefcall(self,stream,operation,args,localcall_fn,environment):
        assert isinstance(stream, output.Stream)

        ctor_args = [ localcall_fn, "\"" + operation + "\"",
                      str(len(operation) + 1) ]

        assign_args = []
        assign_res = []

        for n, argument in enumerate(self.__arguments):
            arg_n = "_call_desc.arg_%d" % n

            argtype = types.Type(argument.paramType())
            ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                         _arg_info(argtype,argument.direction())

            if s_is_holder:
                if argument.is_in():
                    rvalue = args[n]
                    if argtype.array():
                        rvalue = "&" + rvalue + "[0]"
                    if h_is_ptr:
                        rvalue = "&(" + argtype.base(environment) + "&) " \
                                 + rvalue
                    assign_args.append(arg_n + " = " + rvalue + ";")
                else:
                    if s_is_var:
                        arg_n = arg_n + "._retn()"
                    assign_res.append(args[n] + " = " \
                                      + arg_n + ";")
            else:
                rvalue = args[n]
                if argtype.array():
                    rvalue = "&" + rvalue + "[0]"
                if argtype.value() or argtype.valuebox():
                    star = "*"
                else:
                    star = ""
                if h_is_ptr:
                    rvalue = "&(" + argtype.base(environment) + star + "&) " \
                             + rvalue
                assign_args.append(arg_n + " = " + rvalue + ";")

        if self.__has_return_value:
            argtype = types.Type(self.__returntype)
            ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                         _arg_info(argtype,3)
            if s_is_var:
                assign_res.append("return _call_desc.result._retn();")
            else:
                assign_res.append("return _call_desc.result;")

        if self.__contexts:
            assign_context = "_call_desc.ctxt = "\
                             "::CORBA::Context::_duplicate(_ctxt);"
        else:
            assign_context = ""
        
        stream.out(template.interface_operation,
                   call_descriptor = self.__name,
                   call_desc_args = ", ".join(ctor_args),
                   assign_args = "\n".join(assign_args),
                   assign_res = "\n".join(assign_res),
                   assign_context = assign_context)


    def out_implcall(self,stream,operation,localcall_fn):
        assert isinstance(stream, output.Stream)

        ctor_args = [localcall_fn, "\"" + operation + "\"",
                          str(len(operation)+1),str(1) ]
        
        prepare_out_args = []
        
        if self.__has_out_args: 
            for n, argument in enumerate(self.__arguments):
                if argument.is_out() and not argument.is_in():
                    arg_n = "_call_desc.arg_" + str(n)
                    argtype = types.Type(argument.paramType())
                    ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                                    _arg_info(argtype,argument.direction())
                    if not s_is_holder:
                        rvalue = arg_n + "_"
                        if s_is_var:
                            rvalue = rvalue + ".out()"
                        if h_is_ptr:
                            rvalue = "&" + rvalue
                        prepare_out_args.append(arg_n + " = " + rvalue + ";")
            
        # main block of code goes here
        stream.out(template.interface_operation_dispatch,
                   idl_operation_name = operation,
                   call_descriptor = self.__name,
                   call_desc_args = ", ".join(ctor_args),
                   prepare_out_args = "\n".join(prepare_out_args))


    def __out_declaration(self,stream):
        # build up the constructor argument list, the initialisation
        # list and the data members list
        if self.__exceptions:
            user_exceptions_decl = \
            "void userException(cdrStream&, _OMNI_NS(IOP_C)*, const char*);"
        else:
            user_exceptions_decl = ""

        in_arguments_decl = ""
        out_arguments_decl = ""
        if self.__has_in_args or self.__contexts:
            in_arguments_decl = "void marshalArguments(cdrStream&);\n" + \
                                "void unmarshalArguments(cdrStream&);\n"
        if self.__has_out_args or self.__has_return_value:
            out_arguments_decl = \
                              "void unmarshalReturnedValues(cdrStream&);\n" + \
                              "void marshalReturnedValues(cdrStream&);\n"
        data_members = []

        containsValues = 0
        for n, argument in enumerate(self.__arguments):
            ptype = argument.paramType()
            if idltype.containsValueType(ptype):
                if (isinstance(ptype, idltype.Declared) and
                    ptype.scopedName() == ["Messaging", "ExceptionHolder"]):
                    # We don't declare that Messaging::ExceptionHolder
                    # is a valuetype, to prevent unnecessary
                    # marshalling of it in _excep callbacks.
                    pass
                else:
                    containsValues = 1

            holder_n = "arg_" + str(n)
            argtype = types.Type(argument.paramType())
            ((h_is_const,h_is_ptr),\
             (s_is_holder,s_is_var)) = _arg_info(argtype,argument.direction())

            if s_is_var:
                storage = argtype._var()
            else:
                storage = argtype.base()
            storage_n = holder_n
            if not s_is_holder:
                storage_n = storage_n + "_"
            data_members.append(storage + " " + storage_n + ";")

            if not s_is_holder:
                holder = argtype.base()
                if h_is_const:
                    holder = "const " + holder
                if argtype.array():
                    holder = holder + "_slice*"
                if h_is_ptr:
                    holder = holder + "*"
                if argtype.value() or argtype.valuebox():
                    holder = holder + "*"

                data_members.append(holder + " " + holder_n + ";")

        if self.__has_return_value:
            containsValues = (containsValues or
                              idltype.containsValueType(self.__returntype))

            returntype = types.Type(self.__returntype)
            ((h_is_const,h_is_ptr),\
             (s_is_holder,s_is_var)) = _arg_info(returntype,3)

            holder_n = "result"
            if s_is_var:
                storage = returntype._var()
            else:
                storage = returntype.base()
            storage_n = holder_n
            if not s_is_holder:
                storage_n = storage_n + "_"
            data_members.append(storage + " " + storage_n + ";")

        if self.__contexts:
            data_members.append("::CORBA::Context_var ctxt;");

        if containsValues:
            contains_values = "containsValues(1);"
        else:
            contains_values = ""
        
        # Write the proxy class definition
        if self.__ami:
            tmpl = template.interface_proxy_class_ami
        else:
            tmpl = template.interface_proxy_class

        stream.out(tmpl,
                   signature = self.__signature,
                   call_descriptor = self.__name,
                   oneway = self.__oneway,
                   exn_len = len(self.__exceptions),
                   contains_values = contains_values,
                   in_arguments_decl = in_arguments_decl,
                   out_arguments_decl = out_arguments_decl,
                   user_exceptions_decl = user_exceptions_decl,
                   member_data = "\n".join(data_members))


    def __out_contextDescriptor(self,stream):
        if self.__contexts:
            contexts = output.StringStream()
            for context in self.__contexts:
                contexts.out('"' + context + '",')
            stream.out(template.interface_context_array,
                       context_descriptor = self.__context_name,
                       contexts = str(contexts))


    def __out_marshalArgument(self,stream):
        if not (self.__has_in_args or self.__contexts): return
        marshal_block = output.StringStream()
        self.__out_marshalArgument_shared(marshal_block,1)

        if self.__contexts:
            marshal_block.out(template.interface_proxy_marshal_context,
                              name  = self.__context_name,
                              count = str(len(self.__contexts)))

        stream.out(template.interface_proxy_marshal_arguments,
                   call_descriptor = self.__name,
                   marshal_block = marshal_block)

        
    def __out_marshalReturnedValues(self,stream):
        if not (self.__has_out_args or self.__has_return_value): return
        marshal_block = output.StringStream()

        if self.__has_return_value:
            argtype = types.Type(self.__returntype)
            ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                         _arg_info(argtype,3)
            argname = "result"
            if (h_is_ptr):
                argname = "*" + argname
            skutil.marshall(marshal_block, None, argtype, None, argname, "_n")
            
        self.__out_marshalArgument_shared(marshal_block,0)
        stream.out(template.interface_proxy_marshal_returnedvalues,
                   call_descriptor = self.__name,
                   marshal_block = marshal_block)


    def __out_marshalArgument_shared(self,stream,is_in):
        for n, argument in enumerate(self.__arguments):
            if (is_in and not argument.is_in()) or \
               (not is_in and not argument.is_out()): continue
            arg_n = "arg_" + str(n)
            argtype = types.Type(argument.paramType())
            ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                         _arg_info(argtype,argument.direction())
            if h_is_ptr:
                arg_n = "*" + arg_n
            skutil.marshall(stream, None, argtype, None, arg_n, "_n")


    def __out_unmarshalArgument(self,stream):
        if not self.__has_in_args: return

        marshal_block = output.StringStream()

        for n, argument in enumerate(self.__arguments):
            if not argument.is_in(): continue

            argtype = types.Type(argument.paramType())
            ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                         _arg_info(argtype,argument.direction())

            arg_n = "arg_%d" % n
            if s_is_holder:
                storage_n = arg_n
            else:
                storage_n = arg_n + "_"
                
            if s_is_var:
                alloc = ""
                d_type = argtype.deref(1)

                if argtype.array():
                    alloc = argtype.base() + "_alloc()"
                elif not (d_type.typecode()  or
                          d_type.string()    or
                          d_type.wstring()   or
                          d_type.interface() or
                          d_type.value()     or
                          d_type.valuebox()):
                    alloc = "new " + argtype.base()
                if alloc != "":
                    marshal_block.out(storage_n + " = " + alloc + ";")

            skutil.unmarshall(marshal_block, None,
                              argtype, None, storage_n, "_n")

            if not s_is_holder:
                if s_is_var:
                    if argument.direction() == 0:
                        lvalue = storage_n + ".in()"
                    else:
                        lvalue = storage_n + ".inout()"
                else:
                    lvalue = storage_n
                if argtype.array():
                    lvalue = "&" + lvalue + "[0]"
                if h_is_ptr:
                    marshal_block.out(arg_n + " = &" + lvalue + ";")
                else:
                    marshal_block.out(arg_n + " = " + lvalue + ";")

        if self.__contexts:
            marshal_block.out(template.interface_proxy_unmarshal_context)

        stream.out(template.interface_proxy_unmarshal_arguments,
                   call_descriptor = self.__name,
                   marshal_block = marshal_block)


    def __out_unmarshalReturnedValues(self,stream):

        if not (self.__has_out_args or self.__has_return_value): return

        marshal_block = output.StringStream()

        if self.__has_return_value:
            argtype = types.Type(self.__returntype)
            ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                         _arg_info(argtype,3)
            argname = "result"
            if s_is_var:
                alloc = ""
                d_type = argtype.deref(1)
                if argtype.array():
                    alloc = argtype.base() + "_alloc()"
                elif not (d_type.typecode()  or
                          d_type.string()    or
                          d_type.wstring()   or
                          d_type.interface() or
                          d_type.value()     or
                          d_type.valuebox()):
                    alloc = "new " + argtype.base()
                if alloc != "":
                    marshal_block.out(argname + " = " + alloc + ";")
            skutil.unmarshall(marshal_block, None,
                              argtype, None, argname, "_n")

        for n, argument in enumerate(self.__arguments):
            if not argument.is_out(): continue

            argtype = types.Type(argument.paramType())
            ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                         _arg_info(argtype,argument.direction())

            arg_n = "arg_" + str(n)

            d_type = argtype.deref(1)
            if s_is_holder:
                if s_is_var:
                    alloc = ""
                    if argtype.array():
                        alloc = argtype.base() + "_alloc()"
                    elif not (d_type.typecode()  or
                              d_type.string()    or
                              d_type.wstring()   or
                              d_type.interface() or
                              d_type.value()     or
                              d_type.valuebox()):
                        alloc = "new " + argtype.base()
                    if alloc != "":
                        marshal_block.out(arg_n + " = " + alloc + ";")
            elif h_is_ptr:
                if d_type.typecode():
                    marshal_block.out(arg_n + "_ = *" + arg_n + ";\n"+ \
                                      "*" + arg_n + " = " + \
                                      "::CORBA::TypeCode::_nil();")
                elif d_type.interface():
                    nilobjref = d_type.base().replace("_ptr","::_nil()")
                    if isinstance(d_type.type().decl(),idlast.Forward):
                        nilobjref = nilobjref.replace("::_nil()",
                                                      "_Helper::_nil()")
                    
                    marshal_block.out(arg_n + "_ = *" + arg_n + ";\n" + \
                                      "*" + arg_n + " = " + \
                                      nilobjref + ";")
                elif d_type.string():
                    marshal_block.out(arg_n + "_ = *" + arg_n + ";\n"+ \
                                      "*" + arg_n + " = " + \
                                      "(char*) _CORBA_String_helper::empty_string;")
                elif d_type.wstring():
                    marshal_block.out(arg_n + "_ = *" + arg_n + ";\n" + \
                                      "*" + arg_n + " = " + \
                                      "(::CORBA::WChar*) _CORBA_WString_helper::empty_wstring;")
                arg_n = "*" + arg_n
            skutil.unmarshall(marshal_block, None,
                              argtype, None, arg_n, "_n")
                
        stream.out(template.interface_proxy_unmarshal_returnedvalues,
                   call_descriptor = self.__name,
                   marshal_block = marshal_block)
        

    def __out_userException(self, stream):
        if self.__exceptions:
            block = output.StringStream()
            exceptions = skutil.sort_exceptions(self.__exceptions)
            repoIDs = []
            for exception in exceptions:
                scopedName = exception.scopedName()
                repoID = scopedName + ["_PD_repoId"]
                repoID_str = id.Name(repoID).fullyQualify()
                repoIDs.append(repoID_str)
                exname = id.Name(scopedName).fullyQualify()
                block.out(template.interface_proxy_exn_handle,
                          repoID_str = repoID_str, exname = exname)

            # write the user exception template
            stream.out(template.interface_proxy_exn,
                       call_descriptor = self.__name,
                       exception_block = str(block),
                       exception_namelist = ",\n".join(repoIDs))
        else:
            stream.out(template.interface_proxy_empty_exn,
                       call_descriptor = self.__name)


    def out_ami_descriptor(self, stream, callable, node_name, lcfn,
                           poller_impl_name):

        handler_if = callable.interface().amiHandler()
        poller_val = callable.interface().amiPoller()
        
        handler_cls = handler_if.name().fullyQualify()
        poller_cls  = poller_val.name().fullyQualify()

        cds = descriptor.ami_call_descriptor(node_name,
                                             callable.operation_name(),
                                             self.__signature)
        ami_cd_name_c, ami_cd_name_p = cds

        #
        # Callback handler

        op_name = callable.operation_name()
        if op_name.startswith("_get_"):
            handler_op = callable.node()._ami_get_handler
            handler_ex = callable.node()._ami_get_handler_excep
            
        elif op_name.startswith("_set_"):
            handler_op = callable.node()._ami_set_handler
            handler_ex = callable.node()._ami_set_handler_excep
            
        else:
            handler_op = callable.node()._ami_handler
            handler_ex = callable.node()._ami_handler_excep

        callback_args = []
        if self.__has_return_value:
            callback_args.append("result")

        for idx, argument in enumerate(self.__arguments):
            if argument.is_out():
                argtype = types.Type(argument.paramType())
                ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                             _arg_info(argtype,argument.direction())
                if h_is_ptr:
                    callback_args.append("*arg_%d" % idx)
                else:
                    callback_args.append("arg_%d" % idx)


        if_name = callable.interface().name().fullyQualify()
        
        stream.out(template.interface_ami_call_descriptor,
                   if_name           = if_name,
                   call_descriptor_c = ami_cd_name_c,
                   call_descriptor_p = ami_cd_name_p,
                   base_cd           = self.__name,
                   lcfn              = lcfn,
                   op_name           = op_name,
                   handler_op_name   = handler_op.identifier(),
                   handler_ex_name   = handler_ex.identifier(),
                   op_len            = len(op_name) + 1,
                   handler_cls       = handler_cls,
                   poller_cls        = poller_cls,
                   poller_impl_name  = poller_impl_name,
                   callback_args     = ", ".join(callback_args))

        return ami_cd_name_c, ami_cd_name_p


    def _ami_send_args(self, ami_args, environment):
        assign_args = []

        for n, argument in enumerate(self.__arguments):
            if not argument.is_in():
                continue

            arg_ident = next(ami_args)

            argtype = types.Type(argument.paramType())
            ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                         _arg_info(argtype,argument.direction())

            arg_n = "_call_desc->arg_%d" % n

            if s_is_holder:
                assign_args.append("%s = %s;" %
                                   (arg_n, arg_ident))
            else:
                d_type = argtype.deref(1)

                if s_is_var:
                    if argtype.array():
                        rhs = "%s_dup(%s)" % (argtype.base(environment),
                                              arg_ident)

                    elif d_type.typecode():
                        rhs = "CORBA::TypeCode::_duplicate(%s)" % arg_ident

                    elif d_type.string() or d_type.wstring():
                        rhs = arg_ident

                    elif d_type.interface():
                        intf = argtype.base(environment)
                        if intf.endswith("_ptr"):
                            intf = intf[:-4]

                        rhs = "%s::_duplicate(%s)" % (intf, arg_ident)

                    elif d_type.value() or d_type.valuebox():
                        assign_args.append("%s->_add_ref();" % arg_ident)
                        rhs = arg_ident

                    else:
                        rhs = "new %s(%s)" % (argtype.base(environment),
                                              arg_ident)

                    assign_args.append("%s_ = %s;" % (arg_n, rhs))
                    
                    if argument.is_out():
                        qual = "inout"
                    else:
                        qual = "in"

                    if h_is_ptr:
                        assign_args.append("%s = &%s_.%s();" %
                                           (arg_n, arg_n, qual))
                    else:
                        assign_args.append("%s = %s_.%s();" %
                                           (arg_n, arg_n, qual))

                else:
                    if argtype.array():
                        assign_args.append("%s_copy(%s_, %s);" %
                                           (argtype.base(environment),
                                            arg_n, arg_ident))
                    else:
                        assign_args.append("%s_ = %s;" % (arg_n, arg_ident))

                    if h_is_ptr:
                        assign_args.append("%s = &%s_;" % (arg_n, arg_n))
                    else:
                        assign_args.append("%s = %s_;" % (arg_n, arg_n))
        
        return assign_args


    def out_ami_sendc(self, stream, ami_method, cd_names, environment):

        ami_args = iter(ami_method.arg_names())
        next(ami_args) # Skip handler arg

        assign_args = self._ami_send_args(ami_args, environment)

        stream.out(template.interface_ami_sendc,
                   assign_args = "\n".join(assign_args),
                   cd_name     = cd_names[0],
                   ami_handler = ami_method.arg_names()[0])


    def out_ami_sendp(self, stream, ami_method, cd_names, environment):
        
        ami_args = iter(ami_method.arg_names())

        assign_args = self._ami_send_args(ami_args, environment)

        poller_cls = ami_method.return_type()

        stream.out(template.interface_ami_sendp,
                   assign_args  = "\n".join(assign_args),
                   cd_name      = cd_names[1],
                   poller_class = poller_cls.base(gscope=1))


    def out_ami_poller(self, stream, ami_method, poller_method, cd_names):
        assign_res = []
        ami_args       = iter(poller_method.arg_names())
        timeout_arg    = next(ami_args)

        if self.__has_return_value:
            argtype = types.Type(self.__returntype)
            (h_is_const,h_is_ptr),(s_is_holder,s_is_var) = _arg_info(argtype, 3)

            if s_is_var:
                assign_res.append("%s = _call_desc->result._retn();" %
                                  next(ami_args))
            else:
                assign_res.append("%s = _call_desc->result;" %
                                  next(ami_args)) 


        for n, argument in enumerate(self.__arguments):
            if not argument.is_out():
                continue

            arg_ident = next(ami_args)
            arg_n     = "_call_desc->arg_%d" % n

            argtype = types.Type(argument.paramType())
            ((h_is_const,h_is_ptr),(s_is_holder,s_is_var)) = \
                         _arg_info(argtype,argument.direction())

            if s_is_holder:
                if s_is_var:
                    assign_res.append("%s = %s._retn();" % (arg_ident, arg_n))
                else:
                    assign_res.append("%s = %s;" % (arg_ident, arg_n))

            else:
                if s_is_var:
                    assign_res.append("%s = %s_._retn();" % (arg_ident, arg_n))

                else:
                    if argtype.array():
                        assign_res.append("%s_copy(%s, %s_);" %
                                          (argtype.base(), arg_ident, arg_n))
                    else:
                        assign_res.append("%s = %s_;" % (arg_ident, arg_n))

        if assign_res:
            tmpl = template.interface_ami_poller_method
        else:
            tmpl = template.interface_ami_poller_method_empty
            
        stream.out(tmpl,
                   cd_name_c   = cd_names[0],
                   cd_name_p   = cd_names[1],
                   timeout_arg = timeout_arg,
                   assign_res  = "\n".join(assign_res))





def _arg_info(type,direction):
    # Return a tuple containing the information about the mapping of
    # this argument type.
    # The tuple is of this form:
    # (is_const, is_ptr), (same_as_holder, is_var)
    # The 1st element is about the argument holder and
    #     is_const == 1 if the holder is a const
    #     is_ptr == 1 if the holder is a pointer
    # The 2nd element is about the argument storage and
    #     same_as_holder == 1 if the storage is the same variable as the holder
    #     is_var == 1 if the storage is a CORBA _var type, otherwise the
    #                 storage is the base type.
    assert isinstance(type, types.Type)
    
    if type.array():
        if type.variable():
            return _arg_array_mapping[_variable][direction]
        else:
            return _arg_array_mapping[_fixed][direction]
    else:
        d_type = type.deref(1)
        d_kind = d_type.kind()
        if d_kind in _arg_mapping:
            return _arg_mapping[d_kind][direction]
        else:
            assert d_kind == idltype.tk_struct or \
                   d_kind == idltype.tk_union
            if d_type.variable():
                return _arg_struct_mapping[_variable][direction]
            else:
                return _arg_struct_mapping[_fixed][direction]

# (holder_is_const, holder_is_ptr), (storage_same_as_holder, storage_is_var)
# See _arg_info for the meaning of these 0s and 1s.
_arg_mapping = {
    idltype.tk_boolean:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_char:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_octet:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_short:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_ushort:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_long:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_ulong:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_longlong:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_ulonglong:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_float:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_double:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_longdouble:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_enum:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_wchar:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_fixed:
    ( ((0,0),(1,0)), ((0,0),(1,0)), ((0,1),(0,0)), ((0,0),(1,0)) ),
    idltype.tk_string:
    ( ((1,0),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    idltype.tk_wstring:
    ( ((1,0),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    idltype.tk_sequence:
    ( ((1,1),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    idltype.tk_objref:
    ( ((0,0),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    idltype.tk_TypeCode:
    ( ((0,0),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    idltype.tk_any:
    ( ((1,1),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    idltype.tk_value:
    ( ((0,0),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    idltype.tk_value_box:
    ( ((0,0),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    idltype.tk_abstract_interface:
    ( ((0,0),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    idltype.tk_local_interface:
    ( ((0,0),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    }

_fixed = 0
_variable = 1

# See _arg_info for the meaning of these 0s and 1s.
_arg_struct_mapping = {
    _variable:
    ( ((1,1),(0,1)), ((0,0),(1,1)), ((0,1),(0,1)), ((0,0),(1,1)) ),
    _fixed:
    ( ((1,1),(0,0)), ((0,1),(0,0)), ((0,1),(0,0)), ((0,0),(1,0)) )
    }

# See _arg_info for the meaning of these 0s and 1s.
_arg_array_mapping = {
    _variable:
    ( ((1,0),(0,1)), ((0,0),(1,1)), ((0,0),(0,1)), ((0,0),(1,1)) ),
    _fixed:                                                      
    ( ((1,0),(0,0)), ((0,0),(0,0)), ((0,0),(0,0)), ((0,0),(1,1)) )
    }                                                      

