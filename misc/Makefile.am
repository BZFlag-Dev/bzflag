EXTRA_DIST =				\
	art				\
	bzflag.desktop.in		\
	bzfs				\
	bzfsd				\
	checkam				\
	doxyfile			\
	elapsed.sh			\
	footer.sh			\
	libtool.m4			\
	man2html.c			\
	maps				\
	mrtg-bzflag.pl			\
	pingtimes.sh			\
	query				\
	samples				\
	serverstats.sh			\
	shtool				\
	stats				\
	tar				\
	Template.cpp			\
	Template.h			\
	touchcopyright			\
	touchversion			\
	webauth				\
	ws.sh

bzflag.desktop: bzflag.desktop.in
	sed 's|\@DATADIR\@|$(DESTDIR)$(datadir)/bzflag/|' $< > $@
desktopdir = $(DESTDIR)$(datadir)/applications
desktop_DATA = bzflag.desktop

MAINTAINERCLEANFILES =	\
	Makefile.in \
	aclocal.m4 \
	compile \
	config.guess \
	config.guess.backup \
	config.sub \
	config.sub.backup \
	depcomp \
	install-sh \
	ltmain.sh \
	ltmain.sh.backup \
	missing \
	mkinstalldirs

