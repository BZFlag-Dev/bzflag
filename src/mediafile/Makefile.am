if CLIENT_INCLUDED
noinst_LIBRARIES = libMediaFile.a
endif

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include

libMediaFile_a_SOURCES =	\
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
