if MACOS
noinst_LTLIBRARIES = libMac.la
endif

libMac_la_LDFLAGS = -static

MAC_FILES =				\
	MacDataPath.cpp			\
	MacDisplay.cpp			\
	MacDisplay.h			\
	MacMedia.cpp			\
	MacMedia.h			\
	MacPlatformFactory.cpp		\
	MacPlatformFactory.h		\
	MacVisual.cpp			\
	MacVisual.h			\
	MacWindow.cpp			\
	MacWindow.h

if HAVE_SDL

libMac_la_SOURCES =			\
	MacDataPath.cpp			\
	SDLMain.h
libMac_la_LIBADD = SDLMain.lo
all: SDLMain.lo

else

libMac_la_SOURCES =			\
	$(MAC_FILES)			\
	SDLMain.h

endif

SDLMain.lo:
	$(LTCOMPILE) -c -o $@ `test -f 'SDLMain.m' || echo '$(srcdir)/'`SDLMain.m

AM_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/platform $(SDL_CFLAGS) @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST =				\
	$(MAC_FILES)			\
	BZFlag-Info.plist		\
	MacBZFlag.icns			\
	SDLMain.m			\
	mkplist.cpp
