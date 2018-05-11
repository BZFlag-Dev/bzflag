if CLIENT_INCLUDED
noinst_LTLIBRARIES = lib3D.la
endif

AM_CPPFLAGS = $(CONF_CPPFLAGS) -I$(top_srcdir)/include
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES = \
	Makefile.in

lib3D_la_LDFLAGS = -static
lib3D_la_SOURCES =		\
	FontManager.cxx		\
	ImageFont.cxx		\
	TextureFont.cxx		\
	TextureManager.cxx	\
	ImageFont.h		\
	TextureFont.h

EXTRA_DIST = \
	README
