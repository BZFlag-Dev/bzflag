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

install-data-local: $(EXTRA_DIST)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/bzflag/l10n
	@list='$(EXTRA_DIST)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(datadir)/bzflag/l10n/$$f";\
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(datadir)/bzflag/l10n/$$f; \
	done

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/bzflag/l10n/*
