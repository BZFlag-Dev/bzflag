noinst_LTLIBRARIES = libPlatform.la

libPlatform_la_LDFLAGS = -static
libPlatform_la_SOURCES = \
	$(BEOS_SOURCES) \
	$(HPUX_SOURCES) \
	$(IRIX_SOURCES) \
	$(LINUX_NATIVE_SOURCES) \
	$(LINUX_SDL_SOURCES) \
	$(MACOSX_SDL_SOURCES) \
	$(SOLARIS_SOURCES) \
	$(WIN32_SOURCES) \
	BzfDisplay.cxx				\
	BzfJoystick.cxx				\
	BzfMedia.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	PlatformFactory.cxx			\
	VerticalSync.cxx			\
	wave.cxx

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

LINUX_SDL_FILES = \
	EvdevJoystick.cxx			\
	SDLDisplay.cxx				\
	SDLJoystick.cxx				\
	SDLMedia.cxx				\
	SDLPlatformFactory.cxx			\
	SDLVisual.cxx				\
	SDLWindow.cxx

LINUX_NATIVE_FILES =				\
	EvdevJoystick.cxx			\
	LinuxDisplay.cxx			\
	LinuxMedia.cxx				\
	LinuxPlatformFactory.cxx		\
	USBJoystick.cxx				\
	XDisplay.cxx				\
	XVisual.cxx				\
	XWindow.cxx

SOLARIS_FILES =					\
	SolarisMedia.cxx			\
	SolarisPlatformFactory.cxx		\
	XDisplay.cxx				\
	XVisual.cxx				\
	XWindow.cxx

IRIX_FILES =					\
	SGIDisplay.cxx				\
	SGIMedia.cxx				\
	SGIPlatformFactory.cxx			\
	XDisplay.cxx				\
	XVisual.cxx				\
	XWindow.cxx

MACOSX_SDL_FILES =				\
	SDLDisplay.cxx				\
	SDLJoystick.cxx				\
	SDLMedia.cxx				\
	SDLPlatformFactory.cxx			\
	SDLVisual.cxx				\
	SDLWindow.cxx

BEOS_FILES =					\
	BeOSDisplay.cxx				\
	BeOSMedia.cxx				\
	BeOSPlatformFactory.cxx			\
	BeOSVisual.cxx				\
	BeOSWindow.cxx

WIN32_FILES =					\
	DXJoystick.cxx				\
	SDLDisplay.cxx				\
	SDLJoystick.cxx				\
	SDLMedia.cxx				\
	SDLVisual.cxx				\
	SDLWindow.cxx				\
	WinDisplay.cxx				\
	WinJoystick.cxx				\
	WinMedia.cxx				\
	WinPlatformFactory.cxx			\
	WinVisual.cxx				\
	WinWindow.cxx

HPUX_FILES =					\
	HPUXMedia.cxx				\
	HPUXPlatformFactory.cxx

if HAVE_SDL
if LINUX
LINUX_SDL_SOURCES = $(LINUX_SDL_FILES)
endif
else
if LINUX
LINUX_NATIVE_SOURCES = $(LINUX_NATIVE_FILES)
endif
endif

if SOLARIS
SOLARIS_SOURCES = $(SOLARIS_FILES)
endif

if IRIX
IRIX_SOURCES = $(IRIX_FILES)
endif

if MACOS
SUBDIRS = MacOSX

if HAVE_SDL
MACOSX_SDL_SOURCES = $(MACOSX_SDL_FILES)
endif
endif
DIST_SUBDIRS = MacOSX

if BEOS
BEOS_SOURCES = $(BEOS_FILES)
endif

if WIN32
WIN32_SOURCES = $(WIN32_FILES)
endif

if HPUX
HPUX_SOURCES = $(HPUX_FILES)
endif

EXTRA_DIST = \
	$(BEOS_FILES) \
	$(HPUX_FILES) \
	$(IRIX_FILES) \
	$(LINUX_NATIVE_FILES) \
	$(LINUX_SDL_FILES) \
	$(MACOSX_SDL_FILES) \
	$(SOLARIS_FILES) \
	$(WIN32_FILES) \
	NewMedia.cxx \
	README

AM_CPPFLAGS = -I$(top_srcdir)/include $(SDL_CFLAGS) @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

MAINTAINERCLEANFILES = Makefile.in
