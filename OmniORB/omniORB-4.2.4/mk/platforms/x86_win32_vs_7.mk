#
# x86_win32_vc7.mk - make variables and rules specific to Visual Studio 7
#

WindowsNT = 1
x86Processor = 1

compiler_version_suffix=_vc7

WINVER = 0x0400

BINDIR = bin/x86_win32
LIBDIR = lib/x86_win32

ABSTOP = $(shell cd $(TOP); pwd)

#
# Python set-up
#
# You must set a path to a Python interpreter, either version 2.7 or
# version 3.5 or later.

#PYTHON = /cygdrive/c/Python27/python
#PYTHON = /cygdrive/c/Python36/python


# Use the following set of flags to build and use multithreaded DLLs
#
MSVC_DLL_CXXNODEBUGFLAGS       = -MD -EHs -GS -GR -Zi -nologo
MSVC_DLL_CXXLINKNODEBUGOPTIONS = -nologo -DEBUG
MSVC_DLL_CNODEBUGFLAGS         = -MD -GS -GR -Zi -nologo
MSVC_DLL_CLINKNODEBUGOPTIONS   = -nologo -DEBUG
#
MSVC_DLL_CXXDEBUGFLAGS         = -MDd -EHs -RTC1 -GS -GR -Zi -nologo
MSVC_DLL_CXXLINKDEBUGOPTIONS   = -nologo -DEBUG
MSVC_DLL_CDEBUGFLAGS           = -MDd -RTC1 -GS -GR -Zi -nologo
MSVC_DLL_CLINKDEBUGOPTIONS     = -nologo -DEBUG
#
# Or
#
# Use the following set of flags to build and use multithread static libraries
#
MSVC_STATICLIB_CXXNODEBUGFLAGS       = -MT -EHs -GS -GR -Zi -nologo
MSVC_STATICLIB_CXXLINKNODEBUGOPTIONS = -nologo -DEBUG
MSVC_STATICLIB_CNODEBUGFLAGS         = -MT -GS -GR -Zi -nologo
MSVC_STATICLIB_CLINKNODEBUGOPTIONS   = -nologo -DEBUG

MSVC_STATICLIB_CXXDEBUGFLAGS         = -MTd -EHs -RTC1 -GS -GR -Zi -nologo
MSVC_STATICLIB_CXXLINKDEBUGOPTIONS   = -nologo -DEBUG
MSVC_STATICLIB_CDEBUGFLAGS           = -MTd -RTC1 -GS -GR -Zi -nologo
MSVC_STATICLIB_CLINKDEBUGOPTIONS     = -nologo -DEBUG


ifdef BuildDebugBinary

CXXLINKOPTIONS = $(MSVC_DLL_CXXLINKDEBUGOPTIONS)
CXXDEBUGFLAGS  = 
CXXOPTIONS     = $(MSVC_DLL_CXXDEBUGFLAGS)
CLINKOPTIONS   = $(MSVC_DLL_CLINKDEBUGOPTIONS)
CDEBUGFLAGS    = $(MSVC_DLL_CDEBUGFLAGS)

else

CXXLINKOPTIONS = $(MSVC_DLL_CXXLINKNODEBUGOPTIONS)
CXXDEBUGFLAGS  = -O2
CXXOPTIONS     = $(MSVC_DLL_CXXNODEBUGFLAGS)
CLINKOPTIONS   = $(MSVC_DLL_CLINKNODEBUGOPTIONS)
CDEBUGFLAGS    = -O2
COPTIONS       = $(MSVC_DLL_CNODEBUGFLAGS)

endif

#
# Include general win32 things
#

include $(THIS_IMPORT_TREE)/mk/win32.mk


IMPORT_CPPFLAGS += -D__x86__ -D__NT__ -D__OSVERSION__=4


# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = C:\\temp


# Add the location of the Open SSL library

# To build the SSL transport, OPEN_SSL_ROOT must be defined and points to
# the top level directory of the openssl library. The default is to disable
# the build.
#
#OPEN_SSL_ROOT = /cygdrive/c/openssl
#

OPEN_SSL_CPPFLAGS = -I$(OPEN_SSL_ROOT)/include
OPEN_SSL_LIB = $(patsubst %,$(LibPathPattern),$(OPEN_SSL_ROOT)/lib) \
               ssleay32.lib libeay32.lib
OMNIORB_SSL_LIB += $(OPEN_SSL_LIB)
OMNIORB_SSL_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)
