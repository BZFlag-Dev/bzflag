AUTOMAKE_OPTIONS = 1.6 dist-zip dist-bzip2
ACLOCAL_AMFLAGS = -I m4

traversal_dirs = \
	data \
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
	m4 \
	misc \
	plugins \
	tools \
	MSVC \
	BZFlag.xcodeproj

EXTRA_DIST = \
	autogen.sh \
	authors.xml \
	BUGS \
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
	README.WINDOWS \
	README.XMINGW

DISTCLEANFILES = config.cache.*

MAINTAINERCLEANFILES = \
	aclocal.m4 \
	configure \
	bzflag.spec \
	INSTALL \
	Makefile.in \
	m4/libtool.m4 \
	m4/ltoptions.m4 \
	m4/ltsugar.m4 \
	m4/ltversion.m4 \
	m4/lt~obsolete.m4
# delete files from m4 subdirectory here to prevent dependency problems

.PHONY: package

package:
	cd package && $(MAKE) targets

# Remove unwanted files from the distribution.
# Ensure that all files in the repository are also in the distribution.
dist-hook:
	find $(distdir) -type f \( -name config.log -o -name config.status \) -exec rm -f {} \;
	find $(distdir) -type d \( -name .deps -o -name .svn -o -name autom4te.cache \) -prune -exec rm -rf {} \;
	rm -f $(distdir)/src/other/ares/ares_build.h \
	      $(distdir)/src/other/curl/include/curl/curlbuild.h
	entries=`find . -name entries -print | grep -v src/other/` ; \
	files=`perl -wnle 'BEGIN {$$/ = "\f"} $$ARGV =~ m=^\./(.*/)\.svn/entries$$= and $$d = $$1 and /^\n([^\n]+)\nfile\n/ and print $$d,$$1' $$entries < /dev/null` ; \
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
