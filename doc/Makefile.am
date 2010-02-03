
dist_man_MANS = managelogs.8

EXTRA_DIST = managelogs.8.htm

html-man:
	@for i in $(MANS); \
		do \
		echo "Generating $$i.htm ..."; \
		man2html <$$i >$$i.htm ; \
	done
