PO_FILES = $(wildcard *.po)

EXTRA_DIST = $(PO_FILES)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/bzflag/l10n
	$(INSTALL_DATA) $(PO_FILES) $(DESTDIR)$(pkgdatadir)/bzflag/l10n

uninstall-local:
	cd $(DESTDIR)$(pkgdatadir)/bzflag/l10n; rm -rf *
