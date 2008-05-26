lib_LTLIBRARIES = SAMPLE_PLUGIN.la

SAMPLE_PLUGIN_la_SOURCES = chatlog.cpp
SAMPLE_PLUGIN_la_LDFLAGS = -module -avoid-version
SAMPLE_PLUGIN_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
SAMPLE_PLUGIN_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	chatlog.def \
	chatlog.sln \
	chatlog.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
