
#--- No modifiable parameter in this file

include ./make.vars

#--------------------------------------------------------------------

.PHONY: all lib prog install install-lib install-prog clean clean-lib clean-prog html-man install-man install-sys-man check

all lib prog install install-lib install-prog clean clean-lib clean-prog check:
	cd src && $(MAKE) $@

html-man install-man install-sys-man:
	cd doc && $(MAKE) $@

#--------------------------------------------------------------------
