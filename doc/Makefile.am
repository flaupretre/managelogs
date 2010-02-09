
dist_man_MANS = managelogs.8

#----------------------------------------------------------------------

HTML_MANS = managelogs.8.htm

html_mans: $(HTML_MANS)

EXTRA_DIST = $(HTML_MANS)

managelogs.8.htm: managelogs.8
	$(top_srcdir)/build/mk_html_doc.sh <$< >$@
