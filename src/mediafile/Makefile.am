noinst_LTLIBRARIES = libMediaFile.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/src/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libMediaFile_la_LDFLAGS = -static
libMediaFile_la_SOURCES =	\
	AudioFile.cpp		\
	AudioFile.h		\
	ImageFile.cpp		\
	ImageFile.h		\
	MediaFile.cpp		\
	PNGImageFile.cpp	\
	PNGImageFile.h		\
	SGIImageFile.cpp	\
	SGIImageFile.h		\
	WaveAudioFile.cpp	\
	WaveAudioFile.h

EXTRA_DIST = \
	OggAudioFile.cpp	\
	OggAudioFile.h		\
	README
