# SUBDIRS mult always be processed
# so they end up in the dist
SUBDIRS = MacOSX

if LINUX
libPlatform_a_SOURCES =				\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
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
	usbjoy.cxx				\
	XDisplay.cxx				\
	XDisplay.h				\
	XVisual.cxx				\
	XVisual.h				\
	XWindow.cxx				\
	XWindow.h
else
if SOLARIS
libPlatform_a_SOURCES = 			\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
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
else
if IRIX
libPlatform_a_SOURCES =				\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
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
else
if MACOS
libPlatform_a_SOURCES =				\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	wave.cxx				\
	wave.h
else
if WIN32
libPlatform_a_SOURCES =				\
	PlatformFactory.cxx			\
	BzfDisplay.cxx				\
	BzfVisual.cxx				\
	BzfWindow.cxx				\
	BzfMedia.cxx				\
	wave.cxx				\
	wave.h					\
	WinPlatformFactory.cxx 			\
	WinPlatformFactory.h 			\
	WinDisplay.cxx				\
	WinDisplay.h				\
	WinVisual.cxx				\
	WinVisual.h				\
	WinWindow.cxx				\
	WinWindow.h				\
	WinMedia.cxx				\
	WinMedia.h
endif
endif
endif
endif
endif

if CLIENT_INCLUDED
noinst_LIBRARIES = libPlatform.a
endif

EXTRA_libPlatform_a_SOURCES = 		\
 	XDisplay.cxx			\
 	XDisplay.h			\
 	XVisual.cxx			\
 	XVisual.h			\
 	XWindow.cxx			\
 	XWindow.h			\
 	LinuxPlatformFactory.cxx	\
 	LinuxPlatformFactory.h		\
 	LinuxDisplay.cxx		\
 	LinuxDisplay.h			\
 	LinuxMedia.cxx			\
 	LinuxMedia.h			\
 	usbjoy.cxx			\
 	SolarisPlatformFactory.cxx	\
 	SolarisPlatformFactory.h	\
 	SolarisMedia.cxx		\
 	SolarisMedia.h			\
 	SGIPlatformFactory.cxx		\
 	SGIPlatformFactory.h		\
 	SGIDisplay.cxx			\
 	SGIDisplay.h			\
 	SGIMedia.cxx			\
 	SGIMedia.h			\
 	MacPlatformFactory.cxx 		\
 	MacPlatformFactory.h 		\
 	MacDisplay.cxx			\
 	MacDisplay.h			\
 	MacVisual.cxx			\
 	MacVisual.h			\
 	MacWindow.cxx			\
 	MacWindow.h			\
 	MacMedia.cxx			\
 	MacMedia.h			\
 	WinPlatformFactory.cxx 		\
 	WinPlatformFactory.h 		\
 	WinDisplay.cxx			\
 	WinDisplay.h			\
 	WinVisual.cxx			\
 	WinVisual.h			\
 	WinWindow.cxx			\
 	WinWindow.h			\
 	WinMedia.cxx			\
 	WinMedia.h

EXTRA_DIST = MacBZFlag.icns MacBZFlag-prefix.h NewMedia.cxx NewMedia.h

MAINTAINERCLEANFILES = \
	Makefile.in
