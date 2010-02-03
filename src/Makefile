#--- No modifiable parameter in this file

include ../make.vars

#--------------------------------------------------------------------

TARGETS = managelogs

LOBJS = managelogs.lo \
	intr.lo \
	options.lo \
	id.lo

INCLUDES = -I../lib/ext_include

#--------------------------------------------------------------------

.PHONY: all clean install
.SUFFIXES: .c .lo

all: $(TARGETS)

install: install-lib install-prog

clean:
	$(LIBTOOL) --mode=clean /bin/rm -f $(TARGETS) $(LOBJS)

install: all
	@echo Installing program...
	@test -d $(APACHE)/bin || $(MKINSTALLDIRS) $(APACHE)/bin
	$(LIBTOOL) --mode=install cp $(TARGETS) $(APACHE)/bin

managelogs: $(LOBJS)
	$(LINK_PROG) $(LOBJS) -L../lib -llogmanager \
		-rpath $(APACHE)/lib$(LIB_SUFFIX)

check:
	@echo '<TODO>'

.c.lo:
	$(LT_COMPILE)

#--------------------------------------------------------------------
