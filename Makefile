#-------------- Modifiable parameters -------------------------------

#-- Apache install directory

APACHE = /logi/http/apache

#-- C compiler command and options

CC = gcc

CFLAGS = -g -Wall -pthread $(PEDANTIC)

#PEDANTIC = -pedantic

#------

#-- Enable Gzip :
GZIP_OPT =
GZIP_LIBS = -lz
#-- Disable Gzip :
#GZIP_OPT = -D DISABLE_GZIP
#GZIP_LIBS =

#-- Enable Bzip2 :
BZ2_OPT =
BZ2_LIBS = -lbz2
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

INCLUDES = -I$(APACHE)/include/apr-0 $(GZIP_OPT) $(BZ2_OPT)

LIBS = -L $(APACHE)/lib -lapr-0 $(GZIP_LIBS) $(BZ2_LIBS)

INSTALL_DIR = $(APACHE)/bin

#--------------------------------------------------------------------

.PHONY: all clean install splint

all: $(TARGETS)

clean:
	/bin/rm -f $(TARGETS) $(OBJS)

install: $(TARGETS)
	cp $(TARGETS) $(INSTALL_DIR)

managelogs: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	
.c.o:
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(INCLUDES) $<

splint:
	splint $(CPPFLAGS) $(INCLUDES) +posixlib -DPATH_MAX=1024 \
		-skipposixheaders -booltype BOOL -boolfalse NO -booltrue YES \
		*.c


#--------------------------------------------------------------------
