SUBDIRS = omnithread omniORB

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif

ciao::
	@$(MakeSubdirs)

ifndef EmbeddedSystem
ifdef Win32Platform
ifndef MinGW32Build
export::
	(cd $(EXPORT_TREE)/$(BINDIR); editbin /REBASE:BASE=0x68000000,DOWN *_rt.dll; )
	(cd $(EXPORT_TREE)/$(BINDIR); editbin /REBASE:BASE=0x68000000,DOWN *_rtd.dll; )
endif
endif
endif
