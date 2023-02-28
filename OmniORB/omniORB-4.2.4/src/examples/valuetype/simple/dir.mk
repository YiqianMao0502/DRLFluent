CXXSRCS = vclient.cc vserver.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

DIR_IDLFLAGS = -Wbdebug

CXXDEBUGFLAGS = -g

CORBA_INTERFACES = value

vclient = $(patsubst %,$(BinPattern),vclient)
vserver = $(patsubst %,$(BinPattern),vserver)
vcoloc  = $(patsubst %,$(BinPattern),vcoloc)

all:: $(vclient) $(vserver) $(vcoloc)

clean::
	$(RM) $(vclient) $(vserver)

$(vclient): vclient.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(vserver): vserver.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(vcoloc): vcoloc.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
