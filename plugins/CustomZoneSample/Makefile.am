lib_LTLIBRARIES = CustomZoneSample.la

CustomZoneSample_la_SOURCES = CustomZoneSample.cpp
CustomZoneSample_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
CustomZoneSample_la_LDFLAGS = -module -avoid-version -shared
CustomZoneSample_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	CustomZoneSample.bzw \
	CustomZoneSample.def \
	CustomZoneSample.sln \
	CustomZoneSample.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
