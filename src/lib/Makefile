
#--- No modifiable parameter in this file

include ../../make.vars

#--------------------------------------------------------------------

LIB_TARGETS = liblogmanager.la

HEADER_FILES = ./ext_include/logmanager.h

LIB_INSTALL_DIR = $(APACHE)/lib$(LIB_SUFFIX)

TARGETS = $(LIB_TARGETS)

LOBJS = logmanager.lo

INCLUDES = -I../util -I./ext_include

#--------------------------------------------------------------------

.PHONY: all clean install clean-lib install-lib lib
.SUFFIXES: .c .lo

all: lib

clean: clean-lib

clean-lib:
	$(LIBTOOL) --mode=clean /bin/rm -f $(TARGETS) $(LOBJS)

install: install-lib

install-lib: lib
	@echo Installing library...
	@test -d $(LIB_INSTALL_DIR) || $(MKINSTALLDIRS) $(LIB_INSTALL_DIR)
	$(LIBTOOL) --mode=install cp $(LIB_TARGETS) $(LIB_INSTALL_DIR)
	@echo Installing header files...
	@test -d $(APACHE)/include || $(MKINSTALLDIRS) $(APACHE)/include
	$(LIBTOOL) --mode=install cp $(HEADER_FILES) $(APACHE)/include

lib: $(TARGETS)

$(TARGETS): $(LOBJS)
	$(LINK_LIB) $(LOBJS) -rpath $(LIB_INSTALL_DIR)
	
.c.lo:
	$(LT_COMPILE)

splint:
	splint -DPATH_MAX=128 $(CFLAGS) $(INCLUDES) -warnposix \
	-booltype  BOOL -boolfalse NO -booltrue YES -nullpass logmanager.c

#--------------------------------------------------------------------
