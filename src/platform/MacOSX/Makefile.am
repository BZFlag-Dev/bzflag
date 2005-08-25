if MACOS
if CLIENT_INCLUDED
noinst_LTLIBRARIES = libMac.la
endif
endif

libMac_la_LDFLAGS = -static
libMac_la_SOURCES =			\
	MacDisplay.cxx			\
	MacDisplay.h			\
	MacMedia.cxx			\
	MacMedia.h			\
	MacPlatformFactory.cxx		\
	MacPlatformFactory.h		\
	MacVisual.cxx			\
	MacVisual.h			\
	MacWindow.cxx			\
	MacWindow.h			\
	SDLMain.h

libMac_la_LIBADD = SDLMain.lo

all: SDLMain.lo

SDLMain.lo:
	$(LTCOMPILE) -c -o $@ `test -f 'SDLMain.m' || echo '$(srcdir)/'`SDLMain.m

AM_CPPFLAGS = $(SDL_CFLAGS) -I$(top_srcdir)/src/platform

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST =				\
	BZFlag-Info.plist		\
	MacBZFlag.icns			\
	SDLMain.m			\
	mkplist.cxx
