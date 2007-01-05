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
	Dev-C++ \
	MSVC \
	debian \
	m4 \
	misc \
	plugins \
	tools \
	BZFlag.xcode

documentationdir = $(datadir)/doc/bzflag
documentation_DATA = \
	AUTHORS \
	BUGS \
	COPYING \
	ChangeLog \
	DEVINFO \
	NEWS \
	README \
	README.BeOS \
	README.DEVC++ \
	README.HPUX \
	README.IRIX \
	README.Linux \
	README.MINGW32 \
	README.MacOSX \
	README.SDL \
	README.SOLARIS \
	README.UNIX \
	README.WINDOWS \
	README.XMINGW \
	TODO

EXTRA_DIST = \
	$(documentation_DATA) \
	autogen.sh \
	bzflag.info \
	bzflag.info.in \
	bzflag.lsm \
	bzflag.lsm.in \
	bzflag.spec \
	bzflag.spec.in \
	ideas.txt
	INSTALL \
	PORTING

MAINTAINERCLEANFILES = \
	configure \
	bzflag.info \
	bzflag.spec \
	INSTALL \
	Makefile.in

.PHONY: package debian-cvs debian-release

package:
	cd package && $(MAKE) targets

debian-cvs: dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(AMTAR)$(TAR) zxf $(distdir).tar.gz
	(cd $(distdir) && debian/buildsnap cvs)

debian-release: debian/changelog dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(AMTAR)$(TAR) zxf $(distdir).tar.gz
	cp debian/changelog $(distdir)/debian/
	(cd $(distdir) && debian/buildsnap release)

