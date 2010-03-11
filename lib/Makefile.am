
lib_LTLIBRARIES = liblogmanager.la

liblogmanager_la_SOURCES = logmanager.c

AM_CPPFLAGS = $(APR_INCLUDES) $(APR_CPPFLAGS)
AM_CFLAGS = $(APR_CFLAGS) $(Z_CFLAGS) $(BZ2_CFLAGS)
AM_LDFLAGS = $(APR_LDFLAGS)

include_HEADERS = inst_include/logmanager.h

noinst_HEADERS = include/config.h \
	include/bzip2_handler.h bzip2_handler.c \
	include/compress.h      compress.c \
	include/gzip_handler.h  gzip_handler.c \
	include/backup.h        backup.c \
	include/cmd.h           cmd.c \
	include/link.h          link.c \
	include/pid.h           pid.c \
	include/stats.h         stats.c \
	include/status.h        status.c \
	include/write.h         write.c \
	include/debug.h         debug.c \
	include/checksum.h      checksum.c \
	include/plain_handler.h plain_handler.c \
	include/file.h          file.c \
	include/path.h          path.c \
	include/array.h \
	logmanager.c \
	$(top_srcdir)/common/time.c \
	$(top_srcdir)/common/time.h \
	$(top_srcdir)/common/global.h \
	$(top_srcdir)/common/alloc.c \
	$(top_srcdir)/common/alloc.h \
	$(top_srcdir)/common/convert.c \
	$(top_srcdir)/common/convert.h

liblogmanager_la_LIBADD = $(APR_LIBS) $(Z_LIBS) $(BZ2_LIBS)
liblogmanager_la_LDFLAGS = -version-info $(LIB_LT_VERSION):$(LIB_LT_REVISION):$(LIB_LT_AGE)

#------------------------------------------------------------------------

splint:
	splint $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) -DPATH_MAX=128 -warnposix \
	-booltype  BOOL -boolfalse NO -booltrue YES -nullpass logmanager.c
