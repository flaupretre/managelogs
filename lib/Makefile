
#--- No modifiable parameter in this file

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
	cp -p $(LIB) $(APACHE)/lib
	cp -p ../include/logmanager.h $(APACHE)/include

$(LIB): $(OBJS)
	$(AR) cr $(LIB) $(OBJS)
	
.c.o:
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(COMP_FLAGS) $(INCLUDES) $<

splint:
	splint $(CPPFLAGS) $(INCLUDES) $(COMP_FLAGS) +posixlib -DPATH_MAX=1024 \
		-skipposixheaders -booltype BOOL -boolfalse NO -booltrue YES \
		*.c

#--------------------------------------------------------------------
