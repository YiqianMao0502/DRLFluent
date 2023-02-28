include $(BASE_OMNI_TREE)/mk/python.mk

IDLMODULE_MAJOR   = $(OMNIORB_MAJOR_VERSION)
IDLMODULE_MINOR   = $(OMNIORB_MINOR_VERSION)
IDLMODULE_VERSION = 0x2630# => CORBA 2.6, front-end 3.0

DIR_CPPFLAGS += -DIDLMODULE_VERSION="\"$(IDLMODULE_VERSION)\""


ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif


SUBDIRS = cccp

all::
	@$(MakeSubdirs)
export::
	@$(MakeSubdirs)

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif

OBJS  = y.tab.o lex.yy.o idlerr.o idlutil.o idltype.o \
	idlrepoId.o idlscope.o idlexpr.o idlast.o idlvalidate.o \
	idldump.o idlconfig.o idlfixed.o

PYOBJS = idlpython.o

CXXSRCS = y.tab.cc lex.yy.cc idlerr.cc idlutil.cc idltype.cc \
	idlrepoId.cc idlscope.cc idlexpr.cc idlast.cc idlvalidate.cc \
	idldump.cc idlconfig.cc idlfixed.cc idlpython.cc idlc.cc

YYSRC = idl.yy
LLSRC = idl.ll

FLEX = flex -t
BISON = bison -d -o y.tab.c

idlc = $(patsubst %,$(BinPattern),idlc)

# y.tab.h y.tab.cc: $(YYSRC)
# 	@-$(RM) $@
# 	$(BISON) $<
# 	mv -f y.tab.c y.tab.cc

# lex.yy.cc: $(LLSRC) y.tab.h
# 	$(FLEX) $< | sed -e 's/^#include <unistd.h>//' -e 's/<stdout>/lex.yy.cc/' > $@
# 	echo '#ifdef __VMS' >> $@
# 	echo '// Some versions of DEC C++ for OpenVMS set the module name used by the' >> $@
# 	echo '// librarian based on the last #line encountered.' >> $@
# 	echo '#line' `cat $@ | wc -l` '"lex_yy.cc"' >> $@
# 	echo '#endif' >> $@

#############################################################################
#   Test executable                                                         #
#############################################################################

# all:: $(idlc)

# $(idlc): $(OBJS) idlc.o
# 	@(libs=""; $(CXXExecutable))



#############################################################################
#   Make variables for Unix platforms                                       #
#############################################################################

ifdef UnixPlatform
#CXXDEBUGFLAGS = -g
endif

ifeq ($(platform),autoconf)

namespec := _omniidl$(PY_MODULE_SUFFIX) _ $(IDLMODULE_MAJOR) $(IDLMODULE_MINOR)

ifdef PythonSHAREDLIB_SUFFIX
SHAREDLIB_SUFFIX = $(PythonSHAREDLIB_SUFFIX)
endif

SharedLibraryFullNameTemplate = $$1$$2.$(SHAREDLIB_SUFFIX).$$3.$$4
SharedLibrarySoNameTemplate   = $$1$$2.$(SHAREDLIB_SUFFIX).$$3
SharedLibraryLibNameTemplate  = $$1$$2.$(SHAREDLIB_SUFFIX)

ifdef PythonLibraryPlatformLinkFlagsTemplate
SharedLibraryPlatformLinkFlagsTemplate = $(PythonLibraryPlatformLinkFlagsTemplate)
endif

shlib := $(shell $(SharedLibraryFullName) $(namespec))

DIR_CPPFLAGS += $(SHAREDLIB_CPPFLAGS)

#### ugly AIX section start
ifdef AIX

DIR_CPPFLAGS += -I. -I/usr/local/include -DNO_STRCASECMP

libinit = init_omniidl
py_exp = $(PYPREFIX)/lib/python$(PYVERSION)/config/python.exp
ld_so_aix = $(PYPREFIX)/lib/python$(PYVERSION)/config/ld_so_aix

ifdef Compiler_GCC
$(shlib): $(OBJS) $(PYOBJS)
	@(set -x; \
	$(RM) $@; \
	$(ld_so_aix) $(CXX) \
		-o $(shlib) \
		-e $(libinit) \
		-bI:$(py_exp) \
		-Wl,-blibpath:/lib:/usr/lib:$(prefix)/lib \
		$(IMPORT_LIBRARY_FLAGS) \
		$(filter-out $(LibSuffixPattern),$^); \
	)
else


# Previously, xlc builds used this, but modern xlc works correctly
# with the standard rule.

# CXXLINK = makeC++SharedLib_r
#
# $(shlib): $(OBJS) $(PYOBJS)
# 	@(set -x; \
# 	$(RM) $@; \
# 	$(CXXLINK) \
# 		-n $(libinit) \
# 		-o $(shlib) \
# 		-bI:$(py_exp) \
# 		$(IMPORT_LIBRARY_FLAGS) \
# 		-bhalt:4 -T512 -H512 \
# 		$(filter-out $(LibSuffixPattern),$^) \
# 		-p 40 \
# 		; \
# 	)
$(shlib): $(OBJS) $(PYOBJS)
	@(namespec="$(namespec)"; extralibs="$(extralibs)"; $(MakeCXXSharedLibrary))

endif

else
#### ugly AIX section end, normal build command

$(shlib): $(OBJS) $(PYOBJS)
	@(namespec="$(namespec)"; extralibs="$(extralibs)"; $(MakeCXXSharedLibrary))
endif

all:: $(shlib)

export:: $(shlib)
	@(namespec="$(namespec)"; $(ExportSharedLibrary))

ifdef INSTALLTARGET

install:: $(shlib)
	@(dir="$(INSTALLPYEXECDIR)"; namespec="$(namespec)"; \
          $(ExportSharedLibraryToDir))
endif

clean::
	$(RM) *.o
	(dir=.; $(CleanSharedLibrary))

veryclean::
	$(RM) *.o
	(dir=.; $(CleanSharedLibrary))

ifdef Cygwin

SharedLibraryPlatformLinkFlagsTemplate = -shared -Wl,-soname=$$soname,--export-dynamic,--enable-auto-import

extralibs += -L$(PYPREFIX)/lib/python$(PYVERSION)/config \
 -lpython$(PYVERSION).dll

endif

else

#############################################################################
#   Make rules for Windows                                                  #
#############################################################################

ifdef Win32Platform

DIR_CPPFLAGS += -DMSDOS -DOMNIIDL_EXECUTABLE

PYLIBDIR := $(PYPREFIX)/libs $(PYPREFIX)/lib/x86_win32

ifdef MinGW32Build
PYLIB     := -lpython$(subst .,,$(PYVERSION))
CXXLINKOPTIONS += $(patsubst %,-L%,$(PYLIBDIR))
else
PYLIB     := python$(subst .,,$(PYVERSION)).lib
CXXLINKOPTIONS += $(patsubst %,-libpath:%,$(PYLIBDIR))
endif

omniidl = $(patsubst %,$(BinPattern),omniidl)

all:: $(omniidl)

export:: $(omniidl)
	@$(ExportExecutable)

clean::
	$(RM) $(omniidl)

$(omniidl): $(OBJS) $(PYOBJS)
	@(libs="$(PYLIB)"; $(CXXExecutable))

endif



#
# Obsolete rules for non-autoconf builds
#


#############################################################################
#   Make rules for Linux                                                    #
#############################################################################

ifdef Linux

CXXOPTIONS += -fpic

libname = _omniidlmodule.so
soname = $(libname).$(IDLMODULE_MAJOR)
lib = $(soname).$(IDLMODULE_MINOR)

all:: $(lib)

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
	$(RM) $@; \
	$(CXXLINK) $(CXXLINKOPTIONS) -shared -o $@ -Wl,-soname,$(soname) $(IMPORT_LIBRARY_FLAGS) \
	 $(filter-out $(LibSuffixPattern),$^) $(LIBS)\
	)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

clean::
	$(RM) $(lib)

endif


#############################################################################
#   Make rules for Solaris 2.x                                              #
#############################################################################

ifdef SunOS

libname = _omniidlmodule.so
soname = $(libname).$(IDLMODULE_MAJOR)
lib = $(soname).$(IDLMODULE_MINOR)

ifeq ($(notdir $(CXX)),CC)

CXXOPTIONS   += -Kpic

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
	$(RM) $@; \
	if (CC -V 2>&1 | grep '5\.[0-9]'); \
	  then CXX_RUNTIME=-lCrun; \
	  else CXX_RUNTIME=-lC; \
        fi; \
        $(CXX) -ptv -G -o $@ -h $(soname) $(IMPORT_LIBRARY_FLAGS) \
         $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS)) \
         $(filter-out $(LibSuffixPattern),$^) -lposix4 -lnsl $$CXX_RUNTIME \
	)

endif

ifeq ($(notdir $(CXX)),g++)

CXXOPTIONS += -fPIC

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
	$(RM) $@; \
	$(CXXLINK) $(CXXLINKOPTIONS) -shared -o $@ -Wl-soname,$(soname) $(IMPORT_LIBRARY_FLAGS) \
	 $(filter-out $(LibSuffixPattern),$^) \
	)

endif

all:: $(lib)

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )
endif



#############################################################################
#   Make rules for AIX                                                      #
#############################################################################

ifdef AIX

DIR_CPPFLAGS += -I. -I/usr/local/include -DNO_STRCASECMP

lib = _omniidlmodule.so
libinit = init_omniidl
py_exp = $(PYPREFIX)/lib/python$(PYVERSION)/config/python.exp

ifeq ($(notdir $(CXX)),xlC_r)

$(lib): $(OBJS) $(PYOBJS)
	@(set -x; \
	$(RM) $@; \
	$(MAKECPPSHAREDLIB) \
	     -o $(lib) \
	     -bI:$(py_exp) \
	     -n $(libinit) \
	     $(IMPORT_LIBRARY_FLAGS) \
	     -bhalt:4 -T512 -H512 \
	     $(filter-out $(LibSuffixPattern),$^) \
	     -p 40 \
	 ; \
       )

all:: $(lib)

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)

endif

ifeq ($(notdir $(CXX)),g++)

# Build omniidl as an executable by linking in the Python runtime
# library.  Is this really necessary?  Can't we make a Python
# extension?

DIR_CPPFLAGS += -DOMNIIDL_EXECUTABLE
CXXLINKOPTIONS += -L$(PYTHONLIBDIR)

omniidl = $(patsubst %,$(BinPattern),omniidl)

all:: $(omniidl)

export:: $(omniidl)
	@$(ExportExecutable)

clean::
	$(RM) $(omniidl)

$(omniidl): $(OBJS) $(PYOBJS)
	@(libs="-lpython$(PYVERSION) -lpthread -lm"; $(CXXExecutable))

endif

endif


#############################################################################
#   Make rules for FreeBSD                                                    #
#############################################################################

ifdef FreeBSD

CXXOPTIONS += -fPIC

libname = _omniidlmodule.so
soname = $(libname).$(IDLMODULE_MAJOR)
lib = $(soname).$(IDLMODULE_MINOR)

all:: $(lib)

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
       $(RM) $@; \
       $(CXXLINK) $(CXXLINKOPTIONS) -shared -o $@ -Wl,-soname,$(soname) \
       $(IMPORT_LIBRARY_FLAGS) \
       $(filter-out $(LibSuffixPattern),$^) $(LIBS) -lgcc\
       )

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

clean::
	$(RM) $(lib)

endif



#############################################################################
#   Make rules for Digital Unix                                             #
#############################################################################


ifdef OSF1
ifeq ($(notdir $(CXX)),cxx)

libname = _omniidlmodule.so
soname  = $(libname).$(IDLMODULE_MAJOR)
lib = $(soname).$(IDLMODULE_MINOR)

all:: $(lib)

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
         $(RM) $@; \
         ld -shared -soname $(soname) -set_version $(soname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) -lcxxstd -lcxx -lexc -lots -lc \
        )


clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

endif
endif


#############################################################################
#   Make rules for HPUX                                                     #
#############################################################################

ifdef HPUX
ifeq ($(notdir $(CXX)),aCC)

# Note: the python installation must be built to load C++ shared library
#       this usually means that the main function of the python executable
#       is compiled and linked with aCC.

DIR_CPPFLAGS += +Z

ifdef ia64Processor
libname = _omniidlmodule.so
endif

ifndef ia64Processor
libname = _omniidlmodule.sl
endif

soname = $(libname).$(IDLMODULE_MAJOR)
lib = $(soname).$(IDLMODULE_MINOR)

all:: $(lib)

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
         $(RM) $@; \
         aCC -b -Wl,+h$(soname) -o $@  $(IMPORT_LIBRARY_FLAGS) \
           $(patsubst %,-L %,$(IMPORT_LIBRARY_DIRS)) \
           $(filter-out $(LibSuffixPattern),$^) ; \
        )

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

# The alternative is to build omniidl as an executable by linking in the
# python runtime library. Comment out the above and uncomment the following
# if this is preferable.
#
#
# PYLIBDIR := $(PYPREFIX)/lib
#
# DIR_CPPFLAGS += -DOMNIIDL_EXECUTABLE
# CXXLINKOPTIONS += -L$(PYLIBDIR)
#
#
# omniidl = $(patsubst %,$(BinPattern),omniidl)
#
# all:: $(omniidl)
#
# export:: $(omniidl)
# 	@$(ExportExecutable)
#
# clean::
# 	$(RM) $(omniidl)
#
# $(omniidl): $(OBJS) $(PYOBJS)
# 	@(libs="-lpython$(PYVERSION) -lpthread"; $(CXXExecutable))


endif
endif

#############################################################################
#   Make rules for to Reliant Unix                                          #
#############################################################################

# WARNING!  These make rules are untested

ifdef SINIX
ifeq ($(notdir $(CXX)),CC)

DIR_CPPFLAGS += -Kpic

libname = _omniidlmodule.so
soname = $(libname).$(IDLMODULE_MAJOR)
lib = $(soname).$(IDLMODULE_MINOR)

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
         $(RM) $@; \
         CC -G -z text -KPIC -o $@ -h $(soname) \
           $(IMPORT_LIBRARY_FLAGS) $($(IMPORT_LIBRARY_DIRS)) \
           $(filter-out $(LibSuffixPattern),$^); \
        )


all:: $(lib)

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

endif
endif

#############################################################################
#   Make rules for SGI Irix 6.2                                             #
#############################################################################

ifdef IRIX
ifeq ($(notdir $(CXX)),CC)

DIR_CPPFLAGS += -KPIC

ifdef IRIX_n32
ADD_CPPFLAGS = -n32
endif
ifdef IRIX_64
ADD_CPPFLAGS = -64
endif

libname = _omniidlmodule.so
soname = $(libname).$(IDLMODULE_MAJOR)
lib = $(soname).$(IDLMODULE_MINOR)

all:: $(lib)

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
         $(RM) $@; \
         $(LINK.cc) -KPIC -shared -Wl,-h,$(libname) \
           -Wl,-set_version,$(soname) -Wl,-rpath,$(LIBDIR) \
           -o $@ $(IMPORT_LIBRARY_FLAGS) $^ $(LDLIBS); \
        )


clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

endif
endif


#############################################################################
#   Make rules for NextStep                                                 #
#############################################################################

ifdef NextStep

PYPREFIX = $(shell $(PYTHON) -c "import sys; sys.stdout.write(sys.exec_prefix)")
CXXOPTIONS += -I$(PYPREFIX)/include
CXXLINKOPTIONS += -nostdlib -r
SO = .so
libname = _omniidlmodule$(SO)
soname  = $(libname).$(IDLMODULE_MAJOR)
lib     = $(soname).$(IDLMODULE_MINOR)

$(lib): $(OBJS) $(PYOBJS)
      $(CXXLINK) $(CXXLINKOPTIONS) $(OBJS) $(PYOBJS) -o $(lib)

all:: $(lib)

clean::
      $(RM) $(lib)

export:: $(lib)
      @$(ExportLibrary)
      @(set -x; 
              cd $(EXPORT_TREE)/$(LIBDIR); 
              $(RM) $(soname); 
              ln -s $(lib) $(soname); 
              $(RM) $(libname); 
              ln -s $(soname) $(libname); 
      )
endif


#############################################################################
#   Make rules for Darwin                                                   #
#############################################################################

ifdef Darwin

CXXOPTIONS += $(SHAREDLIB_CPPFLAGS)

libname = _omniidlmodule.so
soname  = _omniidlmodule.$(IDLMODULE_MAJOR).so
lib     = _omniidlmodule.$(IDLMODULE_MAJOR).$(IDLMODULE_MINOR).so

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
         $(RM) $@; \
         $(CXX) -bundle -flat_namespace -undefined suppress -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) $(LIBS) \
	)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
		cd $(EXPORT_TREE)/$(LIBDIR); \
		$(RM) $(soname); \
		ln -s $(lib) $(soname); \
		$(RM) $(libname); \
		ln -s $(soname) $(libname); \
	)

all:: $(lib)

clean::
	$(RM) $(lib)

endif

endif
