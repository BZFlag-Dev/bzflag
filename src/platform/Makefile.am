if HAVE_SDL
if LINUX
libPlatform_a_SOURCES =				\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	SDLJoystick.cxx				\
	SDLJoystick.h				\
	EvdevJoystick.cxx			\
	EvdevJoystick.h				\
	SDLPlatformFactory.cxx			\
	SDLPlatformFactory.h			\
	SDLMedia.cxx				\
	SDLMedia.h				\
	SDLDisplay.h				\
	SDLDisplay.cxx
endif
else
if LINUX
libPlatform_a_SOURCES =				\
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
libPlatform_a_SOURCES =				\
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
libPlatform_a_SOURCES =				\
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
libPlatform_a_SOURCES =				\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	SDLJoystick.cxx				\
	SDLJoystick.h				\
	SDLMedia.cxx				\
	SDLMedia.h				\
	SDLDisplay.h				\
	SDLDisplay.cxx				\
	wave.cxx				\
	wave.h
else
libPlatform_a_SOURCES =				\
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
libPlatform_a_SOURCES =				\
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
libPlatform_a_SOURCES =				\
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
	WinJoystick.cxx				\
	WinJoystick.h				\
	WinVisual.cxx				\
	WinVisual.h				\
	WinWindow.cxx				\
	WinWindow.h				\
	WinMedia.cxx				\
	WinMedia.h
endif

if CLIENT_INCLUDED
noinst_LIBRARIES = libPlatform.a
endif

EXTRA_libPlatform_a_SOURCES =		\
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

AM_CPPFLAGS = -I$(top_srcdir)/include $(SDL_CFLAGS)


EXTRA_DIST = \
	NewMedia.cxx \
	NewMedia.h \
	README

MAINTAINERCLEANFILES = \
	Makefile.in
