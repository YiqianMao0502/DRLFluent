# dir.mk for omniORB.

GIOP_SRCS = \
            omniTransport.cc \
	    cdrStream.cc \
            cdrStreamAdapter.cc \
	    cdrMemoryStream.cc \
	    cdrValueChunkStream.cc \
            giopEndpoint.cc \
            giopRope.cc \
            giopStrand.cc \
            giopStream.cc \
            giopServer.cc \
            giopWorker.cc \
            giopRendezvouser.cc \
            GIOP_C.cc \
            GIOP_S.cc \
            giopStreamImpl.cc \
            giopImpl10.cc \
            giopImpl11.cc \
            giopImpl12.cc \
            giopBiDir.cc \
            giopMonitor.cc \
            SocketCollection.cc

TRANSPORT_SRCS = \
            tcpSocket.cc \
            tcpTransportImpl.cc \
            tcpConnection.cc \
            tcpEndpoint.cc \
            tcpAddress.cc \
            tcpActive.cc

UNIXSOCK_SRCS = \
            unixTransportImpl.cc \
            unixConnection.cc \
            unixEndpoint.cc \
            unixAddress.cc \
            unixActive.cc

SSL_SRCS = \
           sslActive.cc \
           sslAddress.cc \
           sslConnection.cc \
           sslEndpoint.cc \
           sslTransportImpl.cc \
           sslContext.cc

CODESET_SRCS = \
	    codeSets.cc \
	    cs-8bit.cc \
	    cs-16bit.cc \
	    cs-8859-1.cc \
	    cs-UTF-8.cc \
	    cs-UTF-16.cc \

BUILTIN_STUB_SRCS = \
	    bootstrapstub.cc \
	    objectStub.cc \
	    poastubs.cc 

ifdef vxWorksPlatform
ifndef vxNamesRequired
BUILTIN_STUB_SRCS += Namingstub.cc
endif
else 
BUILTIN_STUB_SRCS += Namingstub.cc
endif

ORB_SRCS =  \
	    anonObject.cc \
	    callDescriptor.cc \
	    constants.cc \
	    corbaObject.cc \
	    corbaBoa.cc \
            corbaOrb.cc \
            corbaFixed.cc \
	    corbaString.cc \
	    corbaWString.cc \
	    current.cc \
	    dynamicLib.cc \
	    exception.cc \
	    exceptn.cc \
	    initRefs.cc \
	    interceptors.cc \
            invoker.cc \
	    ior.cc \
	    libcWrapper.cc \
            omniIdentity.cc \
	    localIdentity.cc \
	    localObject.cc \
	    logIOstream.cc \
            minorCode.cc \
	    objectAdapter.cc \
	    omniInternal.cc \
	    omniIOR.cc \
	    omniObjRef.cc \
	    omniORB.cc \
	    omniServant.cc \
            orbOptions.cc \
            orbOptionsFile.cc \
	    poa.cc \
	    poamanager.cc \
	    policy.cc \
	    portableserver.cc \
	    proxyFactory.cc \
	    remoteIdentity.cc \
	    inProcessIdentity.cc \
            shutdownIdentity.cc \
            callHandle.cc \
	    tracedthread.cc \
            transportRules.cc \
	    rmutex.cc \
	    uri.cc \
            omniPolicy.cc \
            $(GIOP_SRCS) \
            $(CODESET_SRCS) \
            $(BUILTIN_STUB_SRCS) \
            $(TRANSPORT_SRCS)


DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(patsubst %,-I%/include/omniORB4/internal,$(IMPORT_TREES))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -DUSE_omniORB_logStream
DIR_CPPFLAGS += -D_OMNIORB_LIBRARY
DIR_CPPFLAGS += -DOMNIORB_VERSION_STRING='"$(OMNIORB_VERSION)"'
DIR_CPPFLAGS += -DOMNIORB_VERSION_HEX='$(OMNIORB_VERSION_HEX)'

##########################################################################
# Add magic to find tcp transport source files
CXXVPATH = $(VPATH):$(VPATH:%=%/tcp)

##########################################################################
# Build unix transport if this is a unix platform
ifdef UnixPlatform
  ORB_SRCS += $(UNIXSOCK_SRCS)
  CXXVPATH += $(VPATH:%=%/unix)
endif

##########################################################################
# Build SSL transport as part of core library if required
ifdef OPEN_SSL_ROOT
  ifdef SSL_TRANSPORT_IN_CORE
    ORB_SRCS += $(SSL_SRCS)
    CXXVPATH += $(VPATH:%=%/ssl)
    DIR_CPPFLAGS += -D_OMNIORB_SSL_LIBRARY
    DIR_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)
  endif
endif

##########################################################################
ifdef OMNIORB_CONFIG_DEFAULT_LOCATION
  CONFIG_DEFAULT_LOCATION = $(OMNIORB_CONFIG_DEFAULT_LOCATION)
else
  ifdef UnixPlatform
    CONFIG_DEFAULT_LOCATION = /project/omni/var/omniORB_NEW.cfg
  endif
  ifdef Win32Platform
    CONFIG_DEFAULT_LOCATION = C:\\OMNIORB.CFG
  endif
  ifdef vxWorksPlatform
    CONFIG_DEFAULT_LOCATION = /a2/tmp/omniORB.cfg
  endif
endif
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION='"$(CONFIG_DEFAULT_LOCATION)"'

##########################################################################
ifdef OMNIORB_CONFIG_ENV
  CONFIG_ENV = $(OMNIORB_CONFIG_ENV)
else
  CONFIG_ENV = OMNIORB_CONFIG
endif
DIR_CPPFLAGS += -DCONFIG_ENV='"$(CONFIG_ENV)"'

##########################################################################
ifdef UnixPlatform
#  CXXDEBUGFLAGS = -g
  DIR_CPPFLAGS += -DUnixArchitecture
  ifdef SunOS
    DIR_CPPFLAGS += -DBSD_COMP   # include BSD flags in ioctl.h
  endif
endif

ifdef Win32Platform
  DIR_CPPFLAGS += -D"NTArchitecture"
  EXTRA_LIBS    = $(SOCKET_LIB) $(patsubst %,$(LibNoDebugSearchPattern),advapi32)
  SHARED_ONLY_OBJS = msvcdllstub.o
  MSVC_STATICLIB_CXXNODEBUGFLAGS += -D_WINSTATIC
  MSVC_STATICLIB_CXXDEBUGFLAGS += -D_WINSTATIC
ifndef ETSKernel
  ORB_SRCS += orbOptionsReg.cc
endif
endif

##########################################################################
ifdef Cygwin
# there's a bug in gcc 3.2 (build 20020927) that makes gcc crash
# when optimizing this file ...
static/Namingstub.o: CXXDEBUGFLAGS = -O0
shared/Namingstub.o: CXXDEBUGFLAGS = -O0
endif


#########################################################################

ORB_OBJS      = $(ORB_SRCS:.cc=.o)
CXXSRCS       = $(ORB_SRCS)

vpath %.cc $(CXXVPATH)

LIB_NAME     := omniORB
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(ORB_OBJS)
LIB_IMPORTS  := $(OMNITHREAD_LIB) $(EXTRA_LIBS)
LIB_SHARED_ONLY_OBJS := $(SHARED_ONLY_OBJS)

include $(BASE_OMNI_TREE)/mk/mklib.mk

#########################################################################
ifdef Win32Platform

stublib = static/$(patsubst %,$(LibNoDebugPattern),msvcstub)

all:: $(stublib)

$(stublib): static/msvcdllstub.o
	@$(StaticLinkLibrary)

export:: $(stublib)
	@$(ExportLibrary)

clean::
	$(RM) $(stublib)

veryclean::
	$(RM) $(stublib)

stubdblib = debug/$(patsubst %,$(LibDebugPattern),msvcstub)

all:: $(stubdblib)

$(stubdblib): debug/msvcdllstub.o
	@$(StaticLinkLibrary)

export:: $(stubdblib)
	@$(ExportLibrary)

clean::
	$(RM) $(stubdblib)

veryclean::
	$(RM) $(stubdblib)

endif

#########################################################################
ifdef OPEN_SSL_ROOT
  ifndef SSL_TRANSPORT_IN_CORE

SUBDIRS += ssl

  endif
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif
