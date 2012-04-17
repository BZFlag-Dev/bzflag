lib_LTLIBRARIES = serverControl.la

serverControl_la_SOURCES = serverControl.cpp
serverControl_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils $(AM_CXXFLAGS)
serverControl_la_LDFLAGS = -module -avoid-version -shared
serverControl_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	plugins.cfg \
	README.txt \
	serverControl.def \
	serverControl.sln \
	serverControl.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
