lib_LTLIBRARIES = serverControl.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

serverControl_la_SOURCES = serverControl.cpp
serverControl_la_LDFLAGS = -module -avoid-version
serverControl_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
serverControl_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	plugins.cfg \
	serverControl.def \
	serverControl.sln \
	serverControl.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
