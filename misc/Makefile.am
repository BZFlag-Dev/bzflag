EXTRA_PROGRAMS = 3ds2bzw rrlog

EXTRA_DIST =				\
	art/bzicon-red.svg		\
	art/title.psd			\
	bzbz.pl				\
	bzflag.desktop.in		\
	bzfquery.php			\
	bzfquery.pl			\
	bzfquery.py			\
	bzfquery_example.php		\
	bzfs				\
	bzfs.conf			\
	bzfsd				\
	bzirc.pl			\
	bzls.lua			\
	bzwcvt.pl			\
	checkToken.php			\
	checkam				\
	doxyfile			\
	filter.txt			\
	fonts/DejaVuSansCondensed-Bold.ttf	\
	fonts/DejaVuSansMono-Bold.ttf	\
	fonts/LICENSE.DejaVu		\
	fonts/README			\
	groups.conf			\
	libtool.m4			\
	man2html.c			\
	maps/flagbuffet.bzw		\
	maps/fountains.bzw		\
	maps/hix.bzw			\
	mrtg-bzflag.pl			\
	multilingualSwearList.txt	\
	pingtimes.sh			\
	ServerListFilter.txt		\
	serverstats.sh			\
	shtool				\
	simpleSwearList.txt		\
	stats/Changes			\
	stats/Info.pm			\
	stats/Makefile.PL		\
	stats/MANIFEST			\
	stats/README			\
	stats/t/1.t			\
	tank.obj			\
	Template.cxx			\
	Template.h			\
	touchcopyright			\
	vars.txt			\
	ws.sh

bzflag.desktop: bzflag.desktop.in
	sed 's|\@DATADIR\@|$(pkgdatadir)/|' $< > $@
desktopdir = $(datadir)/applications
desktop_DATA = bzflag.desktop

CLEANFILES = $(EXTRA_PROGRAMS) bzflag.desktop

MAINTAINERCLEANFILES = \
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

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

rrlog_SOURCES = rrlog.cxx
rrlog_CPPFLAGS = -I$(top_srcdir)/src/bzfs
rrlog_LDADD =				\
	../src/date/libDate.la		\
	../src/game/libGame.la		\
	../src/net/libNet.la		\
	../src/common/libCommon.la	\
	$(X_EXTRA_LIBS)

3ds2bzw_SOURCES = 3ds2bzw.cxx
3ds2bzw_LDADD = -l3ds
