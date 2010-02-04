
bin_PROGRAMS = managelogs

managelogs_SOURCES = managelogs.c id.c intr.c options.c

noinst_HEADERS = managelogs.h id.h intr.h options.h

AM_CPPFLAGS = $(APR_INCLUDES) $(APR_CPPFLAGS)
AM_CFLAGS = $(APR_CFLAGS)
AM_LDFLAGS = $(APR_LDFLAGS)

managelogs_LDADD = $(APR_LIBS)

AM_CPPFLAGS += -I$(top_srcdir)/lib/inst_include
managelogs_LDADD += $(top_builddir)/lib/liblogmanager.la


