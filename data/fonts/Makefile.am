EXTRA_DIST = \
	LuxiMono-8.png		\
	LuxiMono-11.png		\
	LuxiMono-16.png		\
	TogaSansBold-8.png	\
	TogaSansBold-16.png	\
	TogaSansBold-32.png	\
	TogaSerifBold-16.png	\
	TogaSerifBold-32.png	\
	VeraMonoBold-8.png	\
	VeraMonoBold-12.png	\
	VeraMonoBold-16.png	\
	LuxiMono-8.fmt		\
	LuxiMono-11.fmt		\
	LuxiMono-16.fmt		\
	TogaSansBold-8.fmt	\
	TogaSansBold-16.fmt	\
	TogaSansBold-32.fmt	\
	TogaSerifBold-16.fmt	\
	TogaSerifBold-32.fmt	\
	VeraMonoBold-8.fmt	\
	VeraMonoBold-12.fmt	\
	VeraMonoBold-16.fmt	\

MAINTAINERCLEANFILES = \
	Makefile.in

install-data-local: $(EXTRA_DIST)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/bzflag
	@list='$(EXTRA_DIST)'; for p in $$list; do \
	if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(datadir)/bzflag/fonts/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(datadir)/bzflag/fonts/$$f; \
	done

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/bzflag/fonts/*
