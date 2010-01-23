#-------------- Modifiable parameters -------------------------------

#-- External libs install directory

APACHE = /logi/http/apache
ZLIB = /logi/http/libs/z
BZ2LIB = /logi/http/libs/bz2

#-- C compiler command and options

CC = gcc
OPTS = -O2
#PEDANTIC = -pedantic
#THREAD= -pthread
CFLAGS = -g -Wall $(THREAD) $(PEDANTIC) $(OPTS)

#------

#-- Enable Gzip :
GZIP_OPT =
GZIP_LIBS = -L $(ZLIB)/lib -lz
#-- Disable Gzip :
#GZIP_OPT = -D DISABLE_GZIP
#GZIP_LIBS =

#-- Enable Bzip2 :
BZ2_OPT =
BZ2_LIBS = -L $(BZ2LIB)/lib -lbz2
#-- Disable Bzip2 :
#BZ2_LIBS =
#BZ2_OPT = -D DISABLE_BZ2

#-------------- End of modifiable parameters ------------------------

TARGETS = managelogs

MANAGELOGS_OBJS = managelogs.o \
	intr.o \
	options.o \
	id.o

INCLUDES = -Iinclude -I$(APACHE)/include/apr-0

LIBS = -L lib -llogmanager -L $(APACHE)/lib -lapr-0 $(GZIP_LIBS) $(BZ2_LIBS)

INSTALL_DIR = $(APACHE)/bin

MOPTS = APACHE="$(APACHE)" ZLIB="$(ZLIB)" BZ2LIB="$(BZ2LIB)" CC="$(CC)" \
	CFLAGS="$(CFLAGS)" GZIP_OPT="$(GZIP_OPT)" BZ2_OPT="$(BZ2_OPT)"

#--------------------------------------------------------------------

.PHONY: all clean install splint lib install-lib

all: $(TARGETS)

clean:
	cd lib && $(MAKE) clean
	/bin/rm -f $(TARGETS) $(MANAGELOGS_OBJS)

install: $(TARGETS) install-lib
	cp -p $(TARGETS) $(INSTALL_DIR)

managelogs: $(MANAGELOGS_OBJS) lib
	$(CC) $(LDFLAGS) -o $@ $(MANAGELOGS_OBJS) $(LIBS)

lib:
	cd lib && $(MAKE) $(MOPTS)

install-lib:
	cd lib && $(MAKE) $(MOPTS) install

.c.o:
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(INCLUDES) $<

splint:
	splint $(CPPFLAGS) $(INCLUDES) +posixlib -DPATH_MAX=1024 \
		-skipposixheaders -booltype BOOL -boolfalse NO -booltrue YES \
		*.c

#--------------------------------------------------------------------
