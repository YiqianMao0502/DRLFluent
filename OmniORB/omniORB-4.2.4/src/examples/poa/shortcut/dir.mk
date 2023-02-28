CXXSRCS = shortcut.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)
CXXDEBUGFLAGS = -g -O3

DIR_IDLFLAGS += -Wbshortcut
CORBA_INTERFACES = shortcut

shortcut = $(patsubst %,$(BinPattern),shortcut)

all:: $(shortcut)

clean::
	$(RM) $(shortcut)

export:: $(shortcut)
	$(ExportExecutable)

$(shortcut): shortcut.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))
