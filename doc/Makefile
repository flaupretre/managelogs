#--- No modifiable parameter in this file

include ../make.vars

#--------------------------------------------------------------------

MANPAGES = managelogs.8

#--------------------------------------------------------------------

.PHONY: all html-man install-man install-sys-man

all: html-man

html-man:
	@for i in $(MANPAGES); \
		do \
		echo "Generating $$i.htm ..."; \
		man2html <$$i >$$i.htm ; \
	done

install-man:
	@echo Installing man pages...
	@test -d $(APACHE)/man/man8 || $(MKINSTALLDIRS) $(APACHE)/man/man8
	$(LIBTOOL) --mode=install cp $(MANPAGES) $(APACHE)/man/man8

install-sys-man:
	@echo Installing man pages in system directory...
	@test -d /usr/share/man/man8 || $(MKINSTALLDIRS) /usr/share/man/man8
	$(LIBTOOL) --mode=install cp $(MANPAGES) /usr/share/man/man8

#--------------------------------------------------------------------
