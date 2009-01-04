noinst_LTLIBRARIES = libPlatform.la

libPlatform_la_LDFLAGS = -static

noinst_HEADERS = \
	BeOSDisplay.h				\
	BeOSMedia.h				\
	BeOSPlatformFactory.h			\
	BeOSVisual.h				\
	BeOSWindow.h				\
	DXJoystick.h				\
	EvdevJoystick.h				\
	HPUXMedia.h				\
	HPUXPlatformFactory.h			\
	LinuxDisplay.h				\
	LinuxMedia.h				\
	LinuxPlatformFactory.h			\
	NewMedia.h				\
	SDLDisplay.h				\
	SDLJoystick.h				\
	SDLMedia.h				\
	SDLPlatformFactory.h			\
	SDLVisual.h				\
	SDLWindow.h				\
	SGIDisplay.h				\
	SGIMedia.h				\
	SGIPlatformFactory.h			\
	SolarisMedia.h				\
	SolarisPlatformFactory.h		\
	USBJoystick.h				\
	WinDisplay.h				\
	WinJoystick.h				\
	WinMedia.h				\
	WinPlatformFactory.h			\
	WinVisual.h				\
	WinWindow.h				\
	XDisplay.h				\
	XVisual.h				\
	XWindow.h				\
	wave.h

if HAVE_SDL
if LINUX
libPlatform_la_SOURCES =			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	EvdevJoystick.cxx			\
	PlatformFactory.cxx			\
	SDLDisplay.cxx				\
	SDLJoystick.cxx				\
	SDLMedia.cxx				\
	SDLPlatformFactory.cxx			\
	SDLVisual.cxx				\
	SDLWindow.cxx				\
	VerticalSync.cxx
endif
else
if LINUX
libPlatform_la_SOURCES =			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	EvdevJoystick.cxx			\
	LinuxDisplay.cxx			\
	LinuxMedia.cxx				\
	LinuxPlatformFactory.cxx		\
	PlatformFactory.cxx			\
	USBJoystick.cxx				\
	VerticalSync.cxx			\
	XDisplay.cxx				\
	XVisual.cxx				\
	XWindow.cxx				\
	wave.cxx
endif
endif

if SOLARIS
libPlatform_la_SOURCES =			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	PlatformFactory.cxx			\
	SolarisMedia.cxx			\
	SolarisPlatformFactory.cxx		\
	VerticalSync.cxx			\
	XDisplay.cxx				\
	XVisual.cxx				\
	XWindow.cxx				\
	wave.cxx
endif

if IRIX
libPlatform_la_SOURCES =			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	PlatformFactory.cxx			\
	SGIDisplay.cxx				\
	SGIMedia.cxx				\
	SGIPlatformFactory.cxx			\
	VerticalSync.cxx			\
	XDisplay.cxx				\
	XVisual.cxx				\
	XWindow.cxx				\
	wave.cxx
endif

if MACOS
SUBDIRS = MacOSX

if HAVE_SDL
libPlatform_la_SOURCES =			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	PlatformFactory.cxx			\
	SDLDisplay.cxx				\
	SDLJoystick.cxx				\
	SDLMedia.cxx				\
	SDLPlatformFactory.cxx			\
	SDLVisual.cxx				\
	SDLWindow.cxx				\
	VerticalSync.cxx			\
	wave.cxx
else
libPlatform_la_SOURCES =			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	PlatformFactory.cxx			\
	VerticalSync.cxx			\
	wave.cxx
endif
endif
DIST_SUBDIRS = MacOSX

if BEOS
libPlatform_la_SOURCES =			\
	BeOSDisplay.cxx				\
	BeOSMedia.cxx				\
	BeOSPlatformFactory.cxx			\
	BeOSVisual.cxx				\
	BeOSWindow.cxx				\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	PlatformFactory.cxx			\
	VerticalSync.cxx			\
	wave.cxx
endif

if WIN32
libPlatform_la_SOURCES =			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	DXJoystick.cxx				\
	PlatformFactory.cxx			\
	SDLDisplay.cxx				\
	SDLJoystick.cxx				\
	SDLMedia.cxx				\
	SDLVisual.cxx				\
	SDLWindow.cxx				\
	VerticalSync.cxx			\
	WinDisplay.cxx				\
	WinJoystick.cxx				\
	WinMedia.cxx				\
	WinPlatformFactory.cxx			\
	WinVisual.cxx				\
	WinWindow.cxx				\
	wave.cxx
endif

if HPUX
libPlatform_la_SOURCES =			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	HPUXMedia.cxx				\
	HPUXPlatformFactory.cxx			\
	PlatformFactory.cxx			\
	VerticalSync.cxx			\
	wave.cxx
endif

EXTRA_DIST =	\
	BeOSDisplay.cxx			\
	BeOSMedia.cxx			\
	BeOSPlatformFactory.cxx		\
	BeOSVisual.cxx			\
	BeOSWindow.cxx			\
	BeOSWindow2.cxx			\
	DXJoystick.cxx			\
	EvdevJoystick.cxx		\
	HPUXMedia.cxx			\
	HPUXPlatformFactory.cxx		\
	LinuxDisplay.cxx		\
	LinuxMedia.cxx			\
	LinuxPlatformFactory.cxx	\
	NewMedia.cxx			\
	README				\
	SDLDisplay.cxx			\
	SDLJoystick.cxx			\
	SDLMedia.cxx			\
	SDLPlatformFactory.cxx		\
	SDLVisual.cxx			\
	SDLWindow.cxx			\
	SGIDisplay.cxx			\
	SGIMedia.cxx			\
	SGIPlatformFactory.cxx		\
	SolarisMedia.cxx		\
	SolarisPlatformFactory.cxx	\
	USBJoystick.cxx			\
	WinDisplay.cxx			\
	WinJoystick.cxx			\
	WinMedia.cxx			\
	WinPlatformFactory.cxx		\
	WinVisual.cxx			\
	WinWindow.cxx			\
	XDisplay.cxx			\
	XVisual.cxx			\
	XWindow.cxx			\
	wave.cxx

AM_CPPFLAGS = -I$(top_srcdir)/include $(SDL_CFLAGS) @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

MAINTAINERCLEANFILES = Makefile.in
