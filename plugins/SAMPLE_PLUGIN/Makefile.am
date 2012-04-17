lib_LTLIBRARIES = SAMPLE_PLUGIN.la

SAMPLE_PLUGIN_la_SOURCES = SAMPLE_PLUGIN.cpp
SAMPLE_PLUGIN_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
SAMPLE_PLUGIN_la_LDFLAGS = -module -avoid-version -shared
SAMPLE_PLUGIN_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	SAMPLE_PLUGIN.def \
	SAMPLE_PLUGIN.sln \
	SAMPLE_PLUGIN.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
