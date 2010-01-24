#--- No modifiable parameter in this file

include ../make.vars

#--------------------------------------------------------------------

TARGETS = managelogs

LOBJS = managelogs.lo \
	intr.lo \
	options.lo \
	id.lo

INCLUDES = -Ilib/ext_include

#--------------------------------------------------------------------

.PHONY: all clean install clean-prog lib clean-lib install-lib install-prog prog
.SUFFIXES: .c .lo

all: lib prog

clean: clean-lib clean-prog

install: install-lib install-prog

clean-prog:
	$(LIBTOOL) --mode=clean /bin/rm -f $(TARGETS) $(LOBJS)

lib clean-lib install-lib:
	cd lib && $(MAKE) $@

install-prog: prog
	@echo Installing program...
	@test -d $(APACHE)/bin || $(MKINSTALLDIRS) $(APACHE)/bin
	$(LIBTOOL) --mode=install cp $(TARGETS) $(APACHE)/bin

prog: $(TARGETS)

managelogs: $(LOBJS)
	$(LINK_PROG) $(LOBJS) -L./lib -llogmanager \
		-rpath $(APACHE)/lib$(LIB_SUFFIX)

check:
	@echo '<TODO>'

.c.lo:
	$(LT_COMPILE_STATIC)

#--------------------------------------------------------------------
