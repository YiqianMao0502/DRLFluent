#
# Standard make variables and rules for all Win32 platforms.
#

Win32Platform = 1

#
# Define macro for path of wrapper executables
#
ifndef EmbeddedSystem
WRAPPER_FPATH = $(BINDIR)
else
WRAPPER_FPATH = $(HOSTBINDIR)
endif


#
# Standard "unix" programs.  Anything here not provided by the GNU-WIN32/OpenNT/UWIN
# system is likely to need a wrapper around it to perform filename translation.
#
ifndef OpenNTBuildTree

# GNU-WIN32 wrappers
XLN = -gnuwin32

# There is a sort in %System32%/sort.exe and in GNU-WIN32. The shell of
# GNU-WIN32 may pick either one depending on the PATH setup of the user.
# To make sure that the GNU-WIN32 version is picked up, give the pathname
# of sort.

SORT = /bin/sort

else

# OpenNT or UWIN wrappers
XLN = -opennt
MKDEPOPT = -opennt
SORT = sort

endif


AR = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/libwrapper $(XLN)
CXX = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/clwrapper $(XLN)
CXXLINK	= $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/linkwrapper $(XLN)
CXXMAKEDEPEND = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/omkdepend $(MKDEPOPT) -D__cplusplus -D_MSC_VER
CC = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/clwrapper $(XLN)
CLINK = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/linkwrapper $(XLN)
CMAKEDEPEND = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/omkdepend $(MKDEPOPT) -D_MSC_VER

RCTOOL          = rc.exe
MANIFESTTOOL    = true

MKDIRHIER	= mkdir -p

INSTALL		= install -c
INSTLIBFLAGS	= 
INSTEXEFLAGS	= 

CP = cp
MV = mv -f

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


ifndef WINVER
WINVER = 0x0400
endif

IMPORT_CPPFLAGS += -D__WIN32__ -D_WIN32_WINNT=$(WINVER)

SOCKET_LIB = ws2_32.lib mswsock.lib


#
# General rule for cleaning.
#

define CleanRule
$(RM) *.o *.lib
endef

define VeryCleanRule
$(RM) *.d
$(RM) *.pyc
$(RM) *.def *.pdb *.ilk *.exp *.manifest *.rc *.res
$(RM) $(CORBA_STUB_FILES)
endef


#
# Patterns for various file types
#

LibPathPattern = -libpath:%
LibNoDebugPattern = %.lib
LibDebugPattern = %d.lib
DLLNoDebugPattern = %_rt.lib
DLLDebugPattern = %_rtd.lib
LibNoDebugSearchPattern = %.lib
LibDebugSearchPattern = %d.lib
DLLNoDebugSearchPattern = %_rt.lib
DLLDebugSearchPattern = %_rtd.lib


ifndef BuildDebugBinary

LibPattern = $(LibNoDebugPattern)
DLLPattern = $(DLLNoDebugPattern)
LibSearchPattern = $(LibNoDebugSearchPattern)
DLLSearchPattern = $(DLLNoDebugSearchPattern)

else

LibPattern = $(LibDebugPattern)
DLLPattern = $(DLLDebugPattern)
LibSearchPattern = $(LibDebugSearchPattern)
DLLSearchPattern = $(DLLDebugSearchPattern)

endif

BinPattern = %.exe


#
# Stuff to generate statically-linked libraries.
#

define StaticLinkLibrary
(set -x; \
 $(RM) $@; \
 $(AR) $@ $^; \
)
endef

ifdef EXPORT_TREE
define ExportLibrary
(dir="$(EXPORT_TREE)/$(LIBDIR)"; \
 files="$^"; \
 for file in $$files; do \
   $(ExportFileToDir); \
 done; \
 for pfile in $$files; do \
   file=$${pfile%.lib}.pdb; \
   if [ -f $$file ]; then \
     $(ExportFileToDir); \
   fi; \
 done;\
)
endef
endif


#
# Stuff to generate executable binaries.
#

IMPORT_LIBRARY_FLAGS = $(patsubst %,$(LibPathPattern),$(IMPORT_LIBRARY_DIRS))

define CXXExecutable
(set -x; \
 $(RM) $@; \
 $(CXXLINK) -out:$@ $(CXXLINKOPTIONS) -PDB:$@.pdb $(IMPORT_LIBRARY_FLAGS) \
      $(filter-out $(LibPattern),$^) $$libs; \
 $(MANIFESTTOOL) /outputresource:"$@;#1" /manifest $@.manifest; \
)
endef

define CExecutable
(set -x; \
 $(RM) $@; \
 $(CLINK) -out:$@ $(CLINKOPTIONS) -PDB:$@.pdb $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibPattern),$^) $$libs; \
 $(MANIFESTTOOL) /outputresource:"$@;#1" /manifest $@.manifest; \
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


###########################################################################
#
# Shared library support stuff
#
ifndef EmbeddedSystem
BuildSharedLibrary = 1
endif

SharedLibraryFullNameTemplate = $(SharedLibraryLibNameTemplate).lib
SharedLibraryLibNameTemplate  = $$1$$2$$3$$4_rt$${extrasuffix:-}
SharedLibraryShortLibName = $$1$$2_rt$${extrasuffix:-}.lib
SharedLibraryDllNameTemplate  = $$1$$2$$3$$4$(compiler_version_suffix)_rt$${extrasuffix:-}
SharedLibraryExportSymbolFileNameTemplate = $$1$$2$${extrasuffix:-}.def
SharedLibraryVersionStringTemplate = $$3.$$4
SharedLibrarySymbolRefLibraryTemplate = $${symrefdir:-static}/$$1$$2$${extrasuffix:-}.lib

define SharedLibraryFullName
fn() { \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi ; \
echo $(SharedLibraryFullNameTemplate); \
}; fn
endef

define SharedLibraryDebugFullName
fn() { \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi ; \
extrasuffix="d"; \
echo $(SharedLibraryFullNameTemplate); \
}; fn
endef

define ParseNameSpec
set $$namespec ; \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi
endef


# MakeCXXExportSymbolDefinitionFile
#   Internal canned command used by MakeCXXSharedLibrary
#
#  - Create a .def file containing all the functions and static class 
#    variables exported by the DLL. The symbols are extracted from the 
#    output of dumpbin.
#
#    The function symbols are extracted using the following template:
#    ... ........ SECT..  notype ()     External      | ?..................
#
#    The static class variable symbols are extracted using the following
#    template:
#    ... ........ SECT..  notype        External      | ?[^?]..............
#
#    Default destructors generated by the compiler and the symbols
#    inside an anonymous namespace are excluded.
#
#    It looks like class variable and function symbols start with two ??
#    and class static variable and static function symbols start with one ?.
#                                                             - SLL
#
define MakeCXXExportSymbolDefinitionFile
symrefdir=$${debug:+debug}; \
symreflib=$(SharedLibrarySymbolRefLibraryTemplate); \
if [ ! -f $$symreflib ]; then echo "Cannot find reference static library $$symreflib"; return 1; fi;  \
set -x; \
echo "LIBRARY $$dllbase" > $$defname; \
echo "VERSION $$version" >> $$defname; \
echo "EXPORTS" >> $$defname; \
DUMPBIN.EXE /SYMBOLS $$symreflib | \
egrep '^[^ ]+ +[^ ]+ +SECT[^ ]+ +[^ ]+ +\(\) +External +\| +\?[^ ]*|^[^ ]+ +[^ ]+ +SECT[^ ]+ +[^ ]+ +External +\| +\?[^?][^ ]*'|\
egrep -v 'deleting destructor[^(]+\(unsigned int\)' | \
egrep -v 'anonymous namespace' | \
egrep -v '@std@' | \
cut -d'|' -f2 | \
cut -d' ' -f2 | $(SORT) -u >> $$defname; \
set +x;
endef


# MakeResourceDefinitionFile
#   Internal canned command used by MakeCXXSharedLibrary

define MakeResourceDefinitionFile
if [ -n "$$2" ]; then \
commaver=$$2,$$3,$$4,0x$$nanovers; \
dotver=$$2.$$3.$$4.$$nanovers; \
else \
commaver=$$3,$$4,0x$$nanovers,0; \
dotver=$$3.$$4.$$nanovers; \
fi; \
set -x; \
echo "#include \"windows.h\"" > $$rcname; \
echo "VS_VERSION_INFO VERSIONINFO" >> $$rcname; \
echo " FILEVERSION $$commaver" >> $$rcname; \
echo " PRODUCTVERSION $(OMNIORB_MAJOR_VERSION),$(OMNIORB_MINOR_VERSION),$(OMNIORB_MICRO_VERSION),0x$$nanovers" >> $$rcname; \
echo " FILEFLAGSMASK 0x3fL" >> $$rcname; \
if [ -n "$$debug" ]; then \
echo " FILEFLAGS VS_FF_DEBUG" >> $$rcname; \
else \
echo " FILEFLAGS 0x0L" >> $$rcname; \
fi; \
echo " FILEOS VOS_UNKNOWN" >> $$rcname; \
echo " FILETYPE VFT_UNKNOWN" >> $$rcname; \
echo " FILESUBTYPE 0x0L" >> $$rcname; \
echo "{    " >> $$rcname; \
echo "    BLOCK \"StringFileInfo\"" >> $$rcname; \
echo "    {" >> $$rcname; \
echo "        BLOCK \"00000000\"" >> $$rcname; \
echo "        {" >> $$rcname; \
echo "            VALUE \"CompanyName\", \"omniORB open source project\0\"" >> $$rcname; \
echo "            VALUE \"FileDescription\", \"omniORB\0\"" >> $$rcname; \
echo "            VALUE \"FileVersion\", \"$$dotver\0\"" >> $$rcname; \
echo "            VALUE \"InternalName\", \"$(SharedLibraryDllNameTemplate).dll\0\"" >> $$rcname; \
echo "            VALUE \"OriginalFilename\", \"$(SharedLibraryDllNameTemplate).dll\0\"" >> $$rcname; \
echo "            VALUE \"ProductName\", \"omniORB\"" >> $$rcname; \
echo "            VALUE \"ProductVersion\", \"$(OMNIORB_MAJOR_VERSION).$(OMNIORB_MINOR_VERSION).$(OMNIORB_MICRO_VERSION).$$nanovers\0\"" >> $$rcname; \
echo "            VALUE \"LegalCopyright\", \"Apasphere Ltd., AT&T Laboratories Cambridge, and others. Freely available under the terms of the GNU LGPL.\0\"" >> $$rcname; \
echo "        }" >> $$rcname; \
echo "    }" >> $$rcname; \
echo "    BLOCK \"VarFileInfo\"" >> $$rcname; \
echo "    {" >> $$rcname; \
echo "        VALUE \"Translation\", 0x0, 0" >> $$rcname; \
echo "    }" >> $$rcname; \
echo "}" >> $$rcname; \
echo "LANGUAGE LANG_NEUTRAL, SUBLANG_NEUTRAL" >> $$rcname; \
$(RCTOOL) $$rcname;
endef

# MakeCXXSharedLibrary- Build shared library
#  Expect shell variable:
#  namespec = <library name> <major ver. no.> <minor ver. no.> <micro ver. no>
#  extralibs = <libraries to add to the link line>
#  debug = 1 (build debug version).
#
#  e.g. namespec="COS 3 0 0" --> COS300_rt.dll
#       extralibs="$(OMNIORB_LIB)"
#
define MakeCXXSharedLibrary
$(ParseNameSpec); \
extrasuffix=$${debug:+d}; \
targetdir=$(@D); \
libname=$(SharedLibraryLibNameTemplate); \
slibname=$(SharedLibraryShortLibName); \
dllbase=$$targetdir/$(SharedLibraryDllNameTemplate); \
dllname=$$dllbase.dll; \
rcname=$$dllbase.rc; \
resname=$$targetdir/$(SharedLibraryDllNameTemplate).res; \
defname=$$targetdir/$(SharedLibraryExportSymbolFileNameTemplate); \
version=$(SharedLibraryVersionStringTemplate); \
nanovers=`echo $(OMNIORB_VERSION_HEX) | cut -c 9-`; \
if [ -n "$$debug" ]; then \
extralinkoption="$(MSVC_DLL_CXXLINKDEBUGOPTIONS)"; \
else \
extralinkoption="$(MSVC_DLL_CXXLINKNODEBUGOPTIONS)"; \
fi; \
if [ -z "$$nodeffile" ]; then \
$(MakeCXXExportSymbolDefinitionFile) \
defflag="-def:$$defname"; \
fi; \
$(MakeResourceDefinitionFile) \
set -x; \
$(RM) $@; \
$(CXXLINK) -out:$$dllname -DLL $$extralinkoption \
$$defflag -IMPLIB:$@ $(IMPORT_LIBRARY_FLAGS) \
$^ $$extralibs $$resname; \
$(MANIFESTTOOL) /outputresource:"$$dllname;#2" /manifest $$dllname.manifest; \
$(CP) $@ $$slibname;
endef

# Export SharedLibrary
#   Expected shell variable:
#   namespec = <library name> <major ver. no.> <minor ver. no.> <micro ver. no>
#    e.g. namespec = "COS 3 0 0"
#
# NT treats DLLs more like executables -- the .dll file needs to go in the
# bin/x86... directory so that it's on your PATH:
#
define ExportSharedLibrary
$(ParseNameSpec); \
extrasuffix=$${debug:+d}; \
targetdir=$(<D); \
libname=$(SharedLibraryLibNameTemplate); \
slibname=$(SharedLibraryShortLibName); \
dllname=$$targetdir/$(SharedLibraryDllNameTemplate).dll; \
pdbname=$$targetdir/$(SharedLibraryDllNameTemplate).pdb; \
(dir="$(EXPORT_TREE)/$(LIBDIR)"; \
 file="$^"; \
 $(ExportFileToDir); \
); \
(dir="$(EXPORT_TREE)/$(LIBDIR)"; \
 file="$$slibname"; \
 $(ExportFileToDir); \
); \
(dir="$(EXPORT_TREE)/$(BINDIR)"; \
 file="$$dllname"; \
 $(ExportExecutableFileToDir); \
); \
if [ -f $$pdbname ]; then \
(dir="$(EXPORT_TREE)/$(BINDIR)"; \
 file="$$pdbname"; \
 $(ExportExecutableFileToDir); \
);\
fi;
endef

# CleanSharedLibrary
#   Expected shell variable:
#      dir = directory name to clean. Default to . (current directory)
#
define CleanSharedLibrary
( set -x; \
$(RM) $${dir:-.}/*.dll $${dir:-.}/*.lib $${dir:-.}/*.exp $${dir:-.}/*.def \
      $${dir:-.}/*.dll.manifest $${dir:-.}/*.ilk $${dir:-.}/*.pdb \
      $${dir:-.}/*.rc $${dir:-.}/*.res)
endef

# CleanStaticLibrary
#   Expected shell variable:
#      dir = directory name to clean. Default to . (current directory)
#
define CleanStaticLibrary
( set -x; \
$(RM) $${dir:-.}/*.lib $${dir:-.}/*.exp $${dir:-.}/*.def \
      $${dir:-.}/*.ilk $${dir:-.}/*.pdb \
      $${dir:-.}/*.rc $${dir:-.}/*.res)
endef

# Pattern rules to build objects files for static and shared library and the
# debug versions for both.
# The convention is to build object files and libraries in different
# subdirectoryies.
#    static - the static library
#    debug  - the static debug library
#    shared - the DLL
#    shareddebug - the DLL debug library
#
# The pattern rules below ensured that the right compiler flags are used
# to compile the source for the library.

static/%.o: %.cc
	$(CXX) -c $(CXXDEBUGFLAGS) $(MSVC_STATICLIB_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ -Fdstatic\\$(LIB_NAME)$(major).pdb $<

debug/%.o: %.cc
	$(CXX) -c  $(MSVC_STATICLIB_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ -Fddebug\\$(LIB_NAME)$(major)d.pdb $<

shared/%DynSK.o: %DynSK.cc
	$(CXX) -c $(CXXDEBUGFLAGS) -DUSE_core_stub_in_nt_dll $(MSVC_DLL_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ -Fdshared\\ $<

shared/%SK.o: %SK.cc
	$(CXX) -c $(CXXDEBUGFLAGS) -DUSE_dyn_stub_in_nt_dll $(MSVC_DLL_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ -Fdshared\\ $<

shared/%.o: %.cc
	$(CXX) -c $(CXXDEBUGFLAGS) $(MSVC_DLL_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ -Fdshared\\ $<


shareddebug/%DynSK.o: %DynSK.cc
	$(CXX) -c  -DUSE_core_stub_in_nt_dll $(MSVC_DLL_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ -Fdshareddebug\\ $<

shareddebug/%SK.o: %SK.cc
	$(CXX) -c  -DUSE_dyn_stub_in_nt_dll $(MSVC_DLL_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ -Fdshareddebug\\ $<

shareddebug/%.o: %.cc
	$(CXX) -c  $(MSVC_DLL_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ -Fdshareddebug\\ $<

static/%.o: %.c
	$(CC) -c $(CDEBUGFLAGS) $(MSVC_STATICLIB_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ -Fdstatic\\$(LIB_NAME)$(major).pdb $<

debug/%.o: %.c
	$(CC) -c $(MSVC_STATICLIB_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ -Fdstatic\\$(LIB_NAME)$(major)d.pdb $<

shared/%.o: %.c
	$(CC) -c $(CDEBUGFLAGS) $(MSVC_DLL_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<


shareddebug/%.o: %.c
	$(CC) -c  $(MSVC_DLL_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<


#
# Replacements for implicit rules
#

%.o: %.c
	$(CC) -c $(CFLAGS) -Fo$@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -Fo$@ $<



#################################################################################
# CORBA stuff
#

include $(BASE_OMNI_TREE)/mk/version.mk

OMNIORB_DLL_NAME = $(shell $(SharedLibraryFullName) $(subst ., ,omniORB.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_DLL_NAME = $(shell $(SharedLibraryDebugFullName) $(subst ., ,omniORB.$(OMNIORB_VERSION)))

OMNIORB_DYNAMIC_DLL_NAME = $(shell $(SharedLibraryFullName) $(subst ., ,omniDynamic.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_DYNAMIC_DLL_NAME = $(shell $(SharedLibraryDebugFullName) $(subst ., ,omniDynamic.$(OMNIORB_VERSION)))

OMNIORB_CODESETS_DLL_NAME = $(shell $(SharedLibraryFullName) $(subst ., ,omniCodeSets.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_CODESETS_DLL_NAME = $(shell $(SharedLibraryDebugFullName) $(subst ., ,omniCodeSets.$(OMNIORB_VERSION)))

OMNIORB_CONNECTIONS_DLL_NAME = $(shell $(SharedLibraryFullName) $(subst ., ,omniConnectionMgmt.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_CONNECTIONS_DLL_NAME = $(shell $(SharedLibraryDebugFullName) $(subst ., ,omniConnectionMgmt.$(OMNIORB_VERSION)))

OMNIORB_ZIOP_DLL_NAME = $(shell $(SharedLibraryFullName) $(subst ., ,omniZIOP.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_ZIOP_DLL_NAME = $(shell $(SharedLibraryDebugFullName) $(subst ., ,omniZIOP.$(OMNIORB_VERSION)))

OMNIORB_ZIOP_DYNAMIC_DLL_NAME = $(shell $(SharedLibraryFullName) $(subst ., ,omniZIOPDynamic.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_ZIOP_DYNAMIC_DLL_NAME = $(shell $(SharedLibraryDebugFullName) $(subst ., ,omniZIOPDynamic.$(OMNIORB_VERSION)))

OMNIORB_COS_DLL_NAME = $(shell $(SharedLibraryFullName) $(subst ., ,COS.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_COS_DLL_NAME = $(shell $(SharedLibraryDebugFullName) $(subst ., ,COS.$(OMNIORB_VERSION)))

OMNIORB_COS_DYNAMIC_DLL_NAME = $(shell $(SharedLibraryFullName) $(subst ., ,COSDynamic.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_COS_DYNAMIC_DLL_NAME = $(shell $(SharedLibraryDebugFullName) $(subst ., ,COSDynamic.$(OMNIORB_VERSION)))


ifndef BuildDebugBinary

omniorb_dll_name := $(OMNIORB_DLL_NAME)
omnidynamic_dll_name := $(OMNIORB_DYNAMIC_DLL_NAME)
omnicodesets_dll_name := $(OMNIORB_CODESETS_DLL_NAME)
omniconnections_dll_name := $(OMNIORB_CONNECTIONS_DLL_NAME)
omniziop_dll_name := $(OMNIORB_ZIOP_DLL_NAME)
omniziopdynamic_dll_name := $(OMNIORB_ZIOP_DYNAMIC_DLL_NAME)
omnicos_dll_name := $(OMNIORB_COS_DLL_NAME)
omnicosdynamic_dll_name := $(OMNIORB_COS_DYNAMIC_DLL_NAME)

else

omniorb_dll_name := $(OMNIORB_DEBUG_DLL_NAME)
omnidynamic_dll_name := $(OMNIORB_DEBUG_DYNAMIC_DLL_NAME)
omnicodesets_dll_name := $(OMNIORB_DEBUG_CODESETS_DLL_NAME)
omniconnections_dll_name := $(OMNIORB_DEBUG_CONNECTIONS_DLL_NAME)
omniziop_dll_name := $(OMNIORB_DEBUG_ZIOP_DLL_NAME)
omniziopdynamic_dll_name := $(OMNIORB_DEBUG_ZIOP_DYNAMIC_DLL_NAME)
omnicos_dll_name := $(OMNIORB_DEBUG_COS_DLL_NAME)
omnicosdynamic_dll_name := $(OMNIORB_DEBUG_COS_DYNAMIC_DLL_NAME)

endif

lib_depend := $(omniorb_dll_name)
omniORB_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(omnidynamic_dll_name)
omniDynamic_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(omnicodesets_dll_name)
omniCodeSets_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(omniconnections_dll_name)
omniConnections_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(omniziop_dll_name)
omniZIOP_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(omniziopdynamic_dll_name)
omniZIOPDynamic_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(omnicos_dll_name)
COS_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(omnicosdynamic_dll_name)
COSDynamic_lib_depend := $(GENERATE_LIB_DEPEND)

OMNIIDL = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/oidlwrapper.exe $(XLN)
OMNIORB_IDL_ONLY = $(OMNIIDL) -T -bcxx -Wbh=.hh -Wbs=SK.cc
OMNIORB_IDL_ANY_FLAGS = -Wba
OMNIORB_IDL = $(OMNIORB_IDL_ONLY) $(OMNIORB_IDL_ANY_FLAGS)
OMNIORB_CPPFLAGS = -D__OMNIORB4__ -I$(CORBA_STUB_DIR) $(OMNITHREAD_CPPFLAGS)
OMNIORB_IDL_OUTPUTDIR_PATTERN = -C%

msvc_work_around_stub = $(patsubst %,$(LibPattern),msvcstub) \
                        -NODEFAULTLIB:libcmt.lib -NODEFAULTLIB:libcmtd.lib 

OMNIORB_LIB = $(omniorb_dll_name) \
		$(omnidynamic_dll_name) \
		$(OMNITHREAD_LIB) $(SOCKET_LIB) advapi32.lib
OMNIORB_LIB_NODYN = $(omniorb_dll_name) $(msvc_work_around_stub) \
		$(OMNITHREAD_LIB) $(SOCKET_LIB) advapi32.lib

OMNIORB_LIB_NODYN_DEPEND := $(omniORB_lib_depend) \
                            $(OMNITHREAD_LIB_DEPEND)
OMNIORB_LIB_DEPEND := $(omniORB_lib_depend) \
                      $(OMNITHREAD_LIB_DEPEND) \
		      $(omniDynamic_lib_depend)

# CodeSets library
OMNIORB_CODESETS_LIB = $(omnicodesets_dll_name)
OMNIORB_CODESETS_LIB_DEPEND := $(omniCodeSets_lib_depend)

# Connections library
OMNIORB_CONNECTIONS_LIB = $(omniconnections_dll_name)
OMNIORB_CONNECTIONS_LIB_DEPEND := $(omniConnections_lib_depend)

# ZIOP library
OMNIORB_ZIOP_LIB = $(omniziop_dll_name)
OMNIORB_ZIOP_LIB_DEPEND := $(omniZiop_lib_depend)

OMNIORB_ZIOP_DYNAMIC_LIB = $(omniziopdynamic_dll_name)
OMNIORB_ZIOP_DYNAMIC_LIB_DEPEND := $(omniZiopDynamic_lib_depend)

# COS library
OMNIORB_COS_LIB = $(omnicos_dll_name)
OMNIORB_COS_LIB_DEPEND := $(COS_lib_depend)

OMNIORB_COS_DYNAMIC_LIB = $(omnicosdynamic_dll_name)
OMNIORB_COS_DYNAMIC_LIB_DEPEND := $(COSDynamic_lib_depend)


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


CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#
# Note that the DLL version is being used, so link to omnithread_rt.lib

# Use native win32 threads
ThreadSystem = NT

# Use pthread_nt, comment out ThreadSystem line above and uncomment the
# following 2 lines.
#ThreadSystem = NTPosix
#OMNITHREAD_CPPFLAGS= -D__POSIX_NT__

OMNITHREAD_LIB = $(patsubst %,$(DLLSearchPattern),omnithread$(OMNITHREAD_MAJOR_VERSION)$(OMNITHREAD_MINOR_VERSION))
lib_depend := $(patsubst %,$(DLLPattern),omnithread$(OMNITHREAD_MAJOR_VERSION)$(OMNITHREAD_MINOR_VERSION))
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)

OMNITHREAD_PLATFORM_LIB =


# omniORB SSL transport
OMNIORB_SSL_VERSION = $(OMNIORB_VERSION)
OMNIORB_SSL_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNIORB_SSL_VERSION)))
OMNIORB_SSL_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNIORB_SSL_VERSION)))
OMNIORB_SSL_MICRO_VERSION = $(word 3,$(subst ., ,$(OMNIORB_SSL_VERSION)))

OMNIORB_SSL_LIB = $(patsubst %,$(DLLSearchPattern),omnisslTP$(OMNIORB_SSL_MAJOR_VERSION)$(OMNIORB_SSL_MINOR_VERSION)$(OMNIORB_SSL_MICRO_VERSION))

lib_depend := $(patsubst %,$(DLLPattern),omnisslTP$(OMNIORB_SSL_MAJOR_VERSION)$(OMNIORB_SSL_MINOR_VERSION)$(OMNIORB_SSL_MICRO_VERSION))
OMNIORB_SSL_LIB_DEPEND := $(GENERATE_LIB_DEPEND)
