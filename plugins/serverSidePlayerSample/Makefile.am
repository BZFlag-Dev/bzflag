lib_LTLIBRARIES = serverSidePlayerSample.la

serverSidePlayerSample_la_SOURCES = serverSidePlayerSample.cpp
serverSidePlayerSample_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
serverSidePlayerSample_la_LDFLAGS = -module -avoid-version -shared
serverSidePlayerSample_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	serverSidePlayerSample.def \
	serverSidePlayerSample.sln \
	serverSidePlayerSample.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
