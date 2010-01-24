#--- No modifiable parameter in this file

include ../make.vars

#--------------------------------------------------------------------

TARGETS = managelogs

MANAGELOGS_LOBJS = managelogs.lo \
	intr.lo \
	options.lo \
	id.lo

LOBJS = $(MANAGELOGS_LOBJS)

INCLUDES = -I../include

#--------------------------------------------------------------------

.PHONY: all clean install
.SUFFIXES: .c .lo

all: $(TARGETS)

clean:
	$(LIBTOOL) --mode=clean /bin/rm -f $(TARGETS) $(LOBJS)

install: $(TARGETS)
	$(LIBTOOL) --mode=install cp $(TARGETS) $(APACHE)/bin

managelogs: $(MANAGELOGS_LOBJS)
	$(LINK_PROG) $(MANAGELOGS_LOBJS) -L../lib -llogmanager \
		-rpath $(APACHE)/lib$(LIB_SUFFIX)

.c.lo:
	$(LT_COMPILE_STATIC)

#--------------------------------------------------------------------
