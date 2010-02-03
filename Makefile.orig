
#--- No modifiable parameter in this file

include ./make.vars

#--------------------------------------------------------------------

.PHONY: all install clean html-man install-man install-sys-man check

all install clean check:
	cd lib && $(MAKE) $@
	cd src && $(MAKE) $@

html-man install-man install-sys-man:
	cd doc && $(MAKE) $@

#--------------------------------------------------------------------
