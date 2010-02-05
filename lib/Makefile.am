
lib_LTLIBRARIES = liblogmanager.la

liblogmanager_la_SOURCES = logmanager.c

AM_CPPFLAGS = $(APR_INCLUDES) $(APR_CPPFLAGS)
AM_CFLAGS = $(APR_CFLAGS) $(Z_CFLAGS) $(BZ2_CFLAGS)
AM_LDFLAGS = $(APR_LDFLAGS)

include_HEADERS = inst_include/logmanager.h

noinst_HEADERS = include/bzip2_handler.h \
	include/compress.h \
	include/config.h \
	include/file.h \
	include/gzip_handler.h \
	include/time.h \
	bzip2_handler.c \
	compress.c \
	file.c \
	gzip_handler.c \
	logmanager.c \
	time.c \
	$(top_srcdir)/util/util.c \
	$(top_srcdir)/util/util.h

liblogmanager_la_LIBADD = $(APR_LIBS) $(Z_LIBS) $(BZ2_LIBS)
liblogmanager_la_LDFLAGS = -version-info $(LIB_LT_VERSION):$(LIB_LT_REVISION):$(LIB_LT_AGE)

#------------------------------------------------------------------------

splint:
	splint $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) -DPATH_MAX=128 -warnposix \
	-booltype  BOOL -boolfalse NO -booltrue YES -nullpass logmanager.c
