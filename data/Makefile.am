SUBDIRS = l10n fonts

EXTRA_DIST =			\
	blue_basetop.png	\
	blue_basewall.png	\
	blue_bolt.png		\
	blue_laser.png		\
	blue_super_bolt.png	\
	blue_tank.png		\
	boom.wav		\
	bounce.wav		\
	boxwall.png		\
	bubble.png		\
	burrow.wav		\
	caution.png		\
	clouds.png		\
	explode1.png		\
	explode2.png		\
	explosion.wav		\
	fire.wav		\
	flag.png		\
	flag_alert.wav		\
	flag_drop.wav		\
	flag_grab.wav		\
	flag_lost.wav		\
	flag_won.wav		\
	flap.wav		\
	frog.png		\
	green_basetop.png	\
	green_basewall.png	\
	green_bolt.png		\
	green_laser.png		\
	green_super_bolt.png	\
	green_tank.png		\
	highflag.png		\
	hunt.wav		\
	hunt_select.wav		\
	hunter_tank.png		\
	jump.wav		\
	jumpjets.png		\
	killteam.wav		\
	land.wav		\
	laser.wav		\
	lock.wav		\
	menu_arrow.png		\
	mesh.png		\
	message_admin.wav	\
	message_private.wav	\
	message_team.wav	\
	missile.png		\
	missile.wav		\
	moon.png		\
	mountain1.png		\
	mountain2.png		\
	mountain3.png		\
	phantom.wav		\
	pop.wav			\
	puddle.png		\
	purple_basetop.png	\
	purple_basewall.png	\
	purple_bolt.png		\
	purple_laser.png	\
	purple_super_bolt.png	\
	purple_tank.png		\
	pyrwall.png		\
	rabbit_bolt.png		\
	rabbit_laser.png	\
	rabbit_super_bolt.png	\
	rabbit_tank.png		\
	raindrop.png		\
	red_basetop.png		\
	red_basewall.png	\
	red_bolt.png		\
	red_laser.png		\
	red_super_bolt.png	\
	red_tank.png		\
	ricochet.wav		\
	roof.png		\
	rogue_bolt.png		\
	rogue_laser.png		\
	rogue_super_bolt.png	\
	rogue_tank.png		\
	shock.wav		\
	snowflake.png		\
	std_ground.png		\
	steamroller.wav		\
	teamgrab.wav		\
	telelink.png		\
	teleport.wav		\
	tetrawall.png		\
	thief.png		\
	thief.wav		\
	title.png		\
	wall.png		\
	water.png		\
	zone_ground.png

MAINTAINERCLEANFILES =	\
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
