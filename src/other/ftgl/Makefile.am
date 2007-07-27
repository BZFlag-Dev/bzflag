noinst_LTLIBRARIES = libftgl.la

AM_CPPFLAGS = -I$(top_srcdir)/src/other/ftgl/inc -I$(top_srcdir)/src/other/freetype/include

libftgl_la_LDFLAGS = -static
libftgl_la_SOURCES = \
	src/FTBitmapGlyph.cpp	\
	src/FTCharmap.cpp	\
	src/FTContour.cpp	\
	src/FTExtrdGlyph.cpp	\
	src/FTFace.cpp		\
	src/FTFont.cpp		\
	src/FTGLBitmapFont.cpp	\
	src/FTGLExtrdFont.cpp	\
	src/FTGLOutlineFont.cpp	\
	src/FTGLPixmapFont.cpp	\
	src/FTGLPolygonFont.cpp	\
	src/FTGLTextureFont.cpp	\
	src/FTGlyph.cpp		\
	src/FTGlyphContainer.cpp	\
	src/FTLibrary.cpp	\
	src/FTOutlineGlyph.cpp	\
	src/FTPixmapGlyph.cpp	\
	src/FTPoint.cpp		\
	src/FTPolyGlyph.cpp	\
	src/FTSize.cpp		\
	src/FTTextureGlyph.cpp	\
	src/FTVectoriser.cpp

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	COPYING.txt \
	HISTORY.txt \
	README.txt \
	docs \
	inc \
	license.txt \
	src
