
#--- No modifiable parameter in this file

include ./make.vars

#--------------------------------------------------------------------

SUBDIRS = lib src

#--------------------------------------------------------------------

.PHONY: all clean install prog lib install-prog install-lib

all: lib prog

clean:
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit 1; done

install: install-lib install-prog

install-lib:
	cd lib && $(MAKE) install

install-prog:
	cd src && $(MAKE) install

lib:
	cd lib && $(MAKE)

prog:
	cd src && $(MAKE)

install-man:
	cp doc/managelogs.8 $(APACHE)/man/man8

install-sys-man:
	cp doc/managelogs.8 /usr/share/man/man8

#--------------------------------------------------------------------
