noinst_LTLIBRARIES = lib3D.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/include \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

if BUILD_GLEW
GLEW_SRCS = ../other/glew/src/glew.c
else
GLEW_SRCS =
endif

lib3D_la_LDFLAGS = -static
lib3D_la_SOURCES = \
	${GLEW_SRCS} \
	FontManager.cxx \
	Model.cxx \
	TextureManager.cxx

EXTRA_DIST = \
	README
