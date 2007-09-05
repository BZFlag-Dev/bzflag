lib_LTLIBRARIES = mapchange.la

mapchange_la_SOURCES = mapchange.cpp
mapchange_la_LDFLAGS = -module -avoid-version
mapchange_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
mapchange_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	mapchange.def \
	mapchange.sln \
	mapchange.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
