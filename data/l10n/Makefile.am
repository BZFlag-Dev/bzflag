EXTRA_DIST = \
	bzflag_cs_CZ.po	\
	bzflag_da.po	\
	bzflag_de.po	\
	bzflag_en_US_l33t.po	\
	bzflag_en_US_redneck.po	\
	bzflag_es.po	\
	bzflag_fr.po	\
	bzflag_it.po	\
	bzflag_kg.po	\
	bzflag_lt.po	\
	bzflag_nl.po	\
	bzflag_pt.po	\
	bzflag_sv.po	\
	bzflag_xx.po	\
	ISO-639-2_values_8bits.txt

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
