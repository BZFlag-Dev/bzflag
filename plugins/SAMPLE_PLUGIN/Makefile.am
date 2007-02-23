lib_LTLIBRARIES = SAMPLE_PLUGIN.la

SAMPLE_PLUGIN_la_SOURCES = SAMPLE_PLUGIN.cpp
SAMPLE_PLUGIN_la_LDFLAGS = -module -avoid-version
SAMPLE_PLUGIN_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
SAMPLE_PLUGIN_la_LIBADD  = $(top_srcdir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	SAMPLE_PLUGIN.def \
	SAMPLE_PLUGIN.sln \
	SAMPLE_PLUGIN.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
