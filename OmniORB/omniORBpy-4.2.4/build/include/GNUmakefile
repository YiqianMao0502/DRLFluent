TOP=..
CURRENT=include
BASE_OMNI_TREE=../..
VPATH=../../include
INSTALL=/usr/bin/install -c

INSTALLDIR = $(INSTALLINCDIR)

HEADERS = omniORBpy.h

include $(TOP)/mk/beforeauto.mk

install:: $(HEADERS)
	@(dir="$(INSTALLDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done )

include $(TOP)/mk/afterauto.mk
