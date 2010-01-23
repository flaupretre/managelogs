#-------------- Modifiable parameters -------------------------------

#-- External libs install directory

APACHE = /logi/http/apache
ZLIB = /logi/http/libs/z
BZ2LIB = /logi/http/libs/bz2

#-- C compiler command and options

CC = gcc
OPTS = -O2
#PEDANTIC = -pedantic
CFLAGS = -g -Wall -pthread $(PEDANTIC) $(OPTS)

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
#BZ2_OPT = -D DISABLE_BZ2
#BZ2_LIBS =

#-------------- End of modifiable parameters ------------------------

TARGETS = managelogs

OBJS = managelogs.o \
	logfile.o \
	file.o \
	util.o \
	compress.o \
	gzip_handler.o \
	bzip2_handler.o

INCLUDES = -I$(APACHE)/include/apr-0 -I$(ZLIB)/include -I$(BZ2LIB)/include

COMP_FLAGS = $(GZIP_OPT) $(BZ2_OPT)

LIBS = -L $(APACHE)/lib -lapr-0 $(GZIP_LIBS) $(BZ2_LIBS)

INSTALL_DIR = $(APACHE)/bin

#--------------------------------------------------------------------

.PHONY: all clean install splint doc cleandoc cleanall

all: $(TARGETS)

clean:
	/bin/rm -f $(TARGETS) $(OBJS)

cleanall: clean cleandoc

install: $(TARGETS)
	cp $(TARGETS) $(INSTALL_DIR)

managelogs: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	
.c.o:
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(COMP_FLAGS) $(INCLUDES) $<

splint:
	splint $(CPPFLAGS) $(INCLUDES) $(COMP_FLAGS) +posixlib -DPATH_MAX=1024 \
		-skipposixheaders -booltype BOOL -boolfalse NO -booltrue YES \
		*.c

doc: doc/_managelogs.htm

doc/_managelogs.htm:
	php /acp/prg/ppc.php doc/src/*.htm

cleandoc:
	/bin/rm -f doc/_managelogs.htm

distrib: cleanall doc

#--------------------------------------------------------------------
