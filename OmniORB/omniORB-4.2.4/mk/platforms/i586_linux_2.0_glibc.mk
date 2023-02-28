#
# i586_linux_2.0_glibc.mk - make variables and rules specific to Linux 2.0
#                           and glibc-2.0 
#

Linux = 1
x86Processor = 1

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


#
# Include general unix things
#

include $(THIS_IMPORT_TREE)/mk/unix.mk


#
# Standard programs
#

AR = ar cq

CPP = /usr/bin/cpp

CXX = /usr/local/egcs/bin/g++
CXXMAKEDEPEND += -D__cplusplus -D__GNUG__ -D__GNUC__
CXXDEBUGFLAGS = -O2 

CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) \
		$(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

CXXOPTIONS      = -Wall -Wno-unused
EgcsMajorVersion = 1
EgcsMinorVersion = 1        # This is actually a post-1.0 egcs snapshot.

CC           = /usr/local/egcs/bin/gcc
CMAKEDEPEND  += -D__GNUC__
CDEBUGFLAGS  = -O

CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS) \
	       $(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

INSTALL = install -c

IMPORT_CPPFLAGS += -D__x86__ -D__linux__ -D__OSVERSION__=2


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

OMNIORB_CONFIG_DEFAULT_LOCATION = /etc/omniORB.cfg

# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = /var/omninames

#
# Shared Library support.     
#
BuildSharedLibrary = 1       # Enable
SHAREDLIB_CPPFLAGS = -fPIC   # compiler flag
#
# everything else is default from unix.mk

