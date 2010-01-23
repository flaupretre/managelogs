#-------------- Modifiable parameters -------------------------------

#-- Apache install directory

APACHE = /logi/http/apache

#-- C compiler command

CC = gcc

#--------------------------------------------------------------------

CFLAGS = -g -Wall -pthread

TARGETS = managelogs

OBJS = managelogs.o logfile.o error.o gz.o plain.o

INCLUDES = -I $(APACHE)/include/apr-0

LIBS = -L $(APACHE)/lib -lapr-0 -lz

INSTALL_DIR = $(APACHE)/bin

#--------------------------------------------------------------------

.PHONY: all clean install

all: $(TARGETS)

clean:
	/bin/rm -f $(TARGETS) $(OBJS)

install: $(TARGETS)
	cp $(TARGETS) $(INSTALL_DIR)

managelogs: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	
.c.o:
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(INCLUDES) $<

#--------------------------------------------------------------------
