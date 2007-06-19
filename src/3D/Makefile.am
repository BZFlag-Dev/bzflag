if CLIENT_INCLUDED
noinst_LTLIBRARIES = lib3D.la
endif

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/other/freetype/include \
	-I$(top_srcdir)/src/other/ftgl/inc \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

lib3D_la_LDFLAGS = -static
lib3D_la_LIBADD = \
	$(top_builddir)/src/common/libCommon.la \
	$(top_builddir)/src/ogl/libGLKit.la \
	$(top_builddir)/src/mediafile/libMediaFile.la

lib3D_la_SOURCES =		\
	FontManager.cxx		\
	TextureManager.cxx

EXTRA_DIST = \
	README
