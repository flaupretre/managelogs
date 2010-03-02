
check_PROGRAMS = genlog

genlog_SOURCES = genlog.c

AM_CPPFLAGS = $(APR_INCLUDES) $(APR_CPPFLAGS)
AM_CFLAGS = $(APR_CFLAGS)
AM_LDFLAGS = $(APR_LDFLAGS)

genlog_LDADD = $(APR_LIBS)

EXTRA_DIST = check.sh

DISTCLEANFILES = check.log

check: all
	$(srcdir)/check.sh
