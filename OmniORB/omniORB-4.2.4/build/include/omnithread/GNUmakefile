TOP=../..
CURRENT=include/omnithread
BASE_OMNI_TREE=../../..
VPATH=../../../include/omnithread
INSTALL=/usr/bin/install -c

INSTALLDIR = $(INSTALLINCDIR)/omnithread

HEADERS = nt.h posix.h pthread_nt.h atomic.h

include $(TOP)/mk/beforeauto.mk

install:: $(HEADERS)
	@(dir="$(INSTALLDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done )

include $(TOP)/mk/afterauto.mk
