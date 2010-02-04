
SUBDIRS = lib src doc

#---------------------------------------------------------------------------
# Building managelogs.spec
# Cannot be done by configure since variables 'xxxdir' are expanded by make
# Trick taken from the autoconf documentation

noinst_PROGRAMS = managelogs.spec
managelogs_spec_SOURCES = managelogs.spec.src

edit = sed \
	-e 's|---bindir---|$(bindir)|g' \
	-e 's|---libdir---|$(libdir)|g' \
	-e 's|---includedir---|$(includedir)|g' \
	-e 's|---mandir---|$(mandir)|g'

managelogs.spec: managelogs.spec.src
	rm -f $@
	$(edit) $< >$@

rpm: managelogs.spec install
	rpmbuild -bb managelogs.spec	
