SUBDIRS = data include man package src
DIST_SUBDIRS = data debian include misc man package win32 src

package: FORCE
	cd package && $(MAKE) targets

FORCE:

EXTRA_DIST = \
	acconfig.h \
	autogen.sh \
	BUGS \
	bzflag.lsm \
	DEVINFO \
	PORTING \
	README.IRIX \
	README.MINGW32 \
	README.SOLARIS \
	README.UNIX \
	README.WIN32 \
	RELNOTES

MAINTAINERCLEANFILES = \
	aclocal.m4 \
	config.guess \
	config.sub \
	configure \
	INSTALL \
	install-sh \
	missing \
	mkinstalldirs \
	Makefile.in
