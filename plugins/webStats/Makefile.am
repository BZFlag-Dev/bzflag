lib_LTLIBRARIES = webStats.la

webStats_la_SOURCES = \
	webStats.cpp

webStats_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
webStats_la_LDFLAGS = -module -avoid-version -shared
webStats_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	templates \
	webStats.def \
	webStats.sln \
	webStats.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
