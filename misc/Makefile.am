EXTRA_DIST =				\
	3ds2bzw.c			\
	bzbz.pl				\
	bzflag.desktop.in		\
	bzfquery.pl			\
	bzfs.conf			\
	bzfs_conf.html			\
	bzfsd				\
	bzfquery.py			\
	bzirc.pl			\
	bzlist.php			\
	bzwcvt.pl			\
	checkam				\
	doxyfile			\
	filter.txt			\
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
	serverstats.sh			\
	simpleSwearList.txt		\
	snap.sh				\
	tar				\
	touchcopyright			\
	touchversion			\
	vars.txt			\
	ws.sh

bzflag.desktop: bzflag.desktop.in
	sed 's|\@DATADIR\@|$(DESTDIR)$(datadir)/bzflag/|' $< > $@
desktop_DATA = bzflag.desktop

MAINTAINERCLEANFILES =	\
	Makefile.in

rrlog:  rrlog.cxx
	$(CXX) $(CXXFLAGS)		\
	  -o rrlog rrlog.cxx		\
	  -I../include -I../src/bzfs	\
	  ../src/date/libDate.a		\
	  ../src/game/libGame.a		\
	  ../src/net/libNet.a		\
	  ../src/common/libCommon.a

3ds2bzw:	3ds2bzw.c
	$(CC) $(CFLAGS) -o 3ds2bzw 3ds2bzw.c -l3ds -lm
