TOP=..
CURRENT=include
BASE_OMNI_TREE=../..
VPATH=../../include
INSTALL=/usr/bin/install -c

INSTALLDIR = $(INSTALLINCDIR)

HEADERS = omnithread.h omniconfig.h

SUBDIRS = omnithread omniORB4

include $(TOP)/mk/beforeauto.mk

install:: $(HEADERS)
	@(dir="$(INSTALLDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done )

install::
	@$(MakeSubdirs)

include $(TOP)/mk/afterauto.mk
