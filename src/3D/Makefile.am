noinst_LTLIBRARIES = lib3D.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/other/freetype/include \
	-I$(top_srcdir)/src/other/ftgl/inc \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

lib3D_la_LDFLAGS = -static
lib3D_la_SOURCES =		\
	FontManager.cxx		\
	TextureManager.cxx	\
	Model.cxx

EXTRA_DIST = \
	README
