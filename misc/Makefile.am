EXTRA_DIST =				\
	3ds2bzw.c			\
	art				\
	bzbz.pl				\
	bzflag.desktop.in		\
	bzfquery.php			\
	bzfquery.pl			\
	bzfquery.py			\
	bzfquery_example.php		\
	bzfs				\
	bzfs.conf			\
	bzfs_conf.html			\
	bzfsd				\
	bzirc.pl			\
	bzwcvt.pl			\
	checkam				\
	doxyfile			\
	filter.txt			\
	flagbuffet.bzw			\
	fountains.bzw			\
	groups.conf			\
	hix.bzw				\
	libtool.m4			\
	man2html.c			\
	mrtg-bzflag.pl			\
	multilingualSwearList.txt	\
	pingtimes.sh			\
	rebuild-bzflag			\
	rrlog.cxx			\
	ServerListFilter.txt		\
	serverstats.sh			\
	shtool				\
	simpleSwearList.txt		\
	snap.sh				\
	stats/Changes			\
	stats/Info.pm			\
	stats/Makefile.PL		\
	stats/MANIFEST			\
	stats/README			\
	stats/t/1.t			\
	tar				\
	Template.cxx			\
	Template.h			\
	touchcopyright			\
	touchversion			\
	vars.txt			\
	ws.sh

bzflag.desktop: bzflag.desktop.in
	sed 's|\@DATADIR\@|$(pkgdatadir)/|' $< > $@
desktopdir = $(datadir)/applications
desktop_DATA = bzflag.desktop

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

Rrlog:  rrlog.cxx
	$(CXX) $(CXXFLAGS)		\
	  -o rrlog rrlog.cxx		\
	  -I../include -I../src/bzfs	\
	  ../src/date/libDate.a		\
	  ../src/game/libGame.a		\
	  ../src/net/libNet.a		\
	  ../src/common/libCommon.a

3ds2bzw:	3ds2bzw.c
	$(CC) $(CFLAGS) -o 3ds2bzw 3ds2bzw.c -l3ds -lm
