lib_LTLIBRARIES = webstats.la

webstats_la_SOURCES = webstats.cpp statTemplates.cpp
webstats_la_LDFLAGS = -module -avoid-version
webstats_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
webstats_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	webstats.def \
	webstats.sln \
	webstats.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
