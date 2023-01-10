if CLIENT_INCLUDED
noinst_LTLIBRARIES = libPlatform.la
endif

libPlatform_la_LDFLAGS = -static

libPlatform_la_SOURCES =			\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	SDLJoystick.cxx				\
	SDLJoystick.h				\
	SDLPlatformFactory.cxx			\
	SDLPlatformFactory.h			\
	SDLMedia.cxx				\
	SDLMedia.h				\
	SDL2Display.cxx				\
	SDL2Display.h				\
	SDL2Visual.cxx				\
	SDL2Visual.h				\
	SDL2Window.cxx				\
	SDL2Window.h

if APPLE
libPlatform_la_SOURCES +=    \
	MacDataPath.cxx
endif

EXTRA_libPlatform_la_SOURCES =		\
	MacDataPath.cxx

AM_CPPFLAGS = -DINSTALL_DATA_DIR=\"$(pkgdatadir)\" \
	$(SDL_CFLAGS) \
	$(CONF_CPPFLAGS) \
	-I$(top_srcdir)/include
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	NewMedia.cxx \
	NewMedia.h \
	SDLMain.h \
	SDLMain.m \
	README

MAINTAINERCLEANFILES = \
	Makefile.in
