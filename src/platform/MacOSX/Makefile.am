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

if HAVE_SDL
libMac_la_LIBADD = SDLMain.lo
all: SDLMain.lo
endif

SDLMain.lo:
	$(LTCOMPILE) -c -o $@ `test -f 'SDLMain.m' || echo '$(srcdir)/'`SDLMain.m

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/platform $(SDL_CFLAGS) @CPPFLAGS@
AM_CXXFLAGS = @CXXFLAGS@

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST =				\
	BZFlag-Info.plist		\
	MacBZFlag.icns			\
	SDLMain.m			\
	mkplist.cxx
