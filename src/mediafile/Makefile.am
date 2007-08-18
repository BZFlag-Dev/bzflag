include $(top_srcdir)/Makefile.common

if CLIENT_INCLUDED
noinst_LTLIBRARIES = libMediaFile.la
endif

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
