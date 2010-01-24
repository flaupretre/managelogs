
#--- No modifiable parameter in this file

include ../make.vars

#--------------------------------------------------------------------

TARGETS = $(TARGET_LIB)

TARGET_LIB = liblogmanager.la

LOBJS = logmanager.lo \
	file.lo \
	time.lo \
	util.lo \
	compress.lo \
	gzip_handler.lo \
	bzip2_handler.lo

#--------------------------------------------------------------------

.PHONY: all clean install
.SUFFIXES: .c .lo

all: $(TARGETS)

clean:
	$(LIBTOOL) --mode=clean /bin/rm -f $(TARGETS) $(LOBJS)

install: install-lib install-include

install-lib: $(TARGET_LIB)
	$(LIBTOOL) --mode=install cp $(TARGET_LIB) $(APACHE)/lib$(LIB_SUFFIX)

install-include:
	$(LIBTOOL) --mode=install cp ../include/logmanager.h $(APACHE)/include

$(TARGET_LIB): $(LOBJS)
	$(LINK_LIB) $(LOBJS) -rpath $(APACHE)/lib$(LIB_SUFFIX)
	
.c.lo:
	$(LT_COMPILE)

#--------------------------------------------------------------------
