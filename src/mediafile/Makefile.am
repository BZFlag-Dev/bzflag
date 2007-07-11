noinst_LTLIBRARIES = libMediaFile.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libMediaFile_la_LDFLAGS = -static
libMediaFile_la_SOURCES =	\
	AudioFile.cxx		\
	AudioFile.h		\
	CacheManager.cxx	\
	ImageFile.cxx		\
	ImageFile.h		\
	MediaFile.cxx		\
	PNGImageFile.cxx	\
	PNGImageFile.h		\
	SGIImageFile.cxx	\
	SGIImageFile.h		\
	WaveAudioFile.cxx	\
	WaveAudioFile.h

EXTRA_DIST = \
	OggAudioFile.cxx	\
	OggAudioFile.h		\
	README
