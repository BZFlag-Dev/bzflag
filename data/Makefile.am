SUBDIRS = l10n

EXTRA_DIST = \
	bbolt.png	\
	blaser.png	\
	btbolt.png	\
	boxwall.png	\
	caution.png	\
	clouds.png	\
	explode1.png	\
	fixedbr.png	\
	fixedmr.png	\
	flag.png	\
	flage.png	\
	gbolt.png	\
	glaser.png	\
	ground.png	\
	gtbolt.png	\
	helvbi.png	\
	helvbr.png	\
	missile.png	\
	mountain.png	\
	pbolt.png	\
	plaser.png	\
	ptbolt.png	\
	pyrwall.png	\
	rbolt.png	\
	rlaser.png	\
	rtbolt.png	\
	roof.png	\
	thief.png	\
	timesbi.png	\
	timesbr.png	\
	title.png	\
	wall.png	\
	wbolt.png	\
	wlaser.png	\
	wtbolt.png	\
	ybolt.png	\
	ylaser.png	\
	ytbolt.png	\
	boom.wav	\
	explosion.wav	\
	fire.wav	\
	flag_alert.wav	\
	flag_drop.wav	\
	flag_grab.wav	\
	flag_lost.wav	\
	flag_won.wav	\
	hunt.wav	\
	hunt_select.wav	\
	jump.wav	\
	killteam.wav	\
	land.wav	\
	laser.wav	\
	lock.wav	\
	missile.wav	\
	phantom.wav	\
	pop.wav		\
	ricochet.wav	\
	shock.wav	\
	steamroller.wav	\
	teamgrab.wav	\
	teleport.wav

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
