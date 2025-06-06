# -*- Makefile -*-
#
# Make rule template for autoconf builds, sourced before dir.mk
#
# Based upon beforedir.mk and platform makefiles

platform         = autoconf
OMNIORB_ROOT     = /mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst
DATADIR          = ${prefix}/share
MAKEFILE_INC_DIR = $(BASE_OMNI_TREE)/$(CURRENT)/
IMPORT_TREES     = $(TOP) $(BASE_OMNI_TREE) $(OMNIORB_ROOT)
THIS_IMPORT_TREE = $(TOP)
EXPORT_TREE      = $(TOP)
PYTHON           = /mnt/iusers01/mace01/m83358ym/.conda/envs/py375/bin/python3.7

# If the build tree is the same as the source tree, autoconf helpfully
# clears any makefile lines that set VPATH. We put it back here.
ifndef VPATH
VPATH=.
endif

#############################################################################
#
# Standard directories in import/export trees
#

INCDIR  = include
IDLDIR  = idl
MAKEDIR = mk
LIBDIR  = lib
BINDIR  = bin

ifeq ($(OMNIORB_ROOT),)
OMNIORB_BINDIR = $(TOP)/$(BINDIR)
else
OMNIORB_BINDIR = $(OMNIORB_ROOT)/$(BINDIR)
endif


#############################################################################
#
# Directories for installation
#

prefix        	 := /mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omniORB-4.2.4/src/lib
exec_prefix   	 := ${prefix}
INSTALLTARGET 	 := 1
INSTALLINCDIR 	 := $(DESTDIR)${prefix}/include
INSTALLBINDIR 	 := $(DESTDIR)${exec_prefix}/bin
INSTALLLIBDIR 	 := $(DESTDIR)${exec_prefix}/lib
INSTALLPYTHONDIR := $(DESTDIR)${prefix}/lib/python3.7/site-packages
INSTALLPYEXECDIR := $(DESTDIR)${exec_prefix}/lib/python3.7/site-packages
INSTALLIDLDIR    := $(DESTDIR)${prefix}/share/idl

#############################################################################
#
# Tool bindir to use depends on make target
#
ifeq ($(MAKECMDGOALS),install)
  TOOLBINDIR = $(INSTALLBINDIR)
else
  TOOLBINDIR = $(TOP)/$(BINDIR)
endif


#############################################################################
#
# These definitions are useful for referring to spaces and commas inside
# GNU make functions
#

empty :=
space := $(empty) $(empty)
comma := ,


#############################################################################
#
# DIR_CPPFLAGS can be defined in dir.mk.
# IMPORT_CPPFLAGS can have platform-independent -D and -I flags added by each
# import tree using +=.  Note that here we already put a -I flag for each
# directory in VPATH and each import tree's include directory.
#
# CXXDEBUGFLAGS and CDEBUGFLAGS are for setting debug/optimisation options to
# the C++ and C compilers respectively.  CXXOPTIONS and COPTIONS are for
# setting any other options to the compilers.
#
# There is nothing magic about these variables, but hopefully this covers
# most of the things people will want to set in other import trees and dir.mk.
#

IMPORT_CPPFLAGS += -I. $(patsubst %,-I%,$(VPATH)) \
                   -I$(TOP)/include -I$(BASE_OMNI_TREE)/include \
                   -D__OSVERSION__=2

CPPFLAGS =  $(DIR_CPPFLAGS) $(IMPORT_CPPFLAGS)

CFLAGS = $(CDEBUGFLAGS) $(COPTIONS) $(CPPFLAGS)

CXXFLAGS = $(CXXDEBUGFLAGS) $(CXXOPTIONS) $(CPPFLAGS)


#############################################################################
#
# GENERATE_LIB_DEPEND is a variable which behaves more like a "function",
# taking a single argument, the current value of $(lib_depend).  What it does
# is search through the IMPORT_LIBRARY_DIRS for the library specified in
# lib_depend. It is used with ":=" (simply-expanded variables) like this:
#
# lib_depend := libwobble.a
# WOBBLE_LIB_DEPEND := $(GENERATE_LIB_DEPEND)
#
# $(WOBBLE_LIB_DEPEND) can now be specified as one of the dependencies of an
# executable which uses "libwobble.a", so that the executable will be rebuilt
# whenever libwobble.a changes.
#

IMPORT_LIBRARY_DIRS = $(patsubst %,%/$(LIBDIR),$(IMPORT_TREES))

GENERATE_LIB_DEPEND = $(firstword \
   $(foreach dir,$(IMPORT_LIBRARY_DIRS),$(wildcard $(dir)/$(lib_depend))))


#############################################################################
#
# Phony targets
#

.PHONY: all export install clean veryclean redepend lastveryclean


#############################################################################
#
# MakeSubdirs is a general rule which runs make in each of SUBDIRS.
# Unfortunately we have to unset MAKEFLAGS otherwise the -I flags which
# make passed to this make will be incorrectly passed down to the sub-make.
#

define MakeSubdirs
(unset MAKEFLAGS; \
 set -e; \
 if [ "$$subdir_makeflags" = "" ]; then \
   subdir_makeflags='$(SUBDIR_MAKEFLAGS)'; \
 fi; \
 if [ "$$subdirs" = "" ]; then \
   subdirs='$(SUBDIRS)'; \
 fi; \
 if [ "$$target" = "" ]; then \
   target='$@'; \
 fi; \
 for dir in $$subdirs ; do \
   $(CreateDir); \
   (cd $$dir ; echo "making $$target in $(CURRENT)/$$dir..." ; \
    eval $(MAKE) $$subdir_makeflags $$target ) ; \
   if [ $$? != 0 ]; then \
     exit 1; \
   fi; \
 done; \
)
endef

# Stop SUBDIRS specified on the command line being passed down.
unexport SUBDIRS


#############################################################################
#
# Useful bits of shell script.  Most take arguments as shell variables which
# can be set before putting them in your make rule.
#

#
# Create directory $$dir if it doesn't already exist.
#

define CreateDir
if [ ! -d $$dir ]; then \
   (umask 022; set -x; $(MKDIRHIER) $$dir); \
fi
endef


#
# Find $$file in $$dirs.  Returns full file name in $$fullfile.
#

define FindFileInDirs
case "$$file" in \
/*) fullfile="$$file";; \
*) \
  fullfile=""; \
  for _dir in $$dirs; do \
    if [ -f $$_dir/$$file ]; then \
      if [ "$$_dir" = "." ]; then \
        fullfile="$$file"; \
      else \
        fullfile="$$_dir/$$file"; \
      fi; \
      break; \
    fi; \
  done; \
  if [ ! "$$fullfile" ]; then \
    echo "ERROR: Cannot find $$file in $$dirs"; \
    exit 1; \
  fi;; \
esac
endef


#
# Find $$file in current directory or $(VPATH) - returns $$fullfile.
#

define FindFileInVpath
dirs='. $(VPATH)'; \
$(FindFileInDirs)
endef


#
# "Export" $$file to $$dir, creating $$dir if necessary.  Searches for
# $$file in $(VPATH) if not found in current directory.
#

define ExportFileToDir
$(CreateDir); \
$(FindFileInVpath); \
base=`basename $$file`; \
if [ -f $$dir/$$base ] && cmp $$fullfile $$dir/$$base >/dev/null; then \
  echo "File $$base hasn't changed."; \
else \
  (set -x; \
   $(INSTALL) $(INSTLIBFLAGS) $$fullfile $$dir); \
fi
endef


#
# "Export" an executable file.  Same as previous one but adds execute
# permission.
#

define ExportExecutableFileToDir
$(CreateDir); \
$(FindFileInVpath); \
base=`basename $$file`; \
if [ -f $$dir/$$base ] && cmp $$fullfile $$dir/$$base >/dev/null; then \
  echo "File $$base hasn't changed."; \
else \
  (set -x; \
   $(INSTALL) $(INSTEXEFLAGS) $$fullfile $$dir); \
fi
endef


#############################################################################
#
# CORBA stuff
#

include $(BASE_OMNI_TREE)/mk/version.mk

# It is now possible to compile interfaces and stubs that depend on
# idl from import trees without generating headers and stubs for those
# imported idls locally.  This is useful when the import tree supplies
# the headers and stubs itself, usually as part of some library.
#
# To arrange this, set DIR_IDLFLAGS and DIR_STUBS_CPPFLAGS in your
# dir.mk to the appropriate include flags.  Usually, these will just
# be the ones in IMPORT_IDLFLAGS and IMPORT_CPPFLAGS defined here, so
#
#   DIR_IDLFLAGS = $(IMPORT_IDLFLAGS)
#   DIR_STUBS_CPPFLAGS = $(IMPORT_CPPFLAGS)
#
# is all you need.  This would be the default if it weren't for the
# need to preserve the old (idl-copying) behaviour for existing dir.mk
# files.

vpath %.idl $(IMPORT_TREES:%=%/idl) \
            $(OMNIORB_ROOT)/share/idl/omniORB $(DATADIR)/idl/omniORB

IMPORT_IDLFLAGS += -I. $(patsubst %,-I%,$(VPATH)) \
		       $(patsubst %,-I%/idl,$(IMPORT_TREES))

CORBA_IDL_FILES = $(CORBA_INTERFACES:%=%.idl)

CORBA_STUB_DIR = $(TOP)/stub

CorbaImplementation = OMNIORB

CORBA_IDL		= $($(CorbaImplementation)_IDL)
CORBA_CPPFLAGS		= $($(CorbaImplementation)_CPPFLAGS)
CORBA_LIB		= $($(CorbaImplementation)_LIB)
CORBA_LIB_DEPEND	= $($(CorbaImplementation)_LIB_DEPEND)
CORBA_LIB_NODYN		= $($(CorbaImplementation)_LIB_NODYN)
CORBA_LIB_NODYN_DEPEND	= $($(CorbaImplementation)_LIB_NODYN_DEPEND)
CORBA_IDL_OUTPUTDIR_PATTERN = $($(CorbaImplementation)_IDL_OUTPUTDIR_PATTERN)

CORBA_STUB_HDR_PATTERN	= $($(CorbaImplementation)_STUB_HDR_PATTERN)
CORBA_STUB_SRC_PATTERN	= $($(CorbaImplementation)_STUB_SRC_PATTERN)
CORBA_STUB_OBJ_PATTERN	= $($(CorbaImplementation)_STUB_OBJ_PATTERN)
CORBA_DYN_STUB_SRC_PATTERN = $($(CorbaImplementation)_DYN_STUB_SRC_PATTERN)
CORBA_DYN_STUB_OBJ_PATTERN = $($(CorbaImplementation)_DYN_STUB_OBJ_PATTERN)

CORBA_STUB_HDRS		= \
	$(CORBA_INTERFACES:%=$(CORBA_STUB_HDR_PATTERN))
CORBA_STUB_SRCS		= $($(CorbaImplementation)_STUB_SRCS)
CORBA_STUB_OBJS		= $($(CorbaImplementation)_STUB_OBJS)
CORBA_STATIC_STUB_SRCS	= $($(CorbaImplementation)_STATIC_STUB_SRCS)
CORBA_STATIC_STUB_OBJS	= $($(CorbaImplementation)_STATIC_STUB_OBJS)
CORBA_DYN_STUB_SRCS	= $($(CorbaImplementation)_DYN_STUB_SRCS)
CORBA_DYN_STUB_OBJS	= $($(CorbaImplementation)_DYN_STUB_OBJS)

CORBA_STUB_FILES = $(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%.idl) \
		   $(CORBA_STUB_HDRS) $(CORBA_STUB_SRCS) $(CORBA_STUB_OBJS) \
		   $(CORBA_STUB_OBJS:.o=.d) $(CORBA_STUB_DIR)/dir.mk \
		   $($(CorbaImplementation)_EXTRA_STUB_FILES)

GENERATED_CXX_HDRS += $(CORBA_STUB_HDRS)


#
# Standard make variables and rules for all UNIX platforms. Override
# later if necessary.
#

UnixPlatform = 1
ThreadSystem = Posix

#
# General rules for cleaning.
#

define CleanRule
$(RM) *.o *.a 
endef

define VeryCleanRule
$(RM) *.d
$(RM) *.pyc
$(RM) -r $(CORBA_STUB_FILES)
endef


#
# Patterns for various file types
#
LibPathPattern    = -L%
LibNoDebugPattern = lib%.a
LibDebugPattern = lib%.a
LibPattern = lib%.a
LibSuffixPattern = %.a
LibSearchPattern = -l%
BinPattern = %
TclScriptPattern = %


#
# Stuff to generate statically-linked libraries.
#

define StaticLinkLibrary
(set -x; \
 $(RM) $@; \
 $(AR) $@ $^; \
 $(RANLIB) $@; \
)
endef

define ExportLibraryToDir
(files="$^"; \
 for file in $$files; do \
   $(ExportFileToDir); \
 done; \
)
endef

ifdef EXPORT_TREE
define ExportLibrary
(dir="$(EXPORT_TREE)/$(LIBDIR)"; \
 $(ExportLibraryToDir) \
)
endef
endif

define InstallLibrary
(dir="$(INSTALLLIBDIR)"; \
 $(ExportLibraryToDir) \
)
endef


#
# Stuff to generate executable binaries.
#
# These rules are used like this
#
# target: objs lib_depends
#         @(libs="libs"; $(...Executable))
#
# The command we want to generate is like this
#
# linker -o target ... objs libs
# i.e. we need to filter out the lib_depends from the command
#

IMPORT_LIBRARY_FLAGS = $(patsubst %,$(LibPathPattern),$(IMPORT_LIBRARY_DIRS))

define CXXExecutable
(set -x; \
 $(RM) $@; \
 $(CXXLINK) -o $@ $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs; \
)
endef

define CExecutable
(set -x; \
 $(RM) $@; \
 $(CLINK) -o $@ $(CLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs; \
)
endef

ifdef EXPORT_TREE
define ExportExecutable
(dir="$(EXPORT_TREE)/$(BINDIR)"; \
 files="$^"; \
 for file in $$files; do \
   $(ExportExecutableFileToDir); \
 done; \
)
endef
endif

define InstallExecutable
(dir="$(INSTALLBINDIR)"; \
 files="$^"; \
 for file in $$files; do \
   $(ExportExecutableFileToDir); \
 done; \
)
endef


# omnithread - platform libraries required by omnithread.
# Use when building omnithread.
OMNITHREAD_PLATFORM_LIB = $(filter-out $(patsubst %,$(LibSearchPattern),\
                          omnithread), $(OMNITHREAD_LIB))

#
# omniORB stuff
#

CorbaImplementation = OMNIORB

lib_depend := $(patsubst %,$(LibPattern),omniORB$(OMNIORB_MAJOR_VERSION))
omniORB_lib_depend := $(GENERATE_LIB_DEPEND)

lib_depend := $(patsubst %,$(LibPattern),omniDynamic$(OMNIORB_MAJOR_VERSION))
omniDynamic_lib_depend := $(GENERATE_LIB_DEPEND)

OMNIORB_DLL_NAME = $(patsubst %,$(LibSearchPattern),\
                   omniORB$(OMNIORB_MAJOR_VERSION))

OMNIORB_DYNAMIC_DLL_NAME = $(patsubst %,$(LibSearchPattern),\
                           omniDynamic$(OMNIORB_MAJOR_VERSION))

OMNIIDL = $(OMNIORB_BINDIR)/omniidl
OMNIORB_IDL_ONLY = $(OMNIIDL) -bcxx
OMNIORB_IDL_ANY_FLAGS = -Wba
OMNIORB_IDL = $(OMNIORB_IDL_ONLY) $(OMNIORB_IDL_ANY_FLAGS)
OMNIORB_CPPFLAGS = -D__OMNIORB$(OMNIORB_MAJOR_VERSION)__ \
                   -I$(CORBA_STUB_DIR) $(OMNITHREAD_CPPFLAGS) \
                   $(patsubst %,-I%/include,$(OMNIORB_ROOT))

OMNIORB_IDL_OUTPUTDIR_PATTERN = -C%

OMNIORB_LIB = $(OMNIORB_DLL_NAME) $(OMNIORB_DYNAMIC_DLL_NAME)
OMNIORB_LIB_NODYN = $(OMNIORB_DLL_NAME)

OMNIORB_LIB_NODYN_DEPEND = $(omniORB_lib_depend)
OMNIORB_LIB_DEPEND = $(omniORB_lib_depend) $(omniDynamic_lib_depend)

OMNIORB_STATIC_STUB_OBJS = \
	$(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%SK.o)
OMNIORB_STATIC_STUB_SRCS = \
	$(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%SK.cc)
OMNIORB_DYN_STUB_OBJS = \
	$(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%DynSK.o)
OMNIORB_DYN_STUB_SRCS = \
	$(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%DynSK.cc)

OMNIORB_STUB_SRCS = $(OMNIORB_STATIC_STUB_SRCS) $(OMNIORB_DYN_STUB_SRCS)
OMNIORB_STUB_OBJS = $(OMNIORB_STATIC_STUB_OBJS) $(OMNIORB_DYN_STUB_OBJS)

OMNIORB_STUB_SRC_PATTERN = $(CORBA_STUB_DIR)/%SK.cc
OMNIORB_STUB_OBJ_PATTERN = $(CORBA_STUB_DIR)/%SK.o
OMNIORB_DYN_STUB_SRC_PATTERN = $(CORBA_STUB_DIR)/%DynSK.cc
OMNIORB_DYN_STUB_OBJ_PATTERN = $(CORBA_STUB_DIR)/%DynSK.o
OMNIORB_STUB_HDR_PATTERN = $(CORBA_STUB_DIR)/%.hh

# thread libraries required by omniORB. Make sure this is the last in
# the list of omniORB related libraries

OMNIORB_LIB += $(OMNITHREAD_LIB) $(SOCKET_LIB)
OMNIORB_LIB_NODYN += $(OMNITHREAD_LIB) $(SOCKET_LIB)
OMNIORB_LIB_DEPEND += $(OMNITHREAD_LIB_DEPEND)
OMNIORB_LIB_NODYN_DEPEND += $(OMNITHREAD_LIB_DEPEND)

OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)


# CodeSets library
OMNIORB_CODESETS_LIB = $(patsubst %,$(LibSearchPattern),omniCodeSets$(OMNIORB_MAJOR_VERSION))
lib_depend := $(patsubst %,$(LibPattern),omniCodeSets$(OMNIORB_MAJOR_VERSION))
OMNIORB_CODESETS_LIB_DEPEND := $(GENERATE_LIB_DEPEND)

# Connections library
OMNIORB_CONNECTIONS_LIB = $(patsubst %,$(LibSearchPattern),omniConnectionMgmt$(OMNIORB_MAJOR_VERSION))
lib_depend := $(patsubst %,$(LibPattern),omniConnectionMgmt$(OMNIORB_MAJOR_VERSION))
OMNIORB_CONNECTIONS_LIB_DEPEND := $(GENERATE_LIB_DEPEND)



# omniORB SSL transport
OMNIORB_SSL_VERSION = $(OMNIORB_VERSION)
OMNIORB_SSL_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNIORB_SSL_VERSION)))
OMNIORB_SSL_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNIORB_SSL_VERSION)))
OMNIORB_SSL_LIB = $(patsubst %,$(LibSearchPattern),\
                    omnisslTP$(OMNIORB_SSL_MAJOR_VERSION))

lib_depend := $(patsubst %,$(LibPattern),omnisslTP$(OMNIORB_SSL_MAJOR_VERSION))
OMNIORB_SSL_LIB_DEPEND := $(GENERATE_LIB_DEPEND)

OMNIORB_SSL_LIB += $(OPEN_SSL_LIB)
OMNIORB_SSL_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)


# ZIOP
OMNIORB_ZIOP_LIB = $(patsubst %,$(LibSearchPattern),omniZIOP$(OMNIORB_MAJOR_VERSION))
lib_depend := $(patsubst %,$(LibPattern),omniZIOP$(OMNIORB_MAJOR_VERSION))
OMNIORB_ZIOP_LIB_DEPEND := $(GENERATE_LIB_DEPEND)


##########################################################################
#
# Shared library support stuff
#
# Default setup. Work for most platforms. For those exceptions, override
# the rules in their platform files.
#
BuildSharedLibrary = 1       # Enable by default
SHAREDLIB_SUFFIX = so

SharedLibraryFullNameTemplate 	= lib$$1$$2.$(SHAREDLIB_SUFFIX).$$3.$$4
SharedLibrarySoNameTemplate   	= lib$$1$$2.$(SHAREDLIB_SUFFIX).$$3
SharedLibraryLibNameTemplate  	= lib$$1$$2.$(SHAREDLIB_SUFFIX)
SharedLibraryImplibNameTemplate = lib$$1$$2.a

SharedLibraryPlatformLinkFlagsTemplate = -shared -Wl,-soname,$$soname

define SharedLibraryFullName
fn() { \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi ; \
echo $(SharedLibraryFullNameTemplate); \
}; fn
endef

define SharedLibraryImplibName
fn() { \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi ; \
echo $(SharedLibraryImplibNameTemplate); \
}; fn
endef

define ParseNameSpec
set $$namespec ; \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi
endef

# MakeCXXSharedLibrary- Build shared library
#  Expect shell variable:
#  namespec = <library name> <major ver. no.> <minor ver. no.> <micro ver. no>
#  extralibs = <libraries to add to the link line>
#
#  e.g. namespec="COS 3 0 0" --> shared library libCOS3.so.0.0
#       extralibs="$(OMNIORB_LIB)"
#
define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 set -x; \
 $(RM) $@; \
 $(CXX)  $(SharedLibraryPlatformLinkFlagsTemplate) -o $@ \
 $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibSuffixPattern),$^) $$extralibs;
endef

# ExportSharedLibrary- export sharedlibrary
#  Expect shell variable:
#  namespec = <library name> <major ver. no.> <minor ver. no.> <micro ver. no>
#  e.g. namespec = "COS 3 0 0" --> shared library libCOS3.so.0.0
#
define ExportSharedLibraryToDir
 $(ExportLibraryToDir); \
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 libname=$(SharedLibraryLibNameTemplate); \
 set -x; \
 cd $$dir; \
 $(RM) $$soname; \
 ln -s $(<F) $$soname; \
 $(RM) $$libname; \
 ln -s $$soname $$libname;
endef

define ExportSharedLibrary
 dir="$(EXPORT_TREE)/$(LIBDIR)"; \
 $(ExportSharedLibraryToDir)
endef

define ExportImplibLibrary
 dir="$(EXPORT_TREE)/$(LIBDIR)"; \
 $(ExportLibraryToDir)
endef

define InstallSharedLibrary
 dir="$(INSTALLLIBDIR)"; \
 $(ExportSharedLibraryToDir)
endef

define InstallImplibLibrary
 dir="$(INSTALLLIBDIR)"; \
 $(ExportLibraryToDir)
endef


define CleanSharedLibrary
( set -x; \
$(RM) $${dir:-.}/*.$(SHAREDLIB_SUFFIX).* )
endef

define CleanImplibLibrary
( set -x; \
$(RM) $${dir:-.}/*.a )
endef


# Pattern rules to build  objects files for static and shared library.
# The convention is to build the static library in the subdirectoy "static" and
# the shared library in the subdirectory "shared".
# The pattern rules below ensure that the right compiler flags are used
# to compile the source for the library.

static/%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -o $@ $<

shared/%.o: %.cc
	$(CXX) -c $(SHAREDLIB_CPPFLAGS) $(CXXFLAGS)  -o $@ $<

static/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

SHAREDLIB_CFLAGS = $(SHAREDLIB_CPPFLAGS)

shared/%.o: %.c
	$(CC) -c $(SHAREDLIB_CFLAGS) $(CFLAGS)  -o $@ $<

#
# Replacements for implicit rules
#

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -o $@ $<


###########################################################################
#
# Things figured out by autoconf
#


#
# Standard unix programs - note that GNU make already defines some of
# these such as AR, RM, etc (see section 10.3 of the GNU make manual).
#

AR              = ar cq
CC              = gcc
CXX             = g++
RANLIB		= ranlib
MKDIRHIER	= /mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omniORBpy-4.2.4/bin/scripts/omkdirhier
INSTLIBFLAGS	= -m 0644
INSTEXEFLAGS	= -m 0755
CP		= cp
MV		= mv -f
CPP		= gcc -E
OMKDEPEND	= /mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/omni_inst/bin/omkdepend
RMDIRHIER	= $(RM) -rf

CXXMAKEDEPEND   = $(OMKDEPEND) -D__cplusplus
CMAKEDEPEND     = $(OMKDEPEND)


#
# Platform, processor and compiler
#

Linux = 1
x8664Processor = 1
Compiler_GCC = 1

#
# File locations
#

OMNIORB_CONFIG_DEFAULT_LOCATION = @OMNIORB_CONFIG@
OMNINAMES_LOG_DEFAULT_LOCATION  = @OMNINAMES_LOGDIR@

#
# OpenSSL
#

OPEN_SSL_ROOT = 
OPEN_SSL_LIB = 
OPEN_SSL_CPPFLAGS = 

#
# Static libraries?
#

ifeq (@ENABLE_STATIC@,no)
NoStaticLibrary = 1
endif

#
# Enable ZIOP?
#

ifeq (yes,yes)
EnableZIOP = 1
endif


###########################################################################
#
# Default compiler rules
#
CDEBUGFLAGS     = -g -O2
CLINK           = $(CC)
CLINKOPTIONS    =  $(CDEBUGFLAGS) $(COPTIONS)
CXXDEBUGFLAGS   = -g -O2
CXXLINK         = $(CXX)
CXXLINKOPTIONS  =  $(CXXDEBUGFLAGS) $(CXXOPTIONS)


###########################################################################
#
# OS independent compiler dependencies
#

ifdef Compiler_GCC
CMAKEDEPEND     += -D__GNUC__
CXXMAKEDEPEND   += -D__GNUG__ -D__GNUC__
CXXOPTIONS       = -Wall -Wno-unused -fexceptions 
EgcsMajorVersion = 1
EgcsMinorVersion = 1
SHAREDLIB_CPPFLAGS = -fPIC
endif



###########################################################################
#
# OS dependencies
#
# So much for autoconfiguration... ;-)

###################
ifdef Linux
IMPORT_CPPFLAGS += -D__linux__
OMNITHREAD_POSIX_CPPFLAGS = -DNoNanoSleep -DPthreadDraftVersion=10
OMNITHREAD_CPPFLAGS = -D_REENTRANT
OMNITHREAD_LIB += -lpthread
endif

###################
ifdef kFreeBSD
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10
OMNITHREAD_CPPFLAGS = -D_REENTRANT -pthread
OMNITHREAD_LIB += -lpthread
endif

###################
ifdef GNU
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10
OMNITHREAD_CPPFLAGS = -D_REENTRANT -pthread
OMNITHREAD_LIB += -lpthread
endif

###################
ifdef SunOS
IMPORT_CPPFLAGS += -D__sunos__
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10
OMNITHREAD_CPPFLAGS = -DUsePthread -D_REENTRANT $(CXXMTFLAG)
SOCKET_LIB = -lsocket -lnsl
THREAD_LIB = -lthread $(CXXMTFLAG)
OMNITHREAD_LIB += -lpthread -lposix4 $(CXXMTFLAG)

ifdef Compiler_GCC
SharedLibraryPlatformLinkFlagsTemplate = -shared -Wl,-h,$$soname
endif
ifdef Compiler_Sun5
SharedLibraryPlatformLinkFlagsTemplate = -G -h $$soname
PythonLibraryPlatformLinkFlagsTemplate = -G -h $$soname -lCrun
CXXMAKEDEPEND += -D__SUNPRO_CC
CXXDEBUGFLAGS = -O2 -g # Remove -g may cause problem with exception handling
                       # This is a problem with Sun C++ 5.0, see README.SunC++5
CXXMTFLAG     = -mt
SHAREDLIB_CPPFLAGS = -KPIC
endif
ifdef Compiler_Sun4
CXXMTFLAG     = -mt
CXXDEBUGFLAGS = -O2 -g
SHAREDLIB_CPPFLAGS = -KPIC
SharedLibraryPlatformLinkFlagsTemplate = -G -h $$soname
PythonLibraryPlatformLinkFlagsTemplate = -G -h $$soname -lC
endif

endif

###################
ifdef OSF1
IMPORT_CPPFLAGS          += -D__osf1__

ifeq (2,3)
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=4 -DNoNanoSleep
else
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 -DNoNanoSleep
endif

OMNITHREAD_CPPFLAGS       = -pthread
CXXOPTIONS               += -pthread

AR = ar clq
OMNITHREAD_LIB += -lmach -lc_r

define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 set -x; \
 $(RM) $@; \
 ld -shared -soname $$soname -set_version $$soname -o $@ \
    $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibSuffixPattern),$^) $$extralibs \
    -lpthread -lcxxstd -lcxx -lexc -lots -lc;
endef

ifdef Compiler_DEC61
DecCxxMajorVersion = 6
DecCxxMinorVersion = 1
CXXOPTIONS        += -ptr $(TOP)/cxx_respository
CXXDEBUGFLAGS      = -O
CXXLINKOPTIONS    += -call_shared
CMAKEDEPEND       += -D__DECC
CXXMAKEDEPEND     += -D__DECCXX
endif
ifdef Compiler_DEC62
DecCxxMajorVersion = 6
DecCxxMinorVersion = 2
CXXOPTIONS        += -ptr $(TOP)/cxx_respository
CXXDEBUGFLAGS      = -O
CXXLINKOPTIONS    += -call_shared
CMAKEDEPEND       += -D__DECC
CXXMAKEDEPEND     += -D__DECCXX -D__DECCXX_VER=60290024
endif

endif

###################
ifdef HPUX
IMPORT_CPPFLAGS += -D__hpux__
INSTLIBFLAGS     = -m 0755

ifeq (2,10)
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=4
OMNITHREAD_CPPFLAGS += -D_REENTRANT
OMNITHREAD_LIB += -ldce -lcma
else
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10
endif

define StaticLinkLibrary
(set -x; \
 $(RM) $@; \
 $(CXX) -c +inst_close $^; \
 $(AR) $@ $^; \
 $(RANLIB) $@; \
)
endef

ifdef ia64Processor
SHAREDLIB_SUFFIX = so
endif

ifndef ia64Processor
SHAREDLIB_SUFFIX = sl
endif

ifdef Compiler_KCC
AR = KCC --thread_safe -o
CXXDEBUGFLAGS = +K0 --one_per --thread_safe --exceptions +Z

CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) -Bdynamic --thread_safe \
		--exceptions 

SHAREDLIB_CPPFLAGS = +Z
SharedLibraryPlatformLinkFlagsTemplate = --thread_safe --soname $$soname
endif

ifdef Compiler_GCC
# -D_CMA_NOWRAPPERS_ is needed otherwise linking omniNames results in
#                    /opt/aCC/lbin/ld: Unsatisfied symbols:
#                    fstreambase::cma_close(void)(code)
CXXOPTIONS    =  -Wall -Wno-unused \
                 -D_CMA_NOWRAPPERS_ 
SharedLibraryPlatformLinkFlagsTemplate = -Wl,-b -Wl,+h$$soname -lCsup				 
endif

ifdef Compiler_aCC
CXXDEBUGFLAGS     = -O
CXXOPTIONS       += -AA -w -mt
CDEBUGFLAGS       = -O
COPTIONS          = -w -mt

SHAREDLIB_CPPFLAGS += +Z
SharedLibraryPlatformLinkFlagsTemplate = -b -Wl,+h$$soname -lCsup
endif

endif

###################
ifdef NextStep
IMPORT_CPPFLAGS += -D__nextstep__

ThreadSystem = Mach
OMNITHREAD_CPPFLAGS = -D_REENTRANT

ifdef Compiler_GCC
CXXOPTIONS +=  -fhandle-exceptions -Wall -Wno-unused
endif
endif

###################
ifdef IRIX
IMPORT_CPPFLAGS += -D__irix__

OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 \
			    -DPthreadSupportThreadPriority
OMNITHREAD_LIB += -lpthread

# *** FIXME. How do we know what ABI to use?  Perhaps it has to be a
# configure option?
ABIFLAG = -n32

ifdef Compiler_SGI
CXXMAKEDEPEND += -D__SGI_CC
CXXDEBUGFLAGS  = -O2 -OPT:Olimit=0
CXXWOFFOPTIONS =  -woff 3303,1110,1182
CXXOPTIONS     =  $(ABIFLAG) -float -ansi -LANG:exceptions=ON $(CXXWOFFOPTIONS)
COPTIONS       = $(ABIFLAG)
endif

endif

###################
ifdef AIX
ifndef Compiler_GCC		# Assuming XlC compiler

IMPORT_CPPFLAGS += -D__aix__

CMAKEDEPEND     += -D_AIX
CXXMAKEDEPEND   += -D_AIX
CDEBUGFLAGS      =
CXXDEBUGFLAGS    =
CXXLINKOPTIONS  += -brtl -bnoipath -blibpath:/usr/lib:$(prefix)/lib

CXXOPTIONS	= -qstaticinline -qmaxmem=8192 -qlonglong -qlongdouble

COPTIONS	= -qmaxmem=8192 -qlonglong -qlongdouble

OMNITHREAD_POSIX_CPPFLAGS = -DNoNanoSleep -DPthreadDraftVersion=10
OMNITHREAD_CPPFLAGS = -D_REENTRANT
OMNITHREAD_LIB += -lpthread

#SHAREDLIB_SUFFIX = so
PythonSHAREDLIB_SUFFIX = so

# this works only for xlc version >= 5
SharedLibraryPlatformLinkFlagsTemplate = -G -qmkshrobj \
               -bnoipath -blibpath:/usr/lib:$(prefix)/lib

endif

ifdef Compiler_GCC

IMPORT_CPPFLAGS += -D__aix__
CMAKEDEPEND     += -D_AIX
CXXMAKEDEPEND   += -D_AIX
CXXLINKOPTIONS  += -Wl,-brtl -Wl,-blibpath:/lib:/usr/lib:$(prefix)/lib
CLINKOPTIONS    += -Wl,-brtl -Wl,-blibpath:/lib:/usr/lib:$(prefix)/lib

# Name all static libraries with -ar.a suffix.
LibPattern = lib%-ar.a
LibDebugPattern = lib%-ar.a
LibNoDebugPattern = lib%-ar.a
LibSuffixPattern = %-ar.a

# Name all shared libraries with .a suffix
LibSharedPattern = lib%.a
LibSharedSuffixPattern = %.a
LibSharedSearchPattern = -l%

# Link with shared libraries
LibSearchPattern = -l%$(OMNIORB_MINOR_VERSION)

# OMNI thread stuff
ThreadSystem = Posix
OMNITHREAD_POSIX_CPPFLAGS = -DNoNanoSleep -DPthreadDraftVersion=10
OMNITHREAD_CPPFLAGS = -D_REENTRANT -D_THREAD_SAFE
OMNITHREAD_PLATFORM_LIB = -lpthreads
OMNITHREAD_LIB = -lomnithread$(OMNITHREAD_MAJOR_VERSION) $(OMNITHREAD_PLATFORM_LIB)

# Shared library support stuff
SHAREDLIB_SUFFIX   = a
PythonSHAREDLIB_SUFFIX = so
SharedLibraryFullNameTemplate = lib$$1$$2$$3$$4.$(SHAREDLIB_SUFFIX)
SharedLibrarySoNameTemplate = lib$$1$$2.$(SHAREDLIB_SUFFIX).$$3
SharedLibraryLibNameTemplate = lib$$1$$2$$3.$(SHAREDLIB_SUFFIX)
SharedLibraryPlatformLinkFlagsTemplate = -shared -fPIC -Wl,-brtl -Wl,-blibpath:/lib:/usr/lib:$(prefix)/lib

# Need to use the script 'ld_so_aix' to make Python modules on AIX.
# For details see the file Misc/AIX-NOTES that is distributed with the Python
# source code.

# The pythonX.Y/config directory only exists in Python v2.3+, so this solution
# doesn't work for earlier versions of Python. (But omniORBpy did't work for
# earlier versions of Python anyway.)
LD_SO_AIX = $(PYPREFIX)/lib/python$(PYVERSION)/config/ld_so_aix
PYTHONEXP = $(PYPREFIX)/lib/python$(PYVERSION)/config/python.exp

define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 set -x; \
 $(RM) $@; \
 $(LD_SO_AIX) $(CXX) -bI:$(PYTHONEXP) $(LD_SO_AIX_FLAGS) -o $@ \
   $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibSuffixPattern),$^) $$extralibs;
endef

endif	# ifndef Compiler_GCC ... else ...

endif	# AIX

###################
ifdef Darwin
IMPORT_CPPFLAGS += -D__darwin__

OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 \
                            -DPthreadSupportThreadPriority -DNoNanoSleep

CXXOPTIONS = -fno-common 
SHAREDLIB_SUFFIX = dylib

SharedLibraryFullNameTemplate = lib$$1$$2.$$3.$$4.$(SHAREDLIB_SUFFIX)
SharedLibrarySoNameTemplate = lib$$1$$2.$$3.$(SHAREDLIB_SUFFIX)
SharedLibraryLibNameTemplate = lib$$1$$2.$(SHAREDLIB_SUFFIX)
SharedLibraryPlatformLinkFlagsTemplate = -dynamiclib \
                                         -install_name $(INSTALLLIBDIR)/$$soname \
                                         -flat_namespace \
                                         -undefined suppress
PythonLibraryPlatformLinkFlagsTemplate = -bundle -flat_namespace \
                                         -undefined suppress
PythonSHAREDLIB_SUFFIX = so


# Re-define 'ExportLibrary' to run 'ranlib' after the file is copied,
# for static libraries as otherwise the linker complains: "table of 
# contents for archive: ???? is out of date; rerun ranlib(1) (can't
# load from it)"
#
define ExportLibraryToDir
(files="$^"; \
 for file in $$files; do \
   $(ExportFileToDir); \
   base=`basename $$file`; \
   if [ $${base%.a} != $$base ]; then (set -x; $(RANLIB) $$dir/$$base); fi; \
 done; \
)
endef

endif

###################
ifdef FreeBSD
IMPORT_CPPFLAGS += -D__freebsd__

OMNITHREAD_CPPFLAGS = -D_REENTRANT -D_THREAD_SAFE
OMNITHREAD_POSIX_CPPFLAGS = -DUsePthread -DPthreadDraftVersion=10
OMNITHREAD_LIB += -pthread
endif

###################
ifdef NetBSD
IMPORT_CPPFLAGS += -D__netbsd__

OMNITHREAD_CPPFLAGS = -D_REENTRANT
OMNITHREAD_POSIX_CPPFLAGS = -DUsePthread -DPthreadDraftVersion=10
OMNITHREAD_LIB += -pthread
endif

###################
ifdef OpenBSD
IMPORT_CPPFLAGS += -D__openbsd__

OMNITHREAD_CPPFLAGS = -D_REENTRANT -D_THREAD_SAFE
OMNITHREAD_POSIX_CPPFLAGS = -DUsePthread -DPthreadDraftVersion=10
OMNITHREAD_LIB += -pthread
endif

###################
ifdef OSR5
IMPORT_CPPFLAGS += -D__osr5__

COPTIONS = -fpcc-struct-return

OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=6 \
			    -DPthreadSupportThreadPriority -DNoNanoSleep
endif

###################
ifdef Cygwin
MKDIRHIER = mkdir -p
extralibs += -L$(PYPREFIX)/lib/python$(PYVERSION)/config \
 -lpython$(PYVERSION).dll
IMPORT_CPPFLAGS += -D__cygwin__
SHAREDLIB_CPPFLAGS =
OMNITHREAD_POSIX_CPPFLAGS = -DNoNanoSleep -DPthreadDraftVersion=10
OMNITHREAD_CPPFLAGS = -D_REENTRANT
OMNITHREAD_LIB += -lpthread
BinPattern = %.exe

SharedLibraryPlatformLinkFlagsTemplate = -shared -Wl,-soname,$$soname,--export-all-symbols,--enable-auto-import

define ExportLibraryToDir
(files="$^"; \
 for file in $$files; do \
   $(ExportExecutableFileToDir); \
 done; \
)
endef

define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 set -x; \
 $(RM) $@; \
 $(CXX) $(SharedLibraryPlatformLinkFlagsTemplate) -o $@ \
 $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibSuffixPattern),$^) $$extralibs;
endef

define ExportSharedLibrary
 dir="$(EXPORT_TREE)/$(BINDIR)"; \
 $(ExportSharedLibraryToDir)
endef

define InstallSharedLibrary
 dir="$(INSTALLBINDIR)"; \
 $(ExportSharedLibraryToDir)
endef

SHAREDLIB_SUFFIX = dll

SharedLibraryFullNameTemplate = lib$$1$$2.$(SHAREDLIB_SUFFIX).$$3.$$4
SharedLibrarySoNameTemplate   = lib$$1$$2.$(SHAREDLIB_SUFFIX).$$3
SharedLibraryLibNameTemplate  = lib$$1$$2.$(SHAREDLIB_SUFFIX)

endif



###########################################################################
#
# Processor
#

ifdef x86Processor
IMPORT_CPPFLAGS += -D__x86__
endif

ifdef x8664Processor
IMPORT_CPPFLAGS += -D__x86_64__
endif

ifdef SparcProcessor
IMPORT_CPPFLAGS += -D__sparc__
endif

ifdef AlphaProcessor
IMPORT_CPPFLAGS += -D__alpha__
endif

ifdef m68kProcessor
IMPORT_CPPFLAGS += -D__m68k__
endif

ifdef IndigoProcessor
IMPORT_CPPFLAGS += -D__mips__
endif

ifdef ArmProcessor
IMPORT_CPPFLAGS += -D__arm__
endif

ifdef s390Processor
IMPORT_CPPFLAGS += -D__s390__
endif

ifdef ia64Processor
IMPORT_CPPFLAGS += -D__ia64__
endif

ifdef HppaProcessor
IMPORT_CPPFLAGS += -D__hppa__
endif

ifdef PowerPCProcessor
IMPORT_CPPFLAGS += -D__powerpc__
endif


###########################################################################
#
# Final things
#

lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)
