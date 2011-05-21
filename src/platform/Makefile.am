noinst_LTLIBRARIES = libPlatform.la

libPlatform_la_LDFLAGS = -static
libPlatform_la_SOURCES = \
	$(BEOS_SOURCES)				\
	$(HPUX_SOURCES)				\
	$(IRIX_SOURCES)				\
	$(LINUX_NATIVE_SOURCES)			\
	$(LINUX_SDL_SOURCES)			\
	$(MACOSX_SDL_SOURCES)			\
	$(SOLARIS_SOURCES)			\
	$(WIN32_SOURCES)			\
	BzfDisplay.cpp				\
	BzfJoystick.cpp				\
	BzfMedia.cpp				\
	BzfVisual.cpp				\
	BzfWindow.cpp				\
	Clipboard.cpp				\
	PlatformFactory.cpp			\
	VerticalSync.cpp			\
	wave.cpp

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

SDL_FILES = \
	SDLDisplay.cpp				\
	SDLJoystick.cpp				\
	SDLMedia.cpp				\
	SDLVisual.cpp				\
	SDLWindow.cpp

X11_FILES = \
	XDisplay.cpp				\
	XVisual.cpp				\
	XWindow.cpp

LINUX_SDL_FILES = \
	$(SDL_FILES) 				\
	SDLPlatformFactory.cpp			\
	EvdevJoystick.cpp

LINUX_NATIVE_FILES =				\
	$(X11_FILES)				\
	EvdevJoystick.cpp			\
	LinuxDisplay.cpp			\
	LinuxMedia.cpp				\
	LinuxPlatformFactory.cpp		\
	USBJoystick.cpp

SOLARIS_FILES =					\
	$(X11_FILES)				\
	SolarisMedia.cpp			\
	SolarisPlatformFactory.cpp

IRIX_FILES =					\
	$(X11_FILES)				\
	SGIDisplay.cpp				\
	SGIMedia.cpp				\
	SGIPlatformFactory.cpp

MACOSX_SDL_FILES =				\
	$(SDL_FILES)				\
	SDLPlatformFactory.cpp

BEOS_FILES =					\
	BeOSDisplay.cpp				\
	BeOSMedia.cpp				\
	BeOSPlatformFactory.cpp			\
	BeOSVisual.cpp				\
	BeOSWindow.cpp

WIN32_FILES =					\
	$(SDL_FILES)				\
	DXJoystick.cpp				\
	WinDisplay.cpp				\
	WinJoystick.cpp				\
	WinMedia.cpp				\
	WinPlatformFactory.cpp			\
	WinVisual.cpp				\
	WinWindow.cpp

HPUX_FILES =					\
	HPUXMedia.cpp				\
	HPUXPlatformFactory.cpp

if HAVE_SDL
if LINUX
LINUX_SDL_SOURCES = $(LINUX_SDL_FILES)
endif
else !HAVE_SDL
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
	NewMedia.cpp \
	README

AM_CPPFLAGS = -I$(top_srcdir)/src/include $(SDL_CFLAGS) @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

MAINTAINERCLEANFILES = Makefile.in
