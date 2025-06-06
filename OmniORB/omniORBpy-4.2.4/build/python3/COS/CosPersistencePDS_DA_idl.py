# Python stubs generated by omniidl from /mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst/share/idl/omniORB/COS/CosPersistencePDS_DA.idl
# DO NOT EDIT THIS FILE!

import omniORB, _omnipy
from omniORB import CORBA, PortableServer
_0_CORBA = CORBA


_omnipy.checkVersion(4,2, __file__, 1)

try:
    property
except NameError:
    def property(*args):
        return None


# #include "CosPersistencePID.idl"
import CosPersistencePID_idl
_0_CosPersistencePID = omniORB.openModule("CosPersistencePID")
_0_CosPersistencePID__POA = omniORB.openModule("CosPersistencePID__POA")

# #include "CosPersistencePDS.idl"
import CosPersistencePDS_idl
_0_CosPersistencePDS = omniORB.openModule("CosPersistencePDS")
_0_CosPersistencePDS__POA = omniORB.openModule("CosPersistencePDS__POA")

#
# Start of module "CosPersistencePDS_DA"
#
__name__ = "CosPersistencePDS_DA"
_0_CosPersistencePDS_DA = omniORB.openModule("CosPersistencePDS_DA", r"/mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst/share/idl/omniORB/COS/CosPersistencePDS_DA.idl")
_0_CosPersistencePDS_DA__POA = omniORB.openModule("CosPersistencePDS_DA__POA", r"/mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst/share/idl/omniORB/COS/CosPersistencePDS_DA.idl")


# typedef ... DAObjectID
class DAObjectID:
    _NP_RepositoryId = "IDL:omg.org/CosPersistencePDS_DA/DAObjectID:1.0"
    def __init__(self, *args, **kw):
        raise RuntimeError("Cannot construct objects of this type.")
_0_CosPersistencePDS_DA.DAObjectID = DAObjectID
_0_CosPersistencePDS_DA._d_DAObjectID  = (omniORB.tcInternal.tv_string,0)
_0_CosPersistencePDS_DA._ad_DAObjectID = (omniORB.tcInternal.tv_alias, DAObjectID._NP_RepositoryId, "DAObjectID", (omniORB.tcInternal.tv_string,0))
_0_CosPersistencePDS_DA._tc_DAObjectID = omniORB.tcInternal.createTypeCode(_0_CosPersistencePDS_DA._ad_DAObjectID)
omniORB.registerType(DAObjectID._NP_RepositoryId, _0_CosPersistencePDS_DA._ad_DAObjectID, _0_CosPersistencePDS_DA._tc_DAObjectID)
del DAObjectID

# interface PID_DA
_0_CosPersistencePDS_DA._d_PID_DA = (omniORB.tcInternal.tv_objref, "IDL:omg.org/CosPersistencePDS_DA/PID_DA:1.0", "PID_DA")
omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/PID_DA:1.0"] = _0_CosPersistencePDS_DA._d_PID_DA
_0_CosPersistencePDS_DA.PID_DA = omniORB.newEmptyClass()
class PID_DA (_0_CosPersistencePID.PID):
    _NP_RepositoryId = _0_CosPersistencePDS_DA._d_PID_DA[1]

    def __init__(self, *args, **kw):
        raise RuntimeError("Cannot construct objects of this type.")

    _nil = CORBA.Object._nil


_0_CosPersistencePDS_DA.PID_DA = PID_DA
_0_CosPersistencePDS_DA._tc_PID_DA = omniORB.tcInternal.createTypeCode(_0_CosPersistencePDS_DA._d_PID_DA)
omniORB.registerType(PID_DA._NP_RepositoryId, _0_CosPersistencePDS_DA._d_PID_DA, _0_CosPersistencePDS_DA._tc_PID_DA)

# PID_DA operations and attributes
PID_DA._d__get_oid = ((),(omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObjectID:1.0"],),None)
PID_DA._d__set_oid = ((omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObjectID:1.0"],),(),None)

# PID_DA object reference
class _objref_PID_DA (_0_CosPersistencePID._objref_PID):
    _NP_RepositoryId = PID_DA._NP_RepositoryId

    def __init__(self, obj):
        _0_CosPersistencePID._objref_PID.__init__(self, obj)

    def _get_oid(self, *args):
        return self._obj.invoke("_get_oid", _0_CosPersistencePDS_DA.PID_DA._d__get_oid, args)

    def _set_oid(self, *args):
        return self._obj.invoke("_set_oid", _0_CosPersistencePDS_DA.PID_DA._d__set_oid, args)

    oid = property(_get_oid, _set_oid)


omniORB.registerObjref(PID_DA._NP_RepositoryId, _objref_PID_DA)
_0_CosPersistencePDS_DA._objref_PID_DA = _objref_PID_DA
del PID_DA, _objref_PID_DA

# PID_DA skeleton
__name__ = "CosPersistencePDS_DA__POA"
class PID_DA (_0_CosPersistencePID__POA.PID):
    _NP_RepositoryId = _0_CosPersistencePDS_DA.PID_DA._NP_RepositoryId


    _omni_op_d = {"_get_oid": _0_CosPersistencePDS_DA.PID_DA._d__get_oid, "_set_oid": _0_CosPersistencePDS_DA.PID_DA._d__set_oid}
    _omni_op_d.update(_0_CosPersistencePID__POA.PID._omni_op_d)

PID_DA._omni_skeleton = PID_DA
_0_CosPersistencePDS_DA__POA.PID_DA = PID_DA
omniORB.registerSkeleton(PID_DA._NP_RepositoryId, PID_DA)
del PID_DA
__name__ = "CosPersistencePDS_DA"

# interface DAObject
_0_CosPersistencePDS_DA._d_DAObject = (omniORB.tcInternal.tv_objref, "IDL:omg.org/CosPersistencePDS_DA/DAObject:1.0", "DAObject")
omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObject:1.0"] = _0_CosPersistencePDS_DA._d_DAObject
_0_CosPersistencePDS_DA.DAObject = omniORB.newEmptyClass()
class DAObject :
    _NP_RepositoryId = _0_CosPersistencePDS_DA._d_DAObject[1]

    def __init__(self, *args, **kw):
        raise RuntimeError("Cannot construct objects of this type.")

    _nil = CORBA.Object._nil


_0_CosPersistencePDS_DA.DAObject = DAObject
_0_CosPersistencePDS_DA._tc_DAObject = omniORB.tcInternal.createTypeCode(_0_CosPersistencePDS_DA._d_DAObject)
omniORB.registerType(DAObject._NP_RepositoryId, _0_CosPersistencePDS_DA._d_DAObject, _0_CosPersistencePDS_DA._tc_DAObject)

# DAObject operations and attributes
DAObject._d_dado_same = ((omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObject:1.0"], ), (omniORB.tcInternal.tv_boolean, ), None)
DAObject._d_dado_oid = ((), (omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObjectID:1.0"], ), None)
DAObject._d_dado_pid = ((), (omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/PID_DA:1.0"], ), None)
DAObject._d_dado_remove = ((), (), None)
DAObject._d_dado_free = ((), (), None)

# DAObject object reference
class _objref_DAObject (CORBA.Object):
    _NP_RepositoryId = DAObject._NP_RepositoryId

    def __init__(self, obj):
        CORBA.Object.__init__(self, obj)

    def dado_same(self, *args):
        return self._obj.invoke("dado_same", _0_CosPersistencePDS_DA.DAObject._d_dado_same, args)

    def dado_oid(self, *args):
        return self._obj.invoke("dado_oid", _0_CosPersistencePDS_DA.DAObject._d_dado_oid, args)

    def dado_pid(self, *args):
        return self._obj.invoke("dado_pid", _0_CosPersistencePDS_DA.DAObject._d_dado_pid, args)

    def dado_remove(self, *args):
        return self._obj.invoke("dado_remove", _0_CosPersistencePDS_DA.DAObject._d_dado_remove, args)

    def dado_free(self, *args):
        return self._obj.invoke("dado_free", _0_CosPersistencePDS_DA.DAObject._d_dado_free, args)

omniORB.registerObjref(DAObject._NP_RepositoryId, _objref_DAObject)
_0_CosPersistencePDS_DA._objref_DAObject = _objref_DAObject
del DAObject, _objref_DAObject

# DAObject skeleton
__name__ = "CosPersistencePDS_DA__POA"
class DAObject (PortableServer.Servant):
    _NP_RepositoryId = _0_CosPersistencePDS_DA.DAObject._NP_RepositoryId


    _omni_op_d = {"dado_same": _0_CosPersistencePDS_DA.DAObject._d_dado_same, "dado_oid": _0_CosPersistencePDS_DA.DAObject._d_dado_oid, "dado_pid": _0_CosPersistencePDS_DA.DAObject._d_dado_pid, "dado_remove": _0_CosPersistencePDS_DA.DAObject._d_dado_remove, "dado_free": _0_CosPersistencePDS_DA.DAObject._d_dado_free}

DAObject._omni_skeleton = DAObject
_0_CosPersistencePDS_DA__POA.DAObject = DAObject
omniORB.registerSkeleton(DAObject._NP_RepositoryId, DAObject)
del DAObject
__name__ = "CosPersistencePDS_DA"

# interface DAObjectFactory
_0_CosPersistencePDS_DA._d_DAObjectFactory = (omniORB.tcInternal.tv_objref, "IDL:omg.org/CosPersistencePDS_DA/DAObjectFactory:1.0", "DAObjectFactory")
omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObjectFactory:1.0"] = _0_CosPersistencePDS_DA._d_DAObjectFactory
_0_CosPersistencePDS_DA.DAObjectFactory = omniORB.newEmptyClass()
class DAObjectFactory :
    _NP_RepositoryId = _0_CosPersistencePDS_DA._d_DAObjectFactory[1]

    def __init__(self, *args, **kw):
        raise RuntimeError("Cannot construct objects of this type.")

    _nil = CORBA.Object._nil


_0_CosPersistencePDS_DA.DAObjectFactory = DAObjectFactory
_0_CosPersistencePDS_DA._tc_DAObjectFactory = omniORB.tcInternal.createTypeCode(_0_CosPersistencePDS_DA._d_DAObjectFactory)
omniORB.registerType(DAObjectFactory._NP_RepositoryId, _0_CosPersistencePDS_DA._d_DAObjectFactory, _0_CosPersistencePDS_DA._tc_DAObjectFactory)

# DAObjectFactory operations and attributes
DAObjectFactory._d_create = ((), (omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObject:1.0"], ), None)

# DAObjectFactory object reference
class _objref_DAObjectFactory (CORBA.Object):
    _NP_RepositoryId = DAObjectFactory._NP_RepositoryId

    def __init__(self, obj):
        CORBA.Object.__init__(self, obj)

    def create(self, *args):
        return self._obj.invoke("create", _0_CosPersistencePDS_DA.DAObjectFactory._d_create, args)

omniORB.registerObjref(DAObjectFactory._NP_RepositoryId, _objref_DAObjectFactory)
_0_CosPersistencePDS_DA._objref_DAObjectFactory = _objref_DAObjectFactory
del DAObjectFactory, _objref_DAObjectFactory

# DAObjectFactory skeleton
__name__ = "CosPersistencePDS_DA__POA"
class DAObjectFactory (PortableServer.Servant):
    _NP_RepositoryId = _0_CosPersistencePDS_DA.DAObjectFactory._NP_RepositoryId


    _omni_op_d = {"create": _0_CosPersistencePDS_DA.DAObjectFactory._d_create}

DAObjectFactory._omni_skeleton = DAObjectFactory
_0_CosPersistencePDS_DA__POA.DAObjectFactory = DAObjectFactory
omniORB.registerSkeleton(DAObjectFactory._NP_RepositoryId, DAObjectFactory)
del DAObjectFactory
__name__ = "CosPersistencePDS_DA"

# interface DAObjectFactoryFinder
_0_CosPersistencePDS_DA._d_DAObjectFactoryFinder = (omniORB.tcInternal.tv_objref, "IDL:omg.org/CosPersistencePDS_DA/DAObjectFactoryFinder:1.0", "DAObjectFactoryFinder")
omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObjectFactoryFinder:1.0"] = _0_CosPersistencePDS_DA._d_DAObjectFactoryFinder
_0_CosPersistencePDS_DA.DAObjectFactoryFinder = omniORB.newEmptyClass()
class DAObjectFactoryFinder :
    _NP_RepositoryId = _0_CosPersistencePDS_DA._d_DAObjectFactoryFinder[1]

    def __init__(self, *args, **kw):
        raise RuntimeError("Cannot construct objects of this type.")

    _nil = CORBA.Object._nil


_0_CosPersistencePDS_DA.DAObjectFactoryFinder = DAObjectFactoryFinder
_0_CosPersistencePDS_DA._tc_DAObjectFactoryFinder = omniORB.tcInternal.createTypeCode(_0_CosPersistencePDS_DA._d_DAObjectFactoryFinder)
omniORB.registerType(DAObjectFactoryFinder._NP_RepositoryId, _0_CosPersistencePDS_DA._d_DAObjectFactoryFinder, _0_CosPersistencePDS_DA._tc_DAObjectFactoryFinder)

# DAObjectFactoryFinder operations and attributes
DAObjectFactoryFinder._d_find_factory = (((omniORB.tcInternal.tv_string,0), ), (omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObjectFactory:1.0"], ), None)

# DAObjectFactoryFinder object reference
class _objref_DAObjectFactoryFinder (CORBA.Object):
    _NP_RepositoryId = DAObjectFactoryFinder._NP_RepositoryId

    def __init__(self, obj):
        CORBA.Object.__init__(self, obj)

    def find_factory(self, *args):
        return self._obj.invoke("find_factory", _0_CosPersistencePDS_DA.DAObjectFactoryFinder._d_find_factory, args)

omniORB.registerObjref(DAObjectFactoryFinder._NP_RepositoryId, _objref_DAObjectFactoryFinder)
_0_CosPersistencePDS_DA._objref_DAObjectFactoryFinder = _objref_DAObjectFactoryFinder
del DAObjectFactoryFinder, _objref_DAObjectFactoryFinder

# DAObjectFactoryFinder skeleton
__name__ = "CosPersistencePDS_DA__POA"
class DAObjectFactoryFinder (PortableServer.Servant):
    _NP_RepositoryId = _0_CosPersistencePDS_DA.DAObjectFactoryFinder._NP_RepositoryId


    _omni_op_d = {"find_factory": _0_CosPersistencePDS_DA.DAObjectFactoryFinder._d_find_factory}

DAObjectFactoryFinder._omni_skeleton = DAObjectFactoryFinder
_0_CosPersistencePDS_DA__POA.DAObjectFactoryFinder = DAObjectFactoryFinder
omniORB.registerSkeleton(DAObjectFactoryFinder._NP_RepositoryId, DAObjectFactoryFinder)
del DAObjectFactoryFinder
__name__ = "CosPersistencePDS_DA"

# interface PDS_DA
_0_CosPersistencePDS_DA._d_PDS_DA = (omniORB.tcInternal.tv_objref, "IDL:omg.org/CosPersistencePDS_DA/PDS_DA:1.0", "PDS_DA")
omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/PDS_DA:1.0"] = _0_CosPersistencePDS_DA._d_PDS_DA
_0_CosPersistencePDS_DA.PDS_DA = omniORB.newEmptyClass()
class PDS_DA (_0_CosPersistencePDS.PDS):
    _NP_RepositoryId = _0_CosPersistencePDS_DA._d_PDS_DA[1]

    def __init__(self, *args, **kw):
        raise RuntimeError("Cannot construct objects of this type.")

    _nil = CORBA.Object._nil


_0_CosPersistencePDS_DA.PDS_DA = PDS_DA
_0_CosPersistencePDS_DA._tc_PDS_DA = omniORB.tcInternal.createTypeCode(_0_CosPersistencePDS_DA._d_PDS_DA)
omniORB.registerType(PDS_DA._NP_RepositoryId, _0_CosPersistencePDS_DA._d_PDS_DA, _0_CosPersistencePDS_DA._tc_PDS_DA)

# PDS_DA operations and attributes
PDS_DA._d_get_data = ((), (omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObject:1.0"], ), None)
PDS_DA._d_set_data = ((omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObject:1.0"], ), (), None)
PDS_DA._d_lookup = ((omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObjectID:1.0"], ), (omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObject:1.0"], ), None)
PDS_DA._d_get_pid = ((), (omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/PID_DA:1.0"], ), None)
PDS_DA._d_get_object_pid = ((omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObject:1.0"], ), (omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/PID_DA:1.0"], ), None)
PDS_DA._d_data_factories = ((), (omniORB.typeMapping["IDL:omg.org/CosPersistencePDS_DA/DAObjectFactoryFinder:1.0"], ), None)

# PDS_DA object reference
class _objref_PDS_DA (_0_CosPersistencePDS._objref_PDS):
    _NP_RepositoryId = PDS_DA._NP_RepositoryId

    def __init__(self, obj):
        _0_CosPersistencePDS._objref_PDS.__init__(self, obj)

    def get_data(self, *args):
        return self._obj.invoke("get_data", _0_CosPersistencePDS_DA.PDS_DA._d_get_data, args)

    def set_data(self, *args):
        return self._obj.invoke("set_data", _0_CosPersistencePDS_DA.PDS_DA._d_set_data, args)

    def lookup(self, *args):
        return self._obj.invoke("lookup", _0_CosPersistencePDS_DA.PDS_DA._d_lookup, args)

    def get_pid(self, *args):
        return self._obj.invoke("get_pid", _0_CosPersistencePDS_DA.PDS_DA._d_get_pid, args)

    def get_object_pid(self, *args):
        return self._obj.invoke("get_object_pid", _0_CosPersistencePDS_DA.PDS_DA._d_get_object_pid, args)

    def data_factories(self, *args):
        return self._obj.invoke("data_factories", _0_CosPersistencePDS_DA.PDS_DA._d_data_factories, args)

omniORB.registerObjref(PDS_DA._NP_RepositoryId, _objref_PDS_DA)
_0_CosPersistencePDS_DA._objref_PDS_DA = _objref_PDS_DA
del PDS_DA, _objref_PDS_DA

# PDS_DA skeleton
__name__ = "CosPersistencePDS_DA__POA"
class PDS_DA (_0_CosPersistencePDS__POA.PDS):
    _NP_RepositoryId = _0_CosPersistencePDS_DA.PDS_DA._NP_RepositoryId


    _omni_op_d = {"get_data": _0_CosPersistencePDS_DA.PDS_DA._d_get_data, "set_data": _0_CosPersistencePDS_DA.PDS_DA._d_set_data, "lookup": _0_CosPersistencePDS_DA.PDS_DA._d_lookup, "get_pid": _0_CosPersistencePDS_DA.PDS_DA._d_get_pid, "get_object_pid": _0_CosPersistencePDS_DA.PDS_DA._d_get_object_pid, "data_factories": _0_CosPersistencePDS_DA.PDS_DA._d_data_factories}
    _omni_op_d.update(_0_CosPersistencePDS__POA.PDS._omni_op_d)

PDS_DA._omni_skeleton = PDS_DA
_0_CosPersistencePDS_DA__POA.PDS_DA = PDS_DA
omniORB.registerSkeleton(PDS_DA._NP_RepositoryId, PDS_DA)
del PDS_DA
__name__ = "CosPersistencePDS_DA"

#
# End of module "CosPersistencePDS_DA"
#
__name__ = "CosPersistencePDS_DA_idl"

_exported_modules = ( "CosPersistencePDS_DA", )

# The end.
