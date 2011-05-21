lib_LTLIBRARIES = webReport.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

webReport_la_SOURCES = \
	webReport.cpp

webReport_la_LDFLAGS = -module -avoid-version
webReport_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
webReport_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	templates \
	webReport.def \
	webReport.sln \
	webReport.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
