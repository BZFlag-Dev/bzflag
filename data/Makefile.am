SUBDIRS = l10n

EXTRA_DIST = \
	bbolt.rgb	\
	explode1.rgb	\
	glaser.rgb	\
	mountain.rgb	\
	rbolt.rgb	\
	title.rgb	\
	blaser.rgb	\
	fixedbr.rgb	\
	ground.rgb	\
	panel.rgb	\
	rlaser.rgb	\
	wall.rgb	\
	boxwall.rgb	\
	fixedmr.rgb	\
	gtbolt.rgb	\
	pbolt.rgb	\
	roof.rgb	\
	ybolt.rgb	\
	btbolt.rgb	\
	flage.rgb	\
	helvbi.rgb	\
	plaser.rgb	\
	rtbolt.rgb	\
	ylaser.rgb	\
	caution.rgb	\
	flag.rgb	\
	helvbr.rgb	\
	ptbolt.rgb	\
	timesbi.rgb	\
	ytbolt.rgb	\
	clouds.rgb	\
	gbolt.rgb	\
	missile.rgb	\
	pyrwall.rgb	\
	timesbr.rgb	\
	boom.wav	\
	flag_drop.wav	\
	jump.wav	\
	lock.wav	\
	ricochet.wav	\
	explosion.wav	\
	flag_grab.wav	\
	killteam.wav	\
	missile.wav	\
	shock.wav	\
	fire.wav	\
	flag_lost.wav	\
	land.wav	\
	phantom.wav	\
	teamgrab.wav	\
	flag_alert.wav	\
	flag_won.wav	\
	laser.wav	\
	pop.wav		\
	teleport.wav	\
	Bzflag.fti	\
	Bzfs.fti	\
	Bzflag.ftr

MAINTAINERCLEANFILES = \
	Makefile.in

install-data-local: $(EXTRA_DIST)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/bzflag
	@list='$(EXTRA_DIST)'; for p in $$list; do \
	if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(datadir)/bzflag/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(datadir)/bzflag/$$f; \
	done

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/bzflag/*
