SUBDIRS = l10n

EXTRA_DIST = $(wildcard *.rgb) $(wildcard *.wav) \
	$(wildcard *.fti) $(wildcard *.ftr) $(wildcard l10n/*.po)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/bzflag
	$(INSTALL_DATA) $(EXTRA_DIST) $(DESTDIR)$(pkgdatadir)/bzflag

uninstall-local:
	cd $(DESTDIR)$(pkgdatadir)/bzflag; rm -rf *
