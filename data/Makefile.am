SUBDIRS = l10n

EXTRA_DIST = \
	bbolt.png	\
	blaser.png	\
	boom.wav	\
	boxwall.png	\
	btbolt.png	\
	caution.png	\
	clouds.png	\
	explode1.png	\
	explosion.wav	\
	fire.wav	\
	fixedbr.png	\
	fixedmr.png	\
	flag.png	\
	flag_alert.wav	\
	flag_drop.wav	\
	flag_grab.wav	\
	flag_lost.wav	\
	flag_won.wav	\
	gbolt.png	\
	glaser.png	\
	ground.png	\
	gtbolt.png	\
	helvbi.png	\
	helvbr.png	\
	hunt.wav	\
	hunt_select.wav	\
	jump.wav	\
	killteam.wav	\
	land.wav	\
	laser.wav	\
	lock.wav	\
	missile.png	\
	missile.wav	\
	mountain1.png	\
	mountain2.png	\
	mountain3.png	\
	pbolt.png	\
	phantom.wav	\
	plaser.png	\
	pop.wav		\
	ptbolt.png	\
	pyrwall.png	\
	rbolt.png	\
	ricochet.wav	\
	rlaser.png	\
	roof.png	\
	rtbolt.png	\
	shock.wav	\
	steamroller.wav	\
	tank.png	\
	teamgrab.wav	\
	teleport.wav	\
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
	ytbolt.png

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
