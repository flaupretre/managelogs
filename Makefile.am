
SUBDIRS = lib src doc check

EXTRA_DIST = ACKNOWLEDGEMENTS NOTICE build/cleanup build/reconfigure \
	build/mk_html_doc.sh build/fix_perms build/autogen.sh \
	build/buildconf

RPM_BASE = /usr/src/redhat

rpm: dist
	cp $(DIST_ARCHIVES) $(RPM_BASE)/SOURCES
	rpmbuild -ba build/@PACKAGE_NAME@.spec $(RPM_BUILD_OPTS)
	/bin/rm -rf $(RPM_BASE)/SOURCES/$(DIST_ARCHIVES)
	/bin/rm -rf $(RPM_BASE)/BUILD/@PACKAGE_NAME@-@PACKAGE_VERSION@
	f=$(RPM_BASE)/RPMS/@ARCHITECTURE@/@PACKAGE_NAME@-@PACKAGE_VERSION@-@RELEASE_NUMBER@.@ARCHITECTURE@.rpm; \
	if [ -f $$f ] ; then cp $$f . ; /bin/rm -f $$f ; fi
	f=$(RPM_BASE)/RPMS/@ARCHITECTURE@/@PACKAGE_NAME@-debuginfo-@PACKAGE_VERSION@-@RELEASE_NUMBER@.@ARCHITECTURE@.rpm; \
	if [ -f $$f ] ; then cp $$f . ; /bin/rm -f $$f ; fi
	f=$(RPM_BASE)/SRPMS/@PACKAGE_NAME@-@PACKAGE_VERSION@-@RELEASE_NUMBER@.src.rpm; \
	if [ -f $$f ] ; then cp $$f . ; /bin/rm -f $$f ; fi

# Valgrind check

vgcheck: install
	cd check && $(MAKE) $@
