EXTRA_DIST =				\
	3ds2bzw.c			\
	bzbz.pl				\
	bzflag.desktop.in		\
	bzfquery.php			\
	bzfquery.pl			\
	bzfquery_example.php		\
	bzfs.conf			\
	bzfs_conf.html			\
	bzfsd				\
	bzfquery.py			\
	bzirc.pl			\
	bzlist.php			\
	bzwcvt.pl			\
	checkam				\
	checkToken.php			\
	doxyfile			\
	elapsed.sh			\
	filter.txt			\
	fountains.bzw			\
	groups.conf			\
	hix.bzw				\
	libtool.m4			\
	man2html.c			\
	mrtg-bzflag.pl			\
	multilingualSwearList.txt	\
	pingtimes.sh			\
	rrlog.cxx			\
	serverstats.sh			\
	simpleSwearList.txt		\
	tar				\
	Template.cxx			\
	Template.h			\
	touchcopyright			\
	touchversion			\
	vars.txt			\
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
