EXTRA_DIST = \
	readme			\
	LuxiMono_8.png		\
	LuxiMono_11.png		\
	LuxiMono_16.png		\
	TogaSansBold_8.png	\
	TogaSansBold_16.png	\
	TogaSansBold_32.png	\
	TogaSerifBold_16.png	\
	TogaSerifBold_32.png	\
	VeraMonoBold_8.png	\
	VeraMonoBold_12.png	\
	VeraMonoBold_16.png	\
	LuxiMono_8.fmt		\
	LuxiMono_11.fmt		\
	LuxiMono_16.fmt		\
	TogaSansBold_8.fmt	\
	TogaSansBold_16.fmt	\
	TogaSansBold_32.fmt	\
	TogaSerifBold_16.fmt	\
	TogaSerifBold_32.fmt	\
	VeraMonoBold_8.fmt	\
	VeraMonoBold_12.fmt	\
	VeraMonoBold_16.fmt

MAINTAINERCLEANFILES = \
	Makefile.in

install-data-local: $(EXTRA_DIST)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/bzflag/fonts
	@list='$(EXTRA_DIST)'; for p in $$list; do \
	if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(datadir)/bzflag/fonts/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(datadir)/bzflag/fonts/$$f; \
	done

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/bzflag/fonts/*
