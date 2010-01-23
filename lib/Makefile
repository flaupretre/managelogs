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
#-- Disable Gzip :
#GZIP_OPT = -D DISABLE_GZIP

#-- Enable Bzip2 :
BZ2_OPT =
#-- Disable Bzip2 :
#BZ2_OPT = -D DISABLE_BZ2

#-------------- End of modifiable parameters ------------------------

TARGETS = managelogs

OBJS = logmanager.o \
	file.o \
	time.o \
	util.o \
	compress.o \
	gzip_handler.o \
	bzip2_handler.o

INCLUDES = -I$(APACHE)/include/apr-0 -I$(ZLIB)/include -I$(BZ2LIB)/include

COMP_FLAGS = $(GZIP_OPT) $(BZ2_OPT)

LIB = liblogmanager.a

TARGETS = $(LIB)

#--------------------------------------------------------------------

.PHONY: all clean install splint

all: $(TARGETS)

clean:
	/bin/rm -f $(TARGETS) $(OBJS)

install: $(TARGETS)
	cp $(LIB) $(APACHE)/lib

$(LIB): $(OBJS)
	$(AR) cr $(LIB) $(OBJS)
	
.c.o:
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(COMP_FLAGS) $(INCLUDES) $<

splint:
	splint $(CPPFLAGS) $(INCLUDES) $(COMP_FLAGS) +posixlib -DPATH_MAX=1024 \
		-skipposixheaders -booltype BOOL -boolfalse NO -booltrue YES \
		*.c

#--------------------------------------------------------------------
