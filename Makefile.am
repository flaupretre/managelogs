
SUBDIRS = lib src doc

EXTRA_DIST = ACKNOWLEDGEMENTS NOTICE build/cleanup build/reconfigure

rpm: dist
	cp $(DIST_ARCHIVES) /usr/src/redhat/SOURCES
	rpmbuild -ba managelogs.spec	
