SUBDIRS = l10n

EXTRA_DIST = $(wildcard *.rgb) $(wildcard *.wav) \
	$(wildcard *.fti) $(wildcard *.ftr) $(wildcard l10n/*.po)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/bzflag
	$(INSTALL_DATA) $(EXTRA_DIST) $(DESTDIR)$(datadir)/bzflag

uninstall-local:
	cd $(DESTDIR)$(datadir)/bzflag; rm -rf *
