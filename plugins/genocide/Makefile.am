lib_LTLIBRARIES = genocide.la

genocide_la_SOURCES = genocide.cpp
genocide_la_LDFLAGS = -module -avoid-version -shared
genocide_la_CPPFLAGS= -I$(top_srcdir)/plugins/plugin_utils -I$(top_srcdir)/include
genocide_la_LIBADD  = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.genocide.txt \
	genocide.sln \
	genocide.vcxproj	\
	genocide.vcxproj.filters \
	premake5.lua

MAINTAINERCLEANFILES =	\
	Makefile.in
