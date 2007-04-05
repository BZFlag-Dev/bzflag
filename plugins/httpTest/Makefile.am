lib_LTLIBRARIES = httpTest.la

SAMPLE_PLUGIN_la_SOURCES = httpTest.cpp
SAMPLE_PLUGIN_la_LDFLAGS = -module -avoid-version
SAMPLE_PLUGIN_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
SAMPLE_PLUGIN_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	httpTest.def \
	httpTest.sln \
	httpTest.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
