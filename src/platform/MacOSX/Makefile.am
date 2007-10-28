if MACOS
noinst_LTLIBRARIES = libMac.la
endif

libMac_la_LDFLAGS = -static

if HAVE_SDL

libMac_la_SOURCES =			\
	MacDataPath.cxx			\
	SDLMain.h
libMac_la_LIBADD = SDLMain.lo
all: SDLMain.lo

else

libMac_la_SOURCES =			\
	MacDataPath.cxx			\
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

endif

SDLMain.lo:
	$(LTCOMPILE) -c -o $@ `test -f 'SDLMain.m' || echo '$(srcdir)/'`SDLMain.m

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/platform $(SDL_CFLAGS) @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST =				\
	BZFlag-Info.plist		\
	MacBZFlag.icns			\
	SDLMain.m			\
	mkplist.cxx
