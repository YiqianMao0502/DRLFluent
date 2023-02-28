
CXXSRCS = eg2_impl.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = becho

# -WbBOA tells the IDL compiler to generate BOA skeletons
OMNIORB_IDL += -WbBOA


eg2_impl   = $(patsubst %,$(BinPattern),eg2_impl)


all:: $(eg2_impl)

export:: $(eg2_impl)

clean::
	$(RM) $(eg2_impl)

$(eg2_impl): eg2_impl.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))
