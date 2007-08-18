include $(top_srcdir)/Makefile.common

if CLIENT_INCLUDED
noinst_LTLIBRARIES = lib3D.la
endif

MAINTAINERCLEANFILES = \
	Makefile.in

lib3D_la_LDFLAGS = -static
lib3D_la_SOURCES =		\
	BitmapFont.cxx		\
	FontManager.cxx		\
	ImageFont.cxx		\
	TextureFont.cxx		\
	TextureManager.cxx	\
	BitmapFont.h		\
	ImageFont.h		\
	TextureFont.h

EXTRA_DIST = \
	README
