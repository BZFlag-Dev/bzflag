# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4-p5 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ../..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/bzflag
pkglibdir = $(libdir)/bzflag
pkgincludedir = $(includedir)/bzflag

top_builddir = ../..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = i686-pc-linux-gnu
host_triplet = i686-pc-linux-gnu
CC = gcc
CONFIG = linux-i386
CXX = c++
INSTALL_DATA_DIR = \"${datadir}/bzflag\"
LIBDIR = $(top_srcdir)/lib
MAKEINFO = /home/timr/work/bzflag/missing makeinfo
PACKAGE = bzflag
RANLIB = ranlib
VERSION = 10707001
VERSIONID = 1.7g1

bin_PROGRAMS = bzflag

bzflag_SOURCES = \
	MainWindow.cxx			\
	MainWindow.h			\
	ControlPanel.cxx		\
	ControlPanel.h			\
	World.cxx			\
	World.h			\
	SceneBuilder.cxx		\
	SceneBuilder.h			\
	SceneRenderer.cxx		\
	BackgroundRenderer.cxx		\
	BackgroundRenderer.h		\
	RadarRenderer.cxx		\
	RadarRenderer.h			\
	HUDRenderer.cxx			\
	HUDRenderer.h			\
	HUDui.cxx			\
	HUDui.h				\
	ShotPath.cxx			\
	ShotPath.h			\
	ShotStrategy.cxx		\
	ShotStrategy.h			\
	Region.cxx			\
	Region.h			\
	RenderNode.cxx			\
	Player.cxx			\
	Player.h			\
	playing.cxx			\
	playing.h			\
	LocalPlayer.cxx			\
	LocalPlayer.h			\
	RemotePlayer.cxx		\
	RemotePlayer.h			\
	RobotPlayer.cxx			\
	RobotPlayer.h			\
	DeadPlayer.cxx			\
	DeadPlayer.h			\
	ServerLink.cxx			\
	ServerLink.h			\
	PlayerLink.cxx			\
	PlayerLink.h			\
	KeyMap.cxx			\
	KeyMap.h			\
	daylight.cxx			\
	daylight.h			\
	texture.cxx			\
	texture.h			\
	resources.cxx			\
	resources.h			\
	sound.cxx			\
	sound.h				\
	bzflag.cxx			\
	HUDDialog.cxx			\
	HUDDialog.h			\
	menus.cxx			\
	menus.h				\
	bzflag.rc			\
	bzflag.ico


noinst_HEADERS = \
	stars.cxx

#LDADD = #../net/libNet.a			\
#	../common/libCommon.a		\
#	../obstacle/libObstacle.a	\
#	../platform/libPlatform.a	\
#	../ogl/libGLKit.a		\
#	../scene/libSceneDB.a		\
#	../geometry/libGeometry.a	\
#	bzflag.res
LDADD = ../net/libNet.a			\
	../common/libCommon.a		\
	../obstacle/libObstacle.a	\
	../platform/libPlatform.a	\
	../ogl/libGLKit.a		\
	../scene/libSceneDB.a		\
	../geometry/libGeometry.a
EXTRA_DIST = bzflag.bmp
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../../include/config.h
CONFIG_CLEAN_FILES = 
PROGRAMS =  $(bin_PROGRAMS)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I../../include
CPPFLAGS =  -I/usr/X11R6/include -DINSTALL_DATA_DIR="\"${datadir}/bzflag\"" -I/usr/include -D_BSD_SOURCE -D_POSIX_SOURCE -Wall -W -mcpu=i486
LDFLAGS =  -L/usr/X11R6/lib
LIBS = -lGLU -lGL -lXi -lXxf86vm -lXext -lm 
X_CFLAGS =  -I/usr/X11R6/include
X_LIBS =  -L/usr/X11R6/lib
X_EXTRA_LIBS = 
X_PRE_LIBS =  -lSM -lICE
bzflag_OBJECTS =  MainWindow.o ControlPanel.o World.o SceneBuilder.o \
SceneRenderer.o BackgroundRenderer.o RadarRenderer.o HUDRenderer.o \
HUDui.o ShotPath.o ShotStrategy.o Region.o RenderNode.o Player.o \
playing.o LocalPlayer.o RemotePlayer.o RobotPlayer.o DeadPlayer.o \
ServerLink.o PlayerLink.o KeyMap.o daylight.o texture.o resources.o \
sound.o bzflag.o HUDDialog.o menus.o
bzflag_LDADD = $(LDADD)
#bzflag_DEPENDENCIES =  ../net/libNet.a ../common/libCommon.a \
#../obstacle/libObstacle.a ../platform/libPlatform.a \
#../ogl/libGLKit.a ../scene/libSceneDB.a \
#../geometry/libGeometry.a bzflag.res
bzflag_DEPENDENCIES =  ../net/libNet.a \
../common/libCommon.a ../obstacle/libObstacle.a \
../platform/libPlatform.a ../ogl/libGLKit.a \
../scene/libSceneDB.a ../geometry/libGeometry.a
bzflag_LDFLAGS = 
CXXFLAGS = -g -O2 -fno-exceptions -O2 -ffast-math -fomit-frame-pointer -fexpensive-optimizations
CXXCOMPILE = $(CXX) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
CXXLD = $(CXX)
CXXLINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@
CFLAGS = -g -O2 -ansi -O2
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
HEADERS =  $(noinst_HEADERS)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gtar
GZIP_ENV = --best
DEP_FILES =  .deps/BackgroundRenderer.P .deps/ControlPanel.P \
.deps/DeadPlayer.P .deps/HUDDialog.P .deps/HUDRenderer.P .deps/HUDui.P \
.deps/KeyMap.P .deps/LocalPlayer.P .deps/MainWindow.P .deps/Player.P \
.deps/PlayerLink.P .deps/RadarRenderer.P .deps/Region.P \
.deps/RemotePlayer.P .deps/RenderNode.P .deps/RobotPlayer.P \
.deps/SceneBuilder.P .deps/SceneRenderer.P .deps/ServerLink.P \
.deps/ShotPath.P .deps/ShotStrategy.P .deps/World.P .deps/bzflag.P \
.deps/daylight.P .deps/menus.P .deps/playing.P .deps/resources.P \
.deps/sound.P .deps/texture.P
SOURCES = $(bzflag_SOURCES)
OBJECTS = $(bzflag_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .cxx .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu src/bzflag/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	     $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	done

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

bzflag: $(bzflag_OBJECTS) $(bzflag_DEPENDENCIES)
	@rm -f bzflag
	$(CXXLINK) $(bzflag_LDFLAGS) $(bzflag_OBJECTS) $(bzflag_LDADD) $(LIBS)
.cxx.o:
	$(CXXCOMPILE) -c $<

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = src/bzflag

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu src/bzflag/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

-include $(DEP_FILES)

mostlyclean-depend:

clean-depend:

distclean-depend:
	-rm -rf .deps

maintainer-clean-depend:

%.o: %.c
	@echo '$(COMPILE) -c $<'; \
	$(COMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

%.lo: %.c
	@echo '$(LTCOMPILE) -c $<'; \
	$(LTCOMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-sed -e 's/^\([^:]*\)\.o[ 	]*:/\1.lo \1.o :/' \
	  < .deps/$(*F).pp > .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm -f .deps/$(*F).pp

%.o: %.cxx
	@echo '$(CXXCOMPILE) -c $<'; \
	$(CXXCOMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

%.lo: %.cxx
	@echo '$(LTCXXCOMPILE) -c $<'; \
	$(LTCXXCOMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-sed -e 's/^\([^:]*\)\.o[ 	]*:/\1.lo \1.o :/' \
	  < .deps/$(*F).pp > .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm -f .deps/$(*F).pp
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am: install-binPROGRAMS
install-exec: install-exec-am

install-data-am:
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-binPROGRAMS
uninstall: uninstall-am
all-am: Makefile $(PROGRAMS) $(HEADERS)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(bindir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-binPROGRAMS mostlyclean-compile \
		mostlyclean-tags mostlyclean-depend mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-binPROGRAMS clean-compile clean-tags clean-depend \
		clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-binPROGRAMS distclean-compile distclean-tags \
		distclean-depend distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-binPROGRAMS \
		maintainer-clean-compile maintainer-clean-tags \
		maintainer-clean-depend maintainer-clean-generic \
		distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: mostlyclean-binPROGRAMS distclean-binPROGRAMS clean-binPROGRAMS \
maintainer-clean-binPROGRAMS uninstall-binPROGRAMS install-binPROGRAMS \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile tags mostlyclean-tags distclean-tags \
clean-tags maintainer-clean-tags distdir mostlyclean-depend \
distclean-depend clean-depend maintainer-clean-depend info-am info \
dvi-am dvi check check-am installcheck-am installcheck install-exec-am \
install-exec install-data-am install-data install-am install \
uninstall-am uninstall all-redirect all-am all installdirs \
mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


bzflag.res: bzflag.rc bzflag.ico
	windres -i bzflag.rc -o bzflag.res -O coff

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
