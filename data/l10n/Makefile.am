EXTRA_DIST = \
	bzflag_da.po	\
	bzflag_en_US_redneck.po	\
	bzflag_it.po	\
	bzflag_pt.po	\
	bzflag_de.po	\
	bzflag_es.po	\
	bzflag_kg.po	\
	bzflag_sv.po	\
	bzflag_en_US_l33t.po	\
	bzflag_fr.po	\
	bzflag_nl.po	\
	bzflag_xx.po

MAINTAINERCLEANFILES = \
	Makefile.in

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/bzflag/l10n
	$(INSTALL_DATA) $(PO_FILES) $(DESTDIR)$(datadir)/bzflag/l10n

uninstall-local:
	cd $(DESTDIR)$(pkgdatadir)/bzflag/l10n; rm -rf *
