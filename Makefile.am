
SUBDIRS = lib src doc check

EXTRA_DIST = ACKNOWLEDGEMENTS NOTICE build/cleanup build/reconfigure \
	build/mk_html_doc.sh build/fix_perms build/autogen.sh \
	build/buildconf

rpm: dist
	cp $(DIST_ARCHIVES) /usr/src/redhat/SOURCES
	rpmbuild -ba build/managelogs.spec $(RPM_BUILD_OPTS)
