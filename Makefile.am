# $Id$

AUTOMAKE_OPTIONS = 1.6 dist-zip dist-bzip2
ACLOCAL_AMFLAGS = -I m4

traversal_dirs = \
	include \
	data \
	doc \
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
	MSVC \
	debian \
	m4 \
	misc \
	plugins \
	tools \
	BZFlag.xcodeproj

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
	bots \
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


# make sure all files are in the repository are also in the dist.
# make sure files that should not be in the distribution in fact are
# not included.
dist-hook:
	files="`find . -name entries -exec grep 'name=' {} /dev/null \; | sed 's/:[[:space:]]*name=\"\(.*\)\"/\1/g' | sed 's/\/\.svn\/entries/\//g' | grep -v 'bzflag-' | grep -v '\/$$'`" ; \
	missing=0 ; \
	for file in $$files ; do \
		if test ! -e $(distdir)/$$file ; then \
			missing=1 ; \
			echo "MISSING FROM DIST: $$file" ; \
		fi ; \
	done ; \
	if test "x$$missing" = "x1" ; then \
		exit 1 ; \
	fi
	find $(distdir) -type f \( -name '.cvsignore' \) -exec rm -f {} \;
	find $(distdir) -type d \( -name 'CVS' -or -name '.svn' \) -prune -exec rm -rf {} \;


# Print out an informative summary.
all-local:
	@$(ECHO) "Done."
	@$(ECHO)
	@$(ECHO) "BZFlag @BZFLAG_VERSION@, Build @BZFLAG_DATE@"
	@$(ECHO)
	@if test "x$(MAKECMDGOALS)" = "xall-am" -o "x$(.TARGETS)" = "xall-am" -o "x$(MAKECMDGOALS)" = "x" -o "x$(.TARGETS)" = "x" ; then \
	  $(ECHO) $(ECHO_N) "Elapsed time since configuration: " ;\
	  sh $(top_srcdir)/misc/elapsed.sh @CONF_TIME@ ;\
	  $(ECHO) $(ECHO_N) "Elapsed compilation time: " ;\
	  sh $(top_srcdir)/misc/elapsed.sh `cat $(top_srcdir)/include/datestamp` ;\
	  $(ECHO) "---" ;\
	  $(ECHO) "Run 'make install' to begin installation into $(prefix)" ;\
	fi
	@$(ECHO)
