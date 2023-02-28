#
# s390_linux_glibc2.1.mk - make variables and rules specific
# to Linux 2.x and glibc-2.1
#

Linux = 1
s390Processor = 1

ABSTOP = $(shell cd $(TOP); pwd)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter. If you do not
# wish to make a complete installation, you may download a minimal
# Python from ftp://ftp.uk.research.att.com/pub/omniORB/python/
# In that case, uncomment the first line below.

#PYTHON = $(ABSTOP)/$(BINDIR)/omnipython
#PYTHON = /usr/local/bin/python
PYTHON = /usr/bin/python

#
# Include general unix things
#

include $(THIS_IMPORT_TREE)/mk/unix.mk


#
# Standard programs
#

AR = ar cq

CPP = /usr/bin/cpp

#############################################################################
# To use g++ uncomment the following lines:                                 # 
#############################################################################
CXX = /usr/bin/g++
CXXMAKEDEPEND += -D__cplusplus -D__GNUG__ -D__GNUC__
CXXDEBUGFLAGS = -O2 

CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) \
		$(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

CXXOPTIONS      = -Wall -Wno-unused
EgcsMajorVersion = 1
EgcsMinorVersion = 1

#############################################################################
# To use KAI C++ uncomment the following lines:                             #
#############################################################################
#KCC = 1
#AR = KCC --thread_safe -o 
#CXX = /usr/local/KAI/KCC.pu-4.0b-1/KCC_BASE/bin/KCC
#CXXMAKEDEPEND += -D__cplusplus -D__GNUG__ -D__GNUC__
#CXXDEBUGFLAGS = +K0 --one_per --thread_safe --exceptions
#
#CXXLINK		= $(CXX)
#CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) --thread_safe 
#CXXOPTIONS      =

#############################################################################
CC           = /usr/bin/gcc
CMAKEDEPEND  += -D__GNUC__
CDEBUGFLAGS  = -O

CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS) \
	       $(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

INSTALL = install -c

IMPORT_CPPFLAGS += -D__s390__ -D__linux__ -D__OSVERSION__=2


#
# CORBA stuff
#

CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#

ThreadSystem = Posix
OMNITHREAD_POSIX_CPPFLAGS = -DNoNanoSleep
OMNITHREAD_CPPFLAGS = -D_REENTRANT
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)

ifndef UseMITthreads
OMNITHREAD_POSIX_CPPFLAGS += -DPthreadDraftVersion=10
OMNITHREAD_LIB += -lpthread
else
OMNITHREAD_POSIX_CPPFLAGS += -DPthreadDraftVersion=8 
OMNITHREAD_CPPFLAGS += -D_MIT_POSIX_THREADS
OMNITHREAD_LIB += -lpthreads
endif

lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)


# Default location of the omniORB configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :

OMNIORB_CONFIG_DEFAULT_LOCATION = /etc/omniORB4.cfg

# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = /var/omninames

#
# Shared Library support.     
#
BuildSharedLibrary = 1       # Enable
SHAREDLIB_CPPFLAGS = -fPIC   # compiler flag

ifeq ($(notdir $(CC)),KCC)
SharedLibraryPlatformLinkFlagsTemplate = --thread_safe --soname $$soname
endif

# Add the location of the Open SSL library

# To build the SSL transport, OPEN_SSL_ROOT must be defined and points to
# the top level directory of the openssl library. The default is to disable
# the build.
#
OPEN_SSL_ROOT = /usr
#

OPEN_SSL_CPPFLAGS = -I$(OPEN_SSL_ROOT)/include
OPEN_SSL_LIB = -L$(OPEN_SSL_ROOT)/lib -lssl -lcrypto
OMNIORB_SSL_LIB += $(OPEN_SSL_LIB)
OMNIORB_SSL_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)

#
# everything else is default from unix.mk

