lib_LTLIBRARIES = webReport.la

webstats_la_SOURCES = \
	webReport.cpp \
	reportTemplates.cpp \
	reportTemplates.h

webReport_la_LDFLAGS = -module -avoid-version
webReport_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
webReport_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	webReport.def \
	webReport.sln \
	webReport.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
