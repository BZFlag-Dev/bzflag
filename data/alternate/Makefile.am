EXTRA_DIST = \
	blue_basetop.png	\
	blue_basewall.png	\
	blue_tank.png		\
	boxwall.png		\
	caution.png		\
	clouds.png		\
	explode1.png		\
	explode2.png		\
	green_basetop.png	\
	green_basewall.png	\
	green_tank.png		\
	highflag.png		\
	hunter_tank.png		\
	menu_arrow.png		\
	moon.png		\
	purple_basetop.png	\
	purple_basewall.png	\
	purple_tank.png		\
	pyrwall.png		\
	rabbit_tank.png		\
	red_basetop.png		\
	red_basewall.png	\
	red_tank.png		\
	rogue_tank.png		\
	std_ground.png		\
	title.png		\
	wall.png		\
	zone_ground.png

MAINTAINERCLEANFILES = \
	Makefile.in

install-data-local: $(EXTRA_DIST)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/bzflag/alternate
	@list='$(EXTRA_DIST)'; for p in $$list; do \
	if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(datadir)/bzflag/alternate/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(datadir)/bzflag/alternate/$$f; \
	done

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/bzflag/alternate/*
