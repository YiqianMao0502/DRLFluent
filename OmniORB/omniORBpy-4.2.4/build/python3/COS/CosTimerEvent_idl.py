# Python stubs generated by omniidl from /mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst/share/idl/omniORB/COS/CosTimerEvent.idl
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


# #include "TimeBase.idl"
import TimeBase_idl
_0_TimeBase = omniORB.openModule("TimeBase")
_0_TimeBase__POA = omniORB.openModule("TimeBase__POA")

# #include "CosTime.idl"
import CosTime_idl
_0_CosTime = omniORB.openModule("CosTime")
_0_CosTime__POA = omniORB.openModule("CosTime__POA")

# #include "CosEventComm.idl"
import CosEventComm_idl
_0_CosEventComm = omniORB.openModule("CosEventComm")
_0_CosEventComm__POA = omniORB.openModule("CosEventComm__POA")

#
# Start of module "CosTimerEvent"
#
__name__ = "CosTimerEvent"
_0_CosTimerEvent = omniORB.openModule("CosTimerEvent", r"/mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst/share/idl/omniORB/COS/CosTimerEvent.idl")
_0_CosTimerEvent__POA = omniORB.openModule("CosTimerEvent__POA", r"/mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst/share/idl/omniORB/COS/CosTimerEvent.idl")


# enum TimeType
_0_CosTimerEvent.TTAbsolute = omniORB.EnumItem("TTAbsolute", 0)
_0_CosTimerEvent.TTRelative = omniORB.EnumItem("TTRelative", 1)
_0_CosTimerEvent.TTPeriodic = omniORB.EnumItem("TTPeriodic", 2)
_0_CosTimerEvent.TimeType = omniORB.Enum("IDL:omg.org/CosTimerEvent/TimeType:1.0", (_0_CosTimerEvent.TTAbsolute, _0_CosTimerEvent.TTRelative, _0_CosTimerEvent.TTPeriodic,))

_0_CosTimerEvent._d_TimeType  = (omniORB.tcInternal.tv_enum, _0_CosTimerEvent.TimeType._NP_RepositoryId, "TimeType", _0_CosTimerEvent.TimeType._items)
_0_CosTimerEvent._tc_TimeType = omniORB.tcInternal.createTypeCode(_0_CosTimerEvent._d_TimeType)
omniORB.registerType(_0_CosTimerEvent.TimeType._NP_RepositoryId, _0_CosTimerEvent._d_TimeType, _0_CosTimerEvent._tc_TimeType)

# enum EventStatus
_0_CosTimerEvent.ESTimeSet = omniORB.EnumItem("ESTimeSet", 0)
_0_CosTimerEvent.ESTimeCleared = omniORB.EnumItem("ESTimeCleared", 1)
_0_CosTimerEvent.ESTriggered = omniORB.EnumItem("ESTriggered", 2)
_0_CosTimerEvent.ESFailedTrigger = omniORB.EnumItem("ESFailedTrigger", 3)
_0_CosTimerEvent.EventStatus = omniORB.Enum("IDL:omg.org/CosTimerEvent/EventStatus:1.0", (_0_CosTimerEvent.ESTimeSet, _0_CosTimerEvent.ESTimeCleared, _0_CosTimerEvent.ESTriggered, _0_CosTimerEvent.ESFailedTrigger,))

_0_CosTimerEvent._d_EventStatus  = (omniORB.tcInternal.tv_enum, _0_CosTimerEvent.EventStatus._NP_RepositoryId, "EventStatus", _0_CosTimerEvent.EventStatus._items)
_0_CosTimerEvent._tc_EventStatus = omniORB.tcInternal.createTypeCode(_0_CosTimerEvent._d_EventStatus)
omniORB.registerType(_0_CosTimerEvent.EventStatus._NP_RepositoryId, _0_CosTimerEvent._d_EventStatus, _0_CosTimerEvent._tc_EventStatus)

# struct TimerEventT
_0_CosTimerEvent.TimerEventT = omniORB.newEmptyClass()
class TimerEventT (omniORB.StructBase):
    _NP_RepositoryId = "IDL:omg.org/CosTimerEvent/TimerEventT:1.0"

    def __init__(self, utc, event_data):
        self.utc = utc
        self.event_data = event_data

_0_CosTimerEvent.TimerEventT = TimerEventT
_0_CosTimerEvent._d_TimerEventT  = (omniORB.tcInternal.tv_struct, TimerEventT, TimerEventT._NP_RepositoryId, "TimerEventT", "utc", omniORB.typeMapping["IDL:omg.org/TimeBase/UtcT:1.0"], "event_data", omniORB.tcInternal.tv_any)
_0_CosTimerEvent._tc_TimerEventT = omniORB.tcInternal.createTypeCode(_0_CosTimerEvent._d_TimerEventT)
omniORB.registerType(TimerEventT._NP_RepositoryId, _0_CosTimerEvent._d_TimerEventT, _0_CosTimerEvent._tc_TimerEventT)
del TimerEventT

# interface TimerEventHandler
_0_CosTimerEvent._d_TimerEventHandler = (omniORB.tcInternal.tv_objref, "IDL:omg.org/CosTimerEvent/TimerEventHandler:1.0", "TimerEventHandler")
omniORB.typeMapping["IDL:omg.org/CosTimerEvent/TimerEventHandler:1.0"] = _0_CosTimerEvent._d_TimerEventHandler
_0_CosTimerEvent.TimerEventHandler = omniORB.newEmptyClass()
class TimerEventHandler :
    _NP_RepositoryId = _0_CosTimerEvent._d_TimerEventHandler[1]

    def __init__(self, *args, **kw):
        raise RuntimeError("Cannot construct objects of this type.")

    _nil = CORBA.Object._nil


_0_CosTimerEvent.TimerEventHandler = TimerEventHandler
_0_CosTimerEvent._tc_TimerEventHandler = omniORB.tcInternal.createTypeCode(_0_CosTimerEvent._d_TimerEventHandler)
omniORB.registerType(TimerEventHandler._NP_RepositoryId, _0_CosTimerEvent._d_TimerEventHandler, _0_CosTimerEvent._tc_TimerEventHandler)

# TimerEventHandler operations and attributes
TimerEventHandler._d__get_status = ((),(omniORB.typeMapping["IDL:omg.org/CosTimerEvent/EventStatus:1.0"],),None)
TimerEventHandler._d_time_set = ((), (omniORB.tcInternal.tv_boolean, omniORB.typeMapping["IDL:omg.org/CosTime/UTO:1.0"]), None)
TimerEventHandler._d_SetTimer = ((omniORB.typeMapping["IDL:omg.org/CosTimerEvent/TimeType:1.0"], omniORB.typeMapping["IDL:omg.org/CosTime/UTO:1.0"]), (), None)
TimerEventHandler._d_cancel_timer = ((), (omniORB.tcInternal.tv_boolean, ), None)
TimerEventHandler._d_set_data = ((omniORB.tcInternal.tv_any, ), (), None)

# TimerEventHandler object reference
class _objref_TimerEventHandler (CORBA.Object):
    _NP_RepositoryId = TimerEventHandler._NP_RepositoryId

    def __init__(self, obj):
        CORBA.Object.__init__(self, obj)

    def _get_status(self, *args):
        return self._obj.invoke("_get_status", _0_CosTimerEvent.TimerEventHandler._d__get_status, args)

    status = property(_get_status)


    def time_set(self, *args):
        return self._obj.invoke("time_set", _0_CosTimerEvent.TimerEventHandler._d_time_set, args)

    def SetTimer(self, *args):
        return self._obj.invoke("SetTimer", _0_CosTimerEvent.TimerEventHandler._d_SetTimer, args)

    def cancel_timer(self, *args):
        return self._obj.invoke("cancel_timer", _0_CosTimerEvent.TimerEventHandler._d_cancel_timer, args)

    def set_data(self, *args):
        return self._obj.invoke("set_data", _0_CosTimerEvent.TimerEventHandler._d_set_data, args)

omniORB.registerObjref(TimerEventHandler._NP_RepositoryId, _objref_TimerEventHandler)
_0_CosTimerEvent._objref_TimerEventHandler = _objref_TimerEventHandler
del TimerEventHandler, _objref_TimerEventHandler

# TimerEventHandler skeleton
__name__ = "CosTimerEvent__POA"
class TimerEventHandler (PortableServer.Servant):
    _NP_RepositoryId = _0_CosTimerEvent.TimerEventHandler._NP_RepositoryId


    _omni_op_d = {"_get_status": _0_CosTimerEvent.TimerEventHandler._d__get_status, "time_set": _0_CosTimerEvent.TimerEventHandler._d_time_set, "SetTimer": _0_CosTimerEvent.TimerEventHandler._d_SetTimer, "cancel_timer": _0_CosTimerEvent.TimerEventHandler._d_cancel_timer, "set_data": _0_CosTimerEvent.TimerEventHandler._d_set_data}

TimerEventHandler._omni_skeleton = TimerEventHandler
_0_CosTimerEvent__POA.TimerEventHandler = TimerEventHandler
omniORB.registerSkeleton(TimerEventHandler._NP_RepositoryId, TimerEventHandler)
del TimerEventHandler
__name__ = "CosTimerEvent"

# interface TimerEventService
_0_CosTimerEvent._d_TimerEventService = (omniORB.tcInternal.tv_objref, "IDL:omg.org/CosTimerEvent/TimerEventService:1.0", "TimerEventService")
omniORB.typeMapping["IDL:omg.org/CosTimerEvent/TimerEventService:1.0"] = _0_CosTimerEvent._d_TimerEventService
_0_CosTimerEvent.TimerEventService = omniORB.newEmptyClass()
class TimerEventService :
    _NP_RepositoryId = _0_CosTimerEvent._d_TimerEventService[1]

    def __init__(self, *args, **kw):
        raise RuntimeError("Cannot construct objects of this type.")

    _nil = CORBA.Object._nil


_0_CosTimerEvent.TimerEventService = TimerEventService
_0_CosTimerEvent._tc_TimerEventService = omniORB.tcInternal.createTypeCode(_0_CosTimerEvent._d_TimerEventService)
omniORB.registerType(TimerEventService._NP_RepositoryId, _0_CosTimerEvent._d_TimerEventService, _0_CosTimerEvent._tc_TimerEventService)

# TimerEventService operations and attributes
TimerEventService._d_register = ((omniORB.typeMapping["IDL:omg.org/CosEventComm/PushConsumer:1.0"], omniORB.tcInternal.tv_any), (omniORB.typeMapping["IDL:omg.org/CosTimerEvent/TimerEventHandler:1.0"], ), None)
TimerEventService._d_unregister = ((omniORB.typeMapping["IDL:omg.org/CosTimerEvent/TimerEventHandler:1.0"], ), (), None)
TimerEventService._d_event_time = ((omniORB.typeMapping["IDL:omg.org/CosTimerEvent/TimerEventT:1.0"], ), (omniORB.typeMapping["IDL:omg.org/CosTime/UTO:1.0"], ), None)

# TimerEventService object reference
class _objref_TimerEventService (CORBA.Object):
    _NP_RepositoryId = TimerEventService._NP_RepositoryId

    def __init__(self, obj):
        CORBA.Object.__init__(self, obj)

    def register(self, *args):
        return self._obj.invoke("register", _0_CosTimerEvent.TimerEventService._d_register, args)

    def unregister(self, *args):
        return self._obj.invoke("unregister", _0_CosTimerEvent.TimerEventService._d_unregister, args)

    def event_time(self, *args):
        return self._obj.invoke("event_time", _0_CosTimerEvent.TimerEventService._d_event_time, args)

omniORB.registerObjref(TimerEventService._NP_RepositoryId, _objref_TimerEventService)
_0_CosTimerEvent._objref_TimerEventService = _objref_TimerEventService
del TimerEventService, _objref_TimerEventService

# TimerEventService skeleton
__name__ = "CosTimerEvent__POA"
class TimerEventService (PortableServer.Servant):
    _NP_RepositoryId = _0_CosTimerEvent.TimerEventService._NP_RepositoryId


    _omni_op_d = {"register": _0_CosTimerEvent.TimerEventService._d_register, "unregister": _0_CosTimerEvent.TimerEventService._d_unregister, "event_time": _0_CosTimerEvent.TimerEventService._d_event_time}

TimerEventService._omni_skeleton = TimerEventService
_0_CosTimerEvent__POA.TimerEventService = TimerEventService
omniORB.registerSkeleton(TimerEventService._NP_RepositoryId, TimerEventService)
del TimerEventService
__name__ = "CosTimerEvent"

#
# End of module "CosTimerEvent"
#
__name__ = "CosTimerEvent_idl"

_exported_modules = ( "CosTimerEvent", )

# The end.
