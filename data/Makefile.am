SUBDIRS = l10n

EXTRA_DIST = \
	blue_bolt.png	\
	blue_laser.png	\
	blue_super_bolt.png	\
	boom.wav	\
	boxwall.png	\
	burrow.wav	\
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
	green_bolt.png	\
	green_laser.png	\
	green_super_bolt.png	\
	ground.png	\
	helvbi.png	\
	helvbr.png	\
	hunt.wav	\
	hunt_select.wav	\
	jump.wav	\
	killteam.wav	\
	land.wav	\
	laser.wav	\
	lock.wav	\
	menu_arrow.png	\
	missile.png	\
	missile.wav	\
	mountain1.png	\
	mountain2.png	\
	mountain3.png	\
	phantom.wav	\
	pop.wav		\
	purple_bolt.png	\
	purple_laser.png	\
	purple_super_bolt.png	\
	pyrwall.png	\
	rabbit_bolt.png	\
	rabbit_laser.png	\
	rabbit_super_bolt.png	\
	red_bolt.png	\
	red_laser.png	\
	red_super_bolt.png	\
	ricochet.wav	\
	roof.png	\
	rogue_bolt.png	\
	rogue_laser.png	\
	rogue_super_bolt.png	\
	shock.wav	\
	steamroller.wav	\
	tank.png	\
	teamgrab.wav	\
	teleport.wav	\
	thief.png	\
	thief.wav	\
	timesbi.png	\
	timesbr.png	\
	title.png	\
	wall.png

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
