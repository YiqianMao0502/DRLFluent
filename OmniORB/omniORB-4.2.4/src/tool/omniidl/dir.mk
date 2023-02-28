ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif

PYSUBDIR = $(shell $(PYTHON) -c 'import sys; sys.stdout.write(sys.version[0] == "3" and "python3" or "python")')

SUBDIRS = cxx $(PYSUBDIR)

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif
