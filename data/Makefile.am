SUBDIRS = l10n

EXTRA_DIST = \
	bbolt.png	\
	explode1.png	\
	glaser.png	\
	mountain.png	\
	rbolt.png	\
	title.png	\
	blaser.png	\
	fixedbr.png	\
	ground.png	\
	panel.png	\
	rlaser.png	\
	wall.png	\
	boxwall.png	\
	fixedmr.png	\
	gtbolt.png	\
	pbolt.png	\
	roof.png	\
	ybolt.png	\
	btbolt.png	\
	flage.png	\
	helvbi.png	\
	plaser.png	\
	rtbolt.png	\
	ylaser.png	\
	caution.png	\
	flag.png	\
	helvbr.png	\
	ptbolt.png	\
	timesbi.png	\
	ytbolt.png	\
	clouds.png	\
	gbolt.png	\
	missile.png	\
	pyrwall.png	\
	timesbr.png	\
	thief.png	\
	wbolt.png	\
	wlaser.png	\
	wtbolt.png	\
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
	steamroller.wav	\
	hunt_select.wav	\
	hunt.wav	\
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
