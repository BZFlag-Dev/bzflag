PO_FILES = $(wildcard $(srcdir)/*.po)

EXTRA_DIST = $(PO_FILES)

MAINTAINERCLEANFILES = \
	Makefile.in

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/bzflag/l10n
	$(INSTALL_DATA) $(PO_FILES) $(DESTDIR)$(datadir)/bzflag/l10n

uninstall-local:
	cd $(DESTDIR)$(pkgdatadir)/bzflag/l10n; rm -rf *
