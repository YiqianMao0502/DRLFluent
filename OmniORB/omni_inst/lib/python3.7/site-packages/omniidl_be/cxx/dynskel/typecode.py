# -*- python -*-
#                           Package   : omniidl
# typecode.py               Created on: 1999/12/2
#                           Author    : David Scott (djs)
#
#    Copyright (C) 2003-2012 Apasphere Ltd
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

"""Produces the instances of CORBA::TypeCode"""

from omniidl import idlast, idltype, idlutil
from omniidl_be.cxx import ast, output, util, config, types, id
from omniidl_be.cxx.dynskel import template

import sys
self = sys.modules[__name__]

stream     = None
tophalf    = None
bottomhalf = None

# For a given type declaration, creates (private) static instances of
# CORBA::TypeCode_ptr for that type, and any necessary for contained
# constructed types. Contained types from other files cannot be used
# because the order of static initialiser execution is not defined.
# eg
#   IDL:   struct testStruct{
#            char a;
#            foo  b;  // b is defined elsewhere
#          };
# becomes: static CORBA::PR_StructMember _0RL_structmember_testStruct[] = {
#            {"a", CORBA::TypeCode::PR_char_tc()},
#            {"b", _0RL_tc_foo} // defined elsewhere
#          };
#          static CORBA::TypeCode_ptr _0RL_tc_testStruct = .....
#
# Types constructed in the main file have an externally visible symbol
# defined:
#          const CORBA::TypeCode_ptr _tc_testStruct = _0RL_tc_testStruct
#

# ----------------------------------------------------------------------
# Utility functions local to this module start here
# ----------------------------------------------------------------------

class NameAlreadyDefined:
    def __str__(self):
        return "Name has already been defined in this scope/block/file/section"


# returns true if the name has already been defined, and need not be defined
# again.
def alreadyDefined(mangledname):
    return mangledname in self.__defined_names

def defineName(mangledname):
    self.__defined_names[mangledname] = 1

def forwardUsed(node):
    sname = idlutil.slashName(node.scopedName())
    self.__forwards_pending[sname] = 1

def resolveForward(node):
    sname = idlutil.slashName(node.scopedName())
    if sname in self.__forwards_pending:
        del self.__forwards_pending[sname]
        return 1
    return 0

# mangleName("_0RL_tc", ["CORBA", "Object"]) -> "_ORL_tc_CORBA_Object"
def mangleName(prefix, scopedName):
    mangled = prefix + id.Name(scopedName).guard()
    return mangled

# Note: The AST has a notion of recursive structs and unions, but it can only
# say whether it is recursive, and not tell you how many nodes up the tree
# the recursive definition is. So we keep track of currently being-defined
# nodes here for that purpose.
self.__currentNodes = []

def startingNode(node):
    self.__currentNodes.append(node)

def finishingNode():
    assert(self.__currentNodes != [])
    self.__currentNodes = self.__currentNodes[:-1]

def currently_being_defined(node):
    return node in self.__currentNodes

def recursive_Depth(node):
    return len(self.__currentNodes) - self.__currentNodes.index(node)


def init(stream):
    self.stream = stream

    # declarations are built in two halves, this is to allow us
    # to keep the same order as the old backend. It could be simpler.
    self.tophalf = stream
    self.bottomhalf = stream
    self.__immediatelyInsideModule = 0

    # Dictionary with keys representing names defined. If two structures both
    # have a member of type foo, we should still only define the TypeCode for
    # foo once.
    self.__defined_names = {}

    # Dictionary of forward-declared structs/unions that have been
    # used in sequence TypeCodes, but not yet defined.
    self.__forwards_pending = {}

    # Normally when walking over the tree we only consider things
    # defined in the current file. However if we encounter a
    # dependency between something in the current file and something
    # defined elsewhere, we set the resolving_dependency flag and
    # recurse again.
    self.__resolving_dependency = 0
    
    return self


# Places TypeCode symbol in appropriate namespace with a non-static const
# declaration (performs MSVC workaround)
def external_linkage(decl, mangled_name = ""):
    assert isinstance(decl, idlast.DeclRepoId)

    # Don't give external linkage if we met this declaration in
    # resolving an out-of-file dependency
    if self.__resolving_dependency:
        return

    where            = bottomhalf
    scopedName       = id.Name(decl.scopedName())
    scope            = scopedName.scope()
    tc_name          = scopedName.prefix("_tc_")
    tc_unscoped_name = tc_name.simple()
    tc_name          = tc_name.fullyQualify()

    if mangled_name == "":
        mangled_name = mangleName(config.state['Private Prefix'] + "_tc_",
                                  decl.scopedName())

    if alreadyDefined(tc_name):
        return

    defineName(tc_name)

    # Needs the workaround if directly inside a module
    if not self.__immediatelyInsideModule:
        where.out("""\
const CORBA::TypeCode_ptr @tc_name@ = @mangled_name@;
""",
                  tc_name = tc_name, mangled_name = mangled_name)
        return

    open_namespace  = ""
    close_namespace = ""

    for s in scope:
        open_namespace  = open_namespace + "namespace " + s + " { "
        close_namespace = close_namespace + "} "

    where.out(template.external_linkage,
              open_namespace = open_namespace,
              close_namespace = close_namespace,
              tc_name = tc_name,
              mangled_name = mangled_name,
              tc_unscoped_name = tc_unscoped_name)


# Gets a TypeCode instance for a type
# Basic types have new typecodes generated, derived types are assumed
# to already exist and a name is passed instead
def mkTypeCode(type, declarator = None, node = None):
    assert isinstance(type, types.Type)

    prefix   = "CORBA::TypeCode::PR_"
    tctrack  = ", &" + config.state['Private Prefix'] + "_tcTrack"

    if declarator:
        assert isinstance(declarator, idlast.Declarator)
        dims = declarator.sizes()
        pre_str = ""
        post_str = ""
        for dim in dims:
            pre_str = pre_str + prefix + "array_tc(" + str(dim) + ", "
            post_str = post_str + tctrack + ")"

        return pre_str + mkTypeCode(type, None, node) + post_str

    type = type.type()

    basic = {
        idltype.tk_short:      "short",
        idltype.tk_long:       "long",
        idltype.tk_ushort:     "ushort",
        idltype.tk_ulong:      "ulong",
        idltype.tk_float:      "float",
        idltype.tk_double:     "double",
        idltype.tk_boolean:    "boolean",
        idltype.tk_char:       "char",
        idltype.tk_wchar:      "wchar",
        idltype.tk_octet:      "octet",
        idltype.tk_any:        "any",
        idltype.tk_TypeCode:   "TypeCode",
        idltype.tk_longlong:   "longlong",
        idltype.tk_ulonglong:  "ulonglong",
        idltype.tk_longdouble: "longdouble"
        }

    if type.kind() in basic:
        return prefix + basic[type.kind()] + "_tc()"

    if isinstance(type, idltype.Base):
        util.fatalError("Internal error generating TypeCode data")

    if isinstance(type, idltype.String):
        return prefix + "string_tc(" + str(type.bound()) + tctrack + ")"

    if isinstance(type, idltype.WString):
        return prefix + "wstring_tc(" + str(type.bound()) + tctrack + ")"

    if isinstance(type, idltype.Sequence):
        seqType = type.seqType()
        if isinstance(seqType, idltype.Declared):
            decl = seqType.decl()
            if hasattr(decl, "recursive") and decl.recursive() and \
                 currently_being_defined(decl):

                depth = recursive_Depth(decl)
                return prefix + "recursive_sequence_tc(" +\
                       str(type.bound()) + ", " + str(depth) + tctrack + ")"
            
        startingNode(type)
        ret = prefix + "sequence_tc(" + str(type.bound()) + ", " +\
              mkTypeCode(types.Type(type.seqType())) + tctrack + ")"
        finishingNode()
        return ret

    if isinstance(type, idltype.Fixed):
        return (prefix + "fixed_tc(%d,%d%s)" %
                (type.digits(),type.scale(),tctrack))

    assert isinstance(type, idltype.Declared)

    if type.kind() == idltype.tk_objref:
        scopedName = type.decl().scopedName()
        if scopedName == ["CORBA", "Object"]:
            return prefix + "Object_tc()"
        scopedName = id.Name(scopedName)
        
        repoID = type.decl().repoId()
        iname = scopedName.simple()
        return (prefix + 'interface_tc("' + repoID + '", "' +
                iname + '"' + tctrack + ')')

    elif type.kind() == idltype.tk_abstract_interface:
        scopedName = id.Name(type.decl().scopedName())
        
        repoID = type.decl().repoId()
        iname = scopedName.simple()
        return (prefix + 'abstract_interface_tc("' + repoID + '", "' +
                iname + '"' + tctrack + ')')

    elif type.kind() == idltype.tk_local_interface:
        scopedName = id.Name(type.decl().scopedName())
        
        repoID = type.decl().repoId()
        iname = scopedName.simple()
        return (prefix + 'local_interface_tc("' + repoID + '", "' +
                iname + '"' + tctrack + ')')

    guard_name = id.Name(type.scopedName()).guard()

    return config.state['Private Prefix'] + "_tc_" + guard_name
        
# ---------------------------------------------------------------
# Tree-walking part of module starts here
# ---------------------------------------------------------------

# Control arrives here
#
def visitAST(node):
    self.__completedModules = {}
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)


def visitModule(node):
    slash_scopedName = '/'.join(node.scopedName())

    if slash_scopedName in self.__completedModules:
        return
    self.__completedModules[slash_scopedName] = 1
    
    # This has a bearing on making symbols externally visible/ linkable
    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 1
    for n in node.definitions():
        n.accept(self)

    # Treat a reopened module as if it had been defined all at once
    for c in node.continuations():
        slash_scopedName = '/'.join(c.scopedName())
        self.__completedModules[slash_scopedName] = 1
        for n in c.definitions():
            n.accept(self)
            
    self.__immediatelyInsideModule = insideModule

# builds an instance of CORBA::PR_structMember containing pointers
# to all the TypeCodes of the structure members
def buildMembersStructure(node):
    struct = output.StringStream()
    mangled_name = mangleName(config.state['Private Prefix'] + \
                              "_structmember_", node.scopedName())
    if alreadyDefined(mangled_name):
        # no need to regenerate
        return struct
    
    defineName(mangled_name)
    
    members = node.members()
    array = []

    for m in members:
        memberType = types.Type(m.memberType())
        for d in m.declarators():
            this_name = id.Name(d.scopedName()).simple()
            typecode = mkTypeCode(memberType, d, node)
            array.append( "{\"" + this_name + "\", " + typecode + "}" )

    if len(members) > 0:
        struct.out("""\
static CORBA::PR_structMember @mangled_name@[] = {
  @members@
};""", members = ",\n".join(array), mangled_name = mangled_name)

    return struct

# Convenience function to total up the number of members, treating
# declarators separately.
def numMembers(node):
    members = node.members()
    num = 0
    for m in members:
        num = num + len(m.declarators())

    return num
            

def visitStruct(node):
    startingNode(node)
    
    # the key here is to redirect the bottom half to a buffer
    # just for now
    oldbottomhalf = self.bottomhalf
    self.bottomhalf = output.StringStream()

    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0

    # create the static typecodes for constructed types by setting
    # the resolving_dependency flag and recursing
    save_resolving_dependency = self.__resolving_dependency
    
    for child in node.members():
        memberType = child.memberType()

        if child.constrType():
            self.__resolving_dependency = save_resolving_dependency
        else:
            self.__resolving_dependency = 1

        if isinstance(memberType, idltype.Declared):
            memberType.decl().accept(self)

        elif isinstance(memberType, idltype.Sequence):
            # anonymous sequence (maybe sequence<sequence<...<T>>>)
            # Find the ultimate base type, and if it's user declared then
            # produce a typecode definition for it.
            base_type = memberType.seqType()
            while isinstance(base_type, idltype.Sequence):
                base_type = base_type.seqType()

            # if a struct is recursive, don't loop forever :)
            if isinstance(base_type, idltype.Declared):
                decl = base_type.decl()
                if not currently_being_defined(decl):
                    base_type.decl().accept(self)
                        
    self.__resolving_dependency = save_resolving_dependency

    tophalf.out(str(buildMembersStructure(node)))

    scopedName = node.scopedName()
    mangled_name = mangleName(config.state['Private Prefix'] + "_tc_",
                              scopedName)
    if not alreadyDefined(mangled_name):
        # only define the name once

        defineName(mangled_name)
    
        structmember_mangled_name =\
                                  mangleName(config.state['Private Prefix'] + \
                                             "_structmember_", scopedName)
        assert alreadyDefined(structmember_mangled_name)
    
        num = numMembers(node)
        repoID = node.repoId()
        struct_name = id.Name(scopedName).simple()

        tophalf.out("""\
#ifdef @mangled_name@
#  undef @mangled_name@
#endif
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_struct_tc("@repoID@", "@name@", @structmember_mangled_name@, @n@, &@pprefix@_tcTrack);
""",
                    mangled_name = mangled_name,
                    structmember_mangled_name = structmember_mangled_name,
                    name = struct_name, n = str(num),
                    repoID = repoID, pprefix=config.state['Private Prefix'])

    self.__immediatelyInsideModule = insideModule

    external_linkage(node)

    # restore the old bottom half
    oldbottomhalf.out(str(self.bottomhalf))
    self.bottomhalf = oldbottomhalf
    finishingNode()


def visitStructForward(node):
    scopedName = node.scopedName()
    mangled_name = mangleName(config.state['Private Prefix'] +
                              "_tc_", scopedName)
    fmangled_name = mangleName(config.state['Private Prefix'] +
                               "_ft_", scopedName)

    if not alreadyDefined(fmangled_name):
        defineName(fmangled_name)

        tophalf.out("""\
static CORBA::TypeCode_ptr @fmangled_name@ = CORBA::TypeCode::PR_forward_tc("@repoId@", &@pprefix@_tcTrack);
#define @mangled_name@ @fmangled_name@
""",
                    mangled_name = mangled_name,
                    fmangled_name = fmangled_name,
                    repoId = node.repoId(),
                    pprefix=config.state['Private Prefix'])


def visitUnion(node):
    scopedName = node.scopedName()
    mangled_name = mangleName(config.state['Private Prefix'] +\
                              "_tc_", scopedName)
    if alreadyDefined(mangled_name):
        return

    startingNode(node)
    
    # the key here is to redirect the bottom half to a buffer
    # just for now
    oldbottomhalf = self.bottomhalf
    self.bottomhalf = output.StringStream()

    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0
    
    # need to build a static array of node members in a similar fashion
    # to structs
    array = []
    switchType = types.Type(node.switchType())
    deref_switchType = switchType.deref()
    if isinstance(switchType.type(), idltype.Declared):
        save_resolving_dependency = self.__resolving_dependency
        if not node.constrType():
            self.__resolving_dependency = 1
        switchType.type().decl().accept(self)
        self.__resolving_dependency = save_resolving_dependency
        
    numlabels = 0
    numcases = 0
    hasDefault = None
    
    for c in node.cases():
        numcases = numcases + 1
        decl = c.declarator()
        caseType = types.Type(c.caseType())

        save_resolving_dependency = self.__resolving_dependency

        if not c.constrType():
            self.__resolving_dependency = 1

        if isinstance(caseType.type(), idltype.Declared):
            caseType.type().decl().accept(self)

        elif caseType.sequence():
            # anonymous sequence
            seqType = caseType.type().seqType()
            while isinstance(seqType, idltype.Sequence):
                seqType = seqType.seqType()
            if isinstance(seqType, idltype.Declared):
                # don't loop forever
                if not currently_being_defined(seqType.decl()):
                    seqType.decl().accept(self)
                
        self.__resolving_dependency = save_resolving_dependency
        
        typecode = mkTypeCode(caseType, decl, node)
        case_name = id.Name(decl.scopedName()).simple()

        for l in c.labels():
            if l.default():
                label = "0"
                hasDefault = numlabels
            else:
                label = switchType.literal(l.value())

            array.append('{"%s", %s, (CORBA::PR_unionDiscriminator)%s}' %
                         (case_name, typecode, label))
            numlabels = numlabels + 1


    discrim_tc = mkTypeCode(deref_switchType)
    repoID = node.repoId()

    union_name = id.Name(scopedName).simple()
    unionmember_mangled_name = mangleName(config.state['Private Prefix'] +
                                          "_unionMember_", scopedName)
    
    default_str = ""
    if hasDefault is None:
        default_str = "-1"
    else:
        default_str = str(hasDefault)

    tophalf.out("""\
static CORBA::PR_unionMember @unionmember_mangled_name@[] = {
  @members@
};
#ifdef @mangled_name@
#  undef @mangled_name@
#endif
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_union_tc("@repoID@", "@name@", @discrim_tc@, @unionmember_mangled_name@, @labels@, @default_str@, &@pprefix@_tcTrack);""",
                mangled_name = mangled_name,
                repoID = repoID,
                discrim_tc = discrim_tc,
                unionmember_mangled_name = unionmember_mangled_name,
                name = union_name,
                labels = str(numlabels),
                default_str = default_str,
                members = ",\n".join(array),
                pprefix = config.state['Private Prefix'])
    
    defineName(unionmember_mangled_name)
    defineName(mangled_name)

    self.__immediatelyInsideModule = insideModule

    external_linkage(node)

    # restore the old bottom half
    oldbottomhalf.out(str(self.bottomhalf))
    self.bottomhalf = oldbottomhalf

    finishingNode()


def visitUnionForward(node):
    scopedName = node.scopedName()
    mangled_name = mangleName(config.state['Private Prefix'] +
                              "_tc_", scopedName)
    fmangled_name = mangleName(config.state['Private Prefix'] +
                               "_ft_", scopedName)

    if not alreadyDefined(fmangled_name):
        defineName(fmangled_name)

        tophalf.out("""\
static CORBA::TypeCode_ptr @fmangled_name@ = CORBA::TypeCode::PR_forward_tc("@repoId@", &@pprefix@_tcTrack);
#define @mangled_name@ @fmangled_name@
""",
                    mangled_name = mangled_name,
                    fmangled_name = fmangled_name,
                    repoId = node.repoId(),
                    pprefix=config.state['Private Prefix'])



def visitEnum(node):
    scopedName = node.scopedName()
    mangled_name = mangleName(config.state['Private Prefix'] +\
                              "_tc_", scopedName)
    if alreadyDefined(mangled_name):
        return
    
    enumerators = node.enumerators()

    names = []
    for enumerator in enumerators:
        names.append('"' + id.Name(enumerator.scopedName()).simple(cxx=0) +
                     '"')

    enum_name = id.Name(scopedName).simple()
    
    repoID = node.repoId()

    tc_name = id.Name(scopedName).prefix("_tc_").fullyQualify()
    enummember_mangled_name = mangleName(config.state['Private Prefix'] +
                                         "_enumMember_", scopedName)

    tophalf.out("""\
static const char* @enummember_mangled_name@[] = { @elements@ };
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_enum_tc("@repoID@", "@name@", @enummember_mangled_name@, @numcases@, &@pprefix@_tcTrack);""",
                enummember_mangled_name = enummember_mangled_name,
                mangled_name = mangled_name,
                elements = ", ".join(names),
                repoID = repoID,
                name = enum_name,
                numcases = str(len(names)),
                pprefix = config.state['Private Prefix'])

    defineName(mangled_name)
    defineName(enummember_mangled_name)

    external_linkage(node)

def visitForward(node):
    pass
    

def visitInterface(node):
    if node.builtIn(): return

    # interfaces containing members with the type of the interface
    # cause a minor (non fatal) problem with ordering of the outputted
    # declarations. This check only serves to correct this cosmetic flaw
    # and make the output of the new system identical to the old one.
    if hasattr(node, "typecode_already_been_here"):
        return

    node.typecode_already_been_here = 1

    startingNode(node)

    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0
    for n in node.declarations():
        n.accept(self)
    self.__immediatelyInsideModule = insideModule
    
    repoID = node.repoId()
    iname = id.Name(node.scopedName()).simple()

    if node.abstract():
        func = "PR_abstract_interface_tc"
    elif node.local():
        func = "PR_local_interface_tc"
    else:
        func = "PR_interface_tc"

    typecode = 'CORBA::TypeCode::' + func + '("' + repoID + '", "' +\
               iname + '", &' + config.state['Private Prefix'] + '_tcTrack)'


    external_linkage(node, typecode)
    finishingNode()


def recurse(type, constr=0):
    assert isinstance(type, types.Type)
    
    deref_type = type.deref()
    if isinstance(type.type(), idltype.Declared):
        base_decl = type.type().decl()
        save_resolving_dependency = self.__resolving_dependency
        if not constr:
            self.__resolving_dependency = 1
        base_decl.accept(self)
        self.__resolving_dependency = save_resolving_dependency

    elif deref_type.sequence():
        seqType = deref_type.type().seqType()
        if isinstance(seqType, idltype.Declared):
            base_decl = seqType.decl()

            save_resolving_dependency = self.__resolving_dependency
            self.__resolving_dependency = 1
            base_decl.accept(self)
            self.__resolving_dependency = save_resolving_dependency

        elif types.Type(seqType).sequence():
            # anonymous sequence
            recurse(types.Type(seqType.seqType()))


def visitDeclarator(declarator):
    # this must be a typedef declarator
    
    node = declarator.alias()
    aliasType = types.Type(node.aliasType())

    recurse(aliasType)
    
    scopedName = declarator.scopedName()
    mangled_name = mangleName(config.state['Private Prefix'] +
                              "_tc_", scopedName)
    if alreadyDefined(mangled_name):
        return
    
    repoID = declarator.repoId()
    typecode = mkTypeCode(aliasType, declarator)

    scopedName = declarator.scopedName()
    typedef_name = id.Name(scopedName).simple()
    
    tophalf.out("""\
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_alias_tc("@repoID@", "@name@", @typecode@, &@pprefix@_tcTrack);

""",
                mangled_name = mangled_name,
                repoID = repoID,
                name = typedef_name,
                typecode = typecode,
                pprefix = config.state['Private Prefix'])
    defineName(mangled_name)

    external_linkage(declarator)


def visitTypedef(node):
    aliasType = types.Type(node.aliasType())

    recurse(aliasType, node.constrType())

    for declarator in node.declarators():
        declarator.accept(self)

        
def visitConst(node):
    pass


def visitException(node):
    scopedName = node.scopedName()
    mangled_name = mangleName(config.state['Private Prefix'] +
                              "_tc_", scopedName)
    if alreadyDefined(mangled_name):
        return
    defineName(mangled_name)

    startingNode(node)
    
    # the key here is to redirect the bottom half to a buffer
    # just for now
    oldbottomhalf = self.bottomhalf
    self.bottomhalf = output.StringStream()

    # create the static typecodes for constructed types by setting
    # the resolving_dependency flag and recursing
    save_resolving_dependency = self.__resolving_dependency

    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0
    
    for child in node.members():
        memberType = child.memberType()

        if child.constrType():
            self.__resolving_dependency = save_resolving_dependency
        else:
            self.__resolving_dependency = 1

        if isinstance(memberType, idltype.Declared):
            memberType.decl().accept(self)

    self.__resolving_dependency = save_resolving_dependency
    self.__immediatelyInsideModule = insideModule

    
    # output the structure of members
    tophalf.out(str(buildMembersStructure(node)))
    
    num = numMembers(node)

    repoID = node.repoId()
    ex_name = id.Name(scopedName).simple()
    structmember_mangled_name = mangleName(config.state['Private Prefix'] + \
                                           "_structmember_", scopedName)
    if num == 0:
        structmember_mangled_name = "(CORBA::PR_structMember*) 0"
    tophalf.out("""\
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_exception_tc("@repoID@", "@name@", @structmember_mangled_name@, @n@, &@pprefix@_tcTrack);""",
                mangled_name = mangled_name,
                name = ex_name, n = str(num),
                structmember_mangled_name = structmember_mangled_name,
                repoID = repoID,
                pprefix = config.state['Private Prefix'])

    external_linkage(node)

    # restore the old bottom half
    oldbottomhalf.out(str(self.bottomhalf))
    self.bottomhalf = oldbottomhalf


    finishingNode()


# builds an instance of CORBA::PR_valueMember containing pointers
# to all the TypeCodes of the value statemembers
def buildStateMembersStructure(node):
    struct = output.StringStream()
    mangled_name = mangleName(config.state['Private Prefix'] + \
                              "_valuemember_", node.scopedName())
    if alreadyDefined(mangled_name):
        # no need to regenerate
        return struct
    
    defineName(mangled_name)
    
    members = node.statemembers()
    array = []

    if members:
        for m in members:
            memberType = types.Type(m.memberType())
            access = m.memberAccess()
            for d in m.declarators():
                this_name = id.Name(d.scopedName()).simple()
                typecode = mkTypeCode(memberType, d, node)
                array.append('{"%s", %s, %d}' % (this_name, typecode, access))

        struct.out("""\
static CORBA::PR_valueMember @mangled_name@[] = {
  @members@
};""", members = ",\n".join(array), mangled_name = mangled_name)
    else:
        struct.out("""\
static CORBA::PR_valueMember* @mangled_name@ = 0;""",
                   mangled_name=mangled_name)

    return struct


# Convenience function to total up the number of members, treating
# declarators separately.
def numStateMembers(node):
    members = node.statemembers()
    num = 0
    for m in members:
        num = num + len(m.declarators())

    return num


def visitValue(node):
    # Used for abstract value too
    
    startingNode(node)
    
    scopedName = node.scopedName()
    mangled_name = mangleName(config.state['Private Prefix'] +
                              "_tc_", scopedName)

    visitValueForward(node)

    # the key here is to redirect the bottom half to a buffer
    # just for now
    oldbottomhalf = self.bottomhalf
    self.bottomhalf = output.StringStream()

    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0

    # handle nested types
    for n in node.declarations():
        n.accept(self)

    # create the static typecodes for constructed types by setting
    # the resolving_dependency flag and recursing
    save_resolving_dependency = self.__resolving_dependency
    
    for child in node.statemembers():
        memberType = child.memberType()

        if child.constrType():
            self.__resolving_dependency = save_resolving_dependency
        else:
            self.__resolving_dependency = 1

        if isinstance(memberType, idltype.Declared):
            decl = memberType.decl()
            if not currently_being_defined(decl):
                decl.accept(self)

        elif isinstance(memberType, idltype.Sequence):
            # anonymous sequence (maybe sequence<sequence<...<T>>>)
            # Find the ultimate base type, and if it's user declared then
            # produce a typecode definition for it.
            base_type = memberType.seqType()
            while isinstance(base_type, idltype.Sequence):
                base_type = base_type.seqType()

            if isinstance(base_type, idltype.Declared):
                decl = base_type.decl()
                if not currently_being_defined(decl):
                    decl.accept(self)
                        
    self.__resolving_dependency = save_resolving_dependency

    tophalf.out(str(buildStateMembersStructure(node)))

    if not alreadyDefined(mangled_name):
        # only define the name once
        defineName(mangled_name)
    
        valuemember_mangled_name = mangleName(config.state['Private Prefix'] +
                                              "_valuemember_", scopedName)
        assert alreadyDefined(valuemember_mangled_name)
    
        num = numStateMembers(node)
        repoID = node.repoId()
        value_name = id.Name(scopedName).simple()

        # Value modifiers
        modifierl = []
        if isinstance(node, idlast.Value):
            if node.custom():
                modifierl.append("CORBA::VM_CUSTOM")
            if node.truncatable():
                modifierl.append("CORBA::VM_TRUNCATABLE")
        else:
            assert isinstance(node, idlast.ValueAbs)
            modifierl.append("CORBA::VM_ABSTRACT")

        if modifierl:
            modifiers = "|".join(modifierl)
        else:
            modifiers = "CORBA::VM_NONE"

        # Concrete base
        inherits = node.inherits()
        if (isinstance(node, idlast.Value) and
            inherits and isinstance(inherits[0], idlast.Value)):

            visitValueForward(inherits[0])
            bscopedName = inherits[0].scopedName()
            concrete_base = mangleName(config.state['Private Prefix'] +
                                       "_tc_", bscopedName)
        else:
            concrete_base = "CORBA::TypeCode::PR_null_tc()"

        tophalf.out("""\
#ifdef @mangled_name@
#  undef @mangled_name@
#endif
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_value_tc("@repoID@", "@name@", @modifiers@, @concrete_base@, @valuemember_mangled_name@, @n@, &@pprefix@_tcTrack);
""",
                    mangled_name = mangled_name,
                    modifiers = modifiers,
                    concrete_base = concrete_base,
                    valuemember_mangled_name = valuemember_mangled_name,
                    name = value_name, n = str(num),
                    repoID = repoID, pprefix=config.state['Private Prefix'])

    self.__immediatelyInsideModule = insideModule

    external_linkage(node)
    # restore the old bottom half
    oldbottomhalf.out(str(self.bottomhalf))
    self.bottomhalf = oldbottomhalf
    finishingNode()


def visitValueForward(node):
    scopedName = node.scopedName()
    mangled_name = mangleName(config.state['Private Prefix'] +
                              "_tc_", scopedName)
    fmangled_name = mangleName(config.state['Private Prefix'] +
                               "_ft_", scopedName)

    if not alreadyDefined(fmangled_name):
        defineName(fmangled_name)

        tophalf.out("""\
static CORBA::TypeCode_ptr @fmangled_name@ = CORBA::TypeCode::PR_forward_tc("@repoId@", &@pprefix@_tcTrack);
#define @mangled_name@ @fmangled_name@
""",
                    mangled_name = mangled_name,
                    fmangled_name = fmangled_name,
                    repoId = node.repoId(),
                    pprefix=config.state['Private Prefix'])


def visitValueAbs(node):
    visitValue(node)

def visitValueBox(node):
    boxedType = types.Type(node.boxedType())

    recurse(boxedType, node.constrType())
    
    scopedName = node.scopedName()
    mangled_name = mangleName(config.state['Private Prefix'] +\
                              "_tc_", scopedName)
    if alreadyDefined(mangled_name):
        return
    
    repoID = node.repoId()
    typecode = mkTypeCode(boxedType)

    scopedName = node.scopedName()
    boxed_name = id.Name(scopedName).simple()
    
    tophalf.out("""\
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_value_box_tc("@repoID@", "@name@", @typecode@, &@pprefix@_tcTrack);

""",
                mangled_name = mangled_name,
                repoID = repoID,
                name = boxed_name,
                typecode = typecode,
                pprefix = config.state['Private Prefix'])

    defineName(mangled_name)
    external_linkage(node)    
