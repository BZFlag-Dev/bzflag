noinst_LTLIBRARIES = libPlatform.la

libPlatform_la_LDFLAGS = -static

if HAVE_SDL
if LINUX
libPlatform_la_SOURCES =			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	EvdevJoystick.cxx			\
	EvdevJoystick.h				\
	PlatformFactory.cxx			\
	SDLDisplay.cxx				\
	SDLDisplay.h				\
	SDLJoystick.cxx				\
	SDLJoystick.h				\
	SDLMedia.cxx				\
	SDLMedia.h				\
	SDLPlatformFactory.cxx			\
	SDLPlatformFactory.h			\
	SDLVisual.cxx				\
	SDLVisual.h				\
	SDLWindow.cxx				\
	SDLWindow.h
endif
else
if LINUX
libPlatform_la_SOURCES =			\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	wave.cxx				\
	wave.h					\
	LinuxPlatformFactory.cxx		\
	LinuxPlatformFactory.h			\
	LinuxDisplay.cxx			\
	LinuxDisplay.h				\
	LinuxMedia.cxx				\
	LinuxMedia.h				\
	EvdevJoystick.cxx			\
	EvdevJoystick.h				\
	USBJoystick.cxx				\
	USBJoystick.h				\
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
	SDLDisplay.h				\
	SDLJoystick.cxx				\
	SDLJoystick.h				\
	SDLMedia.cxx				\
	SDLMedia.h				\
	SDLVisual.cxx				\
	SDLVisual.h				\
	SDLWindow.cxx				\
	SDLWindow.h				\
	wave.cxx				\
	wave.h
else
libPlatform_la_SOURCES =			\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	wave.cxx				\
	wave.h
endif
endif
DIST_SUBDIRS = MacOSX

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
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	DXJoystick.cxx				\
	DXJoystick.h				\
	PlatformFactory.cxx			\
	SDLDisplay.cxx				\
	SDLDisplay.h				\
	SDLJoystick.cxx				\
	SDLJoystick.h				\
	SDLMedia.cxx				\
	SDLMedia.h				\
	SDLVisual.cxx				\
	SDLVisual.h				\
	SDLWindow.cxx				\
	SDLWindow.h				\
	WinDisplay.cxx				\
	WinDisplay.h				\
	WinJoystick.cxx				\
	WinJoystick.h				\
	WinMedia.cxx				\
	WinMedia.h				\
	WinPlatformFactory.cxx			\
	WinPlatformFactory.h			\
	WinVisual.cxx				\
	WinVisual.h				\
	WinWindow.cxx				\
	WinWindow.h				\
	wave.cxx				\
	wave.h
endif

if HPUX
libPlatform_la_SOURCES =			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	HPUXMedia.cxx				\
	HPUXMedia.h				\
	HPUXPlatformFactory.cxx			\
	HPUXPlatformFactory.h			\
	PlatformFactory.cxx			\
	wave.cxx				\
	wave.h
endif

EXTRA_libPlatform_la_SOURCES =		\
	BeOSDisplay.cxx			\
	BeOSDisplay.h			\
	BeOSMedia.cxx			\
	BeOSMedia.h			\
	BeOSPlatformFactory.cxx		\
	BeOSPlatformFactory.h		\
	BeOSVisual.cxx			\
	BeOSVisual.h			\
	BeOSWindow.cxx			\
	BeOSWindow.h			\
	BeOSWindow2.cxx			\
	EvdevJoystick.cxx		\
	EvdevJoystick.h			\
	HPUXMedia.cxx			\
	HPUXMedia.h			\
	HPUXPlatformFactory.cxx		\
	HPUXPlatformFactory.h		\
	LinuxDisplay.cxx		\
	LinuxDisplay.h			\
	LinuxMedia.cxx			\
	LinuxMedia.h			\
	LinuxPlatformFactory.cxx	\
	LinuxPlatformFactory.h		\
	SDLDisplay.cxx			\
	SDLDisplay.h			\
	SDLJoystick.cxx			\
	SDLJoystick.h			\
	SDLMedia.cxx			\
	SDLMedia.h			\
	SDLPlatformFactory.cxx		\
	SDLPlatformFactory.h		\
	SDLVisual.cxx			\
	SDLVisual.h			\
	SDLWindow.cxx			\
	SDLWindow.h			\
	SGIDisplay.cxx			\
	SGIDisplay.h			\
	SGIMedia.cxx			\
	SGIMedia.h			\
	SGIPlatformFactory.cxx		\
	SGIPlatformFactory.h		\
	SolarisMedia.cxx		\
	SolarisMedia.h			\
	SolarisPlatformFactory.cxx	\
	SolarisPlatformFactory.h	\
	USBJoystick.cxx			\
	USBJoystick.h			\
	WinDisplay.cxx			\
	WinDisplay.h			\
	WinJoystick.cxx			\
	WinJoystick.h			\
	WinMedia.cxx			\
	WinMedia.h			\
	WinPlatformFactory.cxx		\
	WinPlatformFactory.h		\
	WinVisual.cxx			\
	WinVisual.h			\
	WinWindow.cxx			\
	WinWindow.h			\
	XDisplay.cxx			\
	XDisplay.h			\
	XVisual.cxx			\
	XVisual.h			\
	XWindow.cxx			\
	XWindow.h			\
	wave.cxx			\
	wave.h

AM_CPPFLAGS = -I$(top_srcdir)/include $(SDL_CFLAGS) @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@


EXTRA_DIST = \
	NewMedia.cxx \
	NewMedia.h \
	README

MAINTAINERCLEANFILES = \
	Makefile.in
