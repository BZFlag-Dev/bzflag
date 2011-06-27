lib_LTLIBRARIES = fastmap.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

fastmap_la_SOURCES = fastmap.cpp
fastmap_la_LDFLAGS = -module -avoid-version -shared
fastmap_la_CXXFLAGS= -I$(top_srcdir)/plugins/plugin_utils
fastmap_la_LIBADD  = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	fastmap.def \
	fastmap.sln \
	fastmap.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
