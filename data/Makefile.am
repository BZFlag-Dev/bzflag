SUBDIRS = l10n

EXTRA_DIST = $(wildcard $(srcdir)/*.rgb) $(wildcard $(srcdir)*.wav) \
	$(wildcard $(srcdir)*.fti) $(wildcard $(srcdir)*.ftr) \
	$(wildcard l10n/$(srcdir)/*.po)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/bzflag
	$(INSTALL_DATA) $(EXTRA_DIST) $(DESTDIR)$(datadir)/bzflag

uninstall-local:
	cd $(DESTDIR)$(datadir)/bzflag; rm -rf *
