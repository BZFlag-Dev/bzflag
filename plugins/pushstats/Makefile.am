lib_LTLIBRARIES = pushstats.la

pushstats_la_SOURCES = pushstats.cpp
pushstats_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
pushstats_la_LDFLAGS = -module -avoid-version -shared
pushstats_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.pushstats.txt \
	pushstats.sln \
	pushstats.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
