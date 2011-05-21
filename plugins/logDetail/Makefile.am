lib_LTLIBRARIES = logDetail.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

logDetail_la_SOURCES = logDetail.cpp
logDetail_la_LDFLAGS = -module -avoid-version
logDetail_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
logDetail_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	logDetail.def \
	logDetail.sln \
	logDetail.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
