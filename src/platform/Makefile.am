if CLIENT_INCLUDED
noinst_LTLIBRARIES = libPlatform.la
endif

libPlatform_la_LDFLAGS = -static

if LINUX
libPlatform_la_SOURCES =			\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	EvdevJoystick.cxx			\
	EvdevJoystick.h
if HAVE_SDL
libPlatform_la_SOURCES +=			\
	SDLJoystick.cxx				\
	SDLJoystick.h				\
	SDLPlatformFactory.cxx			\
	SDLPlatformFactory.h			\
	SDLMedia.cxx				\
	SDLMedia.h
if HAVE_SDL2
libPlatform_la_SOURCES +=			\
	SDL2Display.cxx				\
	SDL2Display.h				\
	SDL2Visual.cxx				\
	SDL2Visual.h				\
	SDL2Window.cxx				\
	SDL2Window.h
else
libPlatform_la_SOURCES +=			\
	SDLDisplay.cxx				\
	SDLDisplay.h
endif
else
libPlatform_la_SOURCES +=			\
	wave.cxx				\
	wave.h					\
	LinuxPlatformFactory.cxx		\
	LinuxPlatformFactory.h			\
	LinuxDisplay.cxx			\
	LinuxDisplay.h				\
	LinuxMedia.cxx				\
	LinuxMedia.h				\
	USBJoystick.cxx				\
	USBJoystick.h				\
	XIJoystick.cxx				\
	XIJoystick.h				\
	XDisplay.cxx				\
	XDisplay.h				\
	XVisual.cxx				\
	XVisual.h				\
	XWindow.cxx				\
	XWindow.h
endif
endif

if SOLARIS
libPlatform_la_SOURCES =			\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	wave.cxx				\
	wave.h					\
	SolarisPlatformFactory.cxx		\
	SolarisPlatformFactory.h		\
	SolarisMedia.cxx			\
	SolarisMedia.h				\
	XDisplay.cxx				\
	XDisplay.h				\
	XVisual.cxx				\
	XVisual.h				\
	XWindow.cxx				\
	XWindow.h
endif

if IRIX
libPlatform_la_SOURCES =			\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	wave.cxx				\
	wave.h					\
	SGIPlatformFactory.cxx			\
	SGIPlatformFactory.h			\
	SGIDisplay.cxx				\
	SGIDisplay.h				\
	SGIMedia.cxx				\
	SGIMedia.h				\
	XDisplay.cxx				\
	XDisplay.h				\
	XVisual.cxx				\
	XVisual.h				\
	XWindow.cxx				\
	XWindow.h
endif


if BEOS
libPlatform_la_SOURCES =			\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	wave.cxx				\
	wave.h					\
	BeOSPlatformFactory.cxx			\
	BeOSPlatformFactory.h			\
	BeOSDisplay.cxx				\
	BeOSDisplay.h				\
	BeOSVisual.cxx				\
	BeOSVisual.h				\
	BeOSWindow.cxx				\
	BeOSWindow.h				\
	BeOSMedia.cxx				\
	BeOSMedia.h
endif

if WIN32
libPlatform_la_SOURCES =			\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	SDLDisplay.h				\
	SDLDisplay.cxx				\
	SDLJoystick.h				\
	SDLJoystick.cxx				\
	SDLMedia.cxx				\
	SDLMedia.h				\
	wave.cxx				\
	wave.h					\
	WinPlatformFactory.cxx			\
	WinPlatformFactory.h			\
	WinDisplay.cxx				\
	WinDisplay.h				\
	DXJoystick.cxx				\
	DXJoystick.h				\
	WinJoystick.cxx				\
	WinJoystick.h				\
	WinVisual.cxx				\
	WinVisual.h				\
	WinWindow.cxx				\
	WinWindow.h				\
	WinMedia.cxx				\
	WinMedia.h
endif

if APPLE
libPlatform_la_SOURCES =    \
	PlatformFactory.cxx     \
	BzfDisplay.cxx          \
	BzfJoystick.cxx         \
	BzfVisual.cxx           \
	BzfWindow.cxx           \
	BzfMedia.cxx            \
	wave.cxx                \
	wave.h                  \
	MacDataPath.cxx

if HAVE_SDL
libPlatform_la_SOURCES +=   \
	SDLJoystick.cxx         \
	SDLJoystick.h           \
	SDLPlatformFactory.cxx  \
	SDLPlatformFactory.h    \
	SDLMedia.cxx            \
	SDLMedia.h
if HAVE_SDL2
libPlatform_la_SOURCES +=   \
	SDL2Display.cxx         \
	SDL2Display.h           \
	SDL2Visual.cxx          \
	SDL2Visual.h            \
	SDL2Window.cxx          \
	SDL2Window.h
else
libPlatform_la_SOURCES +=   \
	SDLMain.h               \
	SDLMain.m               \
	SDLDisplay.cxx          \
	SDLDisplay.h
endif
endif

endif

EXTRA_libPlatform_la_SOURCES =		\
	BeOSPlatformFactory.cxx		\
	BeOSPlatformFactory.h		\
	BeOSDisplay.cxx			\
	BeOSDisplay.h			\
	BeOSVisual.cxx			\
	BeOSVisual.h			\
	BeOSWindow.cxx			\
	BeOSWindow.h			\
	BeOSWindow2.cxx			\
	BeOSMedia.cxx			\
	BeOSMedia.h			\
	LinuxPlatformFactory.cxx	\
	LinuxPlatformFactory.h		\
	LinuxDisplay.cxx		\
	LinuxDisplay.h			\
	LinuxMedia.cxx			\
	LinuxMedia.h			\
	MacDataPath.cxx			\
	SDLJoystick.cxx			\
	SDLJoystick.h			\
	EvdevJoystick.cxx		\
	EvdevJoystick.h			\
	SDLPlatformFactory.cxx		\
	SDLPlatformFactory.h		\
	SDLMedia.cxx			\
	SDLMedia.h			\
	SDLDisplay.h			\
	SDLDisplay.cxx			\
	SGIPlatformFactory.cxx		\
	SGIPlatformFactory.h		\
	SGIDisplay.cxx			\
	SGIDisplay.h			\
	SGIMedia.cxx			\
	SGIMedia.h			\
	SolarisPlatformFactory.cxx	\
	SolarisPlatformFactory.h	\
	SolarisMedia.cxx		\
	SolarisMedia.h			\
	USBJoystick.cxx			\
	USBJoystick.h			\
	WinPlatformFactory.cxx		\
	WinPlatformFactory.h		\
	WinDisplay.cxx			\
	WinDisplay.h			\
	WinJoystick.cxx			\
	WinJoystick.h			\
	WinVisual.cxx			\
	WinVisual.h			\
	WinWindow.cxx			\
	WinWindow.h			\
	WinMedia.cxx			\
	WinMedia.h			\
	XDisplay.cxx			\
	XDisplay.h			\
	XIJoystick.cxx			\
	XIJoystick.h			\
	XVisual.cxx			\
	XVisual.h			\
	XWindow.cxx			\
	XWindow.h			\
	wave.cxx			\
	wave.h

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
