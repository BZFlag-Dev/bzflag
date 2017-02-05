lib_LTLIBRARIES = serverControl.la

serverControl_la_SOURCES = serverControl.cpp
serverControl_la_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
serverControl_la_LDFLAGS = -module -avoid-version -shared
serverControl_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.serverControl.txt \
	serverControl.cfg \
	serverControl.sln \
	serverControl.vcxproj	\
	serverControl.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
