# $Id$

AUTOMAKE_OPTIONS = 1.6 dist-zip dist-bzip2
ACLOCAL_AMFLAGS = -I m4

traversal_dirs = \
	data \
	doc \
	include \
	man \
	package \
	src

if BUILD_PLUGINS
PLUGINS=plugins
else
PLUGINS=
endif

SUBDIRS = \
	$(traversal_dirs) \
	$(PLUGINS)

DIST_SUBDIRS = \
	$(traversal_dirs) \
	plugins \
	debian \
	misc \
	tools \
	win32 \
	Dev-C++ \
	BZFlag.xcode

EXTRA_DIST = \
	autogen.sh \
	BUGS \
	bzflag.info \
	bzflag.lsm \
	bzflag.spec \
	DEVINFO \
	PORTING \
	README.BeOS \
	README.DEVC++ \
	README.IRIX \
	README.Linux \
	README.MINGW32 \
	README.MacOSX \
	README.SDL \
	README.SOLARIS \
	README.UNIX \
	README.WIN32 \
	README.XMINGW	\
	RELNOTES

MAINTAINERCLEANFILES = \
	configure \
	bzflag.info \
	bzflag.spec \
	INSTALL \
	Makefile.in

.PHONY: package FORCE debian-cvs debian-release

package: FORCE
	cd package && $(MAKE) targets

FORCE:

debian-cvs: dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(AMTAR)$(TAR) zxf $(distdir).tar.gz
	(cd $(distdir) && debian/buildsnap cvs)

debian-release: debian/changelog dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(AMTAR)$(TAR) zxf $(distdir).tar.gz
	cp debian/changelog $(distdir)/debian/
	(cd $(distdir) && debian/buildsnap release)

