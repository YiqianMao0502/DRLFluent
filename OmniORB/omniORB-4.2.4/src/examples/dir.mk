SUBDIRS = echo poa boa thread anyExample dii dsi call_back valuetype bidir ami

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
