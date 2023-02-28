PYLIBROOT= $(EXPORT_TREE)/lib/python
PYLIBDIR = $(PYLIBROOT)/omniORB
INSTALLPYLIBDIR = $(INSTALLPYTHONDIR)/omniORB

PYSUBDIR = $(shell $(PYTHON) -c 'import sys; sys.stdout.write(sys.version[0] == "3" and "python3" or "python")')

ir_idl.py: ir.idl
	$(OMNIIDL) -v -p$(BASE_OMNI_TREE)/$(PYSUBDIR)/omniidl_be \
        -I$(BASE_OMNI_TREE)/idl \
        -I$(OMNIORB_ROOT)/idl/omniORB \
        -I$(OMNIORB_ROOT)/share/idl/omniORB \
        -I$(DATADIR)/idl/omniORB \
        -bpython -Wbno_package $^

corbaidl_idl.py: corbaidl.idl
	$(OMNIIDL) -v -p$(BASE_OMNI_TREE)/$(PYSUBDIR)/omniidl_be \
        -I$(BASE_OMNI_TREE)/idl \
        -I$(OMNIORB_ROOT)/idl/omniORB \
        -I$(OMNIORB_ROOT)/share/idl/omniORB \
        -I$(DATADIR)/idl/omniORB \
        -bpython -nf -Wbno_package $^

boxes_idl.py: boxes.idl
	$(OMNIIDL) -v -p$(BASE_OMNI_TREE)/$(PYSUBDIR)/omniidl_be \
        -I$(BASE_OMNI_TREE)/idl \
        -I$(OMNIORB_ROOT)/idl/omniORB \
        -I$(OMNIORB_ROOT)/share/idl/omniORB \
        -I$(DATADIR)/idl/omniORB \
        -bpython -nf -Wbno_package $^

pollable_idl.py: pollable.idl
	$(OMNIIDL) -v -p$(BASE_OMNI_TREE)/$(PYSUBDIR)/omniidl_be \
        -I$(BASE_OMNI_TREE)/idl \
        -I$(OMNIORB_ROOT)/idl/omniORB \
        -I$(OMNIORB_ROOT)/share/idl/omniORB \
        -I$(DATADIR)/idl/omniORB \
        -bpython -nf -Wbno_package $^

messaging_idl.py: messaging.idl
	$(OMNIIDL) -v -p$(BASE_OMNI_TREE)/$(PYSUBDIR)/omniidl_be \
        -I$(BASE_OMNI_TREE)/idl \
        -I$(OMNIORB_ROOT)/idl/omniORB \
        -I$(OMNIORB_ROOT)/share/idl/omniORB \
        -I$(DATADIR)/idl/omniORB \
        -bpython -nf -Wbno_package $^

compression_idl.py: compression.idl
	$(OMNIIDL) -v -p$(BASE_OMNI_TREE)/$(PYSUBDIR)/omniidl_be \
        -I$(BASE_OMNI_TREE)/idl \
        -I$(OMNIORB_ROOT)/idl/omniORB \
        -I$(OMNIORB_ROOT)/share/idl/omniORB \
        -I$(DATADIR)/idl/omniORB \
        -bpython -nf -Wbno_package -Wbmodules=omniORB $^

ziop_idl.py: ziop.idl
	$(OMNIIDL) -v -p$(BASE_OMNI_TREE)/$(PYSUBDIR)/omniidl_be \
        -I$(BASE_OMNI_TREE)/idl \
        -I$(OMNIORB_ROOT)/idl/omniORB \
        -I$(OMNIORB_ROOT)/share/idl/omniORB \
        -I$(DATADIR)/idl/omniORB \
        -bpython -nf -Wbno_package -Wbmodules=omniORB $^

minorfile := $(shell file="$(INCDIR)/omniORB4/minorCode.h"; \
               dirs="$(IMPORT_TREES)"; \
               $(FindFileInDirs); \
               echo "$$fullfile")

ifeq ($(platform),autoconf)
MAKEMINORS = $(BASE_OMNI_TREE)/bin/scripts/makeminors.py
else
MAKEMINORS = $(TOP)/$(CURRENT)/../../bin/scripts/makeminors.py
endif

minorCodes.py: $(minorfile)
	$(PYTHON) $(MAKEMINORS) $^ $@

all:: corbaidl_idl.py ir_idl.py boxes_idl.py pollable_idl.py messaging_idl.py \
      compression_idl.py ziop_idl.py \
      minorCodes.py

clean::
	$(RM) corbaidl_idl.py ir_idl.py boxes_idl.py pollable_idl.py
	$(RM) messaging_idl.py compression_idl.py ziop_idl.py minorCodes.py


FILES = __init__.py CORBA.py PortableServer.py PortableServer__POA.py \
        tcInternal.py URI.py codesets.py any.py BiDirPolicy.py \
        interceptors.py ami.py Compression.py ZIOP.py omniZIOP.py \
        corbaidl_idl.py ir_idl.py boxes_idl.py \
        pollable_idl.py messaging_idl.py compression_idl.py ziop_idl.py \
        minorCodes.py omniConnectionMgmt.py omniPolicy.py

ifdef OPEN_SSL_ROOT
FILES += sslTP.py
endif

export:: $(FILES)
	@(dir="$(PYLIBDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done; \
          cd $(PYLIBDIR); \
	  $(PYTHON) -c "import compileall; compileall.compile_dir('.')"; \
	 )

ifdef INSTALLTARGET

install:: $(FILES)
	@(dir="$(INSTALLPYLIBDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done; \
          cd $(INSTALLPYLIBDIR); \
	  $(PYTHON) -c "import compileall; compileall.compile_dir('.')"; \
	 )
endif
