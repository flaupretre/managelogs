
dist_man_MANS = managelogs.8

EXTRA_DIST = $(HTML_MANS) api.pdf api examples

#-- HTML man page ----

HTML_MANS = managelogs.8.htm

html_mans: $(HTML_MANS)

managelogs.8.htm: managelogs.8
	$(top_srcdir)/build/mk_html_doc.sh <$< >$@

#-- Doxygen API documentation ----

clean_dox:
	( \
	cd $(top_srcdir)/doc; \
	/bin/rm -rf api/* api.pdf; \
	)

dox: Doxyfile clean_dox
	( \
	cd $(top_srcdir)/lib/inst_include; \
	doxygen ../../build/Doxyfile; \
	cd ../../doc/api/latex; \
	$(MAKE) all pdf; \
	/bin/mv refman.pdf ../../api.pdf; \
	)
