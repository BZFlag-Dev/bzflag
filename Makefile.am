SUBDIRS = data include man package src doc
DIST_SUBDIRS = $(SUBDIRS) debian misc win32 Dev-C++

package: FORCE
	cd package && $(MAKE) targets

FORCE:

EXTRA_DIST = \
	autogen.sh \
	BUGS \
	bzflag.info \
	bzflag.lsm \
	bzflag.spec \
	DEVINFO \
	PORTING \
	README.BeOS \
	README.IRIX \
	README.Linux \
	README.MINGW32 \
	README.MacOSX \
	README.SDL \
	README.SOLARIS \
	README.UNIX \
	README.WIN32 \
	RELNOTES

MAINTAINERCLEANFILES = \
	aclocal.m4 \
	config.guess \
	config.sub \
	configure \
	bzflag.info \
	bzflag.spec \
	INSTALL \
	install-sh \
	missing \
	mkinstalldirs \
	Makefile.in

debian-cvs: dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(AMTAR)$(TAR) zxf $(distdir).tar.gz
	(cd $(distdir) && debian/buildsnap cvs)

debian-release: debian/changelog dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(AMTAR)$(TAR) zxf $(distdir).tar.gz
	cp debian/changelog $(distdir)/debian/
	(cd $(distdir) && debian/buildsnap release)
