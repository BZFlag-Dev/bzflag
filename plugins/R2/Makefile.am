lib_LTLIBRARIES = R2.la

R2_la_SOURCES = \
	playerHandler.cpp \
	playerHandler.h \
	R2.cpp
R2_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
R2_la_LDFLAGS = -module -avoid-version -shared
R2_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.R2.txt \
	R2.sln \
	R2.vcxproj	\
	R2.vcxproj.filters \
	premake5.lua

MAINTAINERCLEANFILES =	\
	Makefile.in
