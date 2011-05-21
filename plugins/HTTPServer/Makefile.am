lib_LTLIBRARIES = HTTPServer.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

HTTPServer_la_SOURCES = HTTPServer.cpp
HTTPServer_la_LDFLAGS = -module -avoid-version
HTTPServer_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
HTTPServer_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	HTTPServer.def \
	HTTPServer.sln \
	HTTPServer.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
