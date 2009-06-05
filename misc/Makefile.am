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
	rrlog.cxx			\
	samples				\
	serverstats.sh			\
	shtool				\
	stats				\
	tar				\
	Template.cxx			\
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

rrlog:	rrlog.cxx
	$(LIBTOOL) --tag=CXX --mode=link $(CXX) $(CXXFLAGS) \
	  -o rrlog rrlog.cxx		\
	  -I../include -I../src/bzfs	\
	  ../src/date/libDate.la	\
	  ../src/game/libGame.la	\
	  ../src/net/libNet.la		\
	  ../src/common/libCommon.la
