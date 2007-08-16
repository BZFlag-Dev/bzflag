lib_LTLIBRARIES = fastmap.la

fastmap_la_SOURCES = fastmap.cpp
fastmap_la_LDFLAGS = -module -avoid-version
fastmap_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
fastmap_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	fastmap.def \
	fastmap.sln \
	fastmap.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
