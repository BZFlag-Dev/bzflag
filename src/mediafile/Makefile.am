if CLIENT_INCLUDED
noinst_LTLIBRARIES = libMediaFile.la
endif

AM_CPPFLAGS = -DINSTALL_DATA_DIR=\"$(pkgdatadir)\" $(CONF_CPPFLAGS) \
	      -I$(top_srcdir)/include
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES = \
	Makefile.in

libMediaFile_la_LDFLAGS = -static
libMediaFile_la_SOURCES =	\
	ImageFile.cxx		\
	ImageFile.h		\
	MediaFile.cxx		\
	PNGImageFile.cxx	\
	PNGImageFile.h		\
	SGIImageFile.cxx	\
	SGIImageFile.h		\
	AudioFile.cxx		\
	AudioFile.h		\
	WaveAudioFile.h		\
	WaveAudioFile.cxx

EXTRA_DIST = \
	OggAudioFile.cxx	\
	OggAudioFile.h		\
	README
