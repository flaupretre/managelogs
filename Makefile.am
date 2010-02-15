
SUBDIRS = lib src doc

EXTRA_DIST = ACKNOWLEDGEMENTS NOTICE build/cleanup build/reconfigure build/mk_html_doc.sh

rpm: dist
	cp $(DIST_ARCHIVES) /usr/src/redhat/SOURCES
	rpmbuild -ba build/managelogs.spec $(RPM_BUILD_OPTS)
