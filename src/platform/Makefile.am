SUBDIRS = MacBZFlag.pbproj

if LINUX
______lib_libPlatform_a_LIBADD = \
	LinuxPlatformFactory.$(OBJEXT)	\
	LinuxDisplay.$(OBJEXT)		\
	LinuxMedia.$(OBJEXT)		\
	usbjoy.$(OBJEXT)		\
	XDisplay.$(OBJEXT)		\
	XVisual.$(OBJEXT)		\
	XWindow.$(OBJEXT)
else
if SOLARIS
______lib_libPlatform_a_LIBADD = \
	SolarisPlatformFactory.$(OBJEXT)\
	SolarisMedia.$(OBJEXT)		\
	XDisplay.$(OBJEXT)		\
	XVisual.$(OBJEXT)		\
	XWindow.$(OBJEXT)
else
if IRIX
______lib_libPlatform_a_LIBADD =	\
	SGIPlatformFactory.$(OBJEXT)	\
	SGIDisplay.$(OBJEXT)		\
	SGIMedia.$(OBJEXT)		\
	XDisplay.$(OBJEXT)		\
	XVisual.$(OBJEXT)		\
	XWindow.$(OBJEXT)
else
if MACOS
______lib_libPlatform_a_LIBADD =	\
	MacPlatformFactory.$(OBJEXT) 	\
	MacDisplay.$(OBJEXT)		\
	MacVisual.$(OBJEXT)		\
	MacWindow.$(OBJEXT)		\
	MacMedia.$(OBJEXT)
else
if WIN32
______lib_libPlatform_a_LIBADD =	\
	WinPlatformFactory.$(OBJEXT) 	\
	WinDisplay.$(OBJEXT)		\
	WinVisual.$(OBJEXT)		\
	WinWindow.$(OBJEXT)		\
	WinMedia.$(OBJEXT)
endif
endif
endif
endif
endif

noinst_LIBRARIES = ../../lib/libPlatform.a

______lib_libPlatform_a_SOURCES  = 	\
	PlatformFactory.cxx		\
	BzfDisplay.cxx			\
	BzfVisual.cxx			\
	BzfWindow.cxx			\
	BzfMedia.cxx			\
	wave.cxx			\
	wave.h

EXTRA_______lib_libPlatform_a_SOURCES = \
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

EXTRA_DIST = MacBZFlag.icns MacBZFlag-prefix.h NewMedia.cxx NewMedia.h \
	dsound.h ddraw.h d3dtypes.h
