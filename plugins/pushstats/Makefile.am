lib_LTLIBRARIES = pushstats.la

pushstats_la_SOURCES = pushstats.cpp
pushstats_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
pushstats_la_LDFLAGS = -module -avoid-version
pushstats_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la


EXTRA_DIST = \
	README.txt \
	pushstats.def \
	pushstats.sln \
	pushstats.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
