#
# alpha_osf1_4.0.mk - make variables and rules specific to Digital Unix
# (i.e. OSF1) 4.0.
#

OSF1 = 1
AlphaProcessor = 1


#
# Python set-up
#
# WARNING: Python's configure put a "#define inline __inline" into
# -------  /usr/local/include/python2.0/config.h!
#          This macro has to be suppressed!
#
# You must set a path to a Python 1.5.2 interpreter.

#PYTHON = /usr/local/bin/python


#
# Include general unix things
#

include $(THIS_IMPORT_TREE)/mk/unix.mk


#
# C preprocessor macro definitions for this architecture
#

IMPORT_CPPFLAGS += -D__alpha__ -D__osf1__ -D__OSVERSION__=4


#
# Standard programs
#

AR = ar clq

#
# Use DIGITAL C++ V6.1-029 on DIGITAL UNIX V4.0E (Rev. 1091)
#
DecCxxMajorVersion = 6
DecCxxMinorVersion = 1

CXX         = cxx
CXXOPTIONS  = -ptr $(TOP)/cxx_respository
#
# Let us allways generate thread save code (-pthread includes -D_REENTRANT)
#
CXXOPTIONS += -pthread
#
# For DEC C++ 6.0
# Uncommment the following line to speed up the compilation, but may require
# manually deleted some .pch and cxx_respository/TIMESTAMP files to pick
# up changes in templates or the order of -I flags.
#
# CXXOPTIONS += -ttimestamp -pch

CXXMAKEDEPEND += -D__DECCXX -D__cplusplus
CXXDEBUGFLAGS  = -O

CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) -call_shared

#
# Use DEC C V5.8-009 on Digital UNIX V4.0E (Rev. 1091)
#
CC           = cc
CMAKEDEPEND += -D__DECC
CDEBUGFLAGS  = -O
#
# The following is needed for src/tool/omkdepend/omkdepend to avoid SIGSEGV
#
COPTIONS     = -DNeedVarargsPrototypes
#
# Let us allways generate thread save code (-pthread includes -D_REENTRANT)
#
COPTIONS    += -pthread

CLINK        = $(CC)


#
# When specifying the "rpath" (directories which the run-time linker should
# search for shared libraries) we unfortunately need to do it in a single
# argument.  For this reason we override the default unix CXXExecutable and
# CExecutable rules.  Any -L flags given in $$libs results in another element
# being added to the rpath and we then give the whole rpath at the end of the
# link command line.
#

RPATH = $(subst $(space),:,$(strip $(IMPORT_LIBRARY_DIRS)))

define CXXExecutable
(rpath="$(RPATH)"; \
 for arg in $$libs; do \
   if expr "$$arg" : "-L" >/dev/null; then \
     rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
   fi; \
 done; \
 set -x; \
 $(RM) $@; \
 $(CXXLINK) -o $@ $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs -rpath $$rpath; \
)
endef

define CExecutable
(rpath="$(RPATH)"; \
 for arg in $$libs; do \
   if expr "$$arg" : "-L" >/dev/null; then \
     rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
   fi; \
 done; \
 set -x; \
 $(RM) $@; \
 $(CLINK) -o $@ $(CLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs -rpath $$rpath; \
)
endef


#
# CORBA stuff
#

CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#

ThreadSystem = Posix

OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 -DNoNanoSleep

OMNITHREAD_CPPFLAGS       = -pthread
#
# Note: -pthread includes -D_REENTRANT
#
# The pthread package before 4.0 was POSIX 1003.4a draft 4. If for some
# reason it is necessary to run the same binaries on 4.0 and older systems
# (e.g. 3.2), use the following make variables instead.
#
# OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=4 -DNoNanoSleep 
# OMNITHREAD_CPPFLAGS = -D_PTHREAD_USE_D4 -D_REENTRANT

OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread) \
		 -lpthread -lexc

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
# Platform specific customerisation.
# everything else is default from unix.mk
#

ifeq ($(notdir $(CXX)),cxx)

BuildSharedLibrary = 1       # Enable

SHAREDLIB_CPPFLAGS =

define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 set -x; \
 $(RM) $@; \
  ld -shared -soname $$soname -set_version $$soname -o $@ \
 $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibSuffixPattern),$^) $$extralibs \
  -lcxxstd -lcxx -lexc -lots -lc;
endef

ifeq ($(notdir $(CC)),gcc)
SHAREDLIB_CFLAGS = -fPIC
endif

endif
