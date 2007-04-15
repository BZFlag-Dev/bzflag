lib_LTLIBRARIES = Phoenix.la

Phoenix_la_SOURCES = Phoenix.cpp
Phoenix_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
Phoenix_la_LDFLAGS = -module -avoid-version
Phoenix_la_LIBADD = $(top_srcdir)/plugins/plugin_utils/libplugin_utils.la


EXTRA_DIST = \
	README.txt \
	Phoenix.def \
	Phoenix.sln \
	Phoenix.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
