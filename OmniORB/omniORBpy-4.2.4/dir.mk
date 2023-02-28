PYSUBDIR = $(shell $(PYTHON) -c 'import sys; sys.stdout.write(sys.version[0] == "3" and "python3" or "python")')

SUBDIRS = modules $(PYSUBDIR) include

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif
