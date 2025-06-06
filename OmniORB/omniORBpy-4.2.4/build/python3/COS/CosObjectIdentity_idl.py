# Python stubs generated by omniidl from /mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst/share/idl/omniORB/COS/CosObjectIdentity.idl
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


#
# Start of module "CosObjectIdentity"
#
__name__ = "CosObjectIdentity"
_0_CosObjectIdentity = omniORB.openModule("CosObjectIdentity", r"/mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst/share/idl/omniORB/COS/CosObjectIdentity.idl")
_0_CosObjectIdentity__POA = omniORB.openModule("CosObjectIdentity__POA", r"/mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst/share/idl/omniORB/COS/CosObjectIdentity.idl")


# typedef ... ObjectIdentifier
class ObjectIdentifier:
    _NP_RepositoryId = "IDL:omg.org/CosObjectIdentity/ObjectIdentifier:1.0"
    def __init__(self, *args, **kw):
        raise RuntimeError("Cannot construct objects of this type.")
_0_CosObjectIdentity.ObjectIdentifier = ObjectIdentifier
_0_CosObjectIdentity._d_ObjectIdentifier  = omniORB.tcInternal.tv_ulong
_0_CosObjectIdentity._ad_ObjectIdentifier = (omniORB.tcInternal.tv_alias, ObjectIdentifier._NP_RepositoryId, "ObjectIdentifier", omniORB.tcInternal.tv_ulong)
_0_CosObjectIdentity._tc_ObjectIdentifier = omniORB.tcInternal.createTypeCode(_0_CosObjectIdentity._ad_ObjectIdentifier)
omniORB.registerType(ObjectIdentifier._NP_RepositoryId, _0_CosObjectIdentity._ad_ObjectIdentifier, _0_CosObjectIdentity._tc_ObjectIdentifier)
del ObjectIdentifier

# interface IdentifiableObject
_0_CosObjectIdentity._d_IdentifiableObject = (omniORB.tcInternal.tv_objref, "IDL:omg.org/CosObjectIdentity/IdentifiableObject:1.0", "IdentifiableObject")
omniORB.typeMapping["IDL:omg.org/CosObjectIdentity/IdentifiableObject:1.0"] = _0_CosObjectIdentity._d_IdentifiableObject
_0_CosObjectIdentity.IdentifiableObject = omniORB.newEmptyClass()
class IdentifiableObject :
    _NP_RepositoryId = _0_CosObjectIdentity._d_IdentifiableObject[1]

    def __init__(self, *args, **kw):
        raise RuntimeError("Cannot construct objects of this type.")

    _nil = CORBA.Object._nil


_0_CosObjectIdentity.IdentifiableObject = IdentifiableObject
_0_CosObjectIdentity._tc_IdentifiableObject = omniORB.tcInternal.createTypeCode(_0_CosObjectIdentity._d_IdentifiableObject)
omniORB.registerType(IdentifiableObject._NP_RepositoryId, _0_CosObjectIdentity._d_IdentifiableObject, _0_CosObjectIdentity._tc_IdentifiableObject)

# IdentifiableObject operations and attributes
IdentifiableObject._d__get_constant_random_id = ((),(omniORB.typeMapping["IDL:omg.org/CosObjectIdentity/ObjectIdentifier:1.0"],),None)
IdentifiableObject._d_is_identical = ((omniORB.typeMapping["IDL:omg.org/CosObjectIdentity/IdentifiableObject:1.0"], ), (omniORB.tcInternal.tv_boolean, ), None)

# IdentifiableObject object reference
class _objref_IdentifiableObject (CORBA.Object):
    _NP_RepositoryId = IdentifiableObject._NP_RepositoryId

    def __init__(self, obj):
        CORBA.Object.__init__(self, obj)

    def _get_constant_random_id(self, *args):
        return self._obj.invoke("_get_constant_random_id", _0_CosObjectIdentity.IdentifiableObject._d__get_constant_random_id, args)

    constant_random_id = property(_get_constant_random_id)


    def is_identical(self, *args):
        return self._obj.invoke("is_identical", _0_CosObjectIdentity.IdentifiableObject._d_is_identical, args)

omniORB.registerObjref(IdentifiableObject._NP_RepositoryId, _objref_IdentifiableObject)
_0_CosObjectIdentity._objref_IdentifiableObject = _objref_IdentifiableObject
del IdentifiableObject, _objref_IdentifiableObject

# IdentifiableObject skeleton
__name__ = "CosObjectIdentity__POA"
class IdentifiableObject (PortableServer.Servant):
    _NP_RepositoryId = _0_CosObjectIdentity.IdentifiableObject._NP_RepositoryId


    _omni_op_d = {"_get_constant_random_id": _0_CosObjectIdentity.IdentifiableObject._d__get_constant_random_id, "is_identical": _0_CosObjectIdentity.IdentifiableObject._d_is_identical}

IdentifiableObject._omni_skeleton = IdentifiableObject
_0_CosObjectIdentity__POA.IdentifiableObject = IdentifiableObject
omniORB.registerSkeleton(IdentifiableObject._NP_RepositoryId, IdentifiableObject)
del IdentifiableObject
__name__ = "CosObjectIdentity"

#
# End of module "CosObjectIdentity"
#
__name__ = "CosObjectIdentity_idl"

_exported_modules = ( "CosObjectIdentity", )

# The end.
