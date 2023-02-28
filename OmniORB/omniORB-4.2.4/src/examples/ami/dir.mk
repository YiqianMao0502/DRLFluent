CXXSRCS = echo_callback.cc echo_poller.cc echo_pollable_set.cc echo_dii_pollable_set.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo_ami

OMNIORB_IDL += -Wbami

echo_callback         = $(patsubst %,$(BinPattern),echo_callback)
echo_poller           = $(patsubst %,$(BinPattern),echo_poller)
echo_pollable_set     = $(patsubst %,$(BinPattern),echo_pollable_set)
echo_dii_pollable_set = $(patsubst %,$(BinPattern),echo_dii_pollable_set)

all:: $(echo_callback) $(echo_poller) $(echo_pollable_set) $(echo_dii_pollable_set)

clean::
	$(RM) $(echo_callback) $(echo_poller) $(echo_pollable_set) $(echo_dii_pollable_set)

export:: $(echo_callback) $(echo_poller) $(echo_pollable_set) $(echo_dii_pollable_set)
	@(module="echoexamples"; $(ExportExecutable))

$(echo_callback): echo_callback.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(echo_poller): echo_poller.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(echo_pollable_set): echo_pollable_set.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(echo_dii_pollable_set): echo_dii_pollable_set.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
