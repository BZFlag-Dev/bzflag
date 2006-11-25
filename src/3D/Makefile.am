if CLIENT_INCLUDED
noinst_LTLIBRARIES = lib3D.la
endif

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

lib3D_la_LDFLAGS = -static
lib3D_la_LIBADD = \
	$(top_builddir)/src/common/libCommon.la \
	$(top_builddir)/src/ogl/libGLKit.la \
	$(top_builddir)/src/mediafile/libMediaFile.la

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
