SUBDIRS = English.lproj BZFlag.pbproj

libMac_a_SOURCES =			\
	MacPlatformFactory.cxx		\
	MacPlatformFactory.h		\
	MacDisplay.cxx			\
	MacDisplay.h			\
	MacVisual.cxx			\
	MacVisual.h			\
	MacWindow.cxx			\
	MacWindow.h			\
	MacMedia.cxx			\
	MacMedia.h

if MACOS
if CLIENT_INCLUDED
noinst_LIBRARIES = libMac.a
endif
endif

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST =				\
	BZFlag_Prefix.h			\
	MacBZFlag.icns			\
	main.m				\
	mkplist.cxx
